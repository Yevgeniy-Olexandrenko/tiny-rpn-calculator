// -----------------------------------------------------------------------------
// SSD1306 128x32 Display on I2C Bus
// -----------------------------------------------------------------------------

namespace LCD
{
	const u08 I2C_ADDR = 0x3C;

	const u08 WIDTH = 128;
	const u08 PAGES = 4;

	u08 draw_buf = 0xB4;

	const u08 init_data[] DATAMEM =
	{
		0xC8,       // COM scan remap
		0xA1,       // SEG remap
		0xA8, 0x1F, // 32 MUX
		0xDA, 0x02, // sequential COM pins
		0x8D, 0x14, // charge pump ON
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

	NOINLINE
	void command(u08 cmd)
	{ 
		start_command();
		I2C::Write(cmd);
		I2C::Stop();
	}

	void Init()
	{
		start_command();
		for (u08 i = 0; i < sizeof(init_data); ++i)
			I2C::Write(MEM::DataRead(init_data + i));
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

	NOINLINE
	void BeginWrite(u08 x, u08 y)
	{
		start_command();
		I2C::Write(draw_buf | y);
		I2C::Write(0x10 | (x >> 4));
		I2C::Write(x & 0x0F);
		start_data();
	}

	void Write(u08 b, u08 s)
	{
		while (s--) I2C::Write(b);
	}

	void EndWrite()
	{
		I2C::Stop();
	}

	void Clear()
	{
		for (u08 y = 0; y < PAGES; ++y)
		{
			BeginWrite(0, y);
			Write(0, WIDTH);
			EndWrite();
		}
	}

	void Flip()
	{
		u08 line = (draw_buf & 0x04) ? 0x60 : 0x40;
		draw_buf ^= 0x04;
		command(line);
	}
}