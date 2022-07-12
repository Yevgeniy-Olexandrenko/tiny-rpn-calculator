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