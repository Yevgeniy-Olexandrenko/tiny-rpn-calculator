#pragma once

//#define debug

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
	".  " "DUP" "DRP" "NEG" "EEX";

const char strFuncOps[] PROGMEM = 
	"\03"
	"LST" "RCL" "STO" "-  " "CST"
	"M+ " "*  " "---" "---" "/  "
	"SWP" "+  " "AC " "RTD" "RTU";

#define MENU_OPS_PER_LINE 3

const char strMenuMath[] PROGMEM = 
	"\03"
	"X? " "?,X" "1/X"
	"10;" "LG " "Y; "
	"E; " "LN " ";,Y";

const char strMenuTrig[] PROGMEM = 
	"\03"
	"SIN" "COS" "TAN"  // Sine, Cosine, Tangent
	"ASN" "ACS" "ATN"  // Inverse sine, Inverse cosine, Inverse tangent
	"SNH" "CSH" "TNH"  // Hyperbolic sine, Hyperbolic cosine,  Hyperbolic tangent
	"ASH" "ACH" "ATH"; // Inverse hyperbolic sine, Inverse hyperbolic cosine, Inverse hyperbolic tangent

const char strMenuProg[] PROGMEM = 
	"\03"
	"<1 " "<2 " "<3 "
	"EQ " "NE " "BEG"
	"GT " "LT " "UNT"
	"IF " "ELS" "THN"
	"@1 " "@2 " "@3 ";

const char strMenuSets[] PROGMEM = 
	"\03"
	"D/R" "STM" "SDT"
	"END";

struct Menu
{
	const char* string;
	u08 lastIdx;
	u08 opsBase;
};

const Menu menus[] PROGMEM =
{
	{ strMenuMath, 2, MATH_OPS },
	{ strMenuTrig, 3, TRIG_OPS },
	{ strMenuProg, 4, PROG_OPS },
	{ strMenuSets, 0, SETS_OPS },
};

////////////////////////////////////////////////////////////////////////////////

Menu menu;
u08  select;

////////////////////////////////////////////////////////////////////////////////

#define CHAR_SHIFT  '='
#define CHAR_RECORD '@'
#define CHAR_PLAY   '<'

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

void PrintStack(u08 i, u08 s, u08 y)
{
	f32 f = dget(i);
	PrintCharSize(CHAR_SIZE_M, s);

	if (isnan(f)) { PrintStringAt(FPSTR(strMessage), MSG_ERR, M_DIGIT_FST, y);	}
	else
	{
		if (f < 0) { f = -f; PrintCharAt('-', M_SIGN, y); }
		if (isinf(f)) {	PrintStringAt(FPSTR(strMessage), MSG_INF, M_DIGIT_FST, y); }
		else
		{
			s08 e = (s08)(log(f) / log(10.f));
			u32 m = (u32)(f / _p10(e - (DIGITS - 1)) + 0.5f);

			if (m > 0 && m < _p10(DIGITS - 1))
			{
				m = (u32)(f / _p10(--e - (DIGITS - 1)) + 0.5f);
			}

			s08 int_dig = 1, lead_z = 0;
			if (_abs(e) < DIGITS)
			{
				if (e >= 0)
				{
					int_dig += e;
				}
				else
				{
					int_dig = 0;
					lead_z -= e;
					for (u08 n = lead_z; n--; m /= 10);
				}
				e = 0;
			}
			s08 fra_dig = DIGITS - lead_z - int_dig;

			u08 x = M_DIGIT_LST, nonzero = false;
			for (; fra_dig--; m /= 10, x -= dx)
			{
				u08 ones = _ones(m);
				if (ones || nonzero)
				{
					PrintCharAt('0' + ones, x, y);
					nonzero = true;
				}
			}

			if (nonzero)
			{
				for (; --lead_z > 0; x -= dx) PrintCharAt('0', x, y);
				PrintCharAt('.', x, y);
			}
			
			PrintCharAt('0', x -= POINT_WIDTH, y);
			for (; int_dig--; m /= 10, x -= dx) PrintCharAt('0' + _ones(m), x, y);

			if (e)
			{
				PrintCharSize(CHAR_SIZE_M, s >> 1);
				if (e < 0) { e = -e; PrintCharAt('-', E_SIGN, y); }
				PrintTensOnesAt(e, E_DIGIT1, y);
			}
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

void getOperationStr(Operation op);
char buf[4];

void PrintCalculator()
{
	LCD_Clear();
#ifdef debug
	if (ap)
	{
		PrintStack(3, CHAR_SIZE_S, 0);
		PrintStack(2, CHAR_SIZE_S, 1);
		PrintStack(1, CHAR_SIZE_S, 2);
		PrintStack(0, CHAR_SIZE_S, 3);
	}
	else
#endif
	{
		PrintCharSize(CHAR_SIZE_S, CHAR_SIZE_S);
		PrintCharAt(isFunc ? CHAR_SHIFT : (isDeg ? 'D' : 'R'), MODE_CHAR, 0);

		if (ap) PrintCharAt(CHAR_PLAY, MODE_CHAR, 1);

		if (isMenu)
		{
			PrintCharSize(CHAR_SIZE_M, CHAR_SIZE_M);
			for (u08 i = 0; i < MENU_OPS_PER_LINE; ++i)
			{
				PrintStringAt(FPSTR(menu.string), select * MENU_OPS_PER_LINE + i, 48 * i, 2);
			}
			PrintStack(0, CHAR_SIZE_M, 0);
		}
		else if (isFunc)
		{
			PrintStack(1, CHAR_SIZE_M, 0);
			PrintStack(0, CHAR_SIZE_M, 2);
		}
		else
		{
			//PrintStack(0, CHAR_SIZE_L, 0);

			getOperationStr(last_op);

			PrintCharSize(CHAR_SIZE_M, CHAR_SIZE_M);
			PrintStringAt("P1", 0, 2);
			PrintTensOnesAt(0x00, 48, 2);
			PrintStringAt(buf, 48+48, 2);
			PrintStack(0, CHAR_SIZE_M, 0);
		}
	}

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

void getOperationStr(Operation op)
{
	if (op < OpDot)
	{
		buf[0] = '0' + op;
		buf[1] = 0;
	}
	else
	{
		const char* ops;

		if      (op < FUNC_OPS) { ops = strMainOps;  op -= OpDot;    }
		else if (op < MATH_OPS) { ops = strFuncOps;  op -= FUNC_OPS; }
		else if (op < TRIG_OPS) { ops = strMenuMath; op -= MATH_OPS; }
		else if (op < PROG_OPS) { ops = strMenuTrig; op -= TRIG_OPS; }
		else if (op < SETS_OPS) { ops = strMenuProg; op -= PROG_OPS; }
		else                    { ops = strMenuSets; op -= SETS_OPS; }

		uint8_t sz = pgm_read_byte(ops++);
		memcpy_P(&buf[0], &ops[op * sz], sz);
		buf[sz] = 0;
	}
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
	battery = (uint8_t)(PWR_Level() * 4 + 0.5f);
	switchToCalcMode(false);
	oldkey = KBD_Read();
}

NOINLINE void setupAndSwitchToRTCMode()
{
	RTC_WriteTimeDate();
	switchToRTCMode();
}

void updateCalcMode()
{
	if (ap)
	{
#ifdef debug
		if (ap && isScript() && key != KEY_NONE)
			ExecuteOperation(pgm_read_byte(&scripts[pp++]));
#else
		while (ap && isScript())
		{
			// execute scripted operations
			ExecuteOperation(pgm_read_byte(&scripts[pp++]));
		}
#endif
		if (ap)
		{
			// execute user program
			// TODO
		}
		PrintCalculator();
	}

	else if (key != KEY_NONE)
	{
		if (isMenu)
		{
			switch(key)
			{
			default: isMenu = false; break;
			case KEY_EEXP: if (select > 0) select--; else select = menu.lastIdx; break;
			case KEY_NEG:  if (select < menu.lastIdx) select++; else select = 0; break;
			case KEY_FUNC: enterMenu(MENU_MATH_OPS); break;
			case KEY_NUM7: enterMenu(MENU_TRIG_OPS); break;
			case KEY_NUM8: enterMenu(MENU_PROG_OPS); break;
			case KEY_DOT:  enterMenu(MENU_SETS_OPS); break;
			case KEY_NUM1: case KEY_NUM2: case KEY_NUM3:
				u08 index = select * MENU_OPS_PER_LINE + (key - KEY_NUM1);
				ExecuteOperation(menu.opsBase + index);
				break;
			}
		}
		else
		{
			ExecuteOperation(isFunc ? FUNC_OPS + key : key);
		}
		PrintCalculator();
	}
}

void updateRTCMode()
{
	RTC_ReadTimeDate();
	PrintClock();
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
