
#define DS_CAPACITY 24
f32 ds[DS_CAPACITY];
u08 dp = 0;

f32 EEMEM eeprom_storage[10];
u08 EEMEM eeprom_brightness = 0xFF;

b08 isNewNum = true; // True if stack has to be lifted before entering a new number
u08 decimals = 0;    // Number of decimals entered (input after decimal dot)

b08 isFunc;
b08 isMenu;
b08 isEdit;  // True if last operation was number editing operation
b08 isPushed;

enum { MENU_MATH_OPS, MENU_TRIG_OPS, MENU_PROG_OPS, MENU_SETS_OPS };
void enterMenu(u08 type);

////////////////////////////////////////////////////////////////////////////////
// Operations
////////////////////////////////////////////////////////////////////////////////

/* Operation codes */
typedef const char Operation;
enum 
{
	// basic operations
	OpNum0, OpNum1, OpNum2, OpNum3, OpNum4, OpNum5, OpNum6, OpNum7,
	OpNum8, OpNum9, OpDot,  OpDup,  OpDrop, OpNeg,  OpEExp, AcFunc,

	// shifted operations
	OpC10,  OpRcl, OpSto,  OpSub, OpC14,  OpC15,  OpMul,  AcTrig,
	AcProg, OpDiv, OpSwap, OpAdd, OpAClr, OpRotD, OpRotU, AcMath,

	// math menu operations
	OpC20, OpC21, OpC22,
	OpC23, OpC24, OpC25,
	OpC26, OpC27, OpC28,
	OpC29, OpC2A, OpC2B,

	// trig menu operatons
	OpC2C, OpC2D, OpC2E,
	OpC2F, OpC30, OpC31,
	OpC32, OpC33, OpC34,
	OpC35, OpC36, OpC37,

	// prog menu operations
	OpC38, OpC39, OpC3A,

	// sets menu operations
	OpC3B, OpC3C, OpC3D,

	FUNC_OPS = OpC10,
	MATH_OPS = OpC20,
	TRIG_OPS = OpC2C,
	PROG_OPS = OpC38,
	SETS_OPS = OpC3B,
	OpNop = 0xFF
};

/* Forward declarations */
void FnNop();

void FnNum0(); void FnNum1(); void FnNum2(); void FnNum3(); void FnNum4(); void FnNum5(); void FnNum6(); void FnNum7();
void FnNum8(); void FnNum9(); void FnDot();  void FnDup();  void FnDrop(); void FnNeg();  void FnEExp(); void FnFunc();

void FnRcl();  void FnSto();  void FnSub();  void FnMul();  void FnTrig(); void FnProg(); void FnDiv();  void FnSwap();
void FnAdd();  void FnAClr(); void FnRotD(); void FnRotU(); void FnMath();

void FnInv();
void FnInt();
void FnPw10();

/* Data Stack Operations */
f32 dpush(f32 d) 
{
	if (dp >= DS_CAPACITY) { memcpy(ds, ds + 1, (DS_CAPACITY - 1) * sizeof(f32)); --dp; }
	return (ds[dp++] = d);
}

f32 dpop()
{
	return (dp ? ds[--dp] : 0.f);
}

f32 dget(u08 i)
{
	return (dp - i > 0 ? ds[dp - 1 - i] : 0.f);
}

f32 dtop()
{
	return dget(0);
}

b08 dpopByte(u08& dest, u08 min, u08 max)
{
	u08 b = dpop();
	if (b >= min && b <= max) { dest = b; return true; }
	dpush(NAN); return false;
}

void rotateStack(b08 isUp)
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
	if (dpopByte(i, 0, 9))
	{
		if (isWrite)
			eeprom_write_float(&eeprom_storage[i], dtop());
		else
			dpush(eeprom_read_float(&eeprom_storage[i]));
	}
}

/* Edit the number in X Register of Data Stack */
void checkNewNum()
{
	if (isNewNum)
	{
		isNewNum = false;
		if (isPushed) dpop();
		dpush(0);
	}
}

void enterDigit(u08 digit)
{
	checkNewNum();
	dpush(dtop() < 0 ? -digit : digit);
	if (decimals)
		{ dpush(pow10(decimals++)); FnDiv(); }
	else
		{ FnSwap(); dpush(10); FnMul(); }
	FnAdd();
	isEdit = true;
}

