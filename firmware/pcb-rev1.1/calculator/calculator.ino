#undef  F_CPU
#define F_CPU 16000000UL // 16 MHz

#include <avr/eeprom.h>  // Needed for saving data to EEPROM
#include <avr/power.h>   // Needed for power management
#include <avr/sleep.h>   // Needed for sleeping
#include <util/delay.h>  // Needed for delays
#include <math.h>        // Needed for some math constants

#undef  FPSTR
#undef  F
class __FlashStringHelper;
#define FPSTR(pstr_pointer) (reinterpret_cast<const __FlashStringHelper *>(pstr_pointer))
#define F(string_literal) (FPSTR(PSTR(string_literal)))

#define set_bit(sfr, bit) ((sfr) |= _BV(bit))
#define clr_bit(sfr, bit) ((sfr) &= ~_BV(bit))
#define isb_set(sfr, bit) ((sfr) & _BV(bit))
#define isb_clr(sfr, bit) (!((sfr) & _BV(bit)))

#include "hw-layer.h"
#include "sw-layer.h"
#include "calculator.h"
