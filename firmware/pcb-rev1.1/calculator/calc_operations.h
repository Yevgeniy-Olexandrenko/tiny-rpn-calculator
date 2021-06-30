
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
// Operations
////////////////////////////////////////////////////////////////////////////////

/* Operation codes */
enum 
{
	// basic operations
	OpNum0, OpNum1, OpNum2, OpNum3, OpNum4, OpNum5, OpNum6, OpNum7,
	OpNum8, OpNum9, OpDot,  OpDup,  OpDrop, OpNeg,  OpEExp, AcFunc,

	// shifted operations
	OpC10,  OpRcl, OpSto,  OpSub, OpC14,  OpC15,  OpMul,  AcTrig,
	AcProg, OpDiv, OpSwap, OpAdd, OpAClr, OpRotD, OpRotU, AcMath,

	// alternative and hidden operations
	OpNop,   OpNeg_,  OpC22,   OpC23,   OpC24,   OpSwap_, OpC26,   OpC27,
	OpRotU_, OpRotD_, OpMul_,  OpAdd_,  OpC2C,   OpSub_,  OpDot_,  OpDiv_,
	OpNum0_, OpNum1_, OpNum2_, OpNum3_, OpNum4_, OpNum5_, OpNum6_, OpNum7_,
	OpNum8_, OpNum9_, OpC3A,   OpC3B,   OpDup_,  OpC3D,   OpDrop_, OpC3F,

	SHIFTED_OPS = OpC10
};

/* Forward declarations */
void FnNop();

void FnNum0(); void FnNum1(); void FnNum2(); void FnNum3(); void FnNum4(); void FnNum5(); void FnNum6(); void FnNum7();
void FnNum8(); void FnNum9(); void FnDot();  void FnDup();  void FnDrop(); void FnNeg();  void FnEExp(); void FnFunc();

void FnRcl();  void FnSto();  void FnSub();  void FnMul();  void FnTrig(); void FnProg(); void FnDiv();  void FnSwap();
void FnAdd();  void FnAClr(); void FnRotD(); void FnRotU(); void FnMath();

void FnInv();
void FnInt();

/* Data Stack Operations */
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

/* Helping Functions */
f32 pow10(s08 e) 
{ 
	f32 f = 1.f;
	if (e > 0) 
		while (e--) f *= 10.f;
	else 
		while (e++) f /= 10.f;
	return f;
}

void storageAccess(b08 isWrite) 
{
	u08 i;
	if (dpopChecked(i, 0, 9))
	{
		if (isWrite)
			eeprom_write_float(&eeprom_storage[i], dpop());
		else
			dpush(eeprom_read_float(&eeprom_storage[i]));
	}
}

/* Enter and Clear the number in X Register of Data Stack */
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
			f32 a = pow10(--decimals);
			f32 b = s32(dpop() * a) / a;
			dpush(b);
		}
		else 
			isDot = false;
	}
	else 
	{
		s32 a = dpop() / 10.f;
		if (a) dpush(a); 
		else isNewNum = true;
	}
}

/* Operations Implementation */
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
void FnTrig() { isMenu = true; /* TODO */ }
void FnProg() { isMenu = true; /* TODO */ }
void FnDiv()  { FnInv(); FnMul(); }
void FnSwap() { if (dp > 1) { f32 a = dpop(), b = dpop(); dpush(a); dpush(b); }}
void FnAdd()  { dpush(dpop() + dpop()); }
void FnAClr() { dp = 0; /* clear anything else */ }
void FnRotD() { rotateStack(false); }
void FnRotU() { rotateStack(true); }
void FnMath() { isMenu = true; /* TODO */ }

void FnInv()  { dpush(1.f / dpop()); }
void FnInt()  { dpush(s32(dpop())); }

/* Operations Execution */

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

	/* 20 */ &FnNop,  // True NOP
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

void ExecuteOperation(u08 op)
{
	isFunc = false;
	isMenu = false;

	(*dispatch[op])();

	b08 isNumInput = ((op >= OpNum0 && op <= OpNum9) || op == OpDot || op == OpDrop);
	b08 isNumInputAlt = ((op >= OpNum0_ && op <= OpNum9_) || op == OpDot_ || op == OpDrop_);

	if (!isNumInput && !isNumInputAlt)
	{
		isNewNum = true;
		isDot = false;
		decimals = 0;
	}
}
