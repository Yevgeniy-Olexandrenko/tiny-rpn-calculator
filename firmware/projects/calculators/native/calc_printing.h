#pragma once

////////////////////////////////////////////////////////////////////////////////
// Compact Font 5x8 with Special Characters
////////////////////////////////////////////////////////////////////////////////

#define FONT_BEGIN 0x2A
#define FONT_WIDTH 0x05

#if FONT_IN_EEMEM
#define FONT_STORAGE EEMEM
#define FONT_READ(a) eeprom_read_byte(a)
#else
#define FONT_STORAGE PROGMEM
#define FONT_READ(a) pgm_read_byte(a)
#endif

//	49 * 5 = 245 bytes
const uint8_t font[] FONT_STORAGE =
{
//	-------------------------------
	0b00100010, // 2A *
	0b00010100,
	0b00001000,
	0b00010100,
	0b00100010,
//	-------------------------------	
	0b00001000, // 2B +
	0b00001000,
	0b00111110,
	0b00001000,
	0b00001000,
//	-------------------------------
	0b00010000, // 2C , (squareroot)
	0b00100000,
	0b01111111,
	0b00000001,
	0b00000001,
//	-------------------------------	
	0b00001000, // 2D -
	0b00001000,
	0b00001000,
	0b00001000,
	0b00001000,
//	-------------------------------
	0b00000000, // 2E .
	0b00000000,
	0b00000000,
	0b01100000,
	0b01100000,
//	-------------------------------
	0b00100000, // 2F /
	0b00010000,
	0b00001000,
	0b00000100,
	0b00000010,
//	-------------------------------
	0b01111111, // 30 0
	0b01000001,
	0b01000001,
	0b01000001,
	0b01111111,
//	-------------------------------
	0b00000000, // 31 1
	0b00000000,
	0b00000010,
	0b01111111,
	0b00000000,
//	-------------------------------
	0b01111001, // 32 2
	0b01001001,
	0b01001001,
	0b01001001,
	0b01001111,
//	-------------------------------
	0b01000001, // 33 3
	0b01001001,
	0b01001001,
	0b01001001,
	0b01111111,
//	-------------------------------
	0b00001111, // 34 4
	0b00001000,
	0b00001000,
	0b00001000,
	0b01111111,
//	-------------------------------
	0b01001111, // 35 5
	0b01001001,
	0b01001001,
	0b01001001,
	0b01111001,
//	-------------------------------
	0b01111111, // 36 6
	0b01001001,
	0b01001001,
	0b01001001,
	0b01111001,
//	-------------------------------
	0b00000011, // 37 7
	0b00000001,
	0b00000001,
	0b00000001,
	0b01111111,
//	-------------------------------
	0b01111111, // 38 8
	0b01001001,
	0b01001001,
	0b01001001,
	0b01111111,
//	-------------------------------
	0b01001111, // 39 9
	0b01001001,
	0b01001001,
	0b01001001,
	0b01111111,
//	-------------------------------
	0b00000000, // 3A :
	0b00110110,
	0b00110110,
	0b00000000,
	0b00000000,
//	-------------------------------
	0b00000000, // 3B ; (raised x)
	0b00011011,
	0b00000100,
	0b00011011,
	0b00000000,
//	-------------------------------
	0b00000000, // 3C < (play)
	0b01111111,
	0b00111110,
	0b00011100,
	0b00001000,
//	-------------------------------
	0b00000100, // 3D = (shift sign)
	0b01111110,
	0b01111111,
	0b01111110,
	0b00000100,
//	-------------------------------
	0b00001000, // 3E > (arrow to right)
	0b00001000,
	0b00111110,
	0b00011100,
	0b00001000,
//	-------------------------------
	0b00000000, // 3F ? (raised 2)
	0b00011101,
	0b00010101,
	0b00010111,
	0b00000000,
//	-------------------------------
	0b00011100, // 40 @ (record)
	0b00111110,
	0b00111110,
	0b00111110,
	0b00011100,
//	-------------------------------
	0b01111111, // 41 A
	0b00001001,
	0b00001001,
	0b00001001,
	0b01111111,
//	-------------------------------
	0b01111111, // 42 B
	0b01001001,
	0b01001001,
	0b01001111,
	0b01111000,
//	-------------------------------
	0b01111111, // 43 C
	0b01000001,
	0b01000001,
	0b01000001,
	0b01000000,
//	-------------------------------
	0b01111111, // 44 D
	0b01000001,
	0b01000001,
	0b01000001,
	0b00111110,
//	-------------------------------
	0b01111111, // 45 E
	0b01001001,
	0b01001001,
	0b01001001,
	0b01000001,
//	-------------------------------
	0b01111111, // 46 F
	0b00001001,
	0b00001001,
	0b00001001,
	0b00000001,
//	-------------------------------
	0b01111111, // 47 G
	0b01000001,
	0b01001001,
	0b01001001,
	0b01111001,
//	-------------------------------
	0b01111111, // 48 H
	0b00001000,
	0b00001000,
	0b00001000,
	0b01111111,
//	-------------------------------
	0b00000000, // 49 I
	0b01000001,
	0b01111111,
	0b01000001,
	0b00000000,
//	-------------------------------
	0b01000000, // 4A J
	0b01000000,
	0b01000000,
	0b01000000,
	0b01111111,
//	-------------------------------
	0b01111111, // 4B K
	0b00001000,
	0b00001000,
	0b00001111,
	0b01111000,
//	-------------------------------
	0b01111111, // 4C L
	0b01000000,
	0b01000000,
	0b01000000,
	0b01000000,
//	-------------------------------
	0b01111111, // 4D M
	0b00000100,
	0b00001000,
	0b00000100,
	0b01111111,
//	-------------------------------
	0b01111111, // 4E N
	0b00000100,
	0b00001000,
	0b00010000,
	0b01111111,
//	-------------------------------
	0b01111111, // 4F O
	0b01000001,
	0b01000001,
	0b01000001,
	0b01111111,
//	-------------------------------
	0b01111111, // 50 P
	0b00001001,
	0b00001001,
	0b00001001,
	0b00001111,
//	-------------------------------
	0b01111111, // 51 Q
	0b01000001,
	0b01110001,
	0b01000001,
	0b01111111,
//	-------------------------------
	0b01111111, // 52 R
	0b00001001,
	0b00011001,
	0b00101001,
	0b01001111,
//	-------------------------------
	0b01001111, // 53 S
	0b01001001,
	0b01001001,
	0b01001001,
	0b01111000,
//	-------------------------------
	0b00000001, // 54 T
	0b00000001,
	0b01111111,
	0b00000001,
	0b00000001,
//	-------------------------------
	0b01111111, // 55 U
	0b01000000,
	0b01000000,
	0b01000000,
	0b01111111,
//	-------------------------------
	0b00011111, // 56 V
	0b00100000,
	0b01000000,
	0b00100000,
	0b00011111,
//	-------------------------------
	0b01111111, // 57 W
	0b01000000,
	0b01110000,
	0b01000000,
	0b01111111,
//	-------------------------------
	0b01100011, // 58 X
	0b00010100,
	0b00001000,
	0b00010100,
	0b01100011,
//	-------------------------------
	0b00000011, // 59 Y
	0b00000100,
	0b01111000,
	0b00000100,
	0b00000011,
//	-------------------------------
	0b01110001, // 5A Z
	0b01010001,
	0b01011101,
	0b01000101,
	0b01000111,
//	-------------------------------
};

