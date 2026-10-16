# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

Credit-card-sized RPN calculator with watch: ATtiny85 (8 KB flash, 512 B RAM, 512 B EEPROM) + SSD1306 128x32 OLED + DS3231M RTC + a 16-key keyboard read through a single analog pin. The calculator firmware is an emulator of real HP calculators: it runs the original HP ROM on a virtual machine of the HP CPU.

## Layout

- `firmware/` — current firmware (active development).
  - `libraries/TinyRPNCalculator/` — Arduino library.
  - `sketches/<model>/` — one Arduino sketch per calculator model (`hp35`; `hp21` is new, not yet tested on hardware).
  - `tools/` — Windows C++ helper tools (Visual Studio projects, prebuilt `.exe`): `lcd2hex` converts `.lcd` (XML font files) into C byte arrays; `lst2asm` turns the avr-gcc `.lst` listing into a cleaner `.asm` for inspecting generated code; `hpvmtest` is the desktop test of the HP VMs and TOTP (see below).
- `firmware.old/` — previous firmware generation (hp21, hp35, native calculator, tester, totp). Reference only; don't modify unless asked.
- `hardware/` — PCB revisions (Gerbers, BOM, schematics). `datasheets/`, `documents/` — reference docs (several `documents/*.md` are still empty placeholders).

## Build / flash

VM test: `firmware/tools/hpvmtest/build.bat` compiles the library's `Extras/` VM + model headers for the PC with MSVC (Visual Studio C++ tools, found via `vswhere`), using small AVR stubs, and runs the original ROM on key sequences (`hp21test.exe`: exit code = number of failures; `hp21test "<keys>"` traces execution cycle by cycle). Run it after any change to `HPVMWoodstock.h` / `HP21Model.h`. The same script builds `totptest.exe`, which checks `SHA1.h`/`TOTP.h` against the FIPS 180-2, RFC 2202, RFC 4226 and RFC 6238 vectors; run it after any change to them. The script's exit code is the total number of failures. Build output goes to the ignored `build/` folder.

There is no CLI build script or linter for the firmware. Builds are done in the Arduino IDE:

1. Run `firmware/libraries/TinyRPNCalculator_Install.bat` as administrator — it symlinks the library into `%USERPROFILE%\Documents\Arduino\libraries\TinyRPNCalculator`.
2. Board core: ATTinyCore, "ATtiny25/45/85 (No bootloader)", chip ATtiny85, clock **16 MHz (PLL)**, LTO enabled, millis()/micros() disabled, EEPROM retained, BOD disabled (see `TinyRPNCalculator_ATTinyCore.png`). The library `#error`s unless `__AVR_ATtiny85__`, `F_CPU == 16000000` and `CLOCK_SOURCE == 6`.
3. Fuses for PCB rev 1.x: High `0xD7`, Low `0xF1`, Ext `0xFF`.
4. "Export compiled binary" puts output in `sketches/<model>/build/`. Only `*.ino.hex` and `*.ino.eep` are committed (see `sketches/.gitignore`). **Both must be flashed**: with `ENABLE_OPT_DATAMEM 1`, `DATAMEM` data lives in EEPROM.

Flash size is the main constraint. Each sketch's `.ino` records a "Reference compilation output" (e.g. hp35 with TOTP: 8108 bytes / 98% flash, 226 bytes RAM; hp21: 6480 bytes / 79%, 238 bytes RAM); update it when a change materially affects size, and check size impact of any change. EEPROM (`DATAMEM`) is tight too: hp35 uses all 512 bytes (126 of them the TOTP keys area), hp21 459 of 512. The IDE does not report EEPROM size or overflow: check the data size of `*.ino.eep` after any change to `DATAMEM` data.

## Architecture

