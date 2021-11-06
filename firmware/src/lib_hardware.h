#pragma once

// -----------------------------------------------------------------------------
// PCB Rev 1.x
// Fuses: High - 0xD7, Low - 0xF1, Ext - 0xFF 
// -----------------------------------------------------------------------------

// Supported hardware and software modules:
// BCD - Binary-Coded Decimals conversion
// WDT - Watch Dog Timer configuration
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
#define set_bit(sfr, bit) ((sfr) |= _BV(bit))
#define clr_bit(sfr, bit) ((sfr) &= ~_BV(bit))
#define isb_set(sfr, bit) ((sfr) & _BV(bit))
#define isb_clr(sfr, bit) (!((sfr) & _BV(bit)))

// necessary undefs
#undef NOINLINE
#undef ADC
#undef FPSTR
#undef F

// support for strings in PROGMEM
class __FlashStringHelper;
#define FPSTR(pstr_pointer) (reinterpret_cast<const __FlashStringHelper *>(pstr_pointer))
#define F(string_literal) (FPSTR(PSTR(string_literal)))

// figure out build date and time (Example __DATE__ : "Jul 27 2012" and __TIME__ : "21:06:19")
#define COMPUTE_BUILD_YEAR ((__DATE__[9] - '0') *   10 + (__DATE__[10] - '0'))
#define COMPUTE_BUILD_DAY  (((__DATE__[4] >= '0') ? (__DATE__[4] - '0') * 10 : 0) + (__DATE__[5] - '0'))
#define BUILD_MONTH_IS_JAN (__DATE__[0] == 'J' && __DATE__[1] == 'a' && __DATE__[2] == 'n')
#define BUILD_MONTH_IS_FEB (__DATE__[0] == 'F')
#define BUILD_MONTH_IS_MAR (__DATE__[0] == 'M' && __DATE__[1] == 'a' && __DATE__[2] == 'r')
#define BUILD_MONTH_IS_APR (__DATE__[0] == 'A' && __DATE__[1] == 'p')
#define BUILD_MONTH_IS_MAY (__DATE__[0] == 'M' && __DATE__[1] == 'a' && __DATE__[2] == 'y')
#define BUILD_MONTH_IS_JUN (__DATE__[0] == 'J' && __DATE__[1] == 'u' && __DATE__[2] == 'n')
#define BUILD_MONTH_IS_JUL (__DATE__[0] == 'J' && __DATE__[1] == 'u' && __DATE__[2] == 'l')
#define BUILD_MONTH_IS_AUG (__DATE__[0] == 'A' && __DATE__[1] == 'u')
#define BUILD_MONTH_IS_SEP (__DATE__[0] == 'S')
#define BUILD_MONTH_IS_OCT (__DATE__[0] == 'O')
#define BUILD_MONTH_IS_NOV (__DATE__[0] == 'N')
#define BUILD_MONTH_IS_DEC (__DATE__[0] == 'D')
#define COMPUTE_BUILD_MONTH \
		( \
			(BUILD_MONTH_IS_JAN) ?  1 : \
			(BUILD_MONTH_IS_FEB) ?  2 : \
			(BUILD_MONTH_IS_MAR) ?  3 : \
			(BUILD_MONTH_IS_APR) ?  4 : \
			(BUILD_MONTH_IS_MAY) ?  5 : \
			(BUILD_MONTH_IS_JUN) ?  6 : \
			(BUILD_MONTH_IS_JUL) ?  7 : \
			(BUILD_MONTH_IS_AUG) ?  8 : \
			(BUILD_MONTH_IS_SEP) ?  9 : \
			(BUILD_MONTH_IS_OCT) ? 10 : \
			(BUILD_MONTH_IS_NOV) ? 11 : \
			(BUILD_MONTH_IS_DEC) ? 12 : \
			/* error default */  99 \
		)
