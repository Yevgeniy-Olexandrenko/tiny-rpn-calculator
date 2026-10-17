#!/usr/bin/perl
# Generates the SVG images of the HP-35 user manual next to this script:
#   perl documents/manuals/images/hp35-images.pl [data.h]
# Default data.h: firmware/sketches/hp35/data.h.
#
# Units of the device and keyboard drawings: 0.1 mm. Board outline, keys,
# battery, USB cutout and the lanyard hole are from the Gerber and pick-and-place
# files of the PCB, the display from the device photo. Screens are rendered pixel
# by pixel with the firmware fonts of data.h the same way as the render*Mode()
# functions of code.h and the TXT module of the library do it.
use strict; use warnings; use utf8;
use FindBin;

my $data_h = $ARGV[0] // "$FindBin::Bin/../../../firmware/sketches/hp35/data.h";
open my $fh, '<', $data_h or die "$data_h: $!";
my $src = do { local $/; <$fh> };

# ================================================================ data.h

sub font_bytes {
    my $name = shift;
    $src =~ /const u08 ${name}_bytes\[\] PROGMEM =\s*\{(.*?)\};/s or die "no $name in $data_h";
    my $body = $1; $body =~ s{//[^\n]*}{}g;
    return [ map { hex } $body =~ /0x([0-9A-Fa-f]{2})/g ];
}
my %font = (
    menu5x8    => { bytes => font_bytes('menu5x8'),    w => 5, rows => 1, first => 0x2F },
    digits7x32 => { bytes => font_bytes('digits7x32'), w => 7, rows => 4, first => 0x61 },
    digits7x16 => { bytes => font_bytes('digits7x16'), w => 7, rows => 2, first => 0x61 },
);
$font{$_}{last} = $font{$_}{first} + @{$font{$_}{bytes}} / ($font{$_}{w} * $font{$_}{rows}) - 1 for keys %font;

# items of a menu string: "\06" " SIN  " " COS  " ...
sub menu_items {
    my $name = shift;
    $src =~ /const u08 ${name}\[\] DATAMEM =(.*?);/s or die "no $name in $data_h";
    my @s = $1 =~ /"([^"]*)"/g;
    shift @s; # item width
    return @s;
}

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
    print_char('i', 16, 0);
    print_char('i', 40, 0);
    print_seg_2($a{h},  0, 0);
    print_seg_2($a{m}, 24, 0);
    print_seg_2($a{s}, 48, 0);
    set_font('menu5x8');
    print_string($a{month}, 72, 0);
    print_string(sprintf('%02d', $a{date}), 97, 0);
    print_string(sprintf('%02d', $a{year}), 115, 0);
    print_char('/', 109, 0);
    set_scale(2, 1);
    print_string('HP=35', 72, 2);
    my $x = 72;
    for (1 .. $a{bars}) { print_char('=', $x, 1); $x += $char_dx }
}

# code.h: renderCalcNumber(), renderCalcMenu(), renderCalcMode()
sub render_number {
    my ($codes, $y) = @_;
    my $x = 0;
    for my $i (0..14) {
        set_font('digits7x16') if $i == 12;
        print_seg($num_seg[$codes->[$i]], $x, $y);
        $x += $char_dx;
    }
}
sub render_calc {
    my (%a) = @_;   # x => display, y => display (F active), menu => [3 items], memory => 1
    lcd_clear();
    set_font('digits7x16');
    print_char('j', 121, $a{menu} ? 0 : 2) if $a{memory};
    if ($a{y}) {
        render_number($a{y}, 0);
        render_number($a{x}, 2);
        print_char('i', 121, 0);
    }
    elsif ($a{menu}) {
        render_number($a{x}, 0);
        set_font('menu5x8');
        set_scale(1, 2);
        set_inverse(1);
        print_string($a{menu}[$_], 46 * $_, 2) for 0..2;
        set_inverse(0);
    }
    else {
        set_font('digits7x32');
        render_number($a{x}, 0);
    }
}