void clearDigit() // TODO: check!
{
	if (decimals) 
	{
		f32 a = pow10(--decimals);
		f32 b = s32(dpop() * a) / a;
		dpush(b);
	}
	else 
	{
		s32 a = dpop() / 10.f;
		if (a) dpush(a); 
		else isNewNum = true;
	}
	isEdit = true;
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
void FnDot()  {	if (!decimals) { checkNewNum(); decimals = 1; }	isEdit = true; }
void FnDup()  { dpush(dtop()); }
void FnDrop() { if (isNewNum) dpop(); else clearDigit(); }
void FnNeg()  { dpush(-dpop()); isEdit = !isNewNum; }
void FnEExp() { FnPw10(); FnMul(); }
void FnFunc() { isFunc = true; }

void FnRcl()  { storageAccess(false); }
void FnSto()  { storageAccess(true); }
void FnSub()  { FnNeg(); FnAdd(); }
void FnMul()  { dpush(dpop() * dpop()); }
void FnTrig() { enterMenu(MENU_TRIG_OPS); }
void FnProg() { enterMenu(MENU_PROG_OPS); }
void FnDiv()  { FnInv(); FnMul(); }
void FnSwap() { f32 a = dpop(), b = dpop(); dpush(a); dpush(b); }
void FnAdd()  { dpush(dpop() + dpop()); }
void FnAClr() { dp = 0; /* clear anything else */ }
void FnRotD() { rotateStack(false); }
void FnRotU() { rotateStack(true); }
void FnMath() { enterMenu(MENU_MATH_OPS); }

void FnInv()  { dpush(1.f / dpop()); }
void FnInt()  { dpush(s32(dpop())); }
void FnPw10() { dpush(pow10(dpop())); }

/* Operations Execution */
void (*dispatch[])() = 
{
	/* 00 */ &FnNum0, // Edit
	/* 01 */ &FnNum1, // Edit
	/* 02 */ &FnNum2, // Edit
	/* 03 */ &FnNum3, // Edit
	/* 04 */ &FnNum4, // Edit
	/* 05 */ &FnNum5, // Edit
	/* 06 */ &FnNum6, // Edit
	/* 07 */ &FnNum7, // Edit
	/* 08 */ &FnNum8, // Edit
	/* 09 */ &FnNum9, // Edit
	/* 0A */ &FnDot,
	/* 0B */ &FnDup,
	/* 0C */ &FnDrop, // Edit
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

	/* 10 */ &FnNop,  // TODO
	/* 10 */ &FnNop,  // TODO
	/* 10 */ &FnNop,  // TODO
	/* 10 */ &FnNop,  // TODO
	/* 10 */ &FnNop,  // TODO
	/* 10 */ &FnNop,  // TODO
	/* 10 */ &FnNop,  // TODO
	/* 10 */ &FnNop,  // TODO
	/* 10 */ &FnNop,  // TODO
	/* 10 */ &FnNop,  // TODO
	/* 10 */ &FnNop,  // TODO
	/* 10 */ &FnNop,  // TODO

	/* 10 */ &FnNop,  // TODO
	/* 10 */ &FnNop,  // TODO
	/* 10 */ &FnNop,  // TODO
	/* 10 */ &FnNop,  // TODO
	/* 10 */ &FnNop,  // TODO
	/* 10 */ &FnNop,  // TODO
	/* 10 */ &FnNop,  // TODO
	/* 10 */ &FnNop,  // TODO
	/* 10 */ &FnNop,  // TODO
	/* 10 */ &FnNop,  // TODO
	/* 10 */ &FnNop,  // TODO
	/* 10 */ &FnNop,  // TODO

	/* 10 */ &FnNop,  // TODO
	/* 10 */ &FnNop,  // TODO
	/* 10 */ &FnNop,  // TODO
	
	/* 10 */ &FnNop,  // TODO
	/* 10 */ &FnNop,  // TODO
	/* 10 */ &FnNop,  // TODO
};

void ExecuteOperation(Operation op)
{
	isFunc = false;
	isMenu = false;
	isEdit = false;
	
	if (op != OpNop)
	{
		(*dispatch[op])();

		// TODO: save op to program
	}

	isPushed = (op == OpDup);

	if (!isEdit)
	{
		isNewNum = true;
		decimals = 0;
	}
}
