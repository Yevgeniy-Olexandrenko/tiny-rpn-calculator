#!/usr/bin/perl
# Generates hp35-device.svg, the drawing of the device in the user manual:
#   perl documents/manuals/images/hp35-device.pl [data.h] [out.svg]
# Defaults: firmware/sketches/hp35/data.h and hp35-device.svg next to this script.
# Units: 0.1 mm. Board outline, keys, battery, USB cutout and the lanyard hole
# are from the Gerber and pick-and-place files of the PCB, the display from the
# device photo. The screen is rendered pixel by pixel with the firmware fonts
# (sketches/hp35/data.h) the same way as renderRTCMode() in code.h.
use strict; use warnings; use utf8;
use FindBin;

my $data_h = $ARGV[0] // "$FindBin::Bin/../../../firmware/sketches/hp35/data.h";
my $out    = $ARGV[1] // "$FindBin::Bin/hp35-device.svg";

# ---------------------------------------------------------------- fonts
open my $fh, '<', $data_h or die "$data_h: $!";
my $src = do { local $/; <$fh> };
sub font_bytes {
    my $name = shift;
    $src =~ /const u08 ${name}_bytes\[\] PROGMEM =\s*\{(.*?)\};/s or die "no $name in $data_h";
    my $body = $1; $body =~ s{//[^\n]*}{}g;
    return [ map { hex } $body =~ /0x([0-9A-Fa-f]{2})/g ];
}
my %font = (
    menu5x8    => { bytes => font_bytes('menu5x8'),    w => 5, rows => 1, first => 0x2F },
    digits7x32 => { bytes => font_bytes('digits7x32'), w => 7, rows => 4, first => 0x61 },
);

# ---------------------------------------------------------------- screen
# 128 x 32 pixels, a byte is a column of 8 pixels of a page, bit 0 on top
my @px = map { [(0) x 128] } 0..31;
sub write_col {
    my ($x, $page, $b) = @_;
    return if $x > 127;
    for my $bit (0..7) { $px[$page * 8 + $bit][$x] = 1 if $b & (1 << $bit) }
}

# TXT::PrintChar with horizontal scale sx, returns char_dx
sub print_char {
    my ($f, $ch, $x, $y, $sx) = @_;
    my $F = $font{$f}; $sx //= 1;
    my $off = (ord($ch) - $F->{first}) * $F->{w} * $F->{rows};
    for my $r (0 .. $F->{rows} - 1) {
        my $cx = $x;
        for my $i (0 .. $F->{w} - 1) {
            write_col($cx++, $y + $r, $F->{bytes}[$off + $r * $F->{w} + $i]) for 1..$sx;
        }
    }
    return $F->{w} * $sx + 1;
}
sub print_string { my ($f, $s, $x, $y, $sx) = @_; $x += print_char($f, $_, $x, $y, $sx) for split //, $s }

# TXT::PrintSeg: OR of the segment glyphs 'a'..'h' selected by the bits of seg
my @seg = (0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F);
sub print_seg {
    my ($f, $s, $x, $y) = @_;
    my $F = $font{$f}; my $ds = $F->{w} * $F->{rows};
    for my $r (0 .. $F->{rows} - 1) {
        for my $i (0 .. $F->{w} - 1) {
            my $b = 0;
            for my $k (0..7) { $b |= $F->{bytes}[$k * $ds + $r * $F->{w} + $i] if $s & (1 << $k) }
            write_col($x + $i, $y + $r, $b);
        }
    }
}
# TXT::PrintSegBCD for a decimal number 0..99
sub print_seg_2 { my ($f, $n, $x, $y) = @_; print_seg($f, $seg[$n / 10], $x, $y); print_seg($f, $seg[$n % 10], $x + 8, $y) }

# renderRTCMode(): 14:30:00, OCT 17/26, 4 bars of the battery level
print_char('digits7x32', 'i', 16, 0);
print_char('digits7x32', 'i', 40, 0);
print_seg_2('digits7x32', 14,  0, 0);
print_seg_2('digits7x32', 30, 24, 0);
print_seg_2('digits7x32',  0, 48, 0);
print_string('menu5x8', 'OCT', 72, 0);
print_string('menu5x8', '17',  97, 0);
print_char('menu5x8', '/', 109, 0);
print_string('menu5x8', '26', 115, 0);
print_string('menu5x8', 'HP=35', 72, 2, 2);
print_string('menu5x8', '====',  72, 1, 2);

# lit pixels as rectangles: horizontal runs, equal runs of next rows merged
my (@rects, %open);
for my $y (0..31) {
    my %next;
    for (my $x = 0; $x < 128; ) {
        if (!$px[$y][$x]) { $x++; next }
        my $s = $x; $x++ while $x < 128 && $px[$y][$x];
        my $key = "$s,$x";
        if (exists $open{$key}) { $rects[$open{$key}][3]++; $next{$key} = $open{$key} }
        else { push @rects, [$s, $y, $x - $s, 1]; $next{$key} = $#rects }
    }
    %open = %next;
}
my $pixels = join '', map { sprintf 'M%d %dh%dv%dh-%dz', $_->[0], $_->[1], $_->[2], $_->[3], $_->[2] } @rects;

# ---------------------------------------------------------------- keyboard
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
my $keys = '';
for my $r (0..3) {
    # main legend centered vertically in the grid cell
    my $main_y = ($grid_y[$r] + $grid_y[$r + 1]) / 2 + $main_cap;
    for my $c (0..3) {
        my ($main, $shift) = @{$legends[$r]}[2 * $c, 2 * $c + 1];
        my ($x, $y) = ($key_x[$c], $key_y[$r]);
        $keys .= sprintf qq{  <use href="#key" x="%d" y="%d"/><text class="main" x="%d" y="%d">%s</text><text class="shift" x="%d" y="%d">%s</text>\n},
            $x, $y, $x - 60, $main_y, $main, $x - 35, $y - 25, $shift;
    }
}
my $grid = sprintf 'M%d,%d H%d V%d H%d Z', $grid_x[0], $grid_y[0], $grid_x[-1], $grid_y[-1], $grid_x[0];
$grid .= " M$_,$grid_y[0] V$grid_y[-1]" for @grid_x[1..3];
$grid .= " M$grid_x[0],$_ H$grid_x[-1]" for @grid_y[1..3];

# ---------------------------------------------------------------- svg
# Colors that follow the theme are in the @media block; the display, USB-C and
# battery look the same in both themes.
my $svg = <<"SVG";
<svg xmlns="http://www.w3.org/2000/svg" viewBox="-10 -10 875 560" width="875" height="560" font-family="Segoe UI, Helvetica, Arial, sans-serif">
  <title>Tiny RPN Calculator: primary parts</title>
  <style>
    .board, .overlay { fill: #ffffff; }
    .board   { stroke-width: 2.5; }
    .overlay, .hole { stroke-width: 2; }
    .hole    { fill: none; }
    .slot    { fill: none; stroke-width: 1.5; }
    .board, .overlay, .hole, .slot { stroke: #57606a; }
    .line    { fill: none; stroke: #1f2328; stroke-width: 1.5; }
    .main, .shift, .title, .cap { fill: #1f2328; }
    .main    { font-size: 52px; text-anchor: middle; }
    .shift   { font-size: 26px; }
    .title   { font-size: 46px; font-weight: 700; text-anchor: middle; }
    .subtitle{ fill: #57606a; font-size: 27px; text-anchor: middle; }
    .chip    { fill: #24292f; }
    .pin     { fill: #8c959f; }
    .chiptxt { fill: #ffffff; font-size: 14px; font-weight: 600; text-anchor: middle; }
    .badge   { fill: #0969da; }
    .badgetx { fill: #ffffff; font-size: 13px; font-weight: 700; text-anchor: middle; }
    .glass   { fill: #24292f; stroke: #57606a; stroke-width: 1.5; }
    .panel   { fill: #000000; }
    .ledge   { fill: #57606a; }
    .pixel   { fill: #ffffff; }
    .ribbon  { fill: #d18b2c; }
    .usb, .tab { fill: #d0d7de; stroke: #57606a; stroke-width: 1.5; }
    .usbleg  { fill: #8c959f; }
    .cell    { fill: #eaeef2; stroke: #d18b2c; stroke-width: 10; }
    .label   { fill: #1f2328; font-size: 18px; font-weight: 600; text-anchor: middle; }
    \@media (prefers-color-scheme: dark) {
      .board, .overlay { fill: #0d1117; }
      .board, .overlay, .hole, .slot { stroke: #8b949e; }
      .line    { stroke: #e6edf3; }
      .main, .shift, .title, .cap { fill: #e6edf3; }
      .subtitle{ fill: #8b949e; }
      .chip    { fill: #30363d; }
      .pin     { fill: #6e7681; }
      .badge   { fill: #2f81f7; }
    }
  </style>
  <defs>
    <!-- slot of the keyboard board and the button of the 3 x 6 mm tact switch -->
    <g id="key">
      <path class="slot" d="M-25,-20 H25 L35,-10 V10 L25,20 H-25 L-35,10 V-10 Z"/>
      <rect class="cap" x="-19" y="-9" width="38" height="18" rx="3"/>
    </g>
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
  <rect class="ribbon" x="310" y="58" width="56" height="78" rx="3"/>
  <rect class="glass" x="12" y="24" width="303" height="115" rx="3"/>
  <rect class="panel" x="16" y="28" width="246" height="94"/>
  <rect class="ledge" x="16" y="122" width="246" height="13"/>
  <path class="pixel" transform="translate(26 47) scale(1.75)" d="$pixels"/>

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

  <!-- (2) keyboard: main legend left of the key, shifted legend above it -->
  <path class="line" d="$grid"/>
$keys
  <!-- part numbers -->
  <circle class="badge" cx="288" cy="81" r="11"/><text class="badgetx" x="288" y="86">1</text>
  <circle class="badge" cx="196" cy="186" r="11"/><text class="badgetx" x="196" y="191">2</text>
  <circle class="badge" cx="600" cy="28" r="11"/><text class="badgetx" x="600" y="33">3</text>
  <circle class="badge" cx="735" cy="140" r="11"/><text class="badgetx" x="735" y="145">4</text>
  <circle class="badge" cx="460" cy="136" r="11"/><text class="badgetx" x="460" y="141">5</text>
  <circle class="badge" cx="515" cy="122" r="11"/><text class="badgetx" x="515" y="127">6</text>
  <circle class="badge" cx="596" cy="108" r="11"/><text class="badgetx" x="596" y="113">7</text>
</svg>
SVG

open my $o, '>:encoding(UTF-8)', $out or die "$out: $!";
print $o $svg;
close $o;
printf "%s: %d bytes, screen %d rectangles\n", $out, -s $out, scalar @rects;
