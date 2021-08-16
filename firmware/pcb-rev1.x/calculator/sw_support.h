////////////////////////////////////////////////////////////////////////////////
// Simple Math Helpers
////////////////////////////////////////////////////////////////////////////////

template<typename T> static uint8_t _ones(const T & x) { return (uint8_t)(x % 10); }
template<typename T> static uint8_t _tens(const T & x) { return (uint8_t)((x / 10) % 10); }
template<typename T> static uint8_t _huns(const T & x) { return (uint8_t)((x / 100) % 10); }

template<typename T> static T _min(const T & a, const T & b) { return a < b ? a : b; }
template<typename T> static T _max(const T & a, const T & b) { return a > b ? a : b; }
template<typename T> static T _abs(const T & x) { return x < 0 ? -x : x; }

////////////////////////////////////////////////////////////////////////////////
// Upate Frame Rate Sync
////////////////////////////////////////////////////////////////////////////////

#define FRAME_TIMEOUT WDT_TIMEOUT_64MS  // 15 fps

volatile bool frameWaiting;
volatile uint16_t frameCounter;

void FrameSyncStart()
{
	WDT_Init(WDT_MODE_INT, FRAME_TIMEOUT);
	frameCounter = 0;
}

void FrameSyncStop()
{
	WDT_Init(WDT_MODE_DISABLED, 0);
}

void FrameSyncWait()
{
	frameWaiting = true;
	while (frameWaiting) PWR_Idle();
}

uint16_t FrameTimePassedMs()
{
	return (frameCounter * (16 << FRAME_TIMEOUT));
}

ISR(WDT_vect)
{
	frameWaiting = false;
	frameCounter++;
}

////////////////////////////////////////////////////////////////////////////////
// Print Data On Display
////////////////////////////////////////////////////////////////////////////////

#define	CHAR_SIZE_S 1
#define	CHAR_SIZE_M 2
#define	CHAR_SIZE_L 4

uint8_t cw;
uint8_t ch;

void PrintCharSize(uint8_t width, uint8_t height)
{
	cw = width;
	ch = height;
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

void PrintCharAt(int8_t c, uint8_t x, uint8_t y)
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

void PrintStringAt(const char* s, uint8_t x, uint8_t y)
{
	uint8_t ww = FONT_WIDTH * cw + 1;
	while (char c = *s++)
	{
		PrintCharAt(c, x, y);
		x += ww;
	}
}

void PrintStringAt(const __FlashStringHelper* s, uint8_t i, uint8_t x, uint8_t y)
{
	const char* ptr = (const char*)s;
	uint8_t iw = pgm_read_byte(ptr++);
	uint8_t ww = FONT_WIDTH * cw + 1;

	for (ptr += (i * iw); iw > 0; --iw, ++ptr, x += ww)
	{
		PrintCharAt(pgm_read_byte(ptr), x, y);
	}
}

void PrintTwoDigitAt(uint8_t number, uint8_t x, uint8_t y)
{
	PrintCharAt('0' + _tens(number), x, y);
	PrintCharAt('0' + _ones(number), x + FONT_WIDTH * cw + 1, y);
}
