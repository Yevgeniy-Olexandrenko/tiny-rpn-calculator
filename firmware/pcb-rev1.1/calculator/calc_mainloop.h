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
	",X+" "Y;+" "1/X"  // Squareroot, Raise to the power of, Reciprocal
	"EXP" "LN+" "X?+"  // Exponential, Natural logarithm, Gamma function (due to Nemes)
	"R>P" "P>R" "PV+"  // Rectangular to polar coordinates, Polar to rectangular coordinates, Present value (annuity)
	"ND+" "STA" "LR+"; // Normal distribution (CDF/PDF), Statistics, Linear regression

const char strMenuTrig[] PROGMEM = 
	"\03"
	"SIN" "COS" "TAN"  // Sine, Cosine, Tangent
	"ASN" "ACS" "ATN"  // Inverse sine, Inverse cosine, Inverse tangent
	"SNH" "CSH" "TNH"  // Hyperbolic sine, Hyperbolic cosine,  Hyperbolic tangent
	"ASH" "ACH" "ATH"; // Inverse hyperbolic sine, Inverse hyperbolic cosine, Inverse hyperbolic tangent

const char strMenuProg[] PROGMEM = 
	"\03"
	"OP1" "OP2" "OP3";

const char strMenuSets[] PROGMEM = 
	"\03"
	"OP1" "OP2" "OP3";

struct Menu
{
	const char* string;
	u08 lastIdx;
	u08 opsBase;
};

const Menu menus[] PROGMEM =
{
	{ strMenuMath, 3, MATH_OPS },
	{ strMenuTrig, 3, TRIG_OPS },
	{ strMenuProg, 0, PROG_OPS },
	{ strMenuSets, 0, SETS_OPS },
};

////////////////////////////////////////////////////////////////////////////////

Menu menu;
u08  select;

////////////////////////////////////////////////////////////////////////////////

#define CHAR_SHIFT  '='
#define CHAR_REC    '@'
#define CHAR_PLAY   '<'

#define DIGITS      (6)
#define DIGIT_WIDTH (FONT_WIDTH * CHAR_SIZE_M + 1)
#define POINT_WIDTH (DIGIT_WIDTH - 6)

#define MODE_CHAR   (128 - (DIGIT_WIDTH - 1))

#define M_SIGN      (0)
#define M_DIGIT_FST (M_SIGN + DIGIT_WIDTH)
#define M_DIGIT_LST (M_DIGIT_FST + ((DIGITS - 1) * DIGIT_WIDTH) + POINT_WIDTH)

#define E_DIGIT2    (MODE_CHAR - DIGIT_WIDTH)
#define E_DIGIT1    (E_DIGIT2 - DIGIT_WIDTH)
#define E_SIGN      (E_DIGIT1 - DIGIT_WIDTH)

#define MODE_CHAR   (128 - (DIGIT_WIDTH - 1))

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
			u32 m = (u32)(f / pow10(e - (DIGITS - 1)) + 0.5f);

			if (m > 0 && m < pow10(DIGITS - 1))
			{
				m = (u32)(f / pow10(--e - (DIGITS - 1)) + 0.5f);
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
	DisplayClear();

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

	DisplayRefresh();
}

void PrintCalculator()
{
	DisplayClear();

#if 0
	if (isFunc) PrintCharAt(CHAR_SHIFT, MODE_CHAR, 0);

	PrintStack(3, CHAR_SIZE_S, 0);
	PrintStack(2, CHAR_SIZE_S, 1);
	PrintStack(1, CHAR_SIZE_S, 2);
	PrintStack(0, CHAR_SIZE_S, 3);
#else
	PrintCharSize(CHAR_SIZE_M, CHAR_SIZE_M);
//	if (isTypeRecording) PrintCharAt(CHAR_REC, MODE_CHAR, 2);
//	if (isTypePlaying) PrintCharAt(CHAR_PLAY, MODE_CHAR, 2);
	if (isFunc) PrintCharAt(CHAR_SHIFT, MODE_CHAR, 0);
	
	if (isMenu)
	{
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
#endif

	DisplayRefresh();
}

////////////////////////////////////////////////////////////////////////////////

#define DIMOUT_FRAMES 156   // Frames before display dim out (about 10 sec)
#define POWEROFF_FRAMES 469 // Frames before power off (about 30 sec)

b08 isfirstrun; // Allows first run of loop and printscreen without key query
u08 key; // Holds entered key
u08 oldkey; // Use for debouncing
b08 inCalcMode;
u08 brightness;

void enterMenu(u08 type)
{
	isMenu = true;
	memcpy_P(&menu, &menus[type], sizeof(Menu));
	select = 0;
}

void switchToCalcMode(bool yes)
{
	inCalcMode = yes;
	FrameSyncEnable();
}

int main() 
{
	ADCInit();
	I2CBusInit();
	DisplayInit();
	KeyboardInit();
	sei();

	switchToCalcMode(false);
	DisplayTurnOn();

	// reset
	isfirstrun = true;
	brightness = eeprom_read_byte(&eeprom_brightness);
	
	for (;;)
	{
		if (isfirstrun)
		{
			RTCWrite();

			// TODO
		}
		else
		{
			key = KeyboardRead();
			if (key == oldkey) key = KEY_NONE; else oldkey = key;
		}

		if (key != KEY_NONE)
		{
			switchToCalcMode(true);
		}

		if (frameCounter >= POWEROFF_FRAMES)
		{
			DisplayTurnOff();
			FrameSyncDisable();
			PowerDown();

			oldkey = KeyboardRead();
			switchToCalcMode(!RTCRead());
			DisplayTurnOn();
		}

		DisplayBrightness(frameCounter < DIMOUT_FRAMES ? brightness : 0);

		if (inCalcMode)
		{
			if (key != KEY_NONE || isfirstrun)
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
				isfirstrun = false;
			}
		}
		else
		{
			RTCRead();
			PrintClock();
		}
		FrameSyncWait();
	}
	return 0;
}
