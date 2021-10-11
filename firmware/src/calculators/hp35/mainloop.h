#pragma once

////////////////////////////////////////////////////////////////////////////////
// Key Codes (0x00-0x09 numbers, 0x0A-0x0F special keys)
////////////////////////////////////////////////////////////////////////////////

enum 
{
	KEY_FUNC = KBD::KeyA0, KEY_NUM7 = KBD::KeyB0, KEY_NUM8 = KBD::KeyC0, KEY_NUM9 = KBD::KeyD0,
	KEY_EEXP = KBD::KeyA1, KEY_NUM4 = KBD::KeyB1, KEY_NUM5 = KBD::KeyC1, KEY_NUM6 = KBD::KeyD1,
	KEY_NEG  = KBD::KeyA2, KEY_NUM1 = KBD::KeyB2, KEY_NUM2 = KBD::KeyC2, KEY_NUM3 = KBD::KeyD2,
	KEY_DROP = KBD::KeyA3, KEY_NUM0 = KBD::KeyB3, KEY_DOT  = KBD::KeyC3, KEY_DUP  = KBD::KeyD3,
	KEY_NONE = KBD::KeyNO
};

////////////////////////////////////////////////////////////////////////////////
// HP35 Operation (basic + extended)
////////////////////////////////////////////////////////////////////////////////

#define FUNC_KEY  (HP35::OpNONE - 1)
#define MENU_MATH (HP35::OpNONE - 2)
#define MENU_TRIG (HP35::OpNONE - 3)
#define TRIG_ASIN (HP35::OpNONE - 4)
#define TRIG_ACOS (HP35::OpNONE - 5)
#define TRIG_ATAN (HP35::OpNONE - 6)

const uint8_t main_operations[16 + 16] PROGMEM =
{
	HP35::OpNUM0, HP35::OpNUM1, HP35::OpNUM2, HP35::OpNUM3, HP35::OpNUM4, HP35::OpNUM5, HP35::OpNUM6, HP35::OpNUM7,
	HP35::OpNUM8, HP35::OpNUM9, HP35::OpDOT,  HP35::OpPUSH, HP35::OpCLX,  HP35::OpCHS,  HP35::OpEEX,  FUNC_KEY,

	HP35::OpNONE, HP35::OpRCL,  HP35::OpSTO,  HP35::OpSUB,  HP35::OpPI,   HP35::OpNONE, HP35::OpMUL,  MENU_TRIG,
	HP35::OpNONE, HP35::OpDIV,  HP35::OpSWAP, HP35::OpADD,  HP35::OpCLR,  HP35::OpROT,  HP35::OpNONE, MENU_MATH
};

const uint8_t math_operations[6] PROGMEM =
{
	HP35::OpPOW, HP35::OpSQRT, HP35::OpINV,
	HP35::OpLOG, HP35::OpLN,   HP35::OpEXP
};

const uint8_t trig_operations[6] PROGMEM =
{
	HP35::OpSIN, HP35::OpCOS,  HP35::OpTAN,
	TRIG_ASIN,   TRIG_ACOS,    TRIG_ATAN
};

////////////////////////////////////////////////////////////////////////////////
// Fonts
////////////////////////////////////////////////////////////////////////////////

const uint8_t font5x8[] PROGMEM =
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
	0b01100000,
	0b01100000,
	0b00000000,
	0b00000000,
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

const TXT::Font font PROGMEM =
{
	.widthBytes = 5,
	.heightRows = 1,
	.asciiFirst = 0x2A,
	.asciiLast  = 0x5A,
	.data = font5x8
};

////////////////////////////////////////////////////////////////////////////////
// Strings
////////////////////////////////////////////////////////////////////////////////

#define MSG_ERR 0
#define MSG_INF 1

const char strMessage[] PROGMEM =
	"\03" "ERR" "INF";

const char strMonth[] PROGMEM = 
	"\03"
	"JAN" "FEB" "MAR" "APR" "MAY" "JUN"
	"JUL" "AUG" "SEP" "OCT" "NOV" "DEC";

const char strMainOps[] PROGMEM = 
	"\03"
	".  " "DUP" "CLR" "NEG" "EEX";

