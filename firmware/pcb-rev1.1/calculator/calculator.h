
#define M_E  2.7182818284590452354
#define M_PI 3.1415926535897932384

b08 isFunc = false;
b08 isMenu = false;

#define DATA_STACK_SIZE 24
#define DATA_STACK_CPSZ ((DATA_STACK_SIZE - 1) * sizeof(f32))
f32 ds[DATA_STACK_SIZE];
u08 dp = 0;

f32 EEMEM eeprom_storage[10];
u08 EEMEM eeprom_brightness = 0xFF;

b08 isNewNum = true; // True if stack has to be lifted before entering a new number
u08 decimals = 0;    // Number of decimals entered (input after decimal dot)
b08 isDot = false;   // True if dot was pressed and decimals will be entered

////////////////////////////////////////////////////////////////////////////////
// Key Codes (0x00-0x09 numbers, 0x0A-0x0F special keys)
////////////////////////////////////////////////////////////////////////////////

typedef enum {
	KEY_FUNC = KEY_A0, KEY_NUM7 = KEY_B0, KEY_NUM8 = KEY_C0, KEY_NUM9 = KEY_D0,
	KEY_EEXP = KEY_A1, KEY_NUM4 = KEY_B1, KEY_NUM5 = KEY_C1, KEY_NUM6 = KEY_D1,
	KEY_NEG  = KEY_A2, KEY_NUM1 = KEY_B2, KEY_NUM2 = KEY_C2, KEY_NUM3 = KEY_D2,
	KEY_DROP = KEY_A3, KEY_NUM0 = KEY_B3, KEY_DOT  = KEY_C3, KEY_DUP  = KEY_D3,
	KEY_NONE = KEY_NO
} Key;

////////////////////////////////////////////////////////////////////////////////
// Strings
////////////////////////////////////////////////////////////////////////////////

#define MSG_ERR 0
#define MSG_INF 1

const char PROGMEM strMessage[] =
	"\03" "ERR" "INF";

const char PROGMEM strMonth[] = 
	"\03"
	"JAN" "FEB" "MAR" "APR" "MAY" "JUN"
	"JUL" "AUG" "SEP" "OCT" "NOV" "DEC";

////////////////////////////////////////////////////////////////////////////////
// Data Stack Operations
////////////////////////////////////////////////////////////////////////////////

f32 dpush(f32 d) 
{
	if (dp >= DATA_STACK_SIZE) 
	{
		memcpy(&ds[0], &ds[1], DATA_STACK_CPSZ);
		dp--;
	}
	return (ds[dp++] = d);
}

f32 dpop()
{
	return (dp ? ds[--dp] : 0.f);
}

b08 dpopChecked(u08& dest, u08 min, u08 max)
{
	f32 d = dpop();
	if (d >= min && d <= max) { dest = d; return true; }
	dpush(NAN);
	return false;
}

