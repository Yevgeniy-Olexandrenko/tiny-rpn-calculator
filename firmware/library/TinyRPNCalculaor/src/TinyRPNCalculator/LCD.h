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
		0xA8, 0x1F, // set multiplex (HEIGHT-1): 0x1F for 128x32, 0x3F for 128x64 
		0xDA, 0x02, // set COM pins hardware configuration to sequential
		0x20, 0x00, // set horizontal memory addressing mode
		0x8D, 0x14, // enable charge pump
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
		{
			I2C::Write(pgm_read_byte(init_data + i));
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

	NOINLINE
	void Write(u08 b, u08 s)
	{
		start_data();
		while (s--) I2C::Write(b);
		I2C::Stop();
	}

	void Clear()
	{
		Position(0, 0);
		for (u08 i = PAGES; i > 0; --i) Write(0, WIDTH);
	}

	void Flip()
	{
		rend_buf ^= 0x20;
		draw_buf ^= 0x04;
		command(rend_buf);
	}
}