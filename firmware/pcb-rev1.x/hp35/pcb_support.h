#pragma once

// -----------------------------------------------------------------------------
// PCB Rev 1.1 and newer
// Fuses: High - 0xD7, Low - 0xF1, Ext - 0xFF 
// -----------------------------------------------------------------------------

// Supported hardware and software modules:
// BCD - Binary-Coded Decimals conversion
// WDT - Watch Dog Timer configuration
// ADC - Analog to Digital Converter reading
// I2C - I2C Bus devices reading/writing
// LCD - SSD1306 128x32 Display control
// RTC - DS3231M Real Time Clock control
// KBD - One Pin Analog 16-Key Keyboard reading
// PWR - MPU Power management (Idle + Power down)
// FPS - Frames per Second sync for main loop update

// -----------------------------------------------------------------------------
// Includes and Helping Defines
// -----------------------------------------------------------------------------

#undef  F_CPU
#define F_CPU 16000000UL  // 16 MHz internal

// includes
#include <avr/pgmspace.h> // needed for reading data from PROGMEM
#include <avr/eeprom.h>   // needed for saving data to EEPROM
#include <avr/power.h>    // needed for power management
#include <avr/sleep.h>    // needed for sleeping
#include <util/delay.h>   // needed for delays

// data types definition
typedef bool     b08;
typedef uint8_t  u08;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int8_t   s08;
typedef int16_t  s16;
typedef int32_t  s32;

// bits manipulations
#define set_bit(sfr, bit) ((sfr) |= _BV(bit))
#define clr_bit(sfr, bit) ((sfr) &= ~_BV(bit))
#define isb_set(sfr, bit) ((sfr) & _BV(bit))
#define isb_clr(sfr, bit) (!((sfr) & _BV(bit)))

// support for strings in PROGMEM
#undef  FPSTR
#undef  F
class __FlashStringHelper;
#define FPSTR(pstr_pointer) (reinterpret_cast<const __FlashStringHelper *>(pstr_pointer))
#define F(string_literal) (FPSTR(PSTR(string_literal)))

// figure out build date and time (Example __DATE__ : "Jul 27 2012" and __TIME__ : "21:06:19")
#define COMPUTE_BUILD_YEAR ((__DATE__[ 9] - '0') *   10 + (__DATE__[10] - '0'))
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

// -----------------------------------------------------------------------------
// Packed Binary-Coded Decimals
// -----------------------------------------------------------------------------

NOINLINE u08 BCD_Decode(u08 data)
{
  return (data / 16 * 10) + (data % 16);
}

NOINLINE u08 BCD_Encode(u08 data)
{
  return (data / 10 * 16) + (data % 10);
}

// -----------------------------------------------------------------------------
// Watch Dog Timer
// -----------------------------------------------------------------------------

#define WDT_MODE_DISABLED  0x00 // disabled
#define WDT_MODE_RES       0x08 // to reset the CPU if there is a timeout
#define WDT_MODE_INT       0x40 // timeout will cause an interrupt
#define WDT_MODE_INT_RES   0x48 // first time-out interrupt, the second time out - reset

#define WDT_TIMEOUT_16MS   0x00 // (16 ± 1.6) ms
#define WDT_TIMEOUT_32MS   0x01 // (32 ± 3.2) ms
#define WDT_TIMEOUT_64MS   0x02 // (64 ± 6.4) ms
#define WDT_TIMEOUT_125MS  0x03 // (128 ± 12.8) ms
#define WDT_TIMEOUT_250MS  0x04 // (256 ± 25.6) ms
#define WDT_TIMEOUT_500MS  0x05 // (512 ± 51.2) ms
#define WDT_TIMEOUT_1S     0x06 // (1024 ± 102.4) ms
#define WDT_TIMEOUT_2S     0x07 // (2048 ± 204.8) ms
#define WDT_TIMEOUT_4S     0x08 // (4096 ± 409.6) ms
#define WDT_TIMEOUT_8S     0x09 // (8192 ± 819.2) ms

void WDT_Init(u08 mode, u08 prescaler)
{
	// does not change global interrupts enable flag
	u08 wdtr = mode | ((prescaler > 7) ? 0x20 | (prescaler - 8) : prescaler);
	u08 sreg = SREG;
	cli();
	WDTCR = _BV(WDCE) | _BV(WDE);
	WDTCR = wdtr;
	SREG  = sreg;
}

// -----------------------------------------------------------------------------
// Analog to Digital Converter (10 bit)
// -----------------------------------------------------------------------------

#define ADC_0_PB5 (0b0000)
#define ADC_1_PB2 (0b0001)
#define ADC_2_PB4 (0b0010)
#define ADC_3_PB3 (0b0011)
#define ADC_VCC   (0b1100)
#define ADC_TEMP  (0b1111 | _BV(REFS1))

void ADC_Init()
{
	ADCSRA = _BV(ADEN) | _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);
}

NOINLINE u16 ADC_Read(u08 channel, u08 delay)
{
	ADMUX = channel;
	while (delay--) _delay_ms(1);

	set_bit(ADCSRA, ADSC);
	while (isb_set(ADCSRA, ADSC));

	u08 adcl = ADCL;
	u08 adch = ADCH;
	return (adcl | adch << 8);
}

// -----------------------------------------------------------------------------
// I2C Bus
// -----------------------------------------------------------------------------

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

#define I2C_DELAY() _delay_us(1)

NOINLINE void i2c_scl_h_wait()
{
	I2C_SCL_H();
	while (!I2C_SCL_I());
}

NOINLINE u08 i2c_read_write(u08 data)
{
	for (u08 sda_i, i = 8; i > 0; --i)
	{
		I2C_SDA_H();
		if (!(data & 0x80)) I2C_SDA_L();
		i2c_scl_h_wait();

		sda_i = I2C_SDA_I();
		I2C_SCL_L();

		data <<= 1;
		if (sda_i) data |= 0x01;
	}
	I2C_SDA_H();
	return data;
}

NOINLINE void i2c_start()
{
	i2c_scl_h_wait();
	I2C_SDA_L();
	I2C_DELAY();
	I2C_SCL_L();
}

NOINLINE b08 I2C_Write(u08 data)
{
	i2c_read_write(data);
	i2c_scl_h_wait();

	b08 ack = true;
	if (I2C_SDA_I()) ack = false;

	I2C_SCL_L();
	return ack;
}

NOINLINE u08 I2C_Read(b08 ack)
{
	u08 data = i2c_read_write(0xFF);
	if (ack) I2C_SDA_L();
	i2c_scl_h_wait();
	I2C_DELAY();

	I2C_SCL_L();
	return data;
}

u08 I2C_ReadAck()
{ 
	return I2C_Read(true);
}

u08 I2C_ReadNack()
{
	return I2C_Read(false);
}

b08 I2C_StartWrite(u08 addr)
{
	i2c_start();
	return I2C_Write(addr << 1);
}

b08 I2C_StartRead(u08 addr)
{
	i2c_start();
	return I2C_Write(addr << 1 | 1);
}

NOINLINE void I2C_Stop()
{
	I2C_SDA_L();
	i2c_scl_h_wait();
	I2C_DELAY();
	I2C_SDA_H();
}

// -----------------------------------------------------------------------------
// SSD1306 128x32 Display on I2C Bus
// -----------------------------------------------------------------------------

#define LCD_ADDR  0x3C
#define LCD_COMM  0x00
#define LCD_DATA  0x40
#define LCD_WIDTH 128 
#define LCD_PAGES 4

u08 lcd_draw_buf = 0xB4;
u08 lcd_rend_buf = 0x40;

const u08 lcd_init_data[] PROGMEM =
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

void lcd_start_command()
{ 
	I2C_StartWrite(LCD_ADDR);
	I2C_Write(LCD_COMM);
}

void lcd_start_data()
{ 
	I2C_StartWrite(LCD_ADDR);
	I2C_Write(LCD_DATA);
}

NOINLINE void lcd_command(u08 cmd)
{ 
	lcd_start_command();
	I2C_Write(cmd);
	I2C_Stop();
}

void LCD_Init()
{
	lcd_start_command();
	for (u08 i = 0; i < sizeof(lcd_init_data); i++)
	{
		I2C_Write(pgm_read_byte(&lcd_init_data[i]));
	}
	I2C_Stop();
}

void LCD_TurnOn()
{
	lcd_command(0xAF);
}

void LCD_TurnOff()
{ 
	lcd_command(0xAE);
}

void LCD_Brightness(u08 brightness)
{ 
	lcd_start_command();
	I2C_Write(0x81);
	I2C_Write(brightness);
	I2C_Stop();
}

void LCD_Position(u08 x, u08 y)
{ 
	lcd_start_command();
	I2C_Write(lcd_draw_buf | (y & 0x07));
	I2C_Write(0x10 | (x >> 4));
	I2C_Write(x & 0x0F);
	I2C_Stop();
}

NOINLINE void LCD_Write(u08 b, u08 s)
{
	lcd_start_data();
	while (s--) I2C_Write(b);
	I2C_Stop();
}

void LCD_Clear()
{
	LCD_Position(0, 0);
	for (u08 i = LCD_PAGES; i > 0; --i)
	{
		LCD_Write(0x00, LCD_WIDTH);
	}
}

void LCD_Flip()
{
	lcd_rend_buf ^= 0x20;
	lcd_draw_buf ^= 0x04;
	lcd_command(lcd_rend_buf);
}

// -----------------------------------------------------------------------------
// DS3231M Real Time Clock on I2C Bus
// -----------------------------------------------------------------------------

// Time always stored in 24-hour format!
// Day of the week is not used!
// Century flag is not supported!
// Alarms are not supported!

#define RTC_ADDR         0x68
#define RTC_SECONDS      0x00
#define RTC_MINUTES      0x01
#define RTC_HOURS        0x02
#define RTC_DAY          0x03
#define RTC_DATE         0x04
#define RTC_MONTH        0x05
#define RTC_YEAR         0x06
#define RTC_A1_SECONDS   0x07
#define RTC_A1_MINUTES   0x08
#define RTC_A1_HOUR      0x09
#define RTC_A1_DAY_DATE  0x0A
#define RTC_A2_MINUTES   0x0B
#define RTC_A2_HOUR      0x0C
#define RTC_A2_DAY_DATE  0x0D
#define RTC_CONTROL      0x0E
#define RTC_STATUS       0x0F
#define RTC_AGING_OFFSET 0x10
#define RTC_TEMP_MSB     0x11
#define RTC_TEMP_LSB     0x12

u08 rtc_seconds = BUILD_SEC;   // 0 - 59
u08 rtc_minutes = BUILD_MIN;   // 0 - 59
u08 rtc_hours   = BUILD_HOUR;  // 0 - 23
u08 rtc_date    = BUILD_DAY;   // 1 - 31
u08 rtc_month   = BUILD_MONTH; // 1 - 12
u08 rtc_year    = BUILD_YEAR;  // 0 - 99
s16 rtc_temp;                  // MSB degrees, LSB fractional

void RTC_ReadTimeDate()
{
	if (I2C_StartWrite(RTC_ADDR))
	{
		I2C_Write(RTC_SECONDS);
		I2C_StartRead(RTC_ADDR);
		rtc_seconds = BCD_Decode(I2C_ReadAck());
		rtc_minutes = BCD_Decode(I2C_ReadAck());
		rtc_hours   = BCD_Decode(I2C_ReadAck());
		I2C_ReadAck();
		rtc_date    = BCD_Decode(I2C_ReadAck());
		rtc_month   = BCD_Decode(I2C_ReadAck() & 0x1F);
		rtc_year    = BCD_Decode(I2C_ReadNack() % 100);
		I2C_Stop();
	}
}

void RTC_WriteTimeDate()
{
	if(I2C_StartWrite(RTC_ADDR))
	{
		I2C_Write(RTC_SECONDS);
		I2C_Write(BCD_Encode(rtc_seconds));
		I2C_Write(BCD_Encode(rtc_minutes));
		I2C_Write(BCD_Encode(rtc_hours));
		I2C_Write(1);
		I2C_Write(BCD_Encode(rtc_date));
		I2C_Write(BCD_Encode(rtc_month));
		I2C_Write(BCD_Encode(rtc_year));
		I2C_Stop();
	}
}

void RTC_ReadTemperature()
{
#if !DEBUG_ON_R1_0
	if (I2C_StartWrite(RTC_ADDR))
	{
		I2C_Write(RTC_TEMP_MSB);
		I2C_StartRead(RTC_ADDR);
		u08 msb = I2C_ReadAck();
		u08 lsb = I2C_ReadNack();
		rtc_temp = msb << 8 | lsb;
		I2C_Stop();
	}
#endif
}

// -----------------------------------------------------------------------------
// One Pin Analog 16-Key Keyboard
// -----------------------------------------------------------------------------