# code.h: renderTOTPMode()
sub render_totp {
    my (%a) = @_;
    lcd_clear();
    set_font('digits7x32');
    my $code = $a{code};
    for (my $i = 6; $i-- > 0;) {
        print_seg($num_seg[$code % 10], 8 + ($i < 3 ? $i : $i + 1) * 8, 0);
        $code = int($code / 10);
    }
    set_font('digits7x16');
    print_seg_2($a{left}, 104, 0);
    set_font('menu5x8');
    set_scale(1, 2);
    set_inverse(1);
    print_string(" $a{label} ", 92, 2);
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
# Classes that follow the theme get the light color and a dark one in @media.
my %theme = (           # light, dark
    bg    => ['#ffffff', '#0d1117'],
    edge  => ['#57606a', '#8b949e'],
    ink   => ['#1f2328', '#e6edf3'],
    ink2  => ['#57606a', '#8b949e'],
    chip  => ['#24292f', '#30363d'],
    pin   => ['#8c959f', '#6e7681'],
    badge => ['#0969da', '#2f81f7'],
    tint  => ['#ddf4ff', '#132339'],
);
my %class = (           # static properties, then property => theme color
    board    => ['stroke-width: 2.5', fill => 'bg', stroke => 'edge'],
    overlay  => ['stroke-width: 2', fill => 'bg', stroke => 'edge'],
    hole     => ['fill: none; stroke-width: 2', stroke => 'edge'],
    slot     => ['fill: none; stroke-width: 1.5', stroke => 'edge'],
    line     => ['fill: none; stroke-width: 1.5', stroke => 'ink'],
    cap      => ['', fill => 'ink'],
    main     => ['font-size: 52px; text-anchor: middle', fill => 'ink'],
    shift    => ['font-size: 26px', fill => 'ink'],
    title    => ['font-size: 46px; font-weight: 700; text-anchor: middle', fill => 'ink'],
    subtitle => ['font-size: 27px; text-anchor: middle', fill => 'ink2'],
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
    panel    => ['fill: #000000'],
    ledge    => ['fill: #57606a'],
    pixel    => ['fill: #ffffff'],
    ribbon   => ['fill: #d18b2c'],
    usb      => ['fill: #d0d7de; stroke: #57606a; stroke-width: 1.5'],
    tab      => ['fill: #d0d7de; stroke: #57606a; stroke-width: 1.5'],
    usbleg   => ['fill: #8c959f'],
    cell     => ['fill: #eaeef2; stroke: #d18b2c; stroke-width: 10'],
    label    => ['fill: #1f2328; font-size: 18px; font-weight: 600; text-anchor: middle'],
);
# <style> with the rules of the classes used in the svg body
sub style {
    my $body = shift;
    my %used; $used{$_} = 1 for map { split ' ' } $body =~ /class="([^"]+)"/g;
    my ($light, $dark) = ('', '');
    for my $c (sort keys %used) {
        my ($static, %prop) = @{$class{$c} // die "no style for class $c"};
        my @l = grep { length } $static, map { "$_: $theme{$prop{$_}}[0]" } sort keys %prop;
        my @d = map { "$_: $theme{$prop{$_}}[1]" } sort keys %prop;
        $light .= sprintf "    .%-9s{ %s; }\n", $c, join '; ', @l;
        $dark  .= sprintf "      .%-9s{ %s; }\n", $c, join '; ', @d if @d;
    }
    $dark = "    \@media (prefers-color-scheme: dark) {\n$dark    }\n" if $dark;
    return "  <style>\n$light$dark  </style>\n";
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
my @key_x  = (295, 455, 615, 775);             # key centers, 16 mm pitch
my @key_y  = (250, 330, 410, 490);             # 8 mm pitch
my @grid_x = (215, 375, 535, 695, 840);
my @grid_y = (200, 280, 360, 440, 520);
my @legends = (
    ['F', 'MATH', '7', 'TRIG',  '8', 'PROG', '9', '÷'],
    ['E', 'ROT↑', '4', 'CONST', '5', 'MEM+', '6', '×'],
    ['±', 'ROT↓', '1', 'RCL',   '2', 'STO',  '3', '−'],
    ['C', 'AC',   '0', 'LAST',  '.', 'X↔Y',  '↑', '+'],
);
my $main_cap = 18;                             # half of the cap height of the 52px main legend

my $key_defs = <<'DEFS';
    <!-- slot of the keyboard board and the button of the 3 x 6 mm tact switch -->
    <g id="key">
      <path class="slot" d="M-25,-20 H25 L35,-10 V10 L25,20 H-25 L-35,10 V-10 Z"/>
      <rect class="cap" x="-19" y="-9" width="38" height="18" rx="3"/>
    </g>
DEFS

# grid and keys: main legend centered in the cell left of the key, shifted legend above it
sub keyboard {
    my $grid = sprintf 'M%d,%d H%d V%d H%d Z', $grid_x[0], $grid_y[0], $grid_x[-1], $grid_y[-1], $grid_x[0];
    $grid .= " M$_,$grid_y[0] V$grid_y[-1]" for @grid_x[1..3];
    $grid .= " M$grid_x[0],$_ H$grid_x[-1]" for @grid_y[1..3];
    my $s = qq{  <path class="line" d="$grid"/>\n};
    for my $r (0..3) {
        my $main_y = ($grid_y[$r] + $grid_y[$r + 1]) / 2 + $main_cap;
        for my $c (0..3) {
            my ($main, $shift) = @{$legends[$r]}[2 * $c, 2 * $c + 1];
            my ($x, $y) = ($key_x[$c], $key_y[$r]);
            $s .= sprintf qq{  <use href="#key" x="%d" y="%d"/><text class="main" x="%d" y="%d">%s</text><text class="shift" x="%d" y="%d">%s</text>\n},
                $x, $y, $x - 60, $main_y, $main, $x - 35, $y - 25, $shift;
        }
    }
    return $s;
}
my $kbd_viewbox = sprintf '%d %d %d %d', $grid_x[0] - 10, $grid_y[0] - 10, $grid_x[-1] - $grid_x[0] + 20, $grid_y[-1] - $grid_y[0] + 20;

write_svg('hp35-keyboard.svg', 'Keyboard of the Tiny RPN Calculator', $kbd_viewbox,
    "  <defs>\n$key_defs  </defs>\n" . keyboard());

write_svg('hp35-keyboard-power-on.svg', 'Keys that turn on the device: the two bottom rows', $kbd_viewbox,
    "  <defs>\n$key_defs  </defs>\n" . keyboard()
    . sprintf(qq{  <!-- the two top rows do not turn on the device -->\n  <rect class="dim" x="%d" y="%d" width="%d" height="%d"/>\n},
        $grid_x[0] - 10, $grid_y[0] - 10, $grid_x[-1] - $grid_x[0] + 20, $grid_y[2] - $grid_y[0] + 9));

# ================================================================ screens
# 4 units per pixel on a black glass, notes on the right: [pixel row, text]
sub screen_svg {
    my ($name, $title, @notes) = @_;
    my $pad = 16;
    my $w = 128 * 4 + 2 * $pad;
    my $body = sprintf qq{  <rect class="glass" x="0" y="0" width="%d" height="%d" rx="8"/>\n}, $w, 32 * 4 + 2 * $pad;
    $body .= sprintf qq{  <path class="pixel" transform="translate($pad $pad) scale(4)" d="%s"/>\n}, pixels_path();
    for (@notes) {
        my ($row, $text) = @$_;
        $body .= sprintf qq{  <text class="note" x="%d" y="%d">← %s</text>\n}, $w + 14, $pad + $row * 4 + 8, $text;
    }
    my $vw = $w + 200;                         # same width for all screens, room for notes
    write_svg($name, $title, sprintf('-2 -2 %d %d', $vw + 4, 32 * 4 + 2 * $pad + 4), $body);
}

my %watch = (h => 14, m => 30, s => 0, month => 'OCT', date => 17, year => 26, bars => 4);

render_rtc(%watch);
my $device_screen = pixels_path();
screen_svg('hp35-screen-watch.svg', 'Watch screen: 14:30:00, OCT 17/26, battery level, HP-35');

render_calc(x => display(' 6.022140760 23'));
screen_svg('hp35-screen-number.svg', 'Calculator screen: 6.022140760 x 10^23', [7.5, 'exponent']);

render_calc(x => display(' 3.141592654   '), memory => 1);
screen_svg('hp35-screen-memory.svg', 'Calculator screen: pi, with the S indicator');

render_calc(y => display(' 12.5          '), x => display(' 3.            '), memory => 1);
screen_svg('hp35-screen-function.svg', 'Calculator screen with F active: Y on top, X below', [7.5, 'Y'], [23.5, 'X']);

render_calc(x => display(' 30.           '), menu => [ (menu_items('strMenuTrig'))[0..2] ]);
screen_svg('hp35-screen-menu.svg', 'Calculator screen with the TRIG menu', [7.5, 'X'], [23.5, 'menu']);

render_totp(code => 287082, left => 17, label => 'KEY1');
screen_svg('hp35-screen-totp.svg', 'TOTP screen: code 287 082, 17 seconds left, key KEY1', [7.5, 'seconds left'], [23.5, 'key label']);

# ================================================================ device
my $device = <<"SVG";
  <defs>
$key_defs    <!-- one side of a SOIC-8 -->
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
  <rect class="ribbon" x="310" y="58" width="56" height="78" rx="3"/>
  <rect class="glass" x="12" y="24" width="303" height="115" rx="3"/>
  <rect class="panel" x="16" y="28" width="246" height="94"/>
  <rect class="ledge" x="16" y="122" width="246" height="13"/>
  <path class="pixel" transform="translate(26 47) scale(1.75)" d="$device_screen"/>

  <!-- (5) MCU, ATtiny85 in SOIC-8 5.3 mm -->
  <use href="#soic-pins" x="460" y="40"/>
  <use href="#soic-pins" x="460" y="105"/>
  <rect class="chip" x="433.5" y="46" width="53" height="53" rx="3"/>
  <text class="chiptxt" x="460" y="77">MCU</text>

  <!-- (6) RTC, DS3231M in SOIC-8 3.9 mm -->
  <use href="#soic-pins" x="515" y="40"/>
  <use href="#soic-pins" x="515" y="90"/>
  <rect class="chip" x="490.5" y="45.5" width="49" height="39" rx="3"/>
  <text class="chiptxt" x="515" y="70">RTC</text>

  <!-- (3) USB-C receptacle, its front flush with the top edge -->
  <rect class="usbleg" x="549" y="42" width="8" height="16" rx="1"/>
  <rect class="usbleg" x="643" y="42" width="8" height="16" rx="1"/>
  <rect class="usb" x="556" y="0" width="88" height="72" rx="3"/>
  <text class="label" x="600" y="60">USB-C</text>

  <!-- (7) charger in SOT-23-5, right edge aligned with USB-C -->
  <rect class="pin" x="617" y="93" width="4" height="7"/>
  <rect class="pin" x="637" y="93" width="4" height="7"/>
  <rect class="pin" x="617" y="115" width="4" height="7"/>
  <rect class="pin" x="627" y="115" width="4" height="7"/>
  <rect class="pin" x="637" y="115" width="4" height="7"/>
  <rect class="chip" x="614" y="99" width="30" height="17" rx="2"/>

  <!-- (4) battery: 16 mm coin cell in the cutout of the board -->
  <circle class="cell" cx="735" cy="102.5" r="75"/>
  <rect class="tab" x="700" y="87.5" width="130" height="30" rx="6"/>
  <text class="label" x="735" y="70">BATTERY</text>

  <!-- keyboard board over the lower part: 37 mm high, cut out around the battery -->
  <path class="overlay" d="M0,170 H687.5 A82.55,82.55 0 0 0 782.5,170 H855 V510 a30,30 0 0 1 -30,30 H30 a30,30 0 0 1 -30,-30 Z"/>
  <text class="title" transform="rotate(-90 141 358)" x="141" y="358" textLength="330" lengthAdjust="spacingAndGlyphs">Business Card</text>
  <text class="subtitle" transform="rotate(-90 176 358)" x="176" y="358" textLength="330" lengthAdjust="spacingAndGlyphs">RPN Calculator with Watch</text>

  <!-- (2) keyboard -->
@{[ keyboard() ]}
  <!-- part numbers -->
  <circle class="badge" cx="288" cy="81" r="11"/><text class="badgetx" x="288" y="86">1</text>
  <circle class="badge" cx="196" cy="186" r="11"/><text class="badgetx" x="196" y="191">2</text>
  <circle class="badge" cx="600" cy="28" r="11"/><text class="badgetx" x="600" y="33">3</text>
  <circle class="badge" cx="735" cy="140" r="11"/><text class="badgetx" x="735" y="145">4</text>
  <circle class="badge" cx="460" cy="136" r="11"/><text class="badgetx" x="460" y="141">5</text>
  <circle class="badge" cx="515" cy="122" r="11"/><text class="badgetx" x="515" y="127">6</text>
  <circle class="badge" cx="596" cy="108" r="11"/><text class="badgetx" x="596" y="113">7</text>
SVG
write_svg('hp35-device.svg', 'Tiny RPN Calculator: primary parts', '-10 -10 875 560', $device);

# ================================================================ labels
# "Extension" label for the text of the manual, in the colors of a GitHub label;
# textLength keeps the text inside the frame with any font
write_svg('hp35-extension.svg', 'Extension', '0 0 76 20',
    qq{  <rect class="ext" x="0.5" y="0.5" width="75" height="19" rx="9.5"/>\n}
  . qq{  <text class="exttx" x="38" y="14" textLength="58" lengthAdjust="spacingAndGlyphs">Extension</text>\n});
