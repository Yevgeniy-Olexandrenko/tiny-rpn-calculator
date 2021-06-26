
#define M_E  2.7182818284590452354
#define M_PI 3.1415926535897932384

bool isFunc = false;
bool isMenu = false;

#define DATASTACKSIZE 24
float ds[DATASTACKSIZE];
uint8_t dp = 0;

float EEMEM eeprom_constant[6];

bool isnewnumber = true; // True if stack has to be lifted before entering a new number
uint8_t decimals = 0; // Number of decimals entered (input after decimal dot)
bool isdot = false; // True if dot was pressed and decimals will be entered

////////////////////////////////////////////////////////////////////////////////
// Key Codes (0x00-0x09 numbers, 0x0A-0x0F special keys)
////////////////////////////////////////////////////////////////////////////////

typedef enum {
	KEY_NUM0 = KEY_B3,
	KEY_NUM1 = KEY_B2,
	KEY_NUM2 = KEY_C2,
	KEY_NUM3 = KEY_D2,
	KEY_NUM4 = KEY_B1,
	KEY_NUM5 = KEY_C1,
	KEY_NUM6 = KEY_D1,
	KEY_NUM7 = KEY_B0,
	KEY_NUM8 = KEY_C0,
	KEY_NUM9 = KEY_D0,
	KEY_DOT  = KEY_C3,
	KEY_DUP  = KEY_D3,
	KEY_CLR  = KEY_A3,
	KEY_CHSG = KEY_A2,
	KEY_EXP  = KEY_A1,
	KEY_FUNC = KEY_A0
} Key;

////////////////////////////////////////////////////////////////////////////////
// Operations
////////////////////////////////////////////////////////////////////////////////

typedef enum {
	// basic inputs
	OpNum0, OpNum1, OpNum2, OpNum3, OpNum4, OpNum5, OpNum6, OpNum7,
	OpNum8, OpNum9, OpDot,  OpDup,  OpClr,  OpChSg, OpEnEx, OpFunc,

	// basic functions
	OpConst, OpRcl, OpSto,  OpSub, OpFunc3, OpFung4, OpMul,  OpFunc1,
	OpFunc2, OpDiv, OpSwap, OpAdd, OpPwr,   OpRotD,  OpRotU, OpMenu,

	// constants input functions
	OpCoBat, OpCoE, OpCoPi,

	// menu functions
	OpSqrt, OpPow2, OpInv,
	OpPow,  OpExp,  OpLn,
	OpR2P,  OpP2R,  OpGamm,

	KEYS_INPUTS = OpNum0,
	KEYS_FUNCTIONS = OpConst,
	MENU_CONSTANTS = OpCoBat,
	MENU_FUNCTIONS = OpSqrt,

} Operation;

// stack
void floatstack() 
{
	memcpy(ds, &ds[1], (DATASTACKSIZE - 1) * sizeof(double));
	dp--;
}

float dpush(float d) 
{
	if (dp >= DATASTACKSIZE) floatstack();
	return (ds[dp++] = d);
}

float dpop()
{
	return (dp ? ds[--dp] : 0.f);
}

void enterNumber(uint8_t number)
{
	if (isdot) 
	{
		dpush(number);
		dpush(pow10(++decimals));
		OpDiv();
		OpAdd();
	}
	else if (isnewnumber)
	{
		dpush(number);
	}
	else 
	{
		dpush(10.f);
		OpMul();
		dpush(number);
		OpAdd();
	}
	isnewnumber = false;
}

// Calculates 10 raised to the power of e
float pow10(int8_t e) 
{ 
	float f = 1.f;
	if (e > 0) 
		while (e--) f *= 10.f;
	else 
		while (e++) f /= 10.f;
	return (f);
}

void storageReadWrite(boolean isWrite) 
{
	uint8_t nr = dpop();
	uint8_t addr = EESTO + nr * sizeof(float);
	if (nr < MEMSTO) 
	{
		if (isWrite) 
			EEwrite(addr, dpop());
		else 
		{
			float a;
			EEread(addr, a);
			dpush(a);
		}
	}
}

void OpNum0() { enterNumber(0); }
void OpNum1() { enterNumber(1); }
void OpNum2() { enterNumber(2); }
void OpNum3() { enterNumber(3); }
void OpNum4() { enterNumber(4); }
void OpNum5() { enterNumber(5); }
void OpNum6() { enterNumber(6); }
void OpNum7() { enterNumber(7); }
void OpNum8() { enterNumber(8); }
void OpNum9() { enterNumber(9); }