**Library (`src/TinyRPNCalculator.h`)** is header-only. It includes every module, each a namespace in `src/TinyRPNCalculator/`: `MEM` (PROGMEM/EEPROM data access), `BCD`, `ADC`, `I2C`, `LCD` (SSD1306), `TXT` (text rendering with custom fonts), `RTC` (DS3231M), `KBD` (analog keyboard decoding via resistor ladder), `PWR` (sleep/power-down, battery level), `TMR` (WDT-based frame timer, `Millis`, `Sync()`), `FNT` (fonts). `SYS::Init()`/`SYS::Down()` tie them together. Use the project types `u08/u16/u32/s08/s16/s32/w16` defined there (plain `bool` for booleans).

Compile-time options, `#define`d in the sketch before including the library:
- `ENABLE_OPT_DATAMEM` — `DATAMEM` maps to `EEMEM` (read via `MEM::DataRead/DataCopy`) instead of `PROGMEM`, freeing flash. Small constant tables (key tokens, field bounds, menus) are declared `DATAMEM`; large ones (ROM) stay `PROGMEM`.
- `ENABLE_OPT_NOINLINE` — enables the `NOINLINE` attribute used to trade speed for flash.

The library `#undef`s Arduino's `ADC`, `F`, `FPSTR` and redefines `F()`; it also provides `D()`/`DSTR` for strings in `DATAMEM`.

**HP virtual machine (`src/Extras/`)** — all in `namespace HPVM`, split into two headers that must be included in order:
1. A *model* header (`HP35Model.h`, `HP21Model.h`): op enum (`Op*`, shifted ops = `FlSHFT + base op`), `key_tokens` (op → HP keycode), model constants (idle breakpoint, status bits) and the original ROM packed as `rom_l` (low 2 bits, 4 instructions per byte) + `rom_h` (high 8 bits).
2. A *CPU* header: `HPVMClassic.h` (HP-35/45/55/80 family CPU) or `HPVMWoodstock.h` (HP-21/22/25 family ACT CPU). Public interface: `Operation(op)`, `OpAndWait(op)`, `Idling()`, `Cycle()` (returns true when the display changed). Registers are 14-digit BCD arrays accessible directly (e.g. the sketch reads `HPVM::C` = X register): Classic has `A..F, M` (`D..F` = Y..T) and `Display[15]` of character codes; Woodstock has `A, B, C, Y, Z, T, M1, M2`, the `F1`/`F2` flag inputs (HP-21: battery, DEG/RAD) and `Display[12]` of 7-segment patterns (`TXT::SEG_*`, dot in bit 7) produced by the model's `char_gen`.

Other extras: `BuildTime.h` (compile time for RTC init), `SHA1.h` (SHA-1 and HMAC-SHA1, sized for AVR: keep the object on the stack) and `TOTP.h` (`TOTP::GetCode(key, len, unixTime / TOTP::PERIOD)`, 6 digits); about 1.3 KB of flash together.

**Sketch (`sketches/hp35/`)**: `hp35.ino` sets options and includes library + model + VM, then `data.h` (layout defines, fonts generated by `lcd2hex` from `data/*.lcd`, menus), `totp.h` (TOTP keys area in `DATAMEM`, generated from `data/totp.txt`; the committed one holds only RFC 6238 test keys, never real secrets) and `code.h` (all logic). `code.h` defines its own `int main()` (no Arduino `setup/loop`): a frame loop at `CALC_FRAMES_PER_SEC` synced by `TMR::Sync()`, switching between RTC (watch) mode and calculator mode. Calculator mode runs `HPVM_CYCLES_PER_FRAME` VM cycles per frame, maps the 16 physical keys plus a FUNC key/menus to HP ops, implements extra functions by scripting sequences of `HPVM::OpAndWait`, and dims the display and powers down after timeouts.

Adding a new model means: a model header in `Extras/`, the matching CPU header, and a new sketch folder modeled on `hp35` (`hp21` is the Woodstock example).

For the HP-21 / Woodstock CPU work, read `documents/firmware/hp21-woodstock-architecture.md` first (written in Russian). It is the reference for registers, status bits, instruction encoding, keyboard/idle loop, display, differences from the HP-35 Classic CPU, and the facts confirmed by running `HPVMWoodstock.h` on the ROM (section 11). It is based on Nonpareil sources, and its "unverified" section lists the open questions.
