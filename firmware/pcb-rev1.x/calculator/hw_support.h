#pragma once

////////////////////////////////////////////////////////////////////////////////
// Helping Functions
////////////////////////////////////////////////////////////////////////////////

NOINLINE uint8_t BCD_Decode(uint8_t data)
{
  return (data / 16 * 10) + (data % 16);
}

NOINLINE uint8_t BCD_Encode(uint8_t data)
{
  return (data / 10 * 16) + (data % 10);
}

////////////////////////////////////////////////////////////////////////////////
// Watch Dog Timer
////////////////////////////////////////////////////////////////////////////////

#define WDT_MODE_DISABLED  0x00
#define WDT_MODE_RES       0x08 // To reset the CPU if there is a timeout
#define WDT_MODE_INT       0x40 // Timeout will cause an interrupt
#define WDT_MODE_INT_RES   0x48 // First time-out interrupt , the second time out - reset

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

void WDT_Init(uint8_t mode, uint8_t prescaler)
{
	// does not change global interrupts enable flag
	uint8_t wdtr = mode | ((prescaler > 7) ? 0x20 | (prescaler - 8) : prescaler);
	uint8_t sreg = SREG;
	cli();
	WDTCR = _BV(WDCE) | _BV(WDE);
	WDTCR = wdtr;
	SREG  = sreg;
}

////////////////////////////////////////////////////////////////////////////////
// Analog to Digital Converter
////////////////////////////////////////////////////////////////////////////////

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

NOINLINE uint16_t ADC_Read(uint8_t channel, uint8_t delay)
{
	ADMUX = channel;
	while (delay--) _delay_ms(1);

	set_bit(ADCSRA, ADSC);
	while (isb_set(ADCSRA, ADSC));

	uint8_t adcl = ADCL;
	uint8_t adch = ADCH;
	return (adcl | adch << 8);
}

////////////////////////////////////////////////////////////////////////////////
// I2C Bus
////////////////////////////////////////////////////////////////////////////////

#if SOFTWARE_I2C

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