#define COMPUTE_BUILD_HOUR ((__TIME__[0] - '0') * 10 + __TIME__[1] - '0')
#define COMPUTE_BUILD_MIN  ((__TIME__[3] - '0') * 10 + __TIME__[4] - '0')
#define COMPUTE_BUILD_SEC  ((__TIME__[6] - '0') * 10 + __TIME__[7] - '0')

#define BUILD_DATE_IS_BAD (__DATE__[0] == '?')
#define BUILD_YEAR        ((BUILD_DATE_IS_BAD) ? 0 : COMPUTE_BUILD_YEAR)
#define BUILD_MONTH       ((BUILD_DATE_IS_BAD) ? 1 : COMPUTE_BUILD_MONTH)
#define BUILD_DAY         ((BUILD_DATE_IS_BAD) ? 1 : COMPUTE_BUILD_DAY)

#define BUILD_TIME_IS_BAD (__TIME__[0] == '?')
#define BUILD_HOUR        ((BUILD_TIME_IS_BAD) ? 99 :  COMPUTE_BUILD_HOUR)
#define BUILD_MIN         ((BUILD_TIME_IS_BAD) ? 99 :  COMPUTE_BUILD_MIN)
#define BUILD_SEC         ((BUILD_TIME_IS_BAD) ? 99 :  COMPUTE_BUILD_SEC)

// additional defines
#if ENABLE_OPT_NOINLINE
#define NOINLINE __attribute__ ((noinline))
#else
#define NOINLINE
#endif

// -----------------------------------------------------------------------------
// Packed Binary-Coded Decimals
// -----------------------------------------------------------------------------

namespace BCD
{
	u08 Decode(u08 data)
	{
		return (data / 16 * 10) + (data % 16);
	}

	u08 Encode(u08 data)
	{
		return (data / 10 * 16) + (data % 10);
	}
}

// -----------------------------------------------------------------------------
// Watch Dog Timer
// -----------------------------------------------------------------------------

namespace WDT
{
	enum
	{
		MODE_DISABLED = 0x00, // disabled
		MODE_RES      = 0x08, // to reset the CPU if there is a timeout
		MODE_INT      = 0x40, // timeout will cause an interrupt
		MODE_INT_RES  = 0x48, // first time-out interrupt, the second time out - reset
	};

	enum
	{
		TIMEOUT_16MS  = 0x00, // (16 ± 1.6) ms
		TIMEOUT_32MS  = 0x01, // (32 ± 3.2) ms
		TIMEOUT_64MS  = 0x02, // (64 ± 6.4) ms
		TIMEOUT_125MS = 0x03, // (128 ± 12.8) ms
		TIMEOUT_250MS = 0x04, // (256 ± 25.6) ms
		TIMEOUT_500MS = 0x05, // (512 ± 51.2) ms
		TIMEOUT_1S    = 0x06, // (1024 ± 102.4) ms
		TIMEOUT_2S    = 0x07, // (2048 ± 204.8) ms
		TIMEOUT_4S    = 0x08, // (4096 ± 409.6) ms
		TIMEOUT_8S    = 0x09, // (8192 ± 819.2) ms
	};

	void Init(u08 mode, u08 prescaler)
	{
		// does not change global interrupts enable flag
		u08 wdtr = mode | ((prescaler > 7) ? 0x20 | (prescaler - 8) : prescaler);
		u08 sreg = SREG;
		cli();
		WDTCR = _BV(WDCE) | _BV(WDE);
		WDTCR = wdtr;
		SREG  = sreg;
	}
}

// -----------------------------------------------------------------------------
// Analog to Digital Converter (10 bit)
// -----------------------------------------------------------------------------

namespace ADC
{
	enum
	{
		A0_PB5 = 0b0000,
		A1_PB2 = 0b0001,
		A2_PB4 = 0b0010,
		A3_PB3 = 0b0011,
		VCC    = 0b1100,
		TEMP   = 0b1111 | _BV(REFS1),
	};

	void Init()
	{
		// enable adc, enable interrupt, clear interrupt flag, prescaler is 32
		ADCSRA = _BV(ADEN) | _BV(ADIE) | _BV(ADIF) | _BV(ADPS2) | _BV(ADPS0);
	}

	NOINLINE w16 Read(u08 channel)
	{
		ADMUX = channel;
		set_sleep_mode(SLEEP_MODE_ADC);
		do
		{
			sleep_enable();
			sleep_cpu ();
			sleep_disable ();
		}
		while (isb_set(ADCSRA, ADSC));

		w16 adc;
		adc.lsb = ADCL;
		adc.msb = ADCH;
		return adc;
	}

	EMPTY_INTERRUPT(ADC_vect);
}

// -----------------------------------------------------------------------------
// I2C Bus
// -----------------------------------------------------------------------------

namespace I2C
{
	#define I2C_DDR DDRB
	#define I2C_PIN PINB
	#define I2C_SDA PB0
	#define I2C_SCL PB2

	#define I2C_SDA_H() clr_bit(I2C_DDR, I2C_SDA)
	#define I2C_SDA_L() set_bit(I2C_DDR, I2C_SDA)
	#define I2C_SDA_I() isb_set(I2C_PIN, I2C_SDA)

	#define I2C_SCL_H() clr_bit(I2C_DDR, I2C_SCL)
	#define I2C_SCL_L() set_bit(I2C_DDR, I2C_SCL)
	#define I2C_SCL_I() isb_set(I2C_PIN, I2C_SCL)

	#define I2C_DELAY() __asm__ __volatile__ ("nop\n\t")

	void scl_h_wait()
	{
		I2C_SCL_H();
		while (!I2C_SCL_I());
	}

	u08 read_write(u08 data)
	{
		for (u08 sda_i, i = 8; i > 0; --i)
		{
			I2C_SDA_H();
			if (!(data & 0x80)) I2C_SDA_L();
			scl_h_wait();

			sda_i = I2C_SDA_I();
			I2C_SCL_L();

			data <<= 1;
			if (sda_i) data |= 0x01;
		}
		I2C_SDA_H();
		return data;
	}

	void start()
	{
		scl_h_wait();
		I2C_SDA_L();
		I2C_DELAY();
		I2C_SCL_L();
	}

	NOINLINE b08 Write(u08 data)
	{
		read_write(data);
		scl_h_wait();

		b08 ack = true;
		if (I2C_SDA_I()) ack = false;

		I2C_SCL_L();
		return ack;
	}

	NOINLINE u08 Read(b08 ack)
	{
		u08 data = read_write(0xFF);
		if (ack) I2C_SDA_L();
		scl_h_wait();
		I2C_DELAY();

		I2C_SCL_L();
		return data;
	}

	u08 ReadAck()
	{ 
		return Read(true);
	}

	u08 ReadNack()
	{
		return Read(false);
	}

	b08 StartWrite(u08 addr)
	{
		start();
		return Write(addr << 1);
	}

	b08 StartRead(u08 addr)
	{
		start();
		return Write(addr << 1 | 1);
	}

	NOINLINE void Stop()
	{
		I2C_SDA_L();
		scl_h_wait();
		I2C_DELAY();
		I2C_SDA_H();
	}
}

// -----------------------------------------------------------------------------
// SSD1306 128x32 Display on I2C Bus
// -----------------------------------------------------------------------------

namespace LCD
{
	const u08 I2C_ADDR = 0x3C;

	const u08 WIDTH = 128;
	const u08 PAGES = 4;

	u08 draw_buf = 0xB4;
	u08 rend_buf = 0x40;

	const u08 init_data[] PROGMEM =
	{
		0xC8,       // set scan direction (C0 scan from COM0 to COM[N-1] or C8 mirroring)
		0xA1,       // set segment remap (A0 regular or A1 flip)
		0xA8, 0x1F, // set mux ratio (N+1) where: 14<N<64 ... 3F or 1F
		0xDA, 0x02, // COM config pin mapping:
					//                  right/left left/right
					//      sequential      02        22
					//      alternate       12        32
		0x20, 0x00, // horizontal addressing mode (line feed after EOL)
		0x8D, 0x14  // charge pump (0x14 enable or 0x10 disable)
	};

