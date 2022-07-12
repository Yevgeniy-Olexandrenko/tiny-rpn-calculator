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