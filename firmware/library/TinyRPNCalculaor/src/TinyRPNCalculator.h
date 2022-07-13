#pragma once

// -----------------------------------------------------------------------------
// PCB Rev 1.x
// Fuses: High - 0xD7, Low - 0xF1, Ext - 0xFF 
// -----------------------------------------------------------------------------

// Supported hardware and software modules:
// BCD - Binary-Coded Decimals conversion
// ADC - Analog to Digital Converter reading
// I2C - I2C Bus devices reading/writing
// LCD - SSD1306 128x32 Display control
// TXT - Display text using custom fonts
// RTC - DS3231M Real Time Clock control
// KBD - One Pin Analog 16-Key Keyboard reading
// PWR - MPU Power management (Idle + Power down)
// FPS - Frames per Second sync for main loop update

// -----------------------------------------------------------------------------
// Includes and Helping Defines
// -----------------------------------------------------------------------------

// check for MCU type, clock source and frequency
#if !defined(__AVR_ATtiny85__)
#error "Microcontroller not supported!"
#endif
#if (F_CPU != 16000000UL) || (CLOCK_SOURCE != 6)
#error "Clock source/frequency not supported!"
#endif

// includes
#include <avr/pgmspace.h>  // reading data from PROGMEM
#include <avr/eeprom.h>    // reading/writing data to EEPROM
#include <avr/interrupt.h> // mcu interrupts
#include <avr/power.h>     // mcu power management
#include <avr/sleep.h>     // mcu sleeping
#include <avr/wdt.h>       // mcu watch dog timer

// modules implemented in asm
#define BCD_ASM_IMPL

// data types definition
using b08 = bool;
using u08 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using s08 = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using w16 = union { u16 val; struct { u08 lsb, msb; }; };

// bits manipulations
#define set_bit(sfr, bit) (  (sfr) |=  _BV(bit) )
#define clr_bit(sfr, bit) (  (sfr) &= ~_BV(bit) )
#define isb_set(sfr, bit) (  (sfr) &   _BV(bit) )
#define isb_clr(sfr, bit) (!((sfr) &   _BV(bit)))

// necessary undefs
#undef NOINLINE
#undef ADC
#undef FPSTR
#undef F

// support for strings in PROGMEM
class __FlashStringHelper;
#define FPSTR(pstr_pointer) (reinterpret_cast<const __FlashStringHelper *>(pstr_pointer))
#define F(string_literal) (FPSTR(PSTR(string_literal)))

// additional defines
#if ENABLE_OPT_NOINLINE
#define NOINLINE __attribute__ ((noinline))
#else
#define NOINLINE
#endif

// -----------------------------------------------------------------------------
// Implementations for Modules
// -----------------------------------------------------------------------------

#include "TinyRPNCalculator/BCD.h"
#include "TinyRPNCalculator/ADC.h"
#include "TinyRPNCalculator/I2C.h"
#include "TinyRPNCalculator/LCD.h"
#include "TinyRPNCalculator/TXT.h"
#include "TinyRPNCalculator/RTC.h"
#include "TinyRPNCalculator/KBD.h"
#include "TinyRPNCalculator/PWR.h"
#include "TinyRPNCalculator/FPS.h"

// -----------------------------------------------------------------------------
// PCB Hardware Initialization
// -----------------------------------------------------------------------------

// initialize hardware before calling 'main' function
void __hardware_init() __attribute__ ((naked, used, section(".init8")));
void __hardware_init()
{
	ADC::Init();
	LCD::Init();
	KBD::Init();
	sei();
}