	void start_command()
	{ 
		I2C::StartWrite(I2C_ADDR);
		I2C::Write(0x00);
	}

	void start_data()
	{ 
		I2C::StartWrite(I2C_ADDR);
		I2C::Write(0x40);
	}

	NOINLINE void command(u08 cmd)
	{ 
		start_command();
		I2C::Write(cmd);
		I2C::Stop();
	}

	void Init()
	{
		start_command();
		for (u08 i = 0; i < sizeof(init_data); i++)
		{
			I2C::Write(pgm_read_byte(&init_data[i]));
		}
		I2C::Stop();
	}

	void TurnOn()
	{
		command(0xAF);
	}

	void TurnOff()
	{ 
		command(0xAE);
	}

	void Brightness(u08 brightness)
	{ 
		start_command();
		I2C::Write(0x81);
		I2C::Write(brightness);
		I2C::Stop();
	}

	void Position(u08 x, u08 y)
	{ 
		start_command();
		I2C::Write(draw_buf | (y & 0x07));
		I2C::Write(0x10 | (x >> 4));
		I2C::Write(x & 0x0F);
		I2C::Stop();
	}

	NOINLINE void Write(u08 b, u08 s)
	{
		start_data();
		while (s--) I2C::Write(b);
		I2C::Stop();
	}

	void Clear()
	{
		Position(0, 0);
		for (u08 i = PAGES; i > 0; --i)
		{
			Write(0x00, WIDTH);
		}
	}

	void Flip()
	{
		rend_buf ^= 0x20;
		draw_buf ^= 0x04;
		command(rend_buf);
	}
}

// -----------------------------------------------------------------------------
// Text output
// -----------------------------------------------------------------------------

namespace TXT
{
	enum { x1 = 1, x2 = 2, x4 = 4 };

	enum
	{
		SEG_SPACE = 0x00,
		SEG_DOT   = 0x80,
		SEG_DASH  = 0x40,

		SEG_0 = 0x3F, SEG_1 = 0x06, SEG_2 = 0x5B, SEG_3 = 0x4F, SEG_4 = 0x66,
		SEG_5 = 0x6D, SEG_6 = 0x7D, SEG_7 = 0x07, SEG_8 = 0x7F, SEG_9 = 0x6F,
		
		SEG_A = 0x77, SEG_b = 0x7C, SEG_C = 0x39,
		SEG_d = 0x5E, SEG_E = 0x79, SEG_F = 0x71,
	};

	const u08 num_seg[] PROGMEM =
	{
		SEG_0, SEG_1, SEG_2, SEG_3, SEG_4,
		SEG_5, SEG_6, SEG_7, SEG_8, SEG_9,
		SEG_A, SEG_b, SEG_C,
		SEG_d, SEG_E, SEG_F
	};

	struct Font
	{
		u08 bytesInRow;
		u08 rowsOfBytes;
		u08 asciiFirst;
		u08 asciiLast;
		const u08 * bytes;
	};

	Font font;
	u08  font_sx;
	u08  font_sy;
	u08  char_dx;
	u08  inverse;

	u08 expand4bit(u08 b)
	{
		// 0000abcd -> aabbccdd
		b = (b | (b << 2)) & 0x33;
		b = (b | (b << 1)) & 0x55;
		return b | (b << 1);
	}

	u08 expand2bit(u08 b)
	{
		// 000000ab -> aaaabbbb
		b = (b | (b << 3)) & 0x11;
		for (u08 i = 3; i > 0; --i) b |= (b << 1);
		return b;
	}

	void SetInverse(b08 enable)
	{
		inverse = (enable ? 0xFF : 0x00);
	}

	void SetScale(u08 sx, u08 sy)
	{
		font_sx = sx;
		font_sy = sy;
		char_dx = font.bytesInRow * font_sx + 1;
	}