#if DEBUG_ON_R1_0
#define KBD_PIN PB3
#define KBD_ADC ADC_3_PB3
#else
#define KBD_PIN PB4
#define KBD_ADC ADC_2_PB4
#endif

// Keyboard layout on PCB:
// A0 B0 C0 D0
// A1 B1 C1 D1
// A2 B2 C2 D2
// A3 B3 C3 D3

#define KBD_NO 0xFF
#define KBD_A0 0x0F // F
#define KBD_B0 0x07 // 7
#define KBD_C0 0x08 // 8
#define KBD_D0 0x09 // 9
#define KBD_A1 0x0E // E
#define KBD_B1 0x04 // 4
#define KBD_C1 0x05 // 5
#define KBD_D1 0x06 // 6
#define KBD_A2 0x0D // S
#define KBD_B2 0x01 // 1
#define KBD_C2 0x02 // 2
#define KBD_D2 0x03 // 3
#define KBD_A3 0x0C // C
#define KBD_B3 0x00 // 0
#define KBD_C3 0x0A // D
#define KBD_D3 0x0B // P

const u16 kbd_adc[] PROGMEM =
{
	147, 182, 221, 269, 324, 383, 442, 505,
	573, 635, 692, 762, 827, 863, 893, 913
};

const u08 kbd_code[] PROGMEM = 
{
	KBD_A0, KBD_B0, KBD_C0, KBD_D0, KBD_A1, KBD_B1, KBD_C1, KBD_D1,
	KBD_A2, KBD_B2, KBD_C2, KBD_D2, KBD_A3, KBD_B3, KBD_C3, KBD_D3
};

void KBD_Init()
{
	clr_bit(DDRB,  KBD_PIN);
	clr_bit(PORTB, KBD_PIN);
	set_bit(PCMSK, KBD_PIN);
	set_bit(GIFR,  PCIF);
	set_bit(GIMSK, PCIE);
}

uint8_t KBD_Read()
{
	u16 adcVal = ADC_Read(KBD_ADC, 1);
	if (adcVal > 110)
	{
		for (u08 i = 0; i < 16; ++i)
		{
			u16 adcMax = pgm_read_word(&kbd_adc[i]);
			if (adcVal < adcMax) return pgm_read_byte(&kbd_code[i]);
		}
	}
	return KBD_NO;
}

ISR(PCINT0_vect)
{
	// do nothing
	// just interrupt sleeping
}

// -----------------------------------------------------------------------------
// Power Management
// -----------------------------------------------------------------------------

#define BAT_FULL  4100 // mV
#define BAT_EMPTY 3500 // mV

void pwr_saving(u08 mode)
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

NOINLINE u16 PWR_Voltage()
{
	return (1125300L / ADC_Read(ADC_VCC, 10));
}

u08 PWR_Level()
{
	u16 voltage = PWR_Voltage();
	if (voltage >= BAT_FULL ) return 100;
	if (voltage <= BAT_EMPTY) return 0;
	return ((voltage - BAT_EMPTY) / (BAT_FULL - BAT_EMPTY));
}

void PWR_Idle()
{ 	
	pwr_saving(SLEEP_MODE_IDLE);
}

void PWR_Down()
{
	do pwr_saving(SLEEP_MODE_PWR_DOWN);
	while (PWR_Voltage() <= BAT_EMPTY);
}

// -----------------------------------------------------------------------------
// Frames per Second Synchronization
// -----------------------------------------------------------------------------

#define FRAME_TIMEOUT WDT_TIMEOUT_64MS  // 15 fps

volatile b08 fps_waiting;
volatile u16 fps_counter;

NOINLINE void FPS_SyncStart()
{
	WDT_Init(WDT_MODE_INT, FRAME_TIMEOUT);
	fps_counter = 0;
}

NOINLINE void FPS_SyncStop()
{
	WDT_Init(WDT_MODE_DISABLED, 0);
}

NOINLINE void FPS_SyncWait()
{
	fps_waiting = true;
	while (fps_waiting) PWR_Idle();
}

NOINLINE u16 FPS_SyncMillis()
{
	return (fps_counter * (16 << FRAME_TIMEOUT));
}

ISR(WDT_vect)
{
	fps_waiting = false;
	fps_counter++;
}

// -----------------------------------------------------------------------------
// PCB Hardware Initialization
// -----------------------------------------------------------------------------

void PCB_Init()
{
	ADC_Init();
	LCD_Init();
	KBD_Init();
	sei();
}
