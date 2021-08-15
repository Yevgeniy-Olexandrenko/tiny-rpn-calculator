
//#define debug

////////////////////////////////////////////////////////////////////////////////
// Key Codes (0x00-0x09 numbers, 0x0A-0x0F special keys)
////////////////////////////////////////////////////////////////////////////////

enum 
{
	KEY_FUNC = KEY_A0, KEY_NUM7 = KEY_B0, KEY_NUM8 = KEY_C0, KEY_NUM9 = KEY_D0,
	KEY_EEXP = KEY_A1, KEY_NUM4 = KEY_B1, KEY_NUM5 = KEY_C1, KEY_NUM6 = KEY_D1,
	KEY_NEG  = KEY_A2, KEY_NUM1 = KEY_B2, KEY_NUM2 = KEY_C2, KEY_NUM3 = KEY_D2,
	KEY_DROP = KEY_A3, KEY_NUM0 = KEY_B3, KEY_DOT  = KEY_C3, KEY_DUP  = KEY_D3,
	KEY_NONE = KEY_NO
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

#define MENU_OPS_PER_LINE 3

const char strMenuMath[] PROGMEM = 
	"\03"
	"X?+" "?,X" "1/X"
	"10;" "LG+" "Y;+"
	"E;+" "LN+" ";,Y";

const char strMenuTrig[] PROGMEM = 
	"\03"
	"SIN" "COS" "TAN"  // Sine, Cosine, Tangent
	"ASN" "ACS" "ATN"  // Inverse sine, Inverse cosine, Inverse tangent
	"SNH" "CSH" "TNH"  // Hyperbolic sine, Hyperbolic cosine,  Hyperbolic tangent
	"ASH" "ACH" "ATH"; // Inverse hyperbolic sine, Inverse hyperbolic cosine, Inverse hyperbolic tangent

const char strMenuProg[] PROGMEM = 
	"\03"
	"---" "---" "---"
	"EQ+" "NE+" "---"
	"GT+" "LT+" "---"
	"IF+" "ELS" "THN";

const char strMenuSets[] PROGMEM = 
	"\03"
	"D/R" "TIM" "DAT";

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
	{ strMenuProg, 3, PROG_OPS },
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
			for (; fra_dig--; m /= 10, x -= DIGIT_WIDTH)
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
				for (; --lead_z > 0; x -= DIGIT_WIDTH) PrintCharAt('0', x, y);
				PrintCharAt('.', x, y);
			}
			
			PrintCharAt('0', x -= POINT_WIDTH, y);
			for (; int_dig--; m /= 10, x -= DIGIT_WIDTH) PrintCharAt('0' + _ones(m), x, y);

			if (e)
			{
				PrintCharSize(CHAR_SIZE_M, s >> 1);
				if (e < 0) { e = -e; PrintCharAt('-', E_SIGN, y); }
				PrintTwoDigitAt(e, E_DIGIT1, y);
			}
		}
	}
}

void PrintClock()
{
	LCD_Clear();

	PrintCharSize(CHAR_SIZE_M, CHAR_SIZE_L);
	PrintCharAt(':', 20, 0);
	PrintCharAt(':', 47, 0);
	PrintTwoDigitAt(rtc_hours, 0, 0);
	PrintTwoDigitAt(rtc_minutes, 27, 0);
	PrintTwoDigitAt(rtc_seconds, 54, 0);

	PrintCharSize(CHAR_SIZE_S, CHAR_SIZE_S);
	PrintStringAt(FPSTR(strMonth), rtc_month - 1, 85, 0);

	PrintCharSize(CHAR_SIZE_M, CHAR_SIZE_S);
	PrintTwoDigitAt(rtc_date, 107, 0);
	PrintTwoDigitAt(20, 85, 1);
	PrintTwoDigitAt(rtc_year, 107, 1);

	LCD_Flip();
}

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
			PrintStack(0, CHAR_SIZE_L, 0);
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

void switchToCalcMode()
{
	calcMode = true;
	FrameSyncStart(FRAME_TIMEOUT_64MS);
}

void switchToRTCMode()
{
	calcMode = false;
	FrameSyncStart(FRAME_TIMEOUT_1S);
}

void setupAndSwitchToRTCMode()
{
	RTC_WriteDateAndTime();
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
	RTC_ReadDateAndTime();
	PrintClock();
}

int main() 
{
	ADC_Init();
	I2C_Init();
	LCD_Init();
	KBD_Init();
	sei();

	LCD_TurnOn();
	setupAndSwitchToRTCMode();

	while (true)
	{
		key = KBD_Read();
		if (key != oldkey) oldkey = key; else key = KEY_NONE;
		if (key != KEY_NONE) switchToCalcMode();

		uint16_t timePassedMs = FrameTimePassedMs();

		if (timePassedMs >= POWEROFF_MILLIS || PWR_BatteryLevel() == 0)
		{
			FrameSyncStop();
			LCD_TurnOff();
			PWR_Down();

			/* SLEEPING */

			oldkey = KBD_Read();
			LCD_TurnOn();
			switchToRTCMode();
		}

		LCD_Brightness(calcMode && timePassedMs < DIMOUT_MILLIS ? 0xFF : 0x00);

		if (calcMode) updateCalcMode(); else updateRTCMode();
		FrameSyncWait();
	}
	return 0;
}