void rotateStack(b08 isUp)
{
	if (dp > 2)
	{
		f32 a = dpop(), b = dpop(), c = dpop();
		if (isUp)
		{	// 2 z -> y
			// 1 y -> x
			// 0 x -> z
			dpush(b); dpush(a); dpush(c);
		}
		else
		{	// 2 z -> x
			// 1 y -> z
			// 0 x -> y
			dpush(a); dpush(c); dpush(b);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// Enter and Clear the number in X Register of Data Stack
////////////////////////////////////////////////////////////////////////////////

void enterDigit(u08 digit)
{
	if (isDot) 
	{
		dpush(digit);
		dpush(pow10(++decimals));
		FnDiv();
		FnAdd();
	}
	else if (isNewNum)
	{
		dpush(digit);
	}
	else 
	{
		dpush(10.f);
		FnMul();
		dpush(digit);
		FnAdd();
	}
	isNewNum = false;
}

void clearEntry()
{
	if (isDot) 
	{
		if (decimals) 
		{
#if 0			
			dpush(pow10(--decimals));
			FnDup();
			FnInv();
			FnRotU();
			FnMul();
			FnInt();
			FnMul();
#else
			f32 a = pow10(--decimals);
			f32 d = s32(dpop() * a) / a;
			dpush(d);
#endif
		}
		else isDot = false;
	}
	else 
	{
		s32 a = s32(dpop() / 10.f);
		if (a) dpush(a); else isNewNum = true;
	}
}

////////////////////////////////////////////////////////////////////////////////
// Helping Math Functions
////////////////////////////////////////////////////////////////////////////////

f32 pow10(s08 e) 
{ 
	f32 f = 1.f;
	if (e > 0) 
		while (e--) f *= 10.f;
	else 
		while (e++) f /= 10.f;
	return f;
}

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////

void storageAccess(b08 isWrite) 
{
	u08 i;
	if (dpopChecked(i, 0, 9))
	{
		if (isWrite)
			eeprom_write_float(&eeprom_storage[i], dpop());
		else
			dush(eeprom_read_float(&eeprom_storage[i]));
	}
}

////////////////////////////////////////////////////////////////////////////////
// Operations Implementations
////////////////////////////////////////////////////////////////////////////////

typedef enum {
	// basic operations
	OpNum0, OpNum1, OpNum2, OpNum3, OpNum4, OpNum5, OpNum6, OpNum7,
	OpNum8, OpNum9, OpDot,  OpDup,  OpDrop, OpNeg,  OpEExp, AcFunc,

	// shifted operations
	AcNop,  OpRcl, OpSto,  OpSub, AcNop,  AcNop,  OpMul,  AcTrig,
	AcProg, OpDiv, OpSwap, OpAdd, OpAClr, OpRotD, OpRotU, AcMath
} Operation;

void FnNop()  { /* do nothing */ }
void FnNum0() { enterDigit(0); }
void FnNum1() { enterDigit(1); }
void FnNum2() { enterDigit(2); }
void FnNum3() { enterDigit(3); }
void FnNum4() { enterDigit(4); }
void FnNum5() { enterDigit(5); }
void FnNum6() { enterDigit(6); }
void FnNum7() { enterDigit(7); }
void FnNum8() { enterDigit(8); }
void FnNum9() { enterDigit(9); }
void FnDot()  { if (isNewNum) { dpush(decimals = 0); isNewNum = false; } isDot = true; }
void FnDup()  { if (isNewNum && dp) dpush(ds[dp - 1]); }
void FnDrop() { if (!isNewNum) clearEntry(); else if (dp) --dp; }
void FnNeg()  { dpush(-dpop()); }
void FnEExp() { dpush(pow10(dpop())); FnMul(); }
void FnFunc() { isFunc = true; }

void FnRcl()  { storageAccess(false); }
void FnSto()  { storageAccess(true); }
void FnSub()  { FnNeg(); FnAdd(); }
void FnMul()  { dpush(dpop() * dpop()); }
void FnTrig() { /* TODO */ }
void FnProg() { /* TODO */ }
void FnDiv()  { FnInv(); FnMul(); }
void FnSwap() { if (dp > 1) { f32 a = dpop(), b = dpop(); dpush(a); dpush(b); }}
void FnAdd()  { dpush(dpop() + dpop()); }
void FnAClr() { dp = 0; /* clear anything else */ }
void FnRotD() { rotateStack(false); }
void FnRotU() { rotateStack(true); }
void FnMath() { /* TODO */ }

void FnInv()  { dpush(1.f / dpop()); }
void FnInt()  { dpush(s32(dpop())); }

////////////////////////////////////////////////////////////////////////////////
// Operations Execution
////////////////////////////////////////////////////////////////////////////////

void (*dispatch[])() = 
{
	/* 00 */ &FnNum0, // Enter
	/* 01 */ &FnNum1, // Enter
	/* 02 */ &FnNum2, // Enter
	/* 03 */ &FnNum3, // Enter
	/* 04 */ &FnNum4, // Enter
	/* 05 */ &FnNum5, // Enter
	/* 06 */ &FnNum6, // Enter
	/* 07 */ &FnNum7, // Enter
	/* 08 */ &FnNum8, // Enter
	/* 09 */ &FnNum9, // Enter
	/* 0A */ &FnDot,
	/* 0B */ &FnDup,
	/* 0C */ &FnDrop, // Enter
	/* 0D */ &FnNeg,
	/* 0E */ &FnEExp,
	/* 0F */ &FnFunc, // Action

	/* 10 */ &FnNop,  // TODO
	/* 11 */ &FnRcl,
	/* 12 */ &FnSto,
	/* 13 */ &FnSub,
	/* 14 */ &FnNop,  // TODO
	/* 15 */ &FnNop,  // TODO
	/* 16 */ &FnMul,
	/* 17 */ &FnTrig, // Action
	/* 18 */ &FnProg, // Action
	/* 19 */ &FnDiv,
	/* 1A */ &FnSwap,
	/* 1B */ &FnAdd,
	/* 1C */ &FnAClr,
	/* 1D */ &FnRotD,
	/* 1E */ &FnRotU,
	/* 1F */ &FnMath, // Action

	/* 20 */ &FnNop,
	/* 21 */ &FnNeg,  // ASCII '!'
	/* 22 */ &FnNop,
	/* 23 */ &FnNop,
	/* 24 */ &FnNop,
	/* 25 */ &FnSwap, // ASCII '%'
	/* 26 */ &FnNop,
	/* 27 */ &FnNop,
	/* 28 */ &FnRotU, // ASCII '('
	/* 29 */ &FnRotD, // ASCII ')'
	/* 2A */ &FnMul,  // ASCII '*'
	/* 2B */ &FnAdd,  // ASCII '+'
	/* 2C */ &FnNop,
	/* 2D */ &FnSub,  // ASCII '-'
	/* 2E */ &FnDot,  // ASCII '.'
	/* 2F */ &FnDiv,  // ASCII '/'
	/* 30 */ &FnNum0, // ASCII '0'
	/* 31 */ &FnNum1, // ASCII '1'
	/* 32 */ &FnNum2, // ASCII '2'
	/* 33 */ &FnNum3, // ASCII '3'
	/* 34 */ &FnNum4, // ASCII '4'
	/* 35 */ &FnNum5, // ASCII '5'
	/* 36 */ &FnNum6, // ASCII '6'
	/* 37 */ &FnNum7, // ASCII '7'
	/* 38 */ &FnNum8, // ASCII '8'
	/* 39 */ &FnNum9, // ASCII '9'
	/* 3A */ &FnNop,
	/* 3B */ &FnNop,
	/* 3C */ &FnDup,  // ASCII '<'
	/* 3D */ &FnNop,
	/* 3E */ &FnDrop, // ASCII '>'
	/* 3F */ &FnNop,
};

void ExecuteOperation(Operation operation)
{
	(*dispatch[operation])();

	if (operation >= OpDot && operation != OpDrop)
	{
		isNewNum = true;
		isDot = false;
		decimals = 0;
	}

	isFunc = false;
	isMenu = false;
}

////////////////////////////////////////////////////////////////////////////////

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

static void PrintStack(uint8_t i, int8_t d, uint8_t s, uint8_t y)
{
	float f = stack.arr[i];
	PrintCharSize(CHAR_SIZE_M, s);

	if (isnan(f)) { PrintStringAt(FPSTR(message_str), MSG_ERR, M_DIGIT_FST, y);	}
	else
	{
		if (f < 0) { f = -f; PrintCharAt('-', M_SIGN, y); }
		if (isinf(f)) {	PrintStringAt(FPSTR(message_str), MSG_INF, M_DIGIT_FST, y); }
		else
		{
			int8_t e = (int8_t)(log(f) / log(10.f));
			uint32_t m = (uint32_t)(f / MathPow10(e - (DIGITS - 1)) + 0.5f);

			if (m > 0 && m < MathPow10(DIGITS - 1))
			{
				m = (uint32_t)(f / MathPow10(--e - (DIGITS - 1)) + 0.5f);
			}

			int8_t int_dig = 1, lead_z = 0;
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
					for (uint8_t n = lead_z; n--; m /= 10);
				}
				e = 0;
			}
			int8_t fra_dig = DIGITS - lead_z - int_dig;

			uint8_t x = M_DIGIT_LST, nonzero = false;
			for (; fra_dig--; m /= 10, x -= DIGIT_WIDTH)
			{
				uint8_t ones = _ones(m);
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

static void PrintClock()
{
	DisplayClear();

	PrintCharSize(CHAR_SIZE_M, CHAR_SIZE_L);
	PrintCharAt(':', 20, 0);
	PrintCharAt(':', 47, 0);
	PrintTwoDigitAt(rtc_hours, 0, 0);
	PrintTwoDigitAt(rtc_minutes, 27, 0);
	PrintTwoDigitAt(rtc_seconds, 54, 0);

	PrintCharSize(CHAR_SIZE_S, CHAR_SIZE_S);
	PrintStringAt(FPSTR(month_str), rtc_month - 1, 85, 0);

	PrintCharSize(CHAR_SIZE_M, CHAR_SIZE_S);
	PrintTwoDigitAt(rtc_date, 107, 0);
	PrintTwoDigitAt(20, 85, 1);
	PrintTwoDigitAt(rtc_year, 107, 1);

	DisplayRefresh();
}

static void PrintCalculator()
{
	DisplayClear();

	PrintCharSize(CHAR_SIZE_M, CHAR_SIZE_M);
	if (isTypeRecording) PrintCharAt(CHAR_REC, MODE_CHAR, 2);
	if (isTypePlaying) PrintCharAt(CHAR_PLAY, MODE_CHAR, 2);
	if (isShift) PrintCharAt(CHAR_SHIFT, MODE_CHAR, 0);

	uint8_t d = isNewNumber ? 0 : decimals;
	
	if (isMenu)
	{
		
		for (uint8_t i = 0; i < MENU_OPS_PER_LINE; ++i)
		{
			PrintStringAt(FPSTR(menu_str), select * MENU_OPS_PER_LINE + i, 48 * i, 2);
		}
		PrintStack(Stack::X, d, CHAR_SIZE_M, 0);
	}
	else if (isShift)
	{
		PrintStack(Stack::Y, 0, CHAR_SIZE_M, 0);
		PrintStack(Stack::X, d, CHAR_SIZE_M, 2);
	}
	else
	{
		PrintStack(Stack::X, d, CHAR_SIZE_L, 0);
	}

	DisplayRefresh();
}

////////////////////////////////////////////////////////////////////////////////

#define DIMOUT_FRAMES 156   // Frames before display dim out (about 10 sec)
#define POWEROFF_FRAMES 469 // Frames before power off (about 30 sec)

b08 isfirstrun; // Allows first run of loop and printscreen without key query
Key key; // Holds entered key
Key oldkey; // Use for debouncing
b08 inCalcMode;
u08 brightness;

int main() 
{
	ADCInit();
	I2CBusInit();
	DisplayInit();
	KeyboardInit();
    sei();

	DisplayTurnOn();
	FrameSyncEnable();

	// reset
	isfirstrun = true;
	inCalcMode = false;
	brightness = eeprom_read_byte(&eeprom_brightness);
	
	for (;;)
	{
		FrameSyncWait();

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
			ResetFrameCounter();
			inCalcMode = true;
		}

		if (frameCounter >= POWEROFF_FRAMES)
		{
			DeepSleep();
			oldkey = KeyboardRead();
			inCalcMode = !RTCRead();
		}

		DisplayBrightness(frameCounter < DIMOUT_FRAMES ? brightness : 0);

		if (inCalcMode)
		{
			if (key != KEY_NONE || isfirstrun)
			{
				if (isMenu)
				{
					// TODO
					isMenu = false;
				}
				else
				{
					Operation operation = (isFunc ? 0x10 + key : key);
					ExecuteOperation(operation);
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
	}
	return 0;
}