////////////////////////////////////////////////////////////////////////////////
// Print Data On Display
////////////////////////////////////////////////////////////////////////////////

#define	CHAR_SIZE_S 1
#define	CHAR_SIZE_M 2
#define	CHAR_SIZE_L 4

uint8_t cw;
uint8_t ch;
uint8_t dx;

NOINLINE void PrintCharSize(uint8_t width, uint8_t height)
{
	cw = width;
	ch = height;
	dx = FONT_WIDTH * cw + 1;
}

uint8_t expand4bit(uint8_t b)
{	
	// 0000abcd -> aabbccdd
	b = (b | (b << 2)) & 0x33;
	b = (b | (b << 1)) & 0x55;
	return b | (b << 1);
}

uint8_t expand2bit(uint8_t b)
{							   
	// 000000ab -> aaaabbbb
	b = (b | (b << 3)) & 0x11;
	for (uint8_t i = 3; i > 0; --i) b |= (b << 1);
	return b;
}

NOINLINE void PrintCharAt(int8_t c, uint8_t x, uint8_t y)
{
	c -= FONT_BEGIN;
	for (uint8_t cy = 0; cy < ch; ++cy)
	{
		LCD_Position(x, y + cy);
		for (uint8_t i = 0; i < FONT_WIDTH; ++i)
		{
			uint8_t bitmap = 0x00;
			if (c >= 0)
			{
				bitmap = FONT_READ(&font[FONT_WIDTH * c + i]);
				if (ch == CHAR_SIZE_M)
					bitmap = expand4bit((bitmap >> (cy << 2)) & 0x0f); // Expand 0000abcd
				else if (ch == CHAR_SIZE_L)
					bitmap = expand2bit((bitmap >> (cy << 1)) & 0x03); // Expand 000000ab
			}
			LCD_Write(bitmap, cw);
		}
	}
}

NOINLINE void PrintStringAt(const char* s, uint8_t x, uint8_t y)
{
	while (char c = *s++)
	{
		PrintCharAt(c, x, y);
		x += dx;
	}
}

NOINLINE void PrintStringAt(const __FlashStringHelper* s, uint8_t i, uint8_t x, uint8_t y)
{
	const char* ptr = (const char*)s;
	uint8_t iw = pgm_read_byte(ptr++);

	for (ptr += (i * iw); iw > 0; --iw, ++ptr, x += dx)
	{
		PrintCharAt(pgm_read_byte(ptr), x, y);
	}
}

NOINLINE void PrintTensOnesAt(uint8_t number, uint8_t x, uint8_t y)
{
	PrintCharAt('0' + _tens(number), x, y);
	PrintCharAt('0' + _ones(number), x + dx, y);
}
