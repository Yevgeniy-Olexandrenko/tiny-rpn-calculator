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
// Compact Font 5x8 in EEPROM
////////////////////////////////////////////////////////////////////////////////

#define FONT_OFFSET '+'
#define FONT_WIDTH  5

const uint8_t font[] PROGMEM =
{
	// No ascii signs below 43 ('+') to save memory (flash)

	0x00, 0x00, 0x00, 0x00, 0x00, // + space

	0b00010000, // , squareroot
	0b00100000,
	0b01111111,
	0b00000001,
	0b00000001,
	
	0x08, 0x08, 0x08, 0x08, 0x08, // -

	0b00000000, // .
	0b00000000,
	0b00000000,
	0b01100000,
	0b01100000,

	0x20, 0x10, 0x08, 0x04, 0x02, // /
	0x7f, 0x41, 0x41, 0x41, 0x7f, // 0
	0x00, 0x00, 0x02, 0x7f, 0x00, // 1
	0x79, 0x49, 0x49, 0x49, 0x4f, // 2
	0x41, 0x49, 0x49, 0x49, 0x7f, // 3
	0x0f, 0x08, 0x08, 0x08, 0x7f, // 4
	0x4f, 0x49, 0x49, 0x49, 0x79, // 5
	0x7f, 0x49, 0x49, 0x49, 0x79, // 6
	0x03, 0x01, 0x01, 0x01, 0x7f, // 7
	0x7f, 0x49, 0x49, 0x49, 0x7f, // 8
	0x4f, 0x49, 0x49, 0x49, 0x7f, // 9

	0b00000000, // :
	0b00110110,
	0b00110110,
	0b00000000,
	0b00000000,

	0x00, 0x1b, 0x04, 0x1b, 0x00, // ; raised x
	0x00, 0x7f, 0x3e, 0x1c, 0x08, // < play

	0b00000100, // = shift sign
	0b01111110,
	0b01111111,
	0b01111110,
	0b00000100,

	0x08, 0x08, 0x3e, 0x1c, 0x08, // > arrow to right
	0x00, 0x00, 0x2f, 0x00, 0x00, // ? !
	0x1c, 0x3e, 0x3e, 0x3e, 0x1c, // @ record
	0x7f, 0x09, 0x09, 0x09, 0x7f, // A
	0x7f, 0x49, 0x49, 0x4f, 0x78, // B
	0x7f, 0x41, 0x41, 0x41, 0x40, // C

	0b01111111, // D
	0b01000001,
	0b01000001,
	0b01000001,
	0b00111110,

	0x7F, 0x49, 0x49, 0x49, 0x41, // E
	0x7F, 0x09, 0x09, 0x09, 0x01, // F
	0x7f, 0x41, 0x49, 0x49, 0x79, // G
	0x7F, 0x08, 0x08, 0x08, 0x7F, // H
	0x00, 0x41, 0x7F, 0x41, 0x00, // I
	0x40, 0x40, 0x40, 0x40, 0x7f, // J
	0x7F, 0x08, 0x08, 0x0f, 0x78, // K
	0x7F, 0x40, 0x40, 0x40, 0x40, // L

	0b01111111, // M
	0b00000100,
	0b00001000,
	0b00000100,
	0b01111111,

	0b01111111, // N
	0b00000100,
	0b00001000,
	0b00010000,
	0b01111111,

	0x7f, 0x41, 0x41, 0x41, 0x7f, // O
	0x7F, 0x09, 0x09, 0x09, 0x0f, // P
	0x7f, 0x41, 0x71, 0x41, 0x7f, // Q

	0b01111111, // R
	0b00001001,
	0b00011001,
	0b00101001,
	0b01001111,

	0x4f, 0x49, 0x49, 0x49, 0x78, // S
	0x01, 0x01, 0x7F, 0x01, 0x01, // T
	0x7F, 0x40, 0x40, 0x40, 0x7F, // U

	0b00011111, // V
	0b00100000,
	0b01000000,
	0b00100000,
	0b00011111,

	0x7F, 0x40, 0x70, 0x40, 0x7F, // W

	0b01100011, // X
	0b00010100,
	0b00001000,
	0b00010100,
	0b01100011,

	0b00000011, // Y
	0b00000100,
	0b01111000,
	0b00000100,
	0b00000011,

	0x71, 0x51, 0x5d, 0x45, 0x47, // Z

	// No low case letters to save memory
	// 48 * 5 = 240s bytes
};

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
	c -= FONT_OFFSET;
	for (uint8_t cy = 0; cy < ch; ++cy)
	{
		DisplayPosition(x, y + cy);
		for (uint8_t i = 0; i < FONT_WIDTH; ++i)
		{
			uint8_t bitmap = pgm_read_byte(&font[FONT_WIDTH * c + i]);
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
