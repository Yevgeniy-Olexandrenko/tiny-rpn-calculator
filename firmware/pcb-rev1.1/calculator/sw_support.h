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

volatile uint8_t  frameWaiting;
volatile uint16_t frameCounter;

void FrameSyncStart()
{
	// frame rate is about 15 FPS
	WDTInit(WDT_MODE_INT, WDT_TIMEOUT_64MS);
	frameCounter = 0;
}

void FrameSyncStop()
{
	WDTInit(WDT_MODE_DISABLED, 0);
}

void FrameSyncWait()
{
	frameWaiting = true;
	while (frameWaiting) PowerIdle();
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

void PrintCharAt(uint8_t c, uint8_t x, uint8_t y)
{
	c -= FONT_BEGIN;
	for (uint8_t cy = 0; cy < ch; ++cy)
	{
		DisplayPosition(x, y + cy);
		for (uint8_t i = 0; i < FONT_WIDTH; ++i)
		{
			uint8_t bitmap = FONT_READ(&font[FONT_WIDTH * c + i]);
			if (ch == CHAR_SIZE_M)
				bitmap = expand4bit((bitmap >> (cy << 2)) & 0x0f); // Expand 0000abcd
			else if (ch == CHAR_SIZE_L)
				bitmap = expand2bit((bitmap >> (cy << 1)) & 0x03); // Expand 000000ab
			DisplayWrite(bitmap, cw);
		}
	}
}

void PrintStringAt(const __FlashStringHelper* s, uint8_t i, uint8_t x, uint8_t y)
{
	const char* ptr = (const char*)s;
	uint8_t iw = pgm_read_byte(ptr++);
	uint8_t ww = FONT_WIDTH * cw + 1;

	for(ptr += (i * iw); iw > 0; --iw, ++ptr, x += ww)
	{
		PrintCharAt(pgm_read_byte(ptr), x, y);
	}
}

void PrintTwoDigitAt(uint8_t number, uint8_t x, uint8_t y)
{
	PrintCharAt('0' + _tens(number), x, y);
	PrintCharAt('0' + _ones(number), x + FONT_WIDTH * cw + 1, y);
}
