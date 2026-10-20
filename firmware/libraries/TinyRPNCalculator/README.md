# TinyRPNCalculator Library

An Arduino library for the Tiny RPN Calculator hardware platform: the PCB of the [Business Card RPN Calculator with Watch](../../../README.md). The library controls all the parts of the device: the display, the real-time clock, the keyboard and the power. It also contains virtual machines that run the original ROMs of HP calculators.

All the firmware of the project uses this library: [HP-35 Firmware](../../sketches/hp35) and [HP-21 Firmware](../../sketches/hp21).

## Contents

1. [Description](#1-description)
2. [Hardware](#2-hardware)
3. [Installation](#3-installation)
4. [Use the library](#4-use-the-library)
5. [Examples](#5-examples)
6. [Module reference](#6-module-reference)
7. [Extras](#7-extras)
8. [Limits](#8-limits)
9. [License](#9-license)

## 1. Description

The [ATtiny85](../../../datasheets/ATtiny85.pdf) has 8 KB of flash memory, 512 bytes of RAM and 512 bytes of EEPROM. Thus, the library is small and simple:

- The library is header-only. The compiler sees all the code, and link-time optimization removes the functions that the sketch does not use.
- Each module is a C++ namespace with free functions and variables. There are no classes, no virtual functions and no dynamic memory.
- The library has no dependencies. It does not use the Arduino functions (`pinMode()`, `millis()`, `Wire`). It uses the registers of the microcontroller directly.
- Compile-time options move constant data from flash to EEPROM and make the code smaller (refer to [4.2](#42-compile-time-options)).

| Module | Function |
| --- | --- |
| [SYS](#61-sys-system) | Initialization of all the modules, power-down of the device |
| [TMR](#62-tmr-frame-timer) | Frame timer on the watchdog timer, time in milliseconds |
| [PWR](#63-pwr-power) | Sleep modes, battery voltage and level, charging state |
| [KBD](#64-kbd-keyboard) | 16 keys on one analog input |
| [LCD](#65-lcd-display) | [SSD1306](../../../datasheets/SSD1306.pdf) OLED display, 128 × 32 pixels, with two screen buffers |
| [TXT](#66-txt-text) | Text with custom fonts, scale and inverse, 7-segment digits |
| [FNT](#67-fnt-fonts) | Built-in 5 × 8 font |
| [RTC](#68-rtc-real-time-clock) | [DS3231M](../../../datasheets/DS3231M.pdf) real-time clock: time, date, temperature, Unix time |
| [I2C](#69-i2c-bus) | I2C bus master on two pins (software) |
| [ADC](#610-adc-analog-to-digital-converter) | Analog-to-digital converter with noise reduction sleep |
| [BCD](#611-bcd-binary-coded-decimals) | Packed binary-coded decimals |
| [MEM](#612-mem-constant-data) | Constant data in flash or in EEPROM |

| Extra | Function |
| --- | --- |
| [HPVM](#71-hpvm-hp-virtual-machine) | Virtual machines of the HP Classic CPU (HP-35) and of the HP Woodstock CPU (HP-21), with the original ROMs |
| [SHA1](#72-sha1-and-totp) | SHA-1 and HMAC-SHA1 |
| [TOTP](#72-sha1-and-totp) | Time-based one-time passwords (RFC 6238) |
| [BuildTime](#73-buildtime) | Date and time of the build as compile-time constants |

## 2. Hardware

The library supports only this hardware:

- Microcontroller: ATtiny85 at 16 MHz from the internal PLL.
- PCB: Tiny RPN Calculator rev 1.5. For the circuit, refer to the [schematic](../../../hardware/pcb-rev1.5/Tiny-RPN-Calculator-rev1.5_Schematic.pdf).

The library stops the compilation with an error for a different microcontroller, clock source or frequency.

**Pins of the ATtiny85:**

| Pin | Function | Module |
| --- | --- | --- |
| PB0 | I2C SDA | I2C |
| PB1 | Display reset (RES#) | LCD |
| PB2 | I2C SCL | I2C |
| PB3 | Charger status (STAT), wake-up source | PWR |
| PB4 | Keyboard (ADC2), wake-up source | KBD |
| PB5 | RESET | — |

**Devices on the I2C bus:**

| Address | Device | Module |
| --- | --- | --- |
| `0x3C` | SSD1306 display controller | LCD |
| `0x68` | DS3231M real-time clock | RTC |

**Interrupts:** the library uses the vectors `ADC_vect`, `WDT_vect` and `PCINT0_vect`. Do not define these vectors in a sketch. The library does not use the timers 0 and 1.

## 3. Installation

### 3.1 Install the library

**Tools:** Windows, Arduino IDE 1.8.x.

1. Run `firmware/libraries/TinyRPNCalculator_Install.bat` as administrator. The script makes the symbolic link `Documents\Arduino\libraries\TinyRPNCalculator` to the library folder of the repository.
2. Restart the Arduino IDE.

The sketches then use the library from the repository: a change in the library has an effect on the next build.

> [!TIP]
> On other systems, copy the folder `firmware/libraries/TinyRPNCalculator` to the `libraries` folder of your Arduino sketchbook.

### 3.2 Install the board package

Install one of these board packages with **Tools → Board → Boards Manager**:

| Package | Boards Manager URL |
| --- | --- |
| [ATTinyCore](https://github.com/SpenceKonde/ATTinyCore) 1.5.2 | `http://drazzy.com/package_drazzy.com_index.json` |
| [TinyCore](https://github.com/MCUdude/TinyCore) (MCUdude) 0.8.0 | `https://mcudude.github.io/TinyCore/package_MCUdude_TinyCore_index.json` |

### 3.3 Set the board options

**ATTinyCore:**

| Option | Value |
| --- | --- |
| Board | ATtiny25/45/85 (No bootloader) |
| Chip | ATtiny85 |
| Clock Source | 16 MHz (PLL) |
| Timer 1 Clock | CPU (CPU frequency) |
| LTO | Enabled |
| millis()/micros() | Disabled (saves flash) |
| Save EEPROM | EEPROM retained |
| B.O.D. Level | B.O.D. Disabled (saves power) |

**TinyCore:**

| Option | Value |
| --- | --- |
| Board | ATtiny25/45/85 |
| Clock | 16 MHz internal PLL osc. |
| BOD | BOD disabled |
| EEPROM | EEPROM retained |
| Chip | ATtiny85 |
| Bootloader | No bootloader |

The library folder contains screenshots of these settings: `TinyRPNCalculator_ATTinyCore.png` and `TinyRPNCalculator_TinyCore.png`.

### 3.4 Fuses

The device needs these fuses:

| Fuse | Value | Settings |
| --- | --- | --- |
| Low | `0xF1` | Clock from the internal PLL, 16 MHz, no clock divider |
| High | `0xD7` | SPI programming enabled, EEPROM retained during chip erase, no brown-out detection |
| Extended | `0xFF` | Self-programming disabled |

For the ICSP connector and the avrdude commands, refer to [Firmware installation](../../../documents/manuals/hp35-firmware-user-manual.md#14-firmware-installation) in the HP-35 Firmware — User Manual.

### 3.5 Write the sketch to the device

1. Select **Sketch → Export compiled binary**. The IDE writes the files to the `build` folder of the sketch.
2. Write the `.ino.hex` file to the flash memory.
3. If the sketch sets `ENABLE_OPT_DATAMEM` to `1`, write the `.ino.eep` file to the EEPROM too.

> [!CAUTION]
> With `ENABLE_OPT_DATAMEM`, the constant data of the sketch and of the library is in the EEPROM. If you write only the `.hex` file, the sketch reads incorrect data.

## 4. Use the library

### 4.1 Minimal sketch

```cpp
#define PCB_REVISION 15

#include <TinyRPNCalculator.h>

int main()
{
	SYS::Init();
	LCD::TurnOn();
	TMR::Start(TMR::TIMEOUT_64MS);

	LCD::Clear();
	TXT::SetFont(FNT::Font5x8);
	TXT::PrintString(F("Hello!"), 0, 0);
	LCD::Flip();

	while (true)
	{
		// read the keys, update the display, etc.
		TMR::Sync();
	}
	return 0;
}
```

1. Define the compile-time options (refer to [4.2](#42-compile-time-options)).
2. Include `TinyRPNCalculator.h`. Then include the extras that the sketch uses (refer to [7](#7-extras)).
3. Define `int main()`. The sketch does not need the Arduino functions `setup()` and `loop()`, and the Arduino core does not start its timer.
4. Call `SYS::Init()` first.
5. Turn on the display and start the frame timer.
6. In the main loop, do the work of one frame. Then call `TMR::Sync()`. The MCU sleeps until the next frame.

### 4.2 Compile-time options

Define the options in the sketch before the `#include <TinyRPNCalculator.h>` line.

| Option | Values | Effect |
| --- | --- | --- |
| `PCB_REVISION` | `15` | Selects the PCB. `15` is PCB rev 1.5: display reset on PB1, charger status on PB3. You must set `15`: the default value `14` is not for PCB rev 1.5. |
| `ENABLE_OPT_DATAMEM` | `0` (default), `1` | `1` puts the `DATAMEM` data into the EEPROM, not into the flash (refer to [4.4](#44-constant-data)). |
| `ENABLE_OPT_NOINLINE` | `0` (default), `1` | `1` makes the functions with the `NOINLINE` mark real calls, not inline code. The code becomes smaller and a little slower. |

### 4.3 Main loop and power

The device has no ON/OFF switch. A sketch turns the device off after a time without key pushes:

1. Restart the timer with `TMR::Start()` when the user pushes a key. `TMR::Millis` then gives the time without key pushes.
2. When `TMR::Millis` is more than the time limit, call `SYS::Down()`. The MCU stops in power-down sleep.
3. A key push or a change of the charger status wakes up the MCU. `SYS::Down()` then returns.
4. Turn on the display with `LCD::TurnOn()` and start the timer with `TMR::Start()` again.

RAM keeps its data during the sleep. Thus, the sketch continues with the same state.

Only some keys wake up the MCU: the keys that give a voltage above the input high level of PB4. On PCB rev 1.5, these are the keys of the two bottom rows.

> [!NOTE]
> `SYS::Down()` does not return while the battery voltage is at `PWR::BAT_EMPTY` or less. A wake-up with an empty battery puts the MCU to sleep again.

For a full example, refer to the `Keyboard` example (refer to [5](#5-examples)).

### 4.4 Constant data

The library has three kinds of memory for constant data:

| Mark | Memory | Read with | Use for |
| --- | --- | --- | --- |
| (none) | RAM | Direct access | Variables |
| `PROGMEM` | Flash | `MEM::ProgRead()`, `MEM::ProgCopy()` | Large tables: fonts, ROMs |
| `DATAMEM` | Flash, or EEPROM if `ENABLE_OPT_DATAMEM` is `1` | `MEM::DataRead()`, `MEM::DataCopy()` | Small tables: key maps, menus, strings |

Strings have the same three kinds. `TXT::PrintString()` accepts all of them:

| Macro | String in | Example |
| --- | --- | --- |
| (none) | RAM | `TXT::PrintString(buffer, x, y)` with `const u08* buffer` |
| `F("...")` | Flash | `TXT::PrintString(F("TIME"), x, y)` |
| `D("...")` | `DATAMEM` | `TXT::PrintString(D("TIME"), x, y)` |

`FPSTR(p)` and `DPSTR(p)` change a pointer to a `PROGMEM` or `DATAMEM` array into a string pointer of the same kind. `DSTR("...")` gives a pointer to a string in `DATAMEM`.

> [!IMPORTANT]
> The EEPROM has 512 bytes. The Arduino IDE does not show the EEPROM data size and does not report an overflow. After a change to the `DATAMEM` data, examine the size of the `.ino.eep` file.

### 4.5 Data types and macros

Use the types of the library:

| Type | Definition |
| --- | --- |
| `b08` | `bool`: flags, results of tests, boolean parameters |
| `u08`, `u16`, `u32` | `uint8_t`, `uint16_t`, `uint32_t` |
| `s08`, `s16`, `s32` | `int8_t`, `int16_t`, `int32_t` |
| `w16` | Union of `u16 val` and its two bytes `u08 lsb, msb` |

| Macro | Function |
| --- | --- |
| `set_bit(sfr, bit)` | Sets a bit |
| `clr_bit(sfr, bit)` | Clears a bit |
| `isb_set(sfr, bit)` | True if the bit is set |
| `isb_clr(sfr, bit)` | True if the bit is clear |
| `NOINLINE` | Function attribute `noinline` if `ENABLE_OPT_NOINLINE` is `1`. Put it on its own line above the function. |

The library removes the Arduino definitions of `ADC`, `F` and `FPSTR`. The namespace `ADC` replaces the register macro `ADC`. Use `ADCL` and `ADCH` for the register.

## 5. Examples

Open the examples with **File → Examples → TinyRPNCalculator**. The examples do not use `ENABLE_OPT_DATAMEM`, thus write only the `.hex` file.

| Example | Function | Flash | RAM |
| --- | --- | ---: | ---: |
| [Display](examples/Display/Display.ino) | Text with the built-in font: scales x1, x2, x4 and inverse | 1928 | 16 |
| [Keyboard](examples/Keyboard/Keyboard.ino) | Code of the pushed key, battery voltage and level, charging state, power-down after 10 s | 2176 | 16 |
| [Watch](examples/Watch/Watch.ino) | Time, date and temperature of the real-time clock | 2368 | 22 |
| [TOTP](examples/TOTP/TOTP.ino) | Codes of the RFC 6238 test key | 4004 | 149 |
| [HP35](examples/HP35/HP35.ino) | HP-35 calculator with the F key as a shift key | 4214 | 180 |

Flash and RAM are in bytes, with TinyCore 0.8.0 and the options of [3.3](#33-set-the-board-options).

For the full firmware, refer to the sketches [hp35](../../sketches/hp35) and [hp21](../../sketches/hp21).

## 6. Module reference

Coordinates on the display: `x` is the pixel column (0 to 127). `y` is the page (0 to 3). A page is a row of 8 pixels.

### 6.1 SYS: system

| Function | Description |
| --- | --- |
| `void Init()` | Initializes ADC, I2C, LCD, KBD and PWR. Enables the pin-change interrupt and the interrupts. The display stays off and the timer stays stopped. |
| `void Down()` | Stops the timer, turns off the display and puts the MCU into power-down sleep (refer to [4.3](#43-main-loop-and-power)). |

### 6.2 TMR: frame timer

The watchdog timer gives an interrupt at the end of each frame.

| Item | Description |
| --- | --- |
| `TIMEOUT_16MS`, `TIMEOUT_32MS`, `TIMEOUT_64MS`, `TIMEOUT_125MS` | Frame times |
| `void Start(u08 t)` | Starts the timer with the frame time `t` and sets `Millis` to 0 |
| `void Stop()` | Stops the timer |
| `void Sync()` | Puts the MCU into idle sleep until the end of the frame |
| `volatile u16 Millis` | Time from `Start()` in milliseconds. It increases by the frame time (16, 32, 64 or 128 ms) at the end of each frame and overflows after 65535 ms. |
| `volatile u08 Timeout` | Frame time in milliseconds |

> [!NOTE]
> The watchdog oscillator is not accurate: the real frame time can be different by some percent. Use the real-time clock to measure long times.

### 6.3 PWR: power

| Item | Description |
| --- | --- |
| `BAT_MAX`, `BAT_FULL`, `BAT_EMPTY` | Battery voltages in mV: 4200, 4100, 3500 |
| `UNIT_VOLTAGE` | Resolution of the voltage measurement: 20 mV |
| `u16 Voltage()` | Supply voltage in mV. The ADC measures the internal 1.1 V reference against VCC. |
| `u08 Level()` | Battery level from 0 to 100 %: 0 at `BAT_EMPTY` or less, 100 at `BAT_FULL` or more, linear between them |
| `b08 Charging()` | True while the charger charges the battery |
| `void Idle()` | Idle sleep until the next interrupt |
| `void Down()` | Power-down sleep until a pin change. The MCU sleeps again while the voltage is at `BAT_EMPTY` or less. Use `SYS::Down()`. |
| `void Init()` | Sets PB3 as an input with pull-up and as a wake-up source. `SYS::Init()` calls it. |

### 6.4 KBD: keyboard

The keys connect resistor dividers to PB4. The ADC measures the voltage, and the library finds the key from a table of thresholds. The compiler calculates the thresholds from the resistor values (`KBD_RG1`..`KBD_RG4`, `KBD_RV1`..`KBD_RV4`).

| Function | Description |
| --- | --- |
| `u08 Read()` | Code of the pushed key, or `NONE`. While the user holds the key, `Read()` gives the code of the first measurement. |
| `void Init()` | Sets PB4 as an input and as a wake-up source. `SYS::Init()` calls it. |

**Key codes**, in the positions of the keys on the device:

| Column 1 | Column 2 | Column 3 | Column 4 |
| --- | --- | --- | --- |
| <kbd>F</kbd> `FUNC` (15) | <kbd>7</kbd> `NUM7` (7) | <kbd>8</kbd> `NUM8` (8) | <kbd>9</kbd> `NUM9` (9) |
| <kbd>E</kbd> `EEX` (14) | <kbd>4</kbd> `NUM4` (4) | <kbd>5</kbd> `NUM5` (5) | <kbd>6</kbd> `NUM6` (6) |
| <kbd>±</kbd> `CHS` (13) | <kbd>1</kbd> `NUM1` (1) | <kbd>2</kbd> `NUM2` (2) | <kbd>3</kbd> `NUM3` (3) |
| <kbd>C</kbd> `CLR` (12) | <kbd>0</kbd> `NUM0` (0) | <kbd>.</kbd> `DOT` (10) | <kbd>↑</kbd> `DUP` (11) |

The digit keys have the codes 0 to 9. Thus, a sketch can use the code as a digit or as an index into a table of 16 items.

The aliases `ROTU`, `ROTD`, `SEL1`..`SEL3`, `MATH`, `TRIG`, `PROG`, `CONS` and `SETS` are names of the same keys for menus.

### 6.5 LCD: display

The SSD1306 has RAM for 64 rows, and the display shows 32 rows. The library uses the two halves as two screen buffers: it writes to the hidden buffer, and `Flip()` shows it. Thus, the user does not see a partly drawn screen.

| Item | Description |
| --- | --- |
| `I2C_ADDR`, `WIDTH`, `PAGES` | `0x3C`, 128 columns, 4 pages |
| `void Init()` | Resets and configures the controller. The display stays off. `SYS::Init()` calls it. |
| `void TurnOn()`, `void TurnOff()` | Turns the display on or off |
| `void Brightness(u08 brightness)` | Sets the contrast from 0 to 255 |
| `void Clear()` | Clears the hidden buffer |
| `void Flip()` | Shows the hidden buffer. The buffer that was on the display becomes hidden. |
| `void BeginWrite(u08 x, u08 y)` | Starts to write bytes to the hidden buffer from column `x` of page `y` |
| `void Write(u08 b, u08 s)` | Writes the byte `b` `s` times. Each byte is a column of 8 pixels, bit 0 at the top. |
| `void EndWrite()` | Ends the write |
| `void EndWrite(u08 b, u08 s)` | Writes the byte `b` `s` times and ends the write |

A typical screen update:

```cpp
LCD::Clear();
// draw the screen with TXT or LCD::BeginWrite() ... LCD::EndWrite()
LCD::Flip();
```

### 6.6 TXT: text

**Fonts.** A font is a `TXT::Font` structure in `PROGMEM`:

| Field | Description |
| --- | --- |
| `bytesInRow` | Width of a character in pixels |
| `rowsOfBytes` | Height of a character in pages |
| `asciiFirst`, `asciiLast` | Character codes in the font |
| `bytes` | Pointer to the character data in `PROGMEM`: for each character, `rowsOfBytes` rows of `bytesInRow` column bytes |

`TXT` shows a character that is not in the font as a space. The tool `firmware/tools/lcd2hex` makes the font data from `.lcd` files.

| Item | Description |
| --- | --- |
| `void SetFont(const Font& f)` | Selects the font (a `PROGMEM` structure) and sets the scale to x1 |
| `void SetScale(u08 sx, u08 sy)` | Sets the horizontal scale `sx` (1 or more) and the vertical scale `sy` (`x1`, `x2` or `x4`) |
| `void SetInverse(b08 enable)` | Selects dark characters on a light background |
| `u08 char_dx` | Distance between characters in pixels: width × `sx` + 1. Read it, do not change it. |
| `void PrintChar(u08 ch, u08 x, u08 y)` | Shows a character and one column of space after it |
| `void PrintString(s, x, y)` | Shows a string from RAM, `F()` or `D()` (refer to [4.4](#44-constant-data)) |
| `void PrintString(s, i, x, y)` | Shows item `i` of a string array in `PROGMEM` or `DATAMEM`. The first byte of the array is the length of each item, then the items follow without separators. |
| `void PrintBCD(u08 n, u08 x, u08 y)` | Shows two digits of a packed BCD number |

The vertical scale shows each page of the font on 2 or 4 pages of the display.

**String array** for `PrintString(s, i, x, y)`:

```cpp
const u08 strMonth[] DATAMEM = "\03" "JAN" "FEB" "MAR"; // 3 characters in each item
TXT::PrintString(DPSTR(strMonth), 1, x, y);             // shows FEB
```

**7-segment digits.** In a 7-segment font, the first 8 characters are the images of the segments a to g and of the dot. `PrintSeg()` shows the segments of a pattern together. The scale and the inverse have no effect on it.

| Item | Description |
| --- | --- |
| `SEG_0`..`SEG_9`, `SEG_A`, `SEG_b`, `SEG_C`, `SEG_d`, `SEG_E`, `SEG_F`, `SEG_H`, `SEG_o`, `SEG_P`, `SEG_r`, `SEG_SPACE`, `SEG_DASH`, `SEG_DOT` | Segment patterns: bit 0 is segment a, bit 6 is segment g, bit 7 is the dot |
| `u08 NumToSeg(u08 n)` | Pattern of `n`: 0 to 15 are the hexadecimal digits, 16 is a space, 17 is a dash, 18 is a dot |
| `void PrintSeg(u08 seg, u08 x, u08 y)` | Shows a pattern |
| `void PrintSegBCD(u08 n, u08 x, u08 y)` | Shows two digits of a packed BCD number as patterns |

### 6.7 FNT: fonts

| Item | Description |
| --- | --- |
| `FNT::Font5x8` | 5 × 8 pixels, one page high, characters `!` to `~` (0x21 to 0x7E), 470 bytes |

The linker includes the font only if the sketch uses it.

### 6.8 RTC: real-time clock

The variables keep the time and the date in packed BCD (for example, `0x59` is 59). At start, they contain the build time of the sketch (refer to [7.3](#73-buildtime)). Thus, a sketch can call `WriteTimeDate()` after a reset to set the clock.

| Item | Description |
| --- | --- |
| `u08 Seconds`, `Minutes`, `Hours` | 0 to 59, 0 to 59, 0 to 23 (24-hour format) |
| `u08 Date`, `Month`, `Year` | 1 to 31, 1 to 12, 0 to 99 (2000 to 2099) |
| `void ReadTimeDate()` | Reads the clock into the variables |
| `void WriteTimeDate()` | Writes the variables to the clock |
| `w16 ReadTemperature()` | Temperature of the clock chip: `msb` is the signed value in °C, bits 7..6 of `lsb` are the fraction in 0.25 °C steps |
| `b08 IsLeapYear()` | True if `Year` is a leap year |
| `u08 GetDaysInMonth()` | Number of days in `Month` of `Year` |
| `u32 GetTimestamp(s08 GMTTimeZone)` | Unix time in seconds. The clock keeps the local time: `GMTTimeZone` is the time zone in hours (for example, `+3`). Gives 0 if the date is not valid. |

### 6.9 I2C: bus

The library makes the I2C signals in software on PB0 (SDA) and PB2 (SCL). The pins operate as open-drain outputs, with the pull-up resistors of the PCB. A half period of the clock is approximately 2 µs.

| Function | Description |
| --- | --- |
| `void Init()` | Releases the bus. If a device holds SDA low, up to 9 clock pulses release it. `SYS::Init()` calls it. |
| `void StartWrite(u08 addr)` | Start condition and the 7-bit address for a write |
| `void StartRead(u08 addr)` | Start condition (or repeated start) and the 7-bit address for a read |
| `void Write(u08 data)` | Writes a byte |
| `void ReadAck(u08& data)` | Reads a byte and sends ACK: more bytes follow |
| `void ReadNack(u08& data)` | Reads a byte and sends NACK: the last byte |
| `void Stop()` | Stop condition |

The functions do not examine the ACK bit from the device.

### 6.10 ADC: analog-to-digital converter

| Item | Description |
| --- | --- |
| `A0_PB5`, `A1_PB2`, `A2_PB4`, `A3_PB3` | Input pins, reference VCC |
| `VCC` | Internal 1.1 V reference, measured against VCC |
| `TEMP` | Internal temperature sensor, reference 1.1 V |
| `w16 Read(u08 channel)` | Makes one 10-bit conversion. The MCU sleeps in the ADC noise reduction mode during the conversion. Add `_BV(ADLAR)` to the channel to get the 8 high bits in `msb`. |
| `void Init()` | Sets the ADC clock to 125 kHz and turns off the ADC. `SYS::Init()` calls it. |

The ADC is off between conversions, to decrease the power consumption.

### 6.11 BCD: binary-coded decimals

| Function | Description |
| --- | --- |
| `constexpr u08 Encode(u08 data)` | Binary 0 to 99 to packed BCD |
| `constexpr u08 Decode(u08 data)` | Packed BCD to binary |

### 6.12 MEM: constant data

| Item | Description |
| --- | --- |
| `DATAMEM` | Variable attribute: `EEMEM` if `ENABLE_OPT_DATAMEM` is `1`, else `PROGMEM` |
| `u08 ProgRead(const u08* src)` | Reads a byte from the flash |
| `void ProgCopy(void* dst, const void* src, size_t size)` | Copies bytes from the flash to RAM |
| `u08 DataRead(const u08* src)` | Reads a byte from `DATAMEM` |
| `void DataCopy(void* dst, const void* src, size_t size)` | Copies bytes from `DATAMEM` to RAM |

## 7. Extras

The extras are in `src/Extras`. Include them after `TinyRPNCalculator.h`:

```cpp
#include <TinyRPNCalculator.h>
#include <Extras/HP35Model.h>   // model first
#include <Extras/HPVMClassic.h> // then the CPU of the model
#include <Extras/TOTP.h>        // includes SHA1.h
```

### 7.1 HPVM: HP virtual machine

The virtual machine runs the original ROM of an HP calculator on a copy of its CPU. Each calculator has two headers. Include them in this sequence:

1. The model header: the operations, the key codes of the model and the ROM.
2. The CPU header: the CPU that runs the ROM.

| Calculator | Model header | CPU header | Display | Status |
| --- | --- | --- | --- | --- |
| HP-35 | `HP35Model.h` | `HPVMClassic.h` (Classic CPU) | `Display[15]`: character codes `DIGIT` + digit, `BLANK`, `DASH`, `DOT` | Used in the hp35 firmware |
| HP-21 | `HP21Model.h` | `HPVMWoodstock.h` (Woodstock CPU) | `Display[12]`: 7-segment patterns (`TXT::SEG_*`, dot in bit 7) | Tested on the PC only |

**Operations.** The model header defines the operations `Op*` in the sequence of the keys of the calculator (for example, `HPVM::OpNUM7`, `HPVM::OpSIN_`). A shifted operation is `FlSHFT` + the operation of the key (for example, `HPVM::OpASIN`). `Operation()` pushes the shift key and then the key.

| Item | Description |
| --- | --- |
| `void Operation(u08 op)` | Pushes the key of the operation. If the ROM is busy, the function first runs it until it waits for a key. |
| `void OpAndWait(u08 op)` | Pushes the key and runs the ROM until it waits for the next key |
| `b08 Cycle()` | Runs one CPU cycle. Gives true if the display changed. |
| `b08 Idling()` | True if the ROM waits for a key |
| `void DisplayY(u08 (&disp)[sizeof(Display)])` | Puts the Y register, in the format of `Display`, into `disp` |
| `void Error()` | Clears the stack and shows the error of the calculator |
| `CYCLES_PER_SEC` | CPU cycles in one second of the original calculator |
| `A`, `B`, `C`, ... | CPU registers: arrays of 14 BCD digits. `C` is the X register. |

**Real speed.** To run the calculator at the speed of the original, run `CYCLES_PER_SEC / frames per second` cycles in each frame:

```cpp
for (u16 n = HPVM::CYCLES_PER_SEC / 15; n > 0; --n)
{
	if (HPVM::Cycle())
	{
		// show HPVM::Display
		break;
	}
}
```

**Woodstock flags.** `HPVM::F1` and `HPVM::F2` are inputs of the Woodstock CPU. For the HP-21: `F1 = true` shows the low battery indicator, `F2 = true` selects radians.

For the Woodstock CPU, refer to [HP-21 Woodstock architecture](../../../documents/firmware/hp21-woodstock-architecture.md) (in Russian). The tool `firmware/tools/hpvmtest` runs the virtual machines on a PC, with the same headers.

### 7.2 SHA1 and TOTP

**SHA1** (`SHA1.h`): SHA-1 (FIPS 180-2) and HMAC-SHA1 (RFC 2104). The message length is up to 65535 bytes, the key length is up to 64 bytes. The module uses 90 bytes of static RAM.

| Function | Description |
| --- | --- |
| `void Init()` | Starts a hash |
| `void Write(u08 data)`, `void Write(const u08* data, u08 size)` | Adds message bytes |
| `const u08* Result()` | Ends the hash. Gives a pointer to the 20 bytes of the hash. |
| `void InitHmac(const u08* key, u08 keyLength)` | Starts an HMAC. The key must stay in RAM until `ResultHmac()`. |
| `const u08* ResultHmac()` | Ends the HMAC. Gives a pointer to the 20 bytes of the HMAC. |

The result pointer is valid until the next `Init()` or `InitHmac()`.

**TOTP** (`TOTP.h`): time-based one-time passwords (RFC 6238) with HMAC-SHA1, a period of 30 seconds and 6 digits.

| Item | Description |
| --- | --- |
| `PERIOD` | 30 seconds |
| `struct Key` | `length` (1 to `KEY_MAX` = 32), `label[4]`, `data[32]` |
| `u32 GetCode(const Key& key, u32 counter)` | Code from 0 to 999999. The counter is the Unix time / `PERIOD`. |
| `b08 ReadKey(const u08* area, u08 index, Key& key)` | Copies key `index` from the keys area in `DATAMEM`. Gives false if the key does not exist. |

**Keys area.** The tool `firmware/tools/totp` makes the keys area from labels and Base32 secrets. It can also replace the area in a compiled `.eep` file: the tool finds the area by its signature. Thus, the area must have this layout:

| Bytes | Content |
| --- | --- |
| 5 | Signature: `T`, `O`, `T`, `P` and the size of the area in bytes |
| 1 + 4 + length | Key: length, label of 4 characters (padded with spaces), key bytes. Repeated for each key. |
| 1 | 0: end of the keys, if there is space |

### 7.3 BuildTime

`BuildTime.h` gives the date and the time of the build as compile-time constants: `BUILD_YEAR` (0 to 99), `BUILD_MONTH`, `BUILD_DAY`, `BUILD_HOUR`, `BUILD_MIN`, `BUILD_SEC`. The RTC module uses them for the start values of its variables.

## 8. Limits

- The library supports only the ATtiny85 at 16 MHz from the PLL, on PCB rev 1.5.
- `TMR::Millis` overflows after 65535 ms. Restart the timer for longer intervals, or use the real-time clock.
- The RTC module does not use the day of the week, the century bit and the alarms. It supports the years 2000 to 2099.
- The I2C module does not examine the ACK bit.
- The library takes some names of the global namespace: the module namespaces, the types `b08`..`w16`, the macros `set_bit`, `clr_bit`, `isb_set`, `isb_clr`, `F`, `FPSTR`, `D`, `DPSTR`, `DSTR`, `DATAMEM`, `NOINLINE` and the `I2C_*`, `KBD_*`, `LCD_*`, `PWR_*` macros.

## 9. License

[MIT](LICENSE)
