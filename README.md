# Business Card RPN Calculator with Watch

<a href="documents/manuals/hp35-rev1.5-user-manual.md#3-device-description"><img src="documents/manuals/images/hp35-device.svg" alt="Drawing of the device: display (1), keyboard (2), USB-C connector (3), battery (4), microcontroller (5), real-time clock (6), charger (7)"></a>

A scientific RPN calculator with a watch, on one PCB of the size of a credit card (85.5 × 54 mm).

The calculator is an emulator of a real HP calculator of the 1970s. The firmware runs the original HP ROM on a virtual copy of the HP CPU. Thus, the calculations, the results and the display format are the same as on the original calculator. The firmware also adds functions that the original calculator did not have, for example the watch.

## Hardware

All the parts are on one PCB:

- [ATtiny85](datasheets/ATtiny85.pdf) microcontroller. It has only 8 KB of flash memory, but the firmware, the emulator and the original HP ROM fit in it.
- 0.91-inch OLED display, 128 × 32 pixels, with the [SSD1306](datasheets/SSD1306.pdf) controller.
- [DS3231M](datasheets/DS3231M.pdf) real-time clock. It keeps the time and the date when the device is off.
- 16 keys on one analog input of the microcontroller.
- [LIR1620](datasheets/LIR1620.pdf) rechargeable lithium-ion coin battery with an XB5358D0 protection IC.
- [MCP73832](datasheets/MCP73831.pdf) charger. It charges the battery through the USB-C connector.

The device has no ON/OFF switch. The firmware turns the device off when you do not use it. A key in the two bottom rows turns it on again.

For the full circuit, refer to the [schematic](hardware/pcb-rev1.5/Tiny-RPN-Calculator-rev1.5_Schematic.pdf).

## Firmware

Each firmware emulates one HP calculator. The source code of each firmware is an Arduino sketch.

| Firmware | Calculator | Added functions | Status |
| --- | --- | --- | --- |
| [hp35](firmware/sketches/hp35) | HP-35 (1972), the first pocket calculator with scientific functions | Watch, TOTP codes for two-factor authentication | Ready |
| [hp21](firmware/sketches/hp21) | HP-21 (1975) | Watch | Not tested on the device |

To install the firmware, refer to [Firmware installation](documents/manuals/hp35-rev1.5-user-manual.md#14-firmware-installation) in the User Manual.

## Documentation

| Document | Content |
| --- | --- |
| [User Manual](documents/manuals/hp35-rev1.5-user-manual.md) | How to use the device with the hp35 firmware: calculator, watch, TOTP codes, firmware installation |
| [HP-35 Operating Manual](documents/manuals/hp35-om-en-remake.pdf) | Remake of the original Hewlett-Packard manual: all functions of the HP-35, with examples (PDF, 23 MB) |
| [HP-21 Owner's Handbook](documents/manuals/hp21-oh-en.pdf) | Original Hewlett-Packard manual of the HP-21 (PDF, 16 MB) |

## License

[MIT](LICENSE)
