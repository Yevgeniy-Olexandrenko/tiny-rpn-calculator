#!/usr/bin/perl
# Generates the SVG images of the user manuals next to this script:
#   perl documents/manuals/images/rpn-calc-images.pl
# Reads data.h of the sketches firmware/sketches/hp35 and firmware/sketches/hp21.
# The images of one firmware (its screens, the device with its watch screen) are
# rpn-calc-<model>-*.svg. The keyboards, the Extension label and the base image
# rpn-calc.svg (the device with the HP-35 watch screen, for README.md) are common.
#
# Units of the device and keyboard drawings: 0.1 mm. Board outline, keys,
# battery, USB cutout and the lanyard hole are from the Gerber and pick-and-place
# files of the PCB, the display from the device photo. Screens are rendered pixel
# by pixel with the firmware fonts of data.h the same way as the render*Mode()
# functions of code.h and the TXT module of the library do it.
use strict; use warnings; use utf8;
use FindBin;

my $sketches = "$FindBin::Bin/../../../firmware/sketches";
my $lib_dir  = "$FindBin::Bin/../../../firmware/libraries/TinyRPNCalculator/src";

# ================================================================ data.h
# use_sketch() reads data.h of a sketch: its fonts, #defines and menu strings
# are used by the functions below until the next call

my ($data_h, $src, %font, %define, %lib);