	void SetFont(const Font& f)
	{
		memcpy_P(&font, &f, sizeof(Font));
		SetScale(x1, x1);
	}
	
	void PrintChar(u08 ch, u08 x, u08 y)
	{
		// compute pointer to char data
		u16 dp = 0;
		if (ch >= font.asciiFirst && ch <= font.asciiLast)
		{
			dp = &font.bytes[(ch - font.asciiFirst) * font.bytesInRow * font.rowsOfBytes];
		}

		// iterate through screen pages and char data rows
		for (u08 yi = font.rowsOfBytes; yi > 0; --yi)
		{
			for (u08 si = 0; si < font_sy; ++si)
			{
				LCD::Position(x, y + si);

				// iterate through screen columns and char data bytes
				for (u08 xi = 0, ch = inverse; xi < font.bytesInRow; ++xi)
				{
					// get char data if available
					if (dp)
					{
						ch = pgm_read_byte(dp + xi) ^ inverse;
						if (font_sy == x2)
							ch = expand4bit((ch >> (si << 2)) & 0x0F);
						else if (font_sy == x4)
							ch = expand2bit((ch >> (si << 1)) & 0x03);
					}
					LCD::Write(ch, font_sx);
				}

				// draw spacing
				LCD::Write(inverse, 1);
			}

			// next row of bytes in char data
			if (dp) dp += font.bytesInRow;
			y += font_sy;
		}
	}

	void PrintString(const u08* s, u08 x, u08 y)
	{
		while (u08 c = *s++)
		{
			PrintChar(c, x, y);
			x += char_dx;
		}
	}

	void PrintString(const __FlashStringHelper* s, u08 x, u08 y)
	{
		const u08 * p = (const u08 *)s;
		while (u08 c = pgm_read_byte(p++))
		{
			PrintChar(c, x, y);
			x += char_dx;
		}
	}

	void PrintString(const __FlashStringHelper* s, u08 i, u08 x, u08 y)
	{
		const u08 * p = (const u08 *)s;
		u08 w = pgm_read_byte(p++);

		for (p += (i * w); w > 0; --w)
		{
			PrintChar(pgm_read_byte(p++), x, y);
			x += char_dx;
		}
	}

	void PrintBCD(u08 n, u08 x, u08 y)
	{
		PrintChar('0' + (n >> 4), x, y);
		PrintChar('0' + (n & 15), x + char_dx, y);
	}

	NOINLINE u08 NumToSeg(u08 n)
	{
		return pgm_read_byte(num_seg + n);
	}

	void PrintSeg(u08 seg, u08 x, u08 y)
	{
		u08 ds = (font.bytesInRow * font.rowsOfBytes);
		for (u08 yi = 0; yi < font.rowsOfBytes; ++yi)
		{
			LCD::Position(x, y + yi);

			for (u08 xi = 0; xi < font.bytesInRow; ++xi)
			{
				u16 dp = &font.bytes[yi * font.bytesInRow + xi];
				u08 db = 0x00;

				for (u08 ss = seg; ss != 0; ss >>= 1)
				{
					if (ss & 0x01) db |= pgm_read_byte(dp);
					dp += ds;
				}

				LCD::Write(db, 1);
			}
		}
	}

	void PrintSegBCD(u08 n, u08 x, u08 y)
	{
		PrintSeg(NumToSeg(n >> 4), x, y);
		PrintSeg(NumToSeg(n & 15), x + char_dx, y);
	}
}

// -----------------------------------------------------------------------------
// DS3231M Real Time Clock on I2C Bus
// -----------------------------------------------------------------------------

// Time always stored in 24-hour format!
// Day of the week is not used!
// Century flag is not supported!
// Alarms are not supported!

namespace RTC
{
	const u08 I2C_ADDR = 0x68;