void OpDot()
{
	if (isnewnumber) 
	{
		dpush(0.F);
		decimals = 0; 
		isnewnumber = false;
	}
	isdot = true;
}

void OpPush() {	if (isnewnumber && dp) dpush(ds[dp - 1]); }

void OpClr()
{
	if (!isnewnumber)
	{
		// clear entry
		if (isdot) 
		{
			if (decimals) 
			{
				decimals--;
				double a = pow10(decimals);
				dpush(((long)(dpop() * a) / a));
			}
			else
			{
				isdot = false;
			}
		}
		else 
		{
			long a = dpop() / 10.0F;
			if (!a) 
				isnewnumber = true;
			else 
				dpush(a);
		} 
	}
	else if (dp)
	{
		// clear top of stack
		dp--; 
	}
}

void OpNeg() { dpush(-dpop()); }

void OpEnEx() { dpush(pow10(dpop())); OpMul(); }

void OpFunc() {	isFunc = true; }

void OpConst()
{
	uint8_t index = dpop();
	switch(index)
	{
		case  0: dpush(BatteryRead()); break;
		case  1:  // TODO: random number 0-1
		case  2: dpush(M_E); break;
		case  3: dpush(M_PI); break;
		default: eeprom_read_float(&eeprom_constant[index - 4]);
	}
}

void OpRcl() { storageReadWrite(false); }

void OpSto() { storageReadWrite(true); }

void OpSub() { OpNeg(); OpAdd(); }

void OpFunc3()
{
	//
}

void OpFunc4()
{
	//
}

void OpMul() { dpush(dpop() * dpop()); }

void OpFunc1()
{
	//
}

void OpFunc2()
{
	//
}

void OpDiv() { OpInv(); OpMul(); }

void OpSwap() { if (dp > 1) { float a = dpop(), b = dpop(); dpush(a); dpush(b); }}

void OpAdd() { dpush(dpop() + dpop()); }

void OpPwr()
{
	//
}

void rotateStack(bool isUp)
{
	if (dp > 2)
	{
		float a = dpop(), b = dpop(), c = dpop();
		if (isUp)
		{	// 2 c -> b
			// 1 b -> a
			// 0 a -> c
			dpush(b); dpush(a); dpush(c);
		}
		else
		{	// 2 c -> a
			// 1 b -> c
			// 0 a -> b
			dpush(a); dpush(c); dpush(b);
		}
	}
}

void OpRotD() {	rotateStack(false); }

void OpRotU() { rotateStack(true); }

void OpMenu()
{
	//
}

void OpInv() { dpush(1.f / dpop()); }

void ExecuteOperation(Operation operation)
{
	static void (*dispatch[])() = 
	{
		/* 00 */ &OpNum0,
		/* 01 */ &OpNum1,
		/* 02 */ &OpNum2,
		/* 03 */ &OpNum3,
		/* 04 */ &OpNum4,
		/* 05 */ &OpNum5,
		/* 06 */ &OpNum6,
		/* 07 */ &OpNum7,
		/* 08 */ &OpNum8,
		/* 09 */ &OpNum9,
		/* 0A */ &OpDot,
		/* 0B */ &OpPush,
		/* 0C */ &OpClr,
		/* 0D */ &OpNeg,
		/* 0E */ &OpEnEx,
		/* 0F */ &OpFunc,

		/* 10 */ &OpConst,
		/* 11 */ &OpRcl,
		/* 12 */ &OpSto,
		/* 13 */ &OpSub,
		/* 14 */ &OpFunc3,
		/* 15 */ &OpFunc4,
		/* 16 */ &OpMul,
		/* 17 */ &OpFunc1,
		/* 18 */ &OpFunc2,
		/* 19 */ &OpDiv,
		/* 1A */ &OpSwap,
		/* 1B */ &OpAdd,
		/* 1C */ &OpPwr,
		/* 1D */ &OpRotD,
		/* 1E */ &OpRotU,
		/* 1F */ &OpMenu,
	};

	(*dispatch[operation])();

	if (operation >= OpDot && operation != OpClr)
	{
		decimals = 0;
		isdot = false;
		isnewnumber = true;
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