const char strFuncOps[] PROGMEM = 
	"\03"
	"LST" "RCL" "STO" "-  " "CST"
	"M+ " "*  " "---" "---" "/  "
	"SWP" "+  " "AC " "RTD" "RTU";

#define MENU_OPS_PER_LINE 3

const char strMenuMath[] PROGMEM = 
	"\03"
	"Y; " "?,X" "1/X"  // X^Y, SQRT(X), 1/X
	"LOG" "LN " "E; "; // LOG(X), LN(X), e^X

const char strMenuTrig[] PROGMEM = 
	"\03"
	"SIN" "COS" "TAN"  // Sine, Cosine, Tangent
	"ASN" "ACS" "ATN"; // Inverse sine, Inverse cosine, Inverse tangent

struct Menu
{
	u08 lastIdx;
	const char * string;
	const uint8_t * opsBase;
};

const Menu menus[] PROGMEM =
{
	{ sizeof(math_operations) / MENU_OPS_PER_LINE - 1, strMenuMath, math_operations },
	{ sizeof(trig_operations) / MENU_OPS_PER_LINE - 1, strMenuTrig, trig_operations },
};

////////////////////////////////////////////////////////////////////////////////

b08 isFunc;
b08 isMenu;

Menu menu;
u08  select;

enum { MENU_MATH_OPS, MENU_TRIG_OPS };

////////////////////////////////////////////////////////////////////////////////

#define CHAR_SHIFT  '='



void PrintStack()
{
	TXT::SetFont(font, SCALE_X1, SCALE_X4);
	for (u08 x = 0, i = 0; i < 15; ++i)
	{
		TXT::PrintChar(HP35::Display[i], x, 0);
		TXT::NextCharPos(x);
	}
}

void PrintCalculator()
{
	LCD::Clear();
	TXT::SetFont(font, SCALE_X1, SCALE_X1);

	if (isFunc)
	{
		TXT::PrintChar(CHAR_SHIFT, 127 - 5, 0);
	}

	if (isMenu)
	{
		TXT::SetScale(SCALE_X2, SCALE_X2);
		for (u08 i = 0; i < MENU_OPS_PER_LINE; ++i)
		{
			TXT::PrintString(FPSTR(menu.string), select * MENU_OPS_PER_LINE + i, 48 * i, 2);
		}
	}
	else
	{
		TXT::SetScale(SCALE_X2, SCALE_X4);
	}

	PrintStack();
	LCD::Flip();
}

////////////////////////////////////////////////////////////////////////////////

#define DIMOUT_MILLIS   10000 // Time before display dim out
#define POWEROFF_MILLIS 20000 // Time before power off

u08 key;
u08 oldkey;

void enterMenu(u08 type)
{
	isMenu = true;
	memcpy_P(&menu, &menus[type], sizeof(Menu));
	select = 0;
}

#define CALC_FRAMES_PER_SEC   (15)
#define HP35_CYCLES_PER_FRAME (HP35_CYCLES_PER_SEC / CALC_FRAMES_PER_SEC)

const u08 seqASIN[] PROGMEM = { HP35::OpARC, HP35::OpSIN, HP35::OpNONE };
const u08 seqACOS[] PROGMEM = { HP35::OpARC, HP35::OpCOS, HP35::OpNONE };
const u08 seqATAN[] PROGMEM = { HP35::OpARC, HP35::OpTAN, HP35::OpNONE };

const u08 * hp35seq = 0;

void executeSequence(const u08 * seq)
{
	hp35seq = seq;
}

void executeOperation(u08 operation)
{
	isFunc = false;
	isMenu = false;

	switch (operation)
	{
		default:
			HP35::Operation(operation);
			break;

		case FUNC_KEY:
			isFunc = true;
			break;

		case MENU_MATH:
			enterMenu(MENU_MATH_OPS);
			break;

		case MENU_TRIG:
			enterMenu(MENU_TRIG_OPS);
			break;
		
		case TRIG_ASIN:
			executeSequence(seqASIN);
			break;

		case TRIG_ACOS:
			executeSequence(seqACOS);
			break;
		
		case TRIG_ATAN:
			executeSequence(seqATAN);
			break;
	}
}

u16 cycles = 0;

