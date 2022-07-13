// -----------------------------------------------------------------------------
// I2C Bus
// -----------------------------------------------------------------------------

#define I2C_DDR DDRB
#define I2C_PIN PINB
#define I2C_SDA PB0
#define I2C_SCL PB2

namespace I2C
{
	#define sda_release()  clr_bit(I2C_DDR, I2C_SDA)
	#define sda_pulldown() set_bit(I2C_DDR, I2C_SDA)
	#define sda_read()     isb_set(I2C_PIN, I2C_SDA)
	#define scl_release()  clr_bit(I2C_DDR, I2C_SCL)
	#define scl_pulldown() set_bit(I2C_DDR, I2C_SCL)
	#define scl_read()     isb_set(I2C_PIN, I2C_SCL)
	#define i2c_delay()    __asm__ __volatile__ ("nop")

	void scl_release_wait()
	{
		scl_release();
		while (!scl_read());
	}

	u08 read_write(u08 data = 0xFF)
	{
		for (u08 sda, i = 8; i > 0; --i)
		{
			sda_release();
			if (!(data & 0x80)) sda_pulldown();
			scl_release_wait();

			sda = sda_read();
			scl_pulldown();

			data <<= 1;
			if (sda) data |= 0x01;
		}
		sda_release();
		return data;
	}

	void start()
	{
		scl_release_wait();
		sda_pulldown();
		i2c_delay();
		scl_pulldown();
	}

	NOINLINE b08 Write(u08 data)
	{
		read_write(data);
		scl_release_wait();

		b08 ack = !sda_read();
		scl_pulldown();
		return ack;
	}

	NOINLINE u08 Read(b08 ack)
	{
		u08 data = read_write();
		if (ack) sda_pulldown();
		scl_release_wait();

		i2c_delay();
		scl_pulldown();
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
		sda_pulldown();
		scl_release_wait();
		i2c_delay();
		sda_release();
	}
}