NOINLINE uint8_t i2c_read_write(uint8_t data)
{
	for (uint8_t sda_i, i = 8; i > 0; --i)
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

NOINLINE bool i2c_write(uint8_t data)
{
	i2c_read_write(data);
	i2c_scl_h_wait();

	bool ack = true;
	if (I2C_SDA_I()) ack = false;

	I2C_SCL_L();
	return ack;
}

NOINLINE uint8_t i2c_read(bool ack)
{
	uint8_t data = i2c_read_write(0xFF);
	if (ack) I2C_SDA_L();
	i2c_scl_h_wait();
	I2C_DELAY();

	I2C_SCL_L();
	return data;
}

uint8_t i2c_read_ack()
{ 
	return i2c_read(true);
}

uint8_t i2c_read_nack()
{
	return i2c_read(false);
}

NOINLINE void i2c_start()
{
	i2c_scl_h_wait();
	I2C_SDA_L();
	I2C_DELAY();
	I2C_SCL_L();
}

bool i2c_start_write(uint8_t addr)
{
	i2c_start();
	return i2c_write(addr << 1);
}

bool i2c_start_read(uint8_t addr)
{
	i2c_start();
	return i2c_write(addr << 1 | 1);
}

NOINLINE void i2c_stop()
{
	I2C_SDA_L();
	i2c_scl_h_wait();
	I2C_DELAY();
	I2C_SDA_H();
}

void I2C_Init()
{
	// do nothing
}

bool I2C_Start(uint8_t address, int readcount)
{
	if (readcount == 0)
		return i2c_start_write(address);
	else
		return i2c_start_read(address);
}

bool I2C_Restart(uint8_t address, int readcount) 
{
	return I2C_Start(address, readcount);
}

uint8_t I2C_Read()
{
	return i2c_read_ack();
}

uint8_t I2C_ReadLast()
{
	return i2c_read_nack();
}

bool I2C_Write(uint8_t data)
{
	return i2c_write(data);
}

void I2C_Stop() 
{
	i2c_stop();
}

#else

#define DDR_USI       DDRB
#define PORT_USI      PORTB
#define PIN_USI       PINB

#define PORT_USI_SDA  PORTB0
#define PORT_USI_SCL  PORTB2
#define PIN_USI_SDA   PINB0
#define PIN_USI_SCL   PINB2

#define DDR_USI_CL    DDR_USI
#define PORT_USI_CL   PORT_USI
#define PIN_USI_CL    PIN_USI

#define TWI_FAST_MODE
#ifdef  TWI_FAST_MODE                // TWI FAST mode timing limits. SCL = 100-400kHz
#define DELAY_T2TWI (_delay_us(2))   // >1.3us
#define DELAY_T4TWI (_delay_us(1))   // >0.6us
#else                                // TWI STANDARD mode timing limits. SCL <= 100kHz
#define DELAY_T2TWI (_delay_us(5))   // >4.7us
#define DELAY_T4TWI (_delay_us(4))   // >4.0us
#endif

#define TWI_NACK_BIT 0               // Bit position for (N)ACK bit.

// Prepare register value to: Clear flags, and set USI to shift 8 bits i.e. count 16 clock edges.
const uint8_t USISR_8bit = 1<<USISIF | 1<<USIOIF | 1<<USIPF | 1<<USIDC | 0x0<<USICNT0;

// Prepare register value to: Clear flags, and set USI to shift 1 bit i.e. count 2 clock edges.
const uint8_t USISR_1bit = 1<<USISIF | 1<<USIOIF | 1<<USIPF | 1<<USIDC | 0xE<<USICNT0;

int I2Ccount;

uint8_t i2c_transfer(uint8_t data) 
{
	USISR = data;                                // Set USISR according to data.
												 // Prepare clocking.
	data  = 0<<USISIE | 0<<USIOIE |              // Interrupts disabled
			1<<USIWM1 | 0<<USIWM0 |              // Set USI in Two-wire mode.
			1<<USICS1 | 0<<USICS0 | 1<<USICLK |  // Software clock strobe as source.
			1<<USITC;                            // Toggle Clock Port.
	do {
		DELAY_T2TWI;
		USICR = data;                            // Generate positive SCL edge.
		while (!(PIN_USI_CL & 1<<PIN_USI_SCL));  // Wait for SCL to go high.
		DELAY_T4TWI;
		USICR = data;                            // Generate negative SCL edge.
	} while (!(USISR & 1<<USIOIF));              // Check for transfer complete.

	DELAY_T2TWI;
	data = USIDR;                                // Read out data.
	USIDR = 0xFF;                                // Release SDA.
	DDR_USI |= (1<<PIN_USI_SDA);                 // Enable SDA as output.

	return data;                                 // Return the data from the USIDR
}

void I2C_Init()
{
	PORT_USI |= 1<<PIN_USI_SDA;                  // Enable pullup on SDA.
	PORT_USI_CL |= 1<<PIN_USI_SCL;               // Enable pullup on SCL.

	DDR_USI_CL |= 1<<PIN_USI_SCL;                // Enable SCL as output.
	DDR_USI |= 1<<PIN_USI_SDA;                   // Enable SDA as output.

	USIDR = 0xFF;                                // Preload data register with "released level" data.
	USICR = 0<<USISIE | 0<<USIOIE |              // Disable Interrupts.
			1<<USIWM1 | 0<<USIWM0 |              // Set USI in Two-wire mode.
			1<<USICS1 | 0<<USICS0 | 1<<USICLK |  // Software stobe as counter clock source
			0<<USITC;
	USISR = 1<<USISIF | 1<<USIOIF | 1<<USIPF |   // Clear flags,
			1<<USIDC  |	0<<USICNT0;              // and reset counter.
}

uint8_t I2C_Read()
{
	if ((I2Ccount != 0) && (I2Ccount != -1)) I2Ccount--;

	/* Read a byte */
	DDR_USI &= ~(1<<PIN_USI_SDA);                // Enable SDA as input.
	uint8_t data = i2c_transfer(USISR_8bit);

	/* Prepare to generate ACK (or NACK in case of End Of Transmission) */
	if (I2Ccount == 0) USIDR = 0xFF; else USIDR = 0x00;
	i2c_transfer(USISR_1bit);                    // Generate ACK/NACK.

	return data;                                 // Read successfully completed
}

uint8_t I2C_ReadLast()
{
	I2Ccount = 0;
	return I2C_Read();
}

bool I2C_Write(uint8_t data)
{
	/* Write a byte */
	PORT_USI_CL &= ~(1<<PIN_USI_SCL);            // Pull SCL LOW.
	USIDR = data;                                // Setup data.
	i2c_transfer(USISR_8bit);                    // Send 8 bits on bus.

	/* Clock and verify (N)ACK from slave */
	DDR_USI &= ~(1<<PIN_USI_SDA);                // Enable SDA as input.
	if (i2c_transfer(USISR_1bit) & 1<<TWI_NACK_BIT) return false;

	return true;                                 // Write successfully completed
}

bool I2C_Start(uint8_t address, int readcount)
{
	/* Start transmission by sending address */
	if (readcount != 0) { I2Ccount = readcount; readcount = 1; }
	uint8_t addressRW = address<<1 | readcount;

	/* Release SCL to ensure that (repeated) Start can be performed */
	PORT_USI_CL |= 1<<PIN_USI_SCL;               // Release SCL.
	while (!(PIN_USI_CL & 1<<PIN_USI_SCL));      // Verify that SCL becomes high.
#ifdef TWI_FAST_MODE
	DELAY_T4TWI;
#else
	DELAY_T2TWI;
#endif

	/* Generate Start Condition */
	PORT_USI &= ~(1<<PIN_USI_SDA);               // Force SDA LOW.
	DELAY_T4TWI;
	PORT_USI_CL &= ~(1<<PIN_USI_SCL);            // Pull SCL LOW.
	PORT_USI |= 1<<PIN_USI_SDA;                  // Release SDA.

	if (!(USISR & 1<<USISIF)) return false;

	/*Write address */
	PORT_USI_CL &= ~(1<<PIN_USI_SCL);            // Pull SCL LOW.
	USIDR = addressRW;                           // Setup data.
	i2c_transfer(USISR_8bit);                    // Send 8 bits on bus.

	/* Clock and verify (N)ACK from slave */
	DDR_USI &= ~(1<<PIN_USI_SDA);                // Enable SDA as input.
	if (i2c_transfer(USISR_1bit) & 1<<TWI_NACK_BIT) return false; // No ACK

	return true;                                 // Start successfully completed
}

bool I2C_Restart(uint8_t address, int readcount) 
{
	return I2C_Start(address, readcount);
}

void I2C_Stop() 
{
	PORT_USI &= ~(1<<PIN_USI_SDA);               // Pull SDA low.
	PORT_USI_CL |= 1<<PIN_USI_SCL;               // Release SCL.
	while (!(PIN_USI_CL & 1<<PIN_USI_SCL));      // Wait for SCL to go high.
	DELAY_T4TWI;
	PORT_USI |= 1<<PIN_USI_SDA;                  // Release SDA.
	DELAY_T2TWI;
}

#endif

////////////////////////////////////////////////////////////////////////////////
// SSD1306 128x32 Display on I2C Bus
////////////////////////////////////////////////////////////////////////////////

#define LCD_ADDR  0x3C
#define LCD_COMM  0x00
#define LCD_DATA  0x40
#define LCD_WIDTH 128 
#define LCD_PAGES 4

uint8_t ssd1306_bbuf = 0xB4;
uint8_t ssd1306_fbuf = 0x40;

const uint8_t ssd1306_init_sequence[] PROGMEM =
{
	0xC8,       // Set scan direction (C0 scan from COM0 to COM[N-1] or C8 mirroring)
	0xA1,       // Set segment remap (A0 regular or A1 flip)
	0xA8, 0x1F, // Set mux ratio (N+1) where: 14<N<64 ... 3F or 1F
	0xDA, 0x02, // COM config pin mapping:
	            //                  right/left left/right
	            //      sequential      02        22
	            //      alternate       12        32
	0x20, 0x00, // Horizontal addressing mode (line feed after EOL)
	0x8D, 0x14 // Charge pump (0x14 enable or 0x10 disable)
};

void ssd1306_send_start()
{ 
	I2C_Start(LCD_ADDR, 0);
}

uint8_t ssd1306_send_byte(uint8_t b)
{ 
	return I2C_Write(b);
}

void ssd1306_send_stop()
{ 
	I2C_Stop();
}

void ssd1306_command_start()
{ 
	ssd1306_send_start();
	ssd1306_send_byte(LCD_COMM);
}

void ssd1306_data_start()
{ 
	ssd1306_send_start();
	ssd1306_send_byte(LCD_DATA);
}

void ssd1306_send_command(uint8_t cmd)
{ 
	ssd1306_command_start();
	ssd1306_send_byte(cmd);
	ssd1306_send_stop();
}

void ssd1306_send_data(uint8_t b)
{ 
	if (!ssd1306_send_byte(b))
	{
		ssd1306_send_stop();
		ssd1306_data_start();
		ssd1306_send_byte(b);
	}
}

void LCD_Init()
{
	ssd1306_command_start();
	for (uint8_t i = 0; i < sizeof(ssd1306_init_sequence); i++)
	{
		ssd1306_send_byte(pgm_read_byte(&ssd1306_init_sequence[i]));
	}
	ssd1306_send_stop();
}

void LCD_TurnOn()
{
	ssd1306_send_command(0xAF);
}

void LCD_TurnOff()
{ 
	ssd1306_send_command(0xAE);
}

void LCD_Brightness(uint8_t brightness)
{ 
	ssd1306_command_start();
	ssd1306_send_byte(0x81);
	ssd1306_send_byte(brightness);
	ssd1306_send_stop();
}

void LCD_Position(uint8_t x, uint8_t y)
{ 
	ssd1306_command_start();
	ssd1306_send_byte(ssd1306_bbuf | (y & 0x07));
	ssd1306_send_byte(0x10 | (x >> 4));
	ssd1306_send_byte(x & 0x0f);
	ssd1306_send_stop();
}

void LCD_Write(uint8_t b, uint8_t s)
{
	ssd1306_data_start();
	while (s--) ssd1306_send_data(b);
	ssd1306_send_stop();
}

void LCD_Clear()
{
	LCD_Position(0, 0);
	for (uint8_t i = LCD_PAGES; i > 0; --i)
	{
		LCD_Write(0x00, LCD_WIDTH);
	}
}

void LCD_Flip()
{
	ssd1306_fbuf ^= 0x20;
	ssd1306_send_command(ssd1306_fbuf);
	ssd1306_bbuf ^= 0x04;
}

////////////////////////////////////////////////////////////////////////////////
// DS3231M Real Time Clock on I2C Bus
////////////////////////////////////////////////////////////////////////////////

// Device address
#define RTC_ADDR         0x68

// Registers
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

// Flags
#define RTC_HOUR_12      _BV(6)

// Figure out build date and time (Example __DATE__ : "Jul 27 2012" and __TIME__ : "21:06:19")
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

uint8_t rtc_seconds = BUILD_SEC;   // 0 - 59
uint8_t rtc_minutes = BUILD_MIN;   // 0 - 59
uint8_t rtc_hours   = BUILD_HOUR;  // 0 - 23

uint8_t rtc_date    = BUILD_DAY;   // 1 - 31
uint8_t rtc_month   = BUILD_MONTH; // 1 - 12
uint8_t rtc_year    = BUILD_YEAR;  // 0 - 99

void RTC_ReadDateAndTime()
{
	// Day of the week is not used!
	// Century flag is not supported!
	if (I2C_Start(RTC_ADDR, 0))
	{
		I2C_Write(RTC_SECONDS);
		I2C_Restart(RTC_ADDR, 7);
		rtc_seconds = BCD_Decode(I2C_Read());
		rtc_minutes = BCD_Decode(I2C_Read());
		uint8_t tmp = I2C_Read();
		if (tmp & RTC_HOUR_12) 
			rtc_hours = ((tmp >> 4) & 0x01) * 12 + ((tmp >> 5) & 0x01) * 12;
		else 
			rtc_hours = BCD_Decode(tmp);
		tmp = I2C_Read();
		rtc_date  = BCD_Decode(I2C_Read());
		rtc_month = BCD_Decode(I2C_Read() & 0x1F);
		rtc_year  = BCD_Decode(I2C_ReadLast() % 100);
		I2C_Stop();
	}
}

void RTC_WriteDateAndTime()
{
	// Time always stored in 24-hour format!
	// Day of the week is not used!
	// Century flag is not supported!
	if(I2C_Start(RTC_ADDR, 0))
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

float RTC_ReadTemperature()
{
#if !DEBUG_ON_R1_0
	if (I2C_Start(RTC_ADDR, 0))
	{
		I2C_Write(RTC_TEMP_MSB);
		I2C_Restart(RTC_ADDR, 2);
		uint8_t msb = I2C_Read();
		uint8_t lsb = I2C_ReadLast();
		I2C_Stop();
		return int16_t(msb << 8 | lsb) / 256.f;
	}
#endif
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// One Pin Analog 16-Key Keyboard
//////////////////////////////////////////////////////////////////////////////// 

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

const uint16_t kbd_adc[] PROGMEM =
{
	147, 182, 221, 269, 324, 383, 442, 505,
	573, 635, 692, 762, 827, 863, 893, 913
};

const uint8_t kbd_code[] PROGMEM = 
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

NOINLINE uint8_t KBD_Read()
{
	uint16_t adcVal = ADC_Read(KBD_ADC, 1);
	if (adcVal > 110)
	{
		for (uint8_t i = 0; i < 16; ++i)
		{
			uint16_t adcMax = pgm_read_word(&kbd_adc[i]);
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

////////////////////////////////////////////////////////////////////////////////
// Power Management
///////////////////////////////////////////////////////////////////////////////

#define BAT_FULL  4.1f
#define BAT_EMPTY 3.5f

void pwr_sleeping(uint8_t mode)
{
	set_sleep_mode(mode);
	sleep_enable();
	sleep_cpu();
	sleep_disable();
}

void pwr_saving(uint8_t mode)
{
	clr_bit(ADCSRA, ADEN);
	power_all_disable();
	pwr_sleeping(mode);
#if !SOFTWARE_I2C	
	power_usi_enable();
#endif	
	power_adc_enable();
	set_bit(ADCSRA, ADEN);
}

NOINLINE float PWR_Voltage()
{
	return (1125.3f / ADC_Read(ADC_VCC, 10));
}

float PWR_Level()
{
	float voltage = PWR_Voltage();
	if (voltage >= BAT_FULL ) return 1;
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
