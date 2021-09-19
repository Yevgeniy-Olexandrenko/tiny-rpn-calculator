#pragma once

////////////////////////////////////////////////////////////////////////////////
// Key Codes (0x00-0x09 numbers, 0x0A-0x0F special keys)
////////////////////////////////////////////////////////////////////////////////

enum 
{
	KEY_FUNC = KBD_A0, KEY_NUM7 = KBD_B0, KEY_NUM8 = KBD_C0, KEY_NUM9 = KBD_D0,
	KEY_EEXP = KBD_A1, KEY_NUM4 = KBD_B1, KEY_NUM5 = KBD_C1, KEY_NUM6 = KBD_D1,
	KEY_NEG  = KBD_A2, KEY_NUM1 = KBD_B2, KEY_NUM2 = KBD_C2, KEY_NUM3 = KBD_D2,
	KEY_DROP = KBD_A3, KEY_NUM0 = KBD_B3, KEY_DOT  = KBD_C3, KEY_DUP  = KBD_D3,
	KEY_NONE = KBD_NO
};

////////////////////////////////////////////////////////////////////////////////
// HP35 Operation (basic + extended)
////////////////////////////////////////////////////////////////////////////////

#define FUNC_KEY  (HP35_NONE - 1)
#define MENU_MATH (HP35_NONE - 2)
#define MENU_TRIG (HP35_NONE - 3)
#define TRIG_ASIN (HP35_NONE - 4)
#define TRIG_ACOS (HP35_NONE - 5)
#define TRIG_ATAN (HP35_NONE - 6)

const uint8_t main_operations[16 + 16] PROGMEM =
{
	HP35_NUM0, HP35_NUM1, HP35_NUM2, HP35_NUM3, HP35_NUM4, HP35_NUM5, HP35_NUM6, HP35_NUM7,
	HP35_NUM8, HP35_NUM9, HP35_DOT,  HP35_PUSH, HP35_CLX,  HP35_CHS,  HP35_EEX,  FUNC_KEY,

	HP35_NONE, HP35_RCL,  HP35_STO,  HP35_SUB,  HP35_PI,   HP35_NONE, HP35_MUL,  MENU_TRIG,
	HP35_NONE, HP35_DIV,  HP35_SWAP, HP35_ADD,  HP35_CLR,  HP35_ROT,  HP35_NONE, MENU_MATH
};

const uint8_t math_operations[6] PROGMEM =
{
	HP35_POW, HP35_SQRT, HP35_INV,
	HP35_LOG, HP35_LN,   HP35_EXP
};

const uint8_t trig_operations[6] PROGMEM =
{
	HP35_SIN,  HP35_COS,  HP35_TAN,
	TRIG_ASIN, TRIG_ACOS, TRIG_ATAN
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

	if (HP35_Error)
	{
		PrintStringAt(FPSTR(strMessage), MSG_ERR, M_DIGIT_FST, 0);
	}
	else
	{
		u08 pos = 0;
		for (u08 i = 0; i < 15; i++)
		{
			if (i == 12)
			{
				PrintCharSize(CHAR_SIZE_S, ch >> 1);
			}

			PrintCharAt(HP35_Display[i], pos, 0);
			pos += ((FONT_WIDTH * CHAR_SIZE_S) + 1);
		}
	}
}

uint8_t battery;

void PrintClock()
{
	LCD_Clear();

	PrintCharSize(CHAR_SIZE_M, CHAR_SIZE_L);
	PrintCharAt(':', 20, 0);
	PrintCharAt(':', 47, 0);
	PrintTensOnesAt(rtc_hours, 0, 0);
	PrintTensOnesAt(rtc_minutes, 27, 0);
	PrintTensOnesAt(rtc_seconds, 54, 0);

	PrintCharSize(CHAR_SIZE_S, CHAR_SIZE_S);
	PrintStringAt(FPSTR(strMonth), rtc_month - 1, 85, 0);

	PrintCharSize(CHAR_SIZE_M, CHAR_SIZE_S);
	PrintTensOnesAt(rtc_date, 107, 0);
	PrintTensOnesAt(20, 85, 1);
	PrintTensOnesAt(rtc_year, 107, 1);

	uint8_t i = battery;
	while (i) PrintCharAt('-', 85 + (--i) * dx, 2);
	
	LCD_Flip();
}

void PrintCalculator()
{
	LCD_Clear();

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

	LCD_Flip();
}

////////////////////////////////////////////////////////////////////////////////

#define DIMOUT_MILLIS   10000 // Time before display dim out
#define POWEROFF_MILLIS 20000 // Time before power off

u08 key;
u08 oldkey;
b08 calcMode;

void enterMenu(u08 type)
{
	isMenu = true;
	memcpy_P(&menu, &menus[type], sizeof(Menu));
	select = 0;
}

void switchToCalcMode(bool yes = true)
{
	calcMode = yes;
	FPS_SyncStart();
}

void switchToRTCMode()
{
	LCD_TurnOn();
	RTC_ReadTemperature();
	battery = (uint8_t)((PWR_Level() * 4 + 50) / 100);
	switchToCalcMode(false);
	oldkey = KBD_Read();
}

NOINLINE void setupAndSwitchToRTCMode()
{
	RTC_WriteTimeDate();
	switchToRTCMode();
}

#define CALC_FRAMES_PER_SEC      (15)
#define HP35_CYCLE_TIME_US       (286)
#define HP35_CYCLES_PER_FRAME    (1000000 / HP35_CYCLE_TIME_US / CALC_FRAMES_PER_SEC)
#define OPERATION_UPDATE_CYCLES  (HP35_CYCLES_PER_FRAME * 3)
#define OPERATION_EXECUTE_CYCLES (HP35_CYCLES_PER_FRAME * 5)

void executeOperationAndWait(u08 operation)
{
	HP35_Execute(operation);
	HP35_Update(OPERATION_EXECUTE_CYCLES);
}

void executeOperation(u08 operation)
{
	isFunc = false;
	isMenu = false;

	switch (operation)
	{
		default:
			HP35_Execute(operation);
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
			executeOperationAndWait(HP35_ARC);
			executeOperationAndWait(HP35_SIN);
			break;

		case TRIG_ACOS:
			executeOperationAndWait(HP35_ARC);
			executeOperationAndWait(HP35_COS);
			break;
		
		case TRIG_ATAN:
			executeOperationAndWait(HP35_ARC);
			executeOperationAndWait(HP35_TAN);
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
		else if (HP35_Update(OPERATION_UPDATE_CYCLES))
		{
			PrintCalculator();
		}
	}
}

void updateRTCMode()
{
	if (RTC_ReadTimeDate())
	{
		PrintClock();
	}
}

int main() 
{
	// init hardware and switch to rtc operation mode
	PCB_Init();
	setupAndSwitchToRTCMode();

	while (true)
	{
		// get time passed since last operation mode switch
		uint16_t timePassedMs = FPS_SyncMillis();

		// handle display brightness change
		LCD_Brightness(calcMode && timePassedMs < DIMOUT_MILLIS ? 0xFF : 0x00);

		// handle power down condition
		if (timePassedMs >= POWEROFF_MILLIS)
		{
			// power down and go to sleeping
			FPS_SyncStop();
			LCD_TurnOff();
			PWR_Down();

			// power up an switch to rtc operation mode
			switchToRTCMode();
		}

		// read key press and switch to calculator operation mode
		key = KBD_Read();
		if (key != oldkey) oldkey = key; else key = KEY_NONE;
		if (key != KEY_NONE) switchToCalcMode();

		// update current operation mode and idle until next frame
		if (calcMode) updateCalcMode(); else updateRTCMode();
		FPS_SyncWait();
	}
	return 0;
}
