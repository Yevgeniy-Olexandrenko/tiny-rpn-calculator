// -----------------------------------------------------------------------------
// I2C Bus
// -----------------------------------------------------------------------------

#define I2C_DDR  DDRB
#define I2C_PORT PORTB
#define I2C_PIN  PINB
#define I2C_SDA  PB0
#define I2C_SCL  PB2

// Delay is approximately 2 µs, rounded up, and remains safe for supported F_CPU
//  1 MHz ->  2 cycles
//  4 MHz ->  8 cycles
//  8 MHz -> 16 cycles
// 16 MHz -> 32 cycles
// 20 MHz -> 40 cycles
#define I2C_DELAY_CYCLES ((F_CPU + 499999UL) / 500000UL)

namespace I2C
{
	#define sda_release()  clr_bit(I2C_DDR, I2C_SDA)
	#define sda_pulldown() set_bit(I2C_DDR, I2C_SDA)
	#define sda_read()     isb_set(I2C_PIN, I2C_SDA)
	#define scl_release()  clr_bit(I2C_DDR, I2C_SCL)
	#define scl_pulldown() set_bit(I2C_DDR, I2C_SCL)
	#define i2c_delay()    __builtin_avr_delay_cycles(I2C_DELAY_CYCLES)
	
	NOINLINE
	u08 transfer(u08 data)
	{
		for (u08 i = 8; i; --i)
		{
			sda_release();
			if (!(data & 0x80)) sda_pulldown();
			i2c_delay();
			scl_release();
			i2c_delay();
			data <<= 1;
			if (sda_read()) data |= 1;
			scl_pulldown();
		}
		sda_release();
		return data;
	}

	NOINLINE
	void Write(u08 data)
	{
		transfer(data);
		i2c_delay();
		scl_release();
		i2c_delay();
		scl_pulldown();
	}

	NOINLINE
	void Read(u08& data, b08 ack)
	{
		data = transfer(0xFF);
		if (ack) sda_pulldown();
		i2c_delay();
		scl_release();
		i2c_delay();
		scl_pulldown();
		sda_release();
	}

	NOINLINE
	void ReadAck(u08& data)
	{
		Read(data, true);
	}

	NOINLINE
	void ReadNack(u08& data)
	{
		Read(data, false);
	}

	NOINLINE
	void start(u08 addr_mode)
	{
		sda_release();
		i2c_delay();
		scl_release();
		i2c_delay();
		sda_pulldown();
		i2c_delay();
		scl_pulldown();
		Write(addr_mode);
	}

	void StartWrite(u08 addr)
	{
		start(addr << 1);
	}

	void StartRead(u08 addr)
	{
		start((addr << 1) | 0x01);
	}

	NOINLINE
	void Stop()
	{
		sda_pulldown();
		i2c_delay();
		scl_release();
		i2c_delay();
		sda_release();
		i2c_delay();
	}
	
	void Init()
	{
		const u08 mask = _BV(I2C_SDA) | _BV(I2C_SCL);
		I2C_DDR  &= ~mask;
		I2C_PORT &= ~mask;
		i2c_delay();
		for (u08 i = 9; i && !sda_read(); --i)
		{
			scl_pulldown();
			i2c_delay();
			scl_release();
			i2c_delay();
		}
		sda_pulldown();
		i2c_delay();
		sda_release();
		i2c_delay();
	}

	#undef sda_release
	#undef sda_pulldown
	#undef sda_read
	#undef scl_release
	#undef scl_pulldown
	#undef i2c_delay
}