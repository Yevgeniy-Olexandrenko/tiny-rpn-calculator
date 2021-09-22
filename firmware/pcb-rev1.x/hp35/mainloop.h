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

#define DIGITS      (6)
#define DIGIT_WIDTH ((FONT_WIDTH * CHAR_SIZE_M) + 1)
#define POINT_WIDTH (DIGIT_WIDTH - 6)

#define M_SIGN      (0)
#define M_DIGIT_FST (M_SIGN + DIGIT_WIDTH)
#define M_DIGIT_LST (M_DIGIT_FST + ((DIGITS - 1) * DIGIT_WIDTH) + POINT_WIDTH)

#define MODE_CHAR   (127 - FONT_WIDTH)
#define E_DIGIT2    (MODE_CHAR - 1 - DIGIT_WIDTH)
#define E_DIGIT1    (E_DIGIT2 - DIGIT_WIDTH)
#define E_SIGN      (E_DIGIT1 - DIGIT_WIDTH)

void PrintStack()
{
	PrintCharSize(CHAR_SIZE_S, ch);

	// if (HP35_Error)
	// {
	// 	PrintStringAt(FPSTR(strMessage), MSG_ERR, M_DIGIT_FST, 0);
	// }
	// else
	{
		u08 pos = 0;
		for (u08 i = 0; i < 15; i++)
		{
			// if (i == 12)
			// {
			// 	PrintCharSize(CHAR_SIZE_S, ch >> 1);
			// }

			PrintCharAt(HP35::Display[i], pos, 0);
			//pos += ((FONT_WIDTH * CHAR_SIZE_S) + 1);
			pos += 7 + 1;
		}
	}
}

void PrintCalculator()
{
	LCD::Clear();

	if (isFunc)
	{
		PrintCharSize(CHAR_SIZE_S, CHAR_SIZE_S);
		PrintCharAt(CHAR_SHIFT, MODE_CHAR, 0);
	}

	if (isMenu)
	{
		PrintCharSize(CHAR_SIZE_M, CHAR_SIZE_M);
		for (u08 i = 0; i < MENU_OPS_PER_LINE; ++i)
		{
			PrintStringAt(FPSTR(menu.string), select * MENU_OPS_PER_LINE + i, 48 * i, 2);
		}
	}
	else
	{
		PrintCharSize(CHAR_SIZE_M, CHAR_SIZE_L);
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
#define HP35_OPERATION_CYCLES (HP35_CYCLES_PER_FRAME * 5)

void HP35_OperationWithWait(u08 operation)
{
	HP35::Operation(operation);
	for (u16 i = 0; i < HP35_OPERATION_CYCLES; ++i) HP35::Cycle();
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
			HP35_OperationWithWait(HP35::OpARC);
			HP35::Operation(HP35::OpSIN);
			break;

		case TRIG_ACOS:
			HP35_OperationWithWait(HP35::OpARC);
			HP35::Operation(HP35::OpCOS);
			break;
		
		case TRIG_ATAN:
			HP35_OperationWithWait(HP35::OpARC);
			HP35::Operation(HP35::OpTAN);
			break;
	}
}

void updateCalcMode()
{
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
					u08 operation = pgm_read_byte(menu.opsBase + index);
					executeOperation(operation);
					break;
			}
			PrintCalculator();
		}
	}
	else
	{
		if (key != KEY_NONE)
		{
			u08 operation = pgm_read_byte(main_operations + (isFunc ? 16 : 0) + key);
			executeOperation(operation);
			PrintCalculator();
		}
		else
		{
			for (u16 i = 0; i < HP35_CYCLES_PER_FRAME; ++i)
			{
				if (HP35::Cycle()) PrintCalculator();
			}
		}
	}
}

#if SUPPORT_RTC
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

	PrintCharSize(CHAR_SIZE_M, CHAR_SIZE_L);
	PrintCharAt(':', 20, 0);
	PrintCharAt(':', 47, 0);
	PrintTensOnesAt(RTC::Hours, 0, 0);
	PrintTensOnesAt(RTC::Minutes, 27, 0);
	PrintTensOnesAt(RTC::Seconds, 54, 0);

	PrintCharSize(CHAR_SIZE_S, CHAR_SIZE_S);
	PrintStringAt(FPSTR(strMonth), RTC::Month - 1, 85, 0);

	PrintCharSize(CHAR_SIZE_M, CHAR_SIZE_S);
	PrintTensOnesAt(RTC::Date, 107, 0);
	PrintTensOnesAt(20, 85, 1);
	PrintTensOnesAt(RTC::Year, 107, 1);

	u08 i = battery;
	while (i) PrintCharAt('-', 85 + (--i) * dx, 2);
	
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
#else
void switchToCalcMode()
{
	LCD::TurnOn();
	FPS::SyncStart();
	oldkey = KBD::Read();
}

int main() 
{
	// init hardware and switch to calculator operation mode
	PCB::Init();
	switchToCalcMode();

	while (true)
	{
		// get time passed since last operation mode switch
		uint16_t timePassedMs = FPS::SyncMillis();

		// handle display brightness change
		LCD::Brightness(timePassedMs < DIMOUT_MILLIS ? 0xFF : 0x00);

		// handle power down condition
		if (timePassedMs >= POWEROFF_MILLIS)
		{
			// power down and go to sleeping
			FPS::SyncStop();
			LCD::TurnOff();
			PWR::Down();

			// power up an switch to calculator operation mode
			switchToCalcMode();
		}

		// read key press and switch to calculator operation mode
		key = KBD::Read();
		if (key != oldkey) oldkey = key; else key = KEY_NONE;
		if (key != KEY_NONE) FPS::SyncStart();

		// update current operation mode and idle until next frame
		updateCalcMode();
		FPS::SyncWait();
	}
	return 0;
}
#endif