	enum
	{
		REG_SECONDS      = 0x00,
		REG_MINUTES      = 0x01,
		REG_HOURS        = 0x02,
		REG_DAY          = 0x03,
		REG_DATE         = 0x04,
		REG_MONTH        = 0x05,
		REG_YEAR         = 0x06,
		REG_A1_SECONDS   = 0x07,
		REG_A1_MINUTES   = 0x08,
		REG_A1_HOUR      = 0x09,
		REG_A1_DAY_DATE  = 0x0A,
		REG_A2_MINUTES   = 0x0B,
		REG_A2_HOUR      = 0x0C,
		REG_A2_DAY_DATE  = 0x0D,
		REG_CONTROL      = 0x0E,
		REG_STATUS       = 0x0F,
		REG_AGING_OFFSET = 0x10,
		REG_TEMP_MSB     = 0x11,
		REG_TEMP_LSB     = 0x12,
	};

	u08 Seconds = BCD::Encode(BUILD_SEC);   // 0 - 59
	u08 Minutes = BCD::Encode(BUILD_MIN);   // 0 - 59
	u08 Hours   = BCD::Encode(BUILD_HOUR);  // 0 - 23
	u08 Date    = BCD::Encode(BUILD_DAY);   // 1 - 31
	u08 Month   = BCD::Encode(BUILD_MONTH); // 1 - 12
	u08 Year    = BCD::Encode(BUILD_YEAR);  // 0 - 99

	void ReadTimeDate()
	{
		if (I2C::StartWrite(I2C_ADDR))
		{
			I2C::Write(REG_SECONDS);
			I2C::StartRead(I2C_ADDR);
			Seconds = I2C::ReadAck();
			Minutes = I2C::ReadAck();
			Hours   = I2C::ReadAck() & 0x3F;
			I2C::ReadAck();
			Date    = I2C::ReadAck();
			Month   = I2C::ReadAck() & 0x1F;
			Year    = I2C::ReadNack();
			I2C::Stop();
		}
	}

	void WriteTimeDate()
	{
		if (I2C::StartWrite(I2C_ADDR))
		{
			I2C::Write(REG_SECONDS);
			I2C::Write(Seconds);
			I2C::Write(Minutes);
			I2C::Write(Hours);
			I2C::Write(1);
			I2C::Write(Date);
			I2C::Write(Month);
			I2C::Write(Year);
			I2C::Stop();
		}
	}

	// MSB degrees, LSB fractional
	w16 ReadTemperature()
	{
		w16 temp;
		if (I2C::StartWrite(I2C_ADDR))
		{
			I2C::Write(REG_TEMP_MSB);
			I2C::StartRead(I2C_ADDR);
			temp.msb = I2C::ReadAck();
			temp.lsb = I2C::ReadNack();
			I2C::Stop();
		}
		return temp;
	}
}

// -----------------------------------------------------------------------------
// One Pin Analog 16-Key Keyboard
// -----------------------------------------------------------------------------

namespace KBD
{
#if PCB_REV == 10
	#define KBD_PIN PB3
	#define KBD_ADC ADC::A3_PB3
#else
	#define KBD_PIN PB4
	#define KBD_ADC ADC::A2_PB4
#endif

	enum
	{
		NONE = 0xFF,

		FUNC = 0x0F, NUM7 = 0x07, NUM8 = 0x08, NUM9 = 0x09,
		EEX  = 0x0E, NUM4 = 0x04, NUM5 = 0x05, NUM6 = 0x06,
		CHS  = 0x0D, NUM1 = 0x01, NUM2 = 0x02, NUM3 = 0x03,
		CLR  = 0x0C, NUM0 = 0x00, DOT  = 0x0A, DUP  = 0x0B,

		ROTU = EEX,  ROTD = CHS,
		SEL1 = NUM1, SEL2 = NUM2, SEL3 = NUM3,
		MATH = FUNC, TRIG = NUM7, PROG = NUM8,
		CONS = NUM4, SETS = DOT
	};