sub font_bytes {
    my $name = shift;
    $src =~ /const u08 ${name}_bytes\[\] PROGMEM =\s*\{(.*?)\};/s or die "no $name in $data_h";
    my $body = $1; $body =~ s{//[^\n]*}{}g;
    return [ map { hex } $body =~ /0x([0-9A-Fa-f]{2})/g ];
}

# layout #defines of data.h: integer expressions of other defines (computed with C
# integer division), chars and strings; LCD::WIDTH and HPVM::CYCLES_PER_SEC are
# constants of the library headers, the CPU header is given by the sketch
sub lib_value {
    my ($file, $re) = @_;
    open my $lf, '<', "$lib_dir/$file" or die "$lib_dir/$file: $!";
    my $text = do { local $/; <$lf> };
    $text =~ $re or die "no $re in $file";
    return $1;
}

sub use_sketch {
    my ($model, $cpu_h) = @_;
    $data_h = "$sketches/$model/data.h";
    open my $fh, '<', $data_h or die "$data_h: $!";
    $src = do { local $/; <$fh> };
    %font = (
        menu5x8    => { bytes => font_bytes('menu5x8'),    w => 5, rows => 1, first => 0x2F },
        digits7x32 => { bytes => font_bytes('digits7x32'), w => 7, rows => 4, first => 0x61 },
        digits7x16 => { bytes => font_bytes('digits7x16'), w => 7, rows => 2, first => 0x61 },
    );
    $font{$_}{last} = $font{$_}{first} + @{$font{$_}{bytes}} / ($font{$_}{w} * $font{$_}{rows}) - 1 for keys %font;
    %lib = (
        'LCD::WIDTH'           => lib_value('TinyRPNCalculator/LCD.h', qr/constexpr u08 WIDTH\s*=\s*(\d+);/),
        'HPVM::CYCLES_PER_SEC' => int(lib_value("Extras/$cpu_h", qr/CLOCK_RATE_KHZ\s*=\s*(\d+);/) * 1000 / (14 * 4)),
    );
    %define = ();
    $define{$1} = $2 while $src =~ m{^#define\s+(\w+)\s+(.*?)\s*(?://.*)?\r?$}mg;
}

sub def {
    my $name = shift;
    my $v = $define{$name} // die "no #define $name in $data_h";
    return $1 if $v =~ /^'(.)'$/ or $v =~ /^"(.*)"$/;
    $v =~ s/(\w+::\w+)/$lib{$1} \/\/ die "no value of $1"/ge;
    $v =~ s/\b([A-Z][A-Z0-9_]*)\b/def($1)/ge;
    my $n = eval "use integer; $v";
    die "#define $name: $v: $@" unless defined $n;
    return $n;
}

# items of a menu string: "\06" " SIN  " " COS  " ...; the literals are read one
# by one up to the ';', as an item can have a ';' too (a glyph of the menu font)
sub menu_items {
    my $name = shift;
    my $text = $src;
    $text =~ /const u08 ${name}\[\] DATAMEM =/g or die "no $name in $data_h";
    my @s;
    push @s, $1 while $text =~ m{\G(?:\s+|//[^\n]*)*"([^"]*)"}gc;
    $text =~ m{\G(?:\s+|//[^\n]*)*;}gc or die "no end of $name in $data_h";
    shift @s; # item width
    return @s;
}

# the HP-35 images first, the HP-21 images at the end
use_sketch('hp35', 'HPVMClassic.h');

# ================================================================ screen
# TXT and LCD of the library: 128 x 32 pixels, a byte is a column of 8 pixels
# of a page, bit 0 on top. Display contents are lists of HPVM::Display codes.

my (@px, $cur, $sx, $sy, $char_dx);
my $inverse = 0;

sub lcd_clear { @px = map { [(0) x 128] } 0..31 }
sub lcd_write {
    my ($x, $page, $b) = @_;
    return if $x > 127;
    $px[$page * 8 + $_][$x] = ($b >> $_) & 1 for 0..7;
}
sub set_scale   { ($sx, $sy) = @_; $char_dx = $cur->{w} * $sx + 1 }
sub set_font    { $cur = $font{shift()}; set_scale(1, 1) }
sub set_inverse { $inverse = shift() ? 0xFF : 0x00 }
sub next_pos    { $_[0] += $char_dx }   # TXT::NextPos(x), x is changed in place

sub scale_y {
    my ($db, $si) = @_;
    return $db if $sy == 1;
    my $n = ($si ? $db >> 4 : $db) & 0x0F;                          # expand4bit
    my $b = 0; $b |= (($n >> $_) & 1) * (3 << (2 * $_)) for 0..3;
    return $b;
}

sub print_char {
    my ($ch, $x, $y) = @_;
    my $c = ord $ch;
    my $dp = ($c >= $cur->{first} && $c <= $cur->{last}) ? ($c - $cur->{first}) * $cur->{w} * $cur->{rows} : undef;
    for my $r (0 .. $cur->{rows} - 1) {
        for my $si (0 .. $sy - 1) {
            my $cx = $x;
            for my $i (0 .. $cur->{w} - 1) {
                my $db = defined $dp ? scale_y($cur->{bytes}[$dp + $r * $cur->{w} + $i] ^ $inverse, $si) : $inverse;
                lcd_write($cx++, $y + $r * $sy + $si, $db) for 1..$sx;
            }
            lcd_write($cx, $y + $r * $sy + $si, $inverse);             # spacing
        }
    }
}
sub print_string { my ($s, $x, $y) = @_; for (split //, $s) { print_char($_, $x, $y); $x += $char_dx } }

my @num_seg = (0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F,  # 0..9
               0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71,                          # A b C d E F
               0x00, 0x40, 0x80);                                           # space, dash, dot
sub print_seg {
    my ($seg, $x, $y) = @_;
    my $ds = $cur->{w} * $cur->{rows};
    for my $r (0 .. $cur->{rows} - 1) {
        for my $i (0 .. $cur->{w} - 1) {
            my $b = 0;
            for my $k (0..7) { $b |= $cur->{bytes}[$k * $ds + $r * $cur->{w} + $i] if $seg & (1 << $k) }
            lcd_write($x + $i, $y + $r, $b);
        }
        lcd_write($x + $cur->{w}, $y + $r, 0);
    }
}
# PrintSegBCD for a decimal number 0..99
sub print_seg_2 { my ($n, $x, $y) = @_; print_seg($num_seg[$n / 10], $x, $y); print_seg($num_seg[$n % 10], $x + $char_dx, $y) }

# HPVM::Display codes from text: digits, ' ' blank, '-' dash, '.' dot (15 chars)
sub display {
    my $s = shift;
    die "display needs 15 chars: '$s'" unless length $s == 15;
    return [ map { /\d/ ? $_ : $_ eq ' ' ? 16 : $_ eq '-' ? 17 : 18 } split //, $s ];
}

# code.h: renderRTCMode()
sub render_rtc {
    my (%a) = @_;
    lcd_clear();
    set_font('digits7x32');
    print_char(def('RTC_TIME_COLON'), def('RTC_POS_COLON_HM'), 0);
    print_char(def('RTC_TIME_COLON'), def('RTC_POS_COLON_MS'), 0);
    print_seg_2($a{h}, def('RTC_POS_HOURS'),   0);
    print_seg_2($a{m}, def('RTC_POS_MINUTES'), 0);
    print_seg_2($a{s}, def('RTC_POS_SECONDS'), 0);
    set_font('menu5x8');
    print_string($a{month}, def('RTC_POS_MONTH'), 0);
    print_string(sprintf('%02d', $a{date}), def('RTC_POS_DATE'), 0);
    print_string(sprintf('%02d', $a{year}), def('RTC_POS_YEAR'), 0);
    print_char(def('RTC_DATE_SLASH'), def('RTC_POS_SLASH'), 0);
    set_scale(2, 1);
    print_string(def('RTC_INFO_HP_LABEL'), def('RTC_POS_INFO'), 2);
    my $x = def('RTC_POS_INFO');
    for (1 .. $a{bars}) { print_char(def('RTC_BATTERY_LEVEL'), $x, 1); next_pos($x) }
}

# code.h: renderCalcNumber(), renderCalcMenu(), renderCalcMode()
sub render_number {
    my ($codes, $y) = @_;
    my $x = 0;
    for my $i (0..14) {
        set_font('digits7x16') if $i == 12;
        print_seg($num_seg[$codes->[$i]], $x, $y);
        next_pos($x);
    }
}
sub render_menu {
    my $items = shift;
    set_font('menu5x8');
    set_scale(1, 2);
    set_inverse(1);
    print_string($items->[$_], def('MENU_POS_ITEM') * $_, 2) for 0 .. def('MENU_ITEMS_PER_LINE') - 1;
    set_inverse(0);
}
sub render_calc {
    my (%a) = @_;   # x => display, y => display (F active), menu => [3 items], memory => 1
    lcd_clear();
    set_font('digits7x16');
    print_char(def('FLAG_STORAGE'), def('FLAG_POS'), $a{menu} ? 0 : 2) if $a{memory};
    if ($a{y}) {
        render_number($a{y}, 0);
        render_number($a{x}, 2);
        print_char(def('FLAG_FUNCTION'), def('FLAG_POS'), 0);
    }
    elsif ($a{menu}) {
        render_number($a{x}, 0);
        render_menu($a{menu});
    }
    else {
        set_font('digits7x32');
        render_number($a{x}, 0);
    }
}

# HP-21: HPVM::Display holds 7-segment patterns of 12 positions, the dot in bit 7
my %seg21 = ((map { $_ => $num_seg[$_] } 0..9), ' ' => 0x00, '-' => 0x40,
             'E' => 0x79, 'H' => 0x76, 'o' => 0x5C, 'P' => 0x73, 'r' => 0x50);
# HP-21 display from text: characters of %seg21, a '.' lights the dot of the position before it
sub display21 {
    my $s = shift;
    my @segs;
    for my $ch (split //, $s) {
        if ($ch eq '.') { @segs or die "dot without a position: '$s'"; $segs[-1] |= 0x80; next }
        push @segs, $seg21{$ch} // die "no segments of '$ch': '$s'";
    }
    die "display has 12 positions: '$s'" if @segs > 12;
    push @segs, 0x00 while @segs < 12;
    return \@segs;
}

# hp21 code.h: renderCalcNumber(), renderCalcMode(); the dot is a separate char
sub render_number21 {
    my ($segs, $y) = @_;
    my $x = 0;
    for my $seg (@$segs) {
        print_seg($seg & 0x7F, $x, $y);
        next_pos($x);
        if ($seg & 0x80) { print_seg(0x80, $x, $y); next_pos($x) }
    }
}
sub render_calc21 {
    my (%a) = @_;   # as render_calc(), radians => 1
    lcd_clear();
    set_font('digits7x16');
    my $flags_y = $a{menu} ? 0 : 2;
    print_char(def('FLAG_STORAGE'), def('FLAG_POS'), $flags_y) if $a{memory};
    print_char(def('FLAG_RADIANS'), def('FLAG_POS_2'), $flags_y) if $a{radians};
    if ($a{y}) {
        render_number21($a{y}, 0);
        render_number21($a{x}, 2);
        print_char(def('FLAG_FUNCTION'), def('FLAG_POS'), 0);
    }
    elsif ($a{menu}) {
        render_number21($a{x}, 0);
        render_menu($a{menu});
    }
    else {
        set_font('digits7x32');
        render_number21($a{x}, 0);
    }
}

# code.h: renderTOTPMode()
sub render_totp {
    my (%a) = @_;
    lcd_clear();
    set_font('digits7x32');
    my $code = $a{code};
    for (my $i = 6; $i-- > 0;) {
        print_seg($num_seg[$code % 10], def('TOTP_POS_CODE') + ($i < 3 ? $i : $i + 1) * def('TOTP_CHAR_W'), 0);
        $code = int($code / 10);
    }
    set_font('digits7x16');
    print_seg_2($a{left}, def('TOTP_POS_LEFT'), 0);
    set_font('menu5x8');
    set_scale(1, 2);
    set_inverse(1);
    print_string(" $a{label} ", def('TOTP_POS_LABEL'), 2);
    set_inverse(0);
}

# lit pixels as one path of rectangles: runs along lines, equal runs of next
# lines merged; done along rows and along columns, the shorter path is used
sub merge_runs {
    my ($lines, $len, $lit) = @_;                # $lit->(line, pos)
    my (@rects, %open);                          # [line, pos, lines, length]
    for my $l (0 .. $lines - 1) {
        my %next;
        for (my $p = 0; $p < $len; ) {
            if (!$lit->($l, $p)) { $p++; next }
            my $s = $p; $p++ while $p < $len && $lit->($l, $p);
            my $key = "$s,$p";
            if (exists $open{$key}) { $rects[$open{$key}][2]++; $next{$key} = $open{$key} }
            else { push @rects, [$l, $s, 1, $p - $s]; $next{$key} = $#rects }
        }
        %open = %next;
    }
    return @rects;
}
sub pixels_path {
    my $rows = join '', map { sprintf 'M%d %dh%dv%dh-%dz', $_->[1], $_->[0], $_->[3], $_->[2], $_->[3] }
        merge_runs(32, 128, sub { $px[$_[0]][$_[1]] });
    my $cols = join '', map { sprintf 'M%d %dv%dh%dv-%dz', $_->[0], $_->[1], $_->[3], $_->[2], $_->[3] }
        merge_runs(128, 32, sub { $px[$_[1]][$_[0]] });
    return length $cols < length $rows ? $cols : $rows;
}

# ================================================================ styles
# Flashing pixels are lit for the first half of the period. The HP-35 ROM toggles the
# display of an error every 602 cycles (measured on HPVMClassic.h), the sketch runs
# HPVM_CYCLES_PER_FRAME (238) cycles in a frame of 64 ms (TMR::TIMEOUT_64MS); renderRTCMode() toggles
# the last bar of the battery indicator during the charge every 512 ms (TMR::Millis & 0x200).
my $error_period  = int(2 * 602 / def('HPVM_CYCLES_PER_FRAME') * 64 + 0.5);
my $charge_period = 2 * 512;
my %keyframes = (flash => '50% { opacity: 0; }');

# Classes that follow the theme get the light color and a dark one in @media.
my %theme = (           # light, dark
    bg    => ['#ffffff', '#0d1117'],
    edge  => ['#57606a', '#8b949e'],
    ink   => ['#1f2328', '#e6edf3'],
    chip  => ['#24292f', '#30363d'],
    pin   => ['#8c959f', '#6e7681'],
    badge => ['#0969da', '#2f81f7'],
    tint  => ['#ddf4ff', '#132339'],
);
my %class = (           # static properties, then property => theme color
    board    => ['stroke-width: 2', fill => 'bg', stroke => 'edge'],
    hole     => ['fill: none; stroke-width: 2', stroke => 'edge'],
    slot     => ['fill: none; stroke-width: 1.5', stroke => 'edge'],
    line     => ['fill: none; stroke-width: 2', stroke => 'edge'],
    cap      => ['', fill => 'ink'],
    main     => ['font-size: 52px; text-anchor: middle', fill => 'ink'],
    shift    => ['font-size: 26px', fill => 'ink'],
    title    => ['font-size: 46px; font-weight: 700; text-anchor: middle', fill => 'ink'],
    subtitle => ['font-size: 27px; text-anchor: middle', fill => 'ink'],
    note     => ['font-size: 22px', fill => 'ink'],
    dim      => ['opacity: 0.8', fill => 'bg'],
    chip     => ['', fill => 'chip'],
    pin      => ['', fill => 'pin'],
    badge    => ['', fill => 'badge'],
    chiptxt  => ['fill: #ffffff; font-size: 14px; font-weight: 600; text-anchor: middle'],
    badgetx  => ['fill: #ffffff; font-size: 13px; font-weight: 700; text-anchor: middle'],
    ext      => ['stroke-width: 1', fill => 'tint', stroke => 'badge'],
    exttx    => ['font-size: 12px; font-weight: 600; text-anchor: middle', fill => 'badge'],
    glass    => ['fill: #24292f; stroke: #57606a; stroke-width: 1.5'],
    screen   => ['fill: #151b23; stroke: #3d444d; stroke-width: 1'],
    panel    => ['fill: #000000'],
    ledge    => ['fill: #57606a'],
    pixel    => ['fill: #ffffff'],
    errflash => ["animation: flash ${error_period}ms step-end infinite"],
    barflash => ["animation: flash ${charge_period}ms step-end infinite"],
    ribbon   => ['fill: #d18b2c'],
    usb      => ['fill: #d0d7de; stroke: #57606a; stroke-width: 1.5'],
    tab      => ['fill: #d0d7de; stroke: #57606a; stroke-width: 1.5'],
    usbleg   => ['fill: #8c959f'],
    cell     => ['fill: #eaeef2; stroke: #d18b2c; stroke-width: 10'],
    label    => ['fill: #1f2328; font-size: 18px; font-weight: 600; text-anchor: middle'],
);
# <style> with the rules of the classes used in the svg body; animations get
# their @keyframes and stop for prefers-reduced-motion
sub style {
    my $body = shift;
    my %used; $used{$_} = 1 for map { split ' ' } $body =~ /class="([^"]+)"/g;
    my ($light, $dark, $still, %anim) = ('', '', '');
    for my $c (sort keys %used) {
        my ($static, %prop) = @{$class{$c} // die "no style for class $c"};
        my @l = grep { length } $static, map { "$_: $theme{$prop{$_}}[0]" } sort keys %prop;
        my @d = map { "$_: $theme{$prop{$_}}[1]" } sort keys %prop;
        $light .= sprintf "    .%-9s{ %s; }\n", $c, join '; ', @l;
        $dark  .= sprintf "      .%-9s{ %s; }\n", $c, join '; ', @d if @d;
        if ($static =~ /animation: (\w+)/) { $anim{$1} = 1; $still .= sprintf "      .%-9s{ animation: none; }\n", $c }
    }
    $dark = "    \@media (prefers-color-scheme: dark) {\n$dark    }\n" if $dark;
    my $motion = join '', map { "    \@keyframes $_ { $keyframes{$_} }\n" } sort keys %anim;
    $motion .= "    \@media (prefers-reduced-motion: reduce) {\n$still    }\n" if $still;
    return "  <style>\n$light$dark$motion  </style>\n";
}

sub write_svg {
    my ($name, $title, $viewbox, $body) = @_;
    my (undef, undef, $w, $h) = split ' ', $viewbox;
    my $font = $body =~ /<text/ ? ' font-family="Segoe UI, Helvetica, Arial, sans-serif"' : '';
    my $svg = qq{<svg xmlns="http://www.w3.org/2000/svg" viewBox="$viewbox" width="$w" height="$h"$font>\n}
            . "  <title>$title</title>\n" . style($body) . $body . "</svg>\n";
    my $out = "$FindBin::Bin/$name";
    open my $o, '>:encoding(UTF-8)', $out or die "$out: $!";
    print $o $svg;
    close $o;
    printf "%-32s %6d bytes\n", $name, -s $out;
}

# ================================================================ keyboard
my @key_y  = (245, 325, 405, 485);             # key centers, 8 mm pitch
my @grid_x = (190, 350, 510, 670, 830);        # 4 cells of 16 mm, 2.5 mm from the board edge
my @grid_y = (195, 275, 355, 435, 515);        # centered between the keyboard board top and the bottom edge
my @legends = (
    ['F', 'MATH', '7', 'TRIG',  '8', 'PROG', '9', '÷'],
    ['E', 'ROT↑', '4', 'CONST', '5', 'MEM+', '6', '×'],
    ['±', 'ROT↓', '1', 'RCL',   '2', 'STO',  '3', '−'],
    ['C', 'AC',   '0', 'LAST',  '.', 'X↔Y',  '↑', '+'],
);
my $main_cap = 18;                             # half of the cap height of the 52px main legend
my $slot_gap = 10;                             # from the slot to the right edge of the cell, as to the bottom one
my $slot_w   = 97;                             # from 33% of the 16 mm cell to the gap

# grid and keys: the slot of the keyboard board ends 1 mm before the right edge of the
# cell, the button of the 3 x 6 mm tact switch is in its middle; the main legend is
# centered left of the slot, the shifted legend is above the slot from its left edge
sub keyboard {
    my $grid = sprintf 'M%d,%d H%d V%d H%d Z', $grid_x[0], $grid_y[0], $grid_x[-1], $grid_y[-1], $grid_x[0];
    $grid .= " M$_,$grid_y[0] V$grid_y[-1]" for @grid_x[1..3];
    $grid .= " M$grid_x[0],$_ H$grid_x[-1]" for @grid_y[1..3];
    my $s = qq{  <path class="line" d="$grid"/>\n};
    for my $r (0..3) {
        my $y = $key_y[$r];
        my $main_y = ($grid_y[$r] + $grid_y[$r + 1]) / 2 + $main_cap;
        for my $c (0..3) {
            my ($main, $shift) = @{$legends[$r]}[2 * $c, 2 * $c + 1];
            my ($cell_l, $cell_r) = @grid_x[$c, $c + 1];
            my $sr = $cell_r - $slot_gap;
            my $sl = $sr - $slot_w;
            $s .= sprintf qq{  <path class="slot" d="M%g,%g H%g L%g,%g V%g L%g,%g H%g L%g,%g V%g Z"/><rect class="cap" x="%g" y="%g" width="38" height="18" rx="3"/>\n},
                $sl + 10, $y - 20, $sr - 10, $sr, $y - 10, $y + 10, $sr - 10, $y + 20, $sl + 10, $sl, $y + 10, $y - 10,
                ($sl + $sr) / 2 - 19, $y - 9;
            $s .= sprintf qq{  <text class="main" x="%g" y="%d">%s</text><text class="shift" x="%d" y="%d">%s</text>\n},
                ($cell_l + $sl) / 2, $main_y, $main, $sl, $y - 25, $shift;
        }
    }
    return $s;
}
my $kbd_viewbox = sprintf '%d %d %d %d', $grid_x[0] - 10, $grid_y[0] - 10, $grid_x[-1] - $grid_x[0] + 20, $grid_y[-1] - $grid_y[0] + 20;

write_svg('rpn-calc-keyboard.svg', 'Keyboard of the Business Card RPN Calculator with Watch', $kbd_viewbox, keyboard());

write_svg('rpn-calc-keyboard-power-on.svg', 'Keys that turn on the device: the two bottom rows', $kbd_viewbox,
    keyboard()
    . sprintf(qq{  <!-- the two top rows do not turn on the device -->\n  <rect class="dim" x="%d" y="%d" width="%d" height="%d"/>\n},
        $grid_x[0] - 10, $grid_y[0] - 10, $grid_x[-1] - $grid_x[0] + 20, $grid_y[2] - $grid_y[0] + 9));

# keys that do nothing in a context are dimmed inside their cells, the grid stays: [row, column]
sub dim_keys {
    my ($comment, @keys) = @_;
    return "  <!-- $comment -->\n" . join '', map {
        my ($r, $c) = @$_;
        sprintf qq{  <rect class="dim" x="%d" y="%d" width="%d" height="%d"/>\n},
            $grid_x[$c] + 1, $grid_y[$r] + 1, $grid_x[$c + 1] - $grid_x[$c] - 2, $grid_y[$r + 1] - $grid_y[$r] - 2;
    } @keys;
}

# code.h: updateCalcMode() in a menu: F 7 8 open a menu, E ± change the line, 1 2 3 select an item
write_svg('rpn-calc-keyboard-menu.svg', 'Keys that work in a menu: F, 7, 8, E, ±, 1, 2, 3', $kbd_viewbox,
    keyboard() . dim_keys('the other keys close the menu', [0, 3], [1, 1], [1, 2], [1, 3], [3, 0], [3, 1], [3, 2], [3, 3]));

# ================================================================ screens
# 4 units per pixel on a dark glass with the colors of a GitHub table (muted cell, border), notes on the right: [pixel row, text]
my $scr_pad = 16;
my $scr_w   = 128 * 4 + 2 * $scr_pad;
my $scr_h   = 32 * 4 + 2 * $scr_pad;
my $scr_gap = 16;                              # between the screens of one image

# parts of a screen at the top $y of the image: the glass, the current pixels (none
# if all are dark) and the notes
sub screen_glass { my $y = shift; sprintf qq{  <rect class="screen" x="0.5" y="%g" width="%d" height="%d" rx="8"/>\n}, $y + 0.5, $scr_w - 1, $scr_h - 1 }
sub screen_pixels {
    my ($y, $class) = @_;
    my $d = pixels_path();
    return length $d ? sprintf(qq{  <path class="%s" transform="translate(%d %d) scale(4)" d="%s"/>\n}, $class, $scr_pad, $y + $scr_pad, $d) : '';
}
sub screen_notes {
    my ($y, @notes) = @_;
    return join '', map { sprintf qq{  <text class="note" x="%d" y="%d">← %s</text>\n}, $scr_w + 14, $y + $scr_pad + $_->[0] * 4 + 8, $_->[1] } @notes;
}
sub screen { my ($y, @notes) = @_; screen_glass($y) . screen_pixels($y, 'pixel') . screen_notes($y, @notes) }
# screens one below the other; the same width for all images, room for notes
sub screens_svg {
    my ($name, $title, @screens) = @_;
    my $h = @screens * ($scr_h + $scr_gap) - $scr_gap;
    write_svg($name, $title, sprintf('-2 -2 %d %d', $scr_w + 200 + 4, $h + 4), join '', @screens);
}
sub screen_svg { my ($name, $title, @notes) = @_; screens_svg($name, $title, screen(0, @notes)) }

# all lines of a menu with X on the top line, ± shows the next line; $render
# draws the screen with the items of a line
sub menu_svg {
    my ($name, $title, $menu, $render) = @_;
    my @items = menu_items($menu);
    my @screens;
    for my $line (0 .. @items / 3 - 1) {
        $render->([ @items[3 * $line .. 3 * $line + 2] ]);
        push @screens, screen($line * ($scr_h + $scr_gap), [23.5, 'line ' . ($line + 1)]);
    }
    screens_svg($name, $title, @screens);
}

# a screen with flashing pixels: lit in the frame drawn by $on, dark in the frame drawn by $off
sub flash_svg {
    my ($name, $title, $class, $on, $off, @notes) = @_;
    $on->();
    my @lit = map { [@$_] } @px;
    $off->();
    for my $r (0..31) { $px[$r][$_] && !$lit[$r][$_] and die "$name: pixel ($_, $r) is lit only in the off frame" for 0..127 }
    my $body = screen_glass(0) . screen_pixels(0, 'pixel');
    @px = map { my $r = $_; [ map { $lit[$r][$_] && !$px[$r][$_] ? 1 : 0 } 0..127 ] } 0..31;
    screens_svg($name, $title, $body . screen_pixels(0, "pixel $class") . screen_notes(0, @notes));
}

my %watch = (h => 14, m => 30, s => 0, month => 'OCT', date => 17, year => 26, bars => 4);

render_rtc(%watch);
my $device_screen = pixels_path();
screen_svg('rpn-calc-hp35-screen-watch.svg', 'Watch screen: 14:30:00, OCT 17/26, battery level, HP-35');

flash_svg('rpn-calc-hp35-screen-charge.svg', 'Watch screen during the charge: the last bar of the battery indicator flashes', 'barflash',
    sub { render_rtc(%watch, bars => 3) }, sub { render_rtc(%watch, bars => 2) }, [11, 'flashes']);

render_calc(x => display(' 6.02214076  23'));
screen_svg('rpn-calc-hp35-screen-number.svg', 'Calculator screen: 6.02214076 x 10^23', [7.5, 'exponent']);

render_calc(x => display(' 3.141592654   '), memory => 1);
screen_svg('rpn-calc-hp35-screen-memory.svg', 'Calculator screen: pi, with the S indicator');

# the ROM blanks the whole display, HPVM::Error() of the sketch shows the same
flash_svg('rpn-calc-hp35-screen-error.svg', 'Calculator screen with an error: a flashing 0.', 'errflash',
    sub { render_calc(x => display(' 0.            ')) }, sub { render_calc(x => display(' ' x 15)) }, [15.5, 'flashes']);

render_calc(y => display(' 12.5          '), x => display(' 3.            '), memory => 1);
screen_svg('rpn-calc-hp35-screen-function.svg', 'Calculator screen with F active: Y on top, X below', [7.5, 'Y'], [23.5, 'X']);

render_calc(x => display(' 30.           '), menu => [ (menu_items('strMenuTrig'))[0..2] ]);
screen_svg('rpn-calc-hp35-screen-menu.svg', 'Calculator screen with the TRIG menu', [7.5, 'X'], [23.5, 'menu']);

sub menu35 { my $x = shift; sub { render_calc(x => display($x), menu => shift) } }
menu_svg('rpn-calc-hp35-screen-menu-math.svg', 'MATH menu: line 1 x^y, square root, 1/x; line 2 LOG, LN, e^x', 'strMenuMath', menu35(' 2.            '));
menu_svg('rpn-calc-hp35-screen-menu-trig.svg', 'TRIG menu: line 1 SIN, COS, TAN; line 2 ASIN, ACOS, ATAN', 'strMenuTrig', menu35(' 30.           '));
menu_svg('rpn-calc-hp35-screen-menu-prog.svg', 'PROG menu: line 1 TIME, DATE, ZONE; line 2 TOTP', 'strMenuProg', menu35(' 0.            '));

render_totp(code => 287082, left => 17, label => 'KEY1');
screen_svg('rpn-calc-hp35-screen-totp.svg', 'TOTP screen: code 287 082, 17 seconds left, key KEY1', [7.5, 'seconds left'], [23.5, 'key label']);

# ================================================================ device
# the device with the pixels path of a watch screen on its display
sub device {
    my $screen = shift;
    return <<"SVG";
  <defs>
    <!-- one side of a SOIC-8 -->
    <g id="soic-pins">
      <rect class="pin" x="-21" y="-4" width="4" height="8"/>
      <rect class="pin" x="-8.5" y="-4" width="4" height="8"/>
      <rect class="pin" x="4.5" y="-4" width="4" height="8"/>
      <rect class="pin" x="17" y="-4" width="4" height="8"/>
    </g>
  </defs>

  <!-- main board: 85.5 x 54 mm, 3 mm corners -->
  <rect class="board" x="0" y="0" width="855" height="540" rx="30"/>
  <circle class="hole" cx="825" cy="30" r="12.7"/>

  <!-- (1) display: glass, OLED panel with the watch screen, flexible cable -->
  <rect class="ribbon" x="320" y="58" width="56" height="78" rx="3"/>
  <rect class="glass" x="22" y="24" width="303" height="115" rx="3"/>
  <rect class="panel" x="26" y="28" width="246" height="94"/>
  <rect class="ledge" x="26" y="122" width="246" height="13"/>
  <path class="pixel" transform="translate(36 47) scale(1.75)" d="$screen"/>

  <!-- (2) MCU, ATtiny85 in SOIC-8 5.3 mm -->
  <use href="#soic-pins" x="460" y="40"/>
  <use href="#soic-pins" x="460" y="105"/>
  <rect class="chip" x="433.5" y="46" width="53" height="53" rx="3"/>
  <text class="chiptxt" x="460" y="77">MCU</text>

  <!-- (3) RTC, DS3231M in SOIC-8 3.9 mm -->
  <use href="#soic-pins" x="515" y="40"/>
  <use href="#soic-pins" x="515" y="90"/>
  <rect class="chip" x="490.5" y="45.5" width="49" height="39" rx="3"/>
  <text class="chiptxt" x="515" y="70">RTC</text>

  <!-- (4) USB-C receptacle, its front flush with the top edge -->
  <rect class="usbleg" x="549" y="42" width="8" height="16" rx="1"/>
  <rect class="usbleg" x="643" y="42" width="8" height="16" rx="1"/>
  <rect class="usb" x="556" y="0" width="88" height="72" rx="3"/>
  <text class="label" x="600" y="60">USB-C</text>

  <!-- (5) charger in SOT-23-5, right edge aligned with USB-C -->
  <rect class="pin" x="617" y="93" width="4" height="7"/>
  <rect class="pin" x="637" y="93" width="4" height="7"/>
  <rect class="pin" x="617" y="115" width="4" height="7"/>
  <rect class="pin" x="627" y="115" width="4" height="7"/>
  <rect class="pin" x="637" y="115" width="4" height="7"/>
  <rect class="chip" x="614" y="99" width="30" height="17" rx="2"/>

  <!-- (6) battery: 16 mm coin cell in the cutout of the board -->
  <circle class="cell" cx="735" cy="102.5" r="75"/>
  <rect class="tab" x="700" y="87.5" width="130" height="30" rx="6"/>
  <text class="label" x="735" y="70">BATTERY</text>

  <!-- keyboard board over the lower part: 37 mm high, cut out around the battery -->
  <path class="board" d="M0,170 H687.5 A82.55,82.55 0 0 0 782.5,170 H855 V510 a30,30 0 0 1 -30,30 H30 a30,30 0 0 1 -30,-30 Z"/>
  <text class="title" transform="rotate(-90 121 355)" x="121" y="355" textLength="320" lengthAdjust="spacingAndGlyphs">Business Card</text>
  <text class="subtitle" transform="rotate(-90 161 355)" x="161" y="355" textLength="320" lengthAdjust="spacingAndGlyphs">RPN Calculator with Watch</text>

  <!-- (7) keyboard -->
@{[ keyboard() ]}
SVG
}
my $part_numbers = <<"SVG";
  <!-- part numbers in reading order: the upper part from left to right, then the keyboard -->
  <circle class="badge" cx="298" cy="81" r="11"/><text class="badgetx" x="298" y="86">1</text>
  <circle class="badge" cx="460" cy="136" r="11"/><text class="badgetx" x="460" y="141">2</text>
  <circle class="badge" cx="515" cy="122" r="11"/><text class="badgetx" x="515" y="127">3</text>
  <circle class="badge" cx="600" cy="28" r="11"/><text class="badgetx" x="600" y="33">4</text>
  <circle class="badge" cx="596" cy="108" r="11"/><text class="badgetx" x="596" y="113">5</text>
  <circle class="badge" cx="735" cy="140" r="11"/><text class="badgetx" x="735" y="145">6</text>
  <circle class="badge" cx="43.5" cy="355" r="11"/><text class="badgetx" x="43.5" y="360">7</text>
SVG
write_svg('rpn-calc-hp35-parts.svg', 'Business Card RPN Calculator with Watch: primary parts', '-10 -10 875 560', device($device_screen) . $part_numbers);
# the same drawing without the part numbers, for README.md
write_svg('rpn-calc.svg', 'Business Card RPN Calculator with Watch', '-10 -10 875 560', device($device_screen));

# ================================================================ labels
# "Extension" label for the text of the manual, in the colors of a GitHub label;
# textLength keeps the text inside the frame with any font
write_svg('rpn-calc-extension.svg', 'Extension', '0 0 76 20',
    qq{  <rect class="ext" x="0.5" y="0.5" width="75" height="19" rx="9.5"/>\n}
  . qq{  <text class="exttx" x="38" y="14" textLength="58" lengthAdjust="spacingAndGlyphs">Extension</text>\n});

# ================================================================ HP-21
# the screens of the HP-21 firmware and the device with its watch screen; the
# keyboards and the label above are the same for both firmware
use_sketch('hp21', 'HPVMWoodstock.h');

render_rtc(%watch);
my $device_screen21 = pixels_path();
screen_svg('rpn-calc-hp21-screen-watch.svg', 'Watch screen: 14:30:00, OCT 17/26, battery level, HP-21');

flash_svg('rpn-calc-hp21-screen-charge.svg', 'Watch screen during the charge: the last bar of the battery indicator flashes', 'barflash',
    sub { render_rtc(%watch, bars => 3) }, sub { render_rtc(%watch, bars => 2) }, [11, 'flashes']);

# FIX 2 is the format after the power-on of the ROM, a result that FIX cannot show is in SCI
render_calc21(x => display21(' 3.14'));
my $fix_screen = screen(0, [15.5, 'fixed point']);
render_calc21(x => display21(' 6.0221408 23'));
screens_svg('rpn-calc-hp21-screen-number.svg', 'Calculator screens: 3.14 in fixed point notation, 6.0221408 x 10^23 in scientific notation',
    $fix_screen, screen($scr_h + $scr_gap, [15.5, 'scientific']));

render_calc21(x => display21(' 3.14'), memory => 1, radians => 1);
screen_svg('rpn-calc-hp21-screen-indicators.svg', 'Calculator screen: 3.14, with the R and S indicators');

# the ROM shows Error steadily, HPVM::Error() of the sketch shows the same
render_calc21(x => display21(' Error'));
screen_svg('rpn-calc-hp21-screen-error.svg', 'Calculator screen with an error: Error');

render_calc21(y => display21(' 12.50'), x => display21(' 3.'), memory => 1);
screen_svg('rpn-calc-hp21-screen-function.svg', 'Calculator screen with F active: Y on top, X below', [7.5, 'Y'], [23.5, 'X']);

render_calc21(x => display21(' 30.'), menu => [ (menu_items('strMenuTrig'))[0..2] ]);
screen_svg('rpn-calc-hp21-screen-menu.svg', 'Calculator screen with the TRIG menu', [7.5, 'X'], [23.5, 'menu']);

sub menu21 { my $x = shift; sub { render_calc21(x => display21($x), menu => shift) } }
menu_svg('rpn-calc-hp21-screen-menu-math.svg', 'MATH menu: line 1 y^x, square root, 1/x; line 2 LOG, LN, e^x; line 3 10^x, REC, POL; line 4 M-, Mx, M/',
    'strMenuMath', menu21(' 2.'));
menu_svg('rpn-calc-hp21-screen-menu-trig.svg', 'TRIG menu: line 1 SIN, COS, TAN; line 2 ASIN, ACOS, ATAN', 'strMenuTrig', menu21(' 30.'));
menu_svg('rpn-calc-hp21-screen-menu-prog.svg', 'PROG menu: line 1 DSP, RAD, DEG; line 2 TIME, DATE', 'strMenuProg', menu21(' 0.00'));

write_svg('rpn-calc-hp21-parts.svg', 'Business Card RPN Calculator with Watch: primary parts', '-10 -10 875 560', device($device_screen21) . $part_numbers);
