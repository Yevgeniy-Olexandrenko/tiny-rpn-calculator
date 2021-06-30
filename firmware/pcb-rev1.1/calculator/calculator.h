
#define M_E  2.7182818284590452354
#define M_PI 3.1415926535897932384

bool isFunc = false;
bool isMenu = false;

#define DATA_STACK_SIZE 24
float   ds[DATA_STACK_SIZE];
uint8_t dp = 0;

float eeprom_storage[10] EEMEM;

bool isNewNum = true; // True if stack has to be lifted before entering a new number
uint8_t decimals = 0; // Number of decimals entered (input after decimal dot)
bool isDot = false; // True if dot was pressed and decimals will be entered

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

const char strMessage[] PROGMEM =
	"\03" "ERR" "INF";

const char strMonth[] PROGMEM = 
	"\03"
	"JAN" "FEB" "MAR" "APR" "MAY" "JUN"
	"JUL" "AUG" "SEP" "OCT" "NOV" "DEC";

////////////////////////////////////////////////////////////////////////////////
// Data Stack Operations
////////////////////////////////////////////////////////////////////////////////

float dpush(float d) 
{
	if (dp >= DATA_STACK_SIZE) 
	{
		memcpy(&ds[0], &ds[1], (DATA_STACK_SIZE - 1) * sizeof(float));
		dp--;
	}
	return (ds[dp++] = d);
}

float dpop()
{
	return (dp ? ds[--dp] : 0.f);
}

bool dpopChecked(uint8_t & dest, uint8_t min, uint8_t max)
{
	float n = dpop();
	if (n >= min && n <= max) { dest = n; return true; }
	dpush(NAN);
	return false;
}

void rotateStack(bool isUp)
{
	if (dp > 2)
	{
		float a = dpop(), b = dpop(), c = dpop();
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

void enterDigit(uint8_t digit)
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
			float n = pow10(--decimals);
			dpush(((long)(dpop() * n) / n));
		}
		else
		{
			isDot = false;
		}
	}
	else 
	{
		long n = dpop() / 10.f;
		if (!n) 
			isNewNum = true;
		else 
			dpush(n);
	}
}

////////////////////////////////////////////////////////////////////////////////
// Helping Math Functions
////////////////////////////////////////////////////////////////////////////////

float pow10(int8_t e) 
{ 
	float n = 1.f;
	if (e > 0) 
		while (e--) n *= 10.f;
	else 
		while (e++) n /= 10.f;
	return n;
}

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////

void storageAccess(boolean isWrite) 
{
	uint8_t i;
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
void FnSwap() { if (dp > 1) { float a = dpop(), b = dpop(); dpush(a); dpush(b); }}
void FnAdd()  { dpush(dpop() + dpop()); }
void FnAClr() { dp = 0; /* clear anything else */ }
void FnRotD() { rotateStack(false); }
void FnRotU() { rotateStack(true); }
void FnMath() { /* TODO */ }

void FnInv()  { dpush(1.f / dpop()); }

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
};

void ExecuteOperation(Operation operation)
{
	(*dispatch[operation])();
	if (operation >= OpDot && operation != OpDrop)
	{
		decimals = 0;
		isDot = false;
		isNewNum = true;
	}
	isFunc = false;
}























uint16_t keyVoltage;
uint8_t  keyIndex;

int main() 
{
	ADCInit();
	I2CBusInit();
	DisplayInit();
	KeyboardInit();
    sei();

	DisplayTurnOn();
	FrameSyncEnable();
	
	for (;;)
	{
        FrameSyncWait();

        keyVoltage = ADCRead(KEYBOARD_ADC, 1);
        keyIndex = KeyboardRead();

        DisplayClear();
        PrintCharSize(CHAR_SIZE_M, CHAR_SIZE_M);

        //
        PrintCharAt('0' + _huns(keyVoltage), (FONT_WIDTH * cw + 1) * 0, ch * 0);
        PrintCharAt('0' + _tens(keyVoltage), (FONT_WIDTH * cw + 1) * 1, ch * 0);
        PrintCharAt('0' + _ones(keyVoltage), (FONT_WIDTH * cw + 1) * 2, ch * 0);
        //
        PrintCharAt('0' + _huns(keyIndex), (FONT_WIDTH * cw + 1) * 0, ch * 1);
        PrintCharAt('0' + _tens(keyIndex), (FONT_WIDTH * cw + 1) * 1, ch * 1);
        PrintCharAt('0' + _ones(keyIndex), (FONT_WIDTH * cw + 1) * 2, ch * 1);
        //

        DisplayRefresh();
    }
}