	const u16 adc_lut[] PROGMEM =
	{
		147, 182, 221, 269, // A0 B0 C0 D0
		324, 383, 442, 505, // A1 B1 C1 D1
		573, 635, 692, 762, // A2 B2 C2 D2
		827, 863, 893, 913  // A3 B3 C3 D3
	};

	const u08 code_lut[] PROGMEM = 
	{
		FUNC, NUM7, NUM8, NUM9, // A0 B0 C0 D0
		EEX,  NUM4, NUM5, NUM6, // A1 B1 C1 D1
		CHS,  NUM1, NUM2, NUM3, // A2 B2 C2 D2
		CLR,  NUM0, DOT,  DUP   // A3 B3 C3 D3
	};

	u08 key;

	NOINLINE u08 read_raw_key()
	{
		w16 adc = ADC::Read(KBD_ADC);
		if (adc.val > 110)
		{
			for (u08 i = 0; i < 16; ++i)
			{
				u16 adcMax = pgm_read_word(&adc_lut[i]);
				if (adc.val < adcMax) return pgm_read_byte(&code_lut[i]);
			}
		}
		return NONE;
	}

	void Init()
	{
		clr_bit(DDRB,  KBD_PIN); // chose pin as input
		clr_bit(PORTB, KBD_PIN); // disable pull-up resistor
		set_bit(PCMSK, KBD_PIN); // chose pin as interrupt source
		set_bit(GIMSK, PCIE);    // enable pin change interruptions
		set_bit(GIFR,  PCIF);    // clear the interruption flag
		key = NONE;
	}

	u08 Read()
	{
		u08 raw = read_raw_key();
		if (raw == NONE && key != NONE) key = NONE;
		if (raw != NONE && key == NONE) key = raw;
		return key;
	}

	// just interrupt sleeping
	EMPTY_INTERRUPT(PCINT0_vect);
}

// -----------------------------------------------------------------------------
// Power Management
// -----------------------------------------------------------------------------

namespace PWR
{
	const u16 BAT_FULL  = 4100; // mV
	const u16 BAT_EMPTY = 3500; // mV

	void saving(u08 mode)
	{
		clr_bit(ADCSRA, ADEN);
		power_all_disable();
		set_sleep_mode(mode);
		sleep_enable();
		sleep_cpu();
		sleep_disable();
		power_adc_enable();
		set_bit(ADCSRA, ADEN);
	}

	NOINLINE u16 Voltage()
	{
		return (1125300L / ADC::Read(ADC::VCC).val);
	}

	u08 Level()
	{
		u16 voltage = Voltage();
		if (voltage <= BAT_EMPTY) return 0;
		if (voltage >= BAT_FULL ) return 100;
		return ((voltage - BAT_EMPTY) / ((BAT_FULL - BAT_EMPTY) / 100));
	}

	void Idle()
	{
		saving(SLEEP_MODE_IDLE);
	}

	void Down()
	{
		do saving(SLEEP_MODE_PWR_DOWN);
		while (Voltage() <= BAT_EMPTY);
	}
}

// -----------------------------------------------------------------------------
// Frames per Second Synchronization
// -----------------------------------------------------------------------------

namespace FPS
{
	enum
	{
		TIMEOUT_15_FPS = WDT::TIMEOUT_64MS,
		TIMEOUT_30_FPS = WDT::TIMEOUT_32MS,
		TIMEOUT_60_FPS = WDT::TIMEOUT_16MS,
	};

	u08 timeout;
	volatile b08 waiting;
	volatile u16 counter;

	NOINLINE void SyncStart(u08 t)
	{
		WDT::Init(WDT::MODE_INT, t);
		timeout = (16 << t);
		counter = 0;
	}

	void SyncStop()
	{
		WDT::Init(WDT::MODE_DISABLED, 0);
	}

	void SyncWait()
	{
		waiting = true;
		while (waiting) PWR::Idle();
	}

	u16 SyncMillis()
	{
		return (counter * timeout);
	}

	ISR(WDT_vect)
	{
		waiting = false;
		counter++;
	}
}

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