void updateCalcMode()
{
	bool print = false;

	if (isMenu)
	{
		if (key != KEY_NONE)
		{
			switch(key)
			{
				default: isMenu = false; break;
				case KEY_EEXP: if (select > 0) select--; else select = menu.lastIdx; break;
				case KEY_NEG:  if (select < menu.lastIdx) select++; else select = 0; break;
				case KEY_FUNC: enterMenu(MENU_MATH_OPS); break;
				case KEY_NUM7: enterMenu(MENU_TRIG_OPS); break;
				case KEY_NUM1: case KEY_NUM2: case KEY_NUM3:
					u08 index = select * MENU_OPS_PER_LINE + (key - KEY_NUM1);
					u08 calcOp = pgm_read_byte(menu.opsBase + index);
					executeOperation(calcOp);
					break;
			}
			PrintCalculator();
		}
	}
	else
	{
		if (hp35seq)
		{
			if (!cycles && HP35::Idling)
			{
				u08 hp35op = pgm_read_byte(hp35seq++);
				if (hp35op == HP35::OpNONE) hp35seq = 0;
				else HP35::Operation(hp35op);
			}
		}

		else if (key != KEY_NONE)
		{
			u08 calcOp = pgm_read_byte(main_operations + (isFunc ? 16 : 0) + key);
			executeOperation(calcOp);
			PrintCalculator();
		}
		
		for (cycles += HP35_CYCLES_PER_FRAME; cycles > 0; --cycles)
		{
			if (HP35::Cycle()) 
			{
				PrintCalculator();
				break;
			}
		}
	}
}

b08 calcMode;
u08 battery;

void switchToCalcMode(bool yes = true)
{
	calcMode = yes;
	FPS::SyncStart();
}

void switchToRTCMode()
{
	LCD::TurnOn();
	battery = (uint8_t)((PWR::Level() * 4 + 50) / 100);
	switchToCalcMode(false);
	oldkey = KBD::Read();
}

NOINLINE void setupAndSwitchToRTCMode()
{
	RTC::WriteTimeDate();
	switchToRTCMode();
}

void PrintClock()
{
	LCD::Clear();
	TXT::SetFont(font, SCALE_X2, SCALE_X4);

	TXT::PrintChar(':', 20, 0);
	TXT::PrintChar(':', 47, 0);
	TXT::PrintTensOnes(RTC::Hours, 0, 0);
	TXT::PrintTensOnes(RTC::Minutes, 27, 0);
	TXT::PrintTensOnes(RTC::Seconds, 54, 0);

	TXT::SetScale(SCALE_X1, SCALE_X1);
	TXT::PrintString(FPSTR(strMonth), RTC::Month - 1, 79, 0);
	TXT::PrintTensOnes(RTC::Date, 99, 0);
	TXT::PrintTensOnes(RTC::Year, 117, 0);
	TXT::PrintChar('/', 111, 0);

	TXT::SetScale(SCALE_X2, SCALE_X1);
	TXT::PrintString(F("HP35"), 79, 2);
	u08 i = battery;
	while (i) TXT::PrintChar('-', 79 + (--i) * TXT::char_dx, 1);

	LCD::Flip();
}

void updateRTCMode()
{
	RTC::ReadTimeDate();
	PrintClock();
}

int main() 
{
	// init hardware and switch to rtc operation mode
	PCB::Init();
	setupAndSwitchToRTCMode();

	while (true)
	{
		// get time passed since last operation mode switch
		uint16_t timePassedMs = FPS::SyncMillis();

		// handle display brightness change
		LCD::Brightness(calcMode && timePassedMs < DIMOUT_MILLIS ? 0xFF : 0x00);

		// handle power down condition
		if (timePassedMs >= POWEROFF_MILLIS)
		{
			// power down and go to sleeping
			FPS::SyncStop();
			LCD::TurnOff();
			PWR::Down();

			// power up an switch to rtc operation mode
			switchToRTCMode();
		}

		// read key press and switch to calculator operation mode
		key = KBD::Read();
		if (key != oldkey) oldkey = key; else key = KEY_NONE;
		if (key != KEY_NONE) switchToCalcMode();

		// update current operation mode and idle until next frame
		if (calcMode) updateCalcMode(); else updateRTCMode();
		FPS::SyncWait();
	}
	return 0;
}
