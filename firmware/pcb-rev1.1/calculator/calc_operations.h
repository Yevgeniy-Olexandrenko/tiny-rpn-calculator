typedef uint8_t Operation;
typedef uint8_t OpScript;

#define DS_CAPACITY 24
f32 ds[DS_CAPACITY];
u08 dp = 0;

f32 lastx = 0;
f32 memory[10];

#define AS_CAPACITY 24
u16 as[AS_CAPACITY];
u08 ap = 0;

Operation programs[3][90] EEMEM;
u16 pp = 0;
u08 cl = 0;

b08 isNewNum = true; // True if stack has to be lifted before entering a new number
u08 decimals = 0;    // Number of decimals entered (input after decimal dot)

b08 isFunc;
b08 isMenu;
b08 isEdit;
b08 isPushed;
b08 isLast;

enum { MENU_MATH_OPS, MENU_TRIG_OPS, MENU_PROG_OPS, MENU_SETS_OPS };

void enterMenu(u08 type);
void setupRTC();

////////////////////////////////////////////////////////////////////////////////
// Operations Declarations
////////////////////////////////////////////////////////////////////////////////

enum /* Operation codes */
{
	// basic operations
	OpNum0, OpNum1, OpNum2, OpNum3, OpNum4, OpNum5, OpNum6, OpNum7,
	OpNum8, OpNum9, OpDot,  OpDup,  OpDrop, OpNeg,  OpEExp, AcFunc,

	// shifted operations
	OpLast, OpRcl, OpSto,  OpSub, OpCst,  OpMAdd, OpMul,  AcTrig,
	AcProg, OpDiv, OpSwap, OpAdd, OpAClr, OpRotD, OpRotU, AcMath,

	// math menu operations
	OpSqr,  OpSqrt, OpInv,
	OpPw10, OpLg,   OpPow,
	OpExp,  OpLn,   OpNRT,
	
	// trig menu operatons
	OpSin,  OpCos,  OpTan,
	OpASin, OpACos, OpATan,
	OpC2C,  OpC2D,  OpC2E,
	OpC2F,  OpC30,  OpC31,

	// prog menu operations
	OpC32,  OpC33,  OpC34,
	OpEq,   OpNe,   OpC37,
	OpGt,   OpLt,   OpC3A,
	OpIf,   OpElse, OpThen,

	// sets menu operations
	OpC3E, OpStm, OpSdt,

	OpEnd,
	FUNC_OPS = OpLast,
	MATH_OPS = OpSqrt,
	TRIG_OPS = OpSin,
	PROG_OPS = OpC32,
	SETS_OPS = OpC3E,
};

enum /* Scripted Operations */
{
	SoSin, SoTan, SoASin, SoACos, SoSqrt, SoPow,
};

const Operation scripts[] PROGMEM =
{
	OpEnd,
	// SoSin:  SIN = cos(90-x)
	OpNum9, OpNum0, OpSwap, OpSub, OpCos, OpEnd,
	// SoTan:  TAN = sin/cos
	OpDup, OpSin, OpSwap, OpCos, OpDiv, OpEnd,
	// SoASin: ASIN = atan(1/(sqrt(1/x/x-1))
	OpDup, OpMul, OpInv, OpNum1, OpSub, OpSqrt, OpInv, OpATan, OpEnd,
	// SoACos: ACOS = atan(sqrt(1/x/x-1))
	OpDup, OpMul, OpInv, OpNum1, OpSub, OpSqrt, OpATan, OpEnd,
	// SoSqrt: SQRT = exp(ln(x)/2) may be replaced with POW
	OpDup, OpNum0, OpNe, OpIf, OpLn, OpNum2, OpDiv, OpExp, OpThen, OpEnd,
	// SoPow:  POW y^x = exp(x*ln(y))
	OpSwap, OpLn, OpMul, OpExp, OpEnd,
};

/* Forward declarations */
void FnNop();

void FnNum0(); void FnNum1(); void FnNum2(); void FnNum3(); void FnNum4(); void FnNum5(); void FnNum6(); void FnNum7();
void FnNum8(); void FnNum9(); void FnDot();  void FnDup();  void FnDrop(); void FnNeg();  void FnEExp(); void FnFunc();

void FnRcl();  void FnSto();  void FnSub();  void FnMul();  void FnTrig(); void FnProg(); void FnDiv();  void FnSwap();
void FnAdd();  void FnAClr(); void FnRotD(); void FnRotU(); void FnMath();

void FnSqrt();
void FnPow();
void FnInv();
void FnExp();
void FnLn();
void FnInt();
void FnPw10();

void FnSin();  void FnCos();  void FnTan();
void FnASin(); void FnACos(); void FnATan();

void FnEnd();

////////////////////////////////////////////////////////////////////////////////
// Data and Address Stacks
////////////////////////////////////////////////////////////////////////////////

void onError();

/* Data Stack Operations */
void dmove()      { memcpy(ds, ds + 1, (DS_CAPACITY - 1) * sizeof(f32)); }
void dpush(f32 d) { if (dp >= DS_CAPACITY) { dmove(); --dp; } ds[dp++] = d; }
f32  dpop()       { return (dp ? ds[--dp] : 0.f); }
f32  dget(u08 i)  { return (dp - i > 0 ? ds[dp - 1 - i] : 0.f); }
f32  dtop()       { return dget(0); }
f32  dtopx()      { if (isLast) { lastx = dtop(); isLast = false; } return dtop(); }
f32  dpopx()      { dtopx(); return dpop(); }

b08 dpop(u08& dest, u08 min, u08 max)
{
	u08 b = dpop();
	if (b >= min && b <= max) { dest = b; return true; }
	onError(); return false;
}

void rotateStack(b08 isUp)
{
	f32 x = dpop(), y = dpop(), z = dpop();
	if (isUp)
	{	// 2 z -> y
		// 1 y -> x
		// 0 x -> z
		dpush(y); dpush(x); dpush(z);
	}
	else
	{	// 2 z -> x
		// 1 y -> z
		// 0 x -> y
		dpush(x); dpush(z); dpush(y);
	}
}

/* Address Stack Operations */
void apush() { if (ap < AS_CAPACITY) as[ap++] = pp; else onError(); }
void apop()  { if (ap) pp = as[--ap]; else onError(); }

////////////////////////////////////////////////////////////////////////////////
// Helpers
////////////////////////////////////////////////////////////////////////////////

#define _to_rad(x) ((x) * (PI / 180))
#define _to_deg(x) ((x) * (180 / PI))

f32 pow10(s08 e) 
{ 
	f32 f = 1.f;
	if (e > 0) 
		while (e--) f *= 10.f;
	else 
		while (e++) f /= 10.f;
	return f;
}

void memoryAccess(b08 isWrite) 
{
	u08 i;
	if (dpop(i, 0, 9))
	{
		if (isWrite)
			memory[i] = dtop();
		else
			dpush(memory[i]);
	}
}

void enterConstant()
{
	u08 i;
	if (dpop(i, 0, 3))
	{
		switch(i)
		{
			case 0: dpush(ADCReadVcc() ); return;
			case 1: dpush(ADCReadTemp()); return;
			case 2: dpush(M_E ); return;
			case 3: dpush(M_PI); return;
		}
	}
}

void callScript(OpScript script)
{
	apush();
	for (pp = 0, ++script; script;)
	{
		if (pgm_read_byte(&scripts[pp++]) == OpEnd) --script;
	}
}

bool isScript()
{
	return pp < sizeof(scripts);
}

void conditionSeek()
{
	u08 cl = 0;
	while (true)
	{
		Operation op = OpEnd;
		if (isScript())
			op = pgm_read_byte(&scripts[pp++]);

		//else if (mp < sizeof(mem) + EEU)
		//	c = EEPROM[mp++ - sizeof(mem) + EEUSTART];
		
		if (op == OpEnd) { pp--; break; }
		else if (op == OpIf) cl++; // nested IF found
		else if (cl && op == OpThen) cl--; // nested IF ended
		else if (!cl && (op == OpElse || op == OpThen)) break;
	}
}

void onError() { ap = 0; dpush(NAN); }

////////////////////////////////////////////////////////////////////////////////
// Edit the number in X Register of Data Stack
////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////
// Operations Implementations
////////////////////////////////////////////////////////////////////////////////

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
void FnDrop() { if (isNewNum) dpopx(); else clearDigit(); }
void FnNeg()  { dpush(-dpopx()); isEdit = !isNewNum; }
void FnEExp() { isLast = false; FnPw10(); FnMul(); }
void FnFunc() { isFunc = true; }

void FnLast() { dpush(lastx); }
void FnRcl()  { memoryAccess(false); }
void FnSto()  { memoryAccess(true); }
void FnSub()  { FnNeg(); FnAdd(); }
void FnCst()  { enterConstant(); }
void FnMAdd() { FnRcl(); FnAdd(); FnSto(); }
void FnMul()  { dpush(dpopx() * dpop()); }
void FnTrig() { enterMenu(MENU_TRIG_OPS); }
void FnProg() { enterMenu(MENU_PROG_OPS); }
void FnDiv()  { FnInv(); FnMul(); }
void FnSwap() { f32 a = dpop(), b = dpop(); dpush(a); dpush(b); }
void FnAdd()  { dpush(dpopx() + dpop()); }
void FnAClr() { dp = 0; lastx = 0; /* clear anything else */ }
void FnRotD() { rotateStack(false); }
void FnRotU() { rotateStack(true); }
void FnMath() { enterMenu(MENU_MATH_OPS); }

void FnSqr()  { FnDup(); FnMul(); }
void FnSqrt() { callScript(SoSqrt); }
void FnInv()  { dpush(1.f / dpopx()); }
void FnPw10() { dpush(pow10(dpopx())); }
void FnLg()   { FnLn(); dpush(M_LOG10E); FnMul(); }
void FnPow()  { callScript(SoPow); }
void FnExp()
{
	b08 isNeg = false;
	if (dtopx() < 0) { FnNeg(); isNeg = true; }
	dpush(1);
	for (u08 i = 255; i; i--)
	{
		FnSwap(); FnDup(); FnRotU(); dpush(i);
		FnDiv();  FnMul(); dpush(1); FnAdd();
	}
	if (isNeg) FnInv();
	FnSwap(); FnDrop();
}
void FnLn()   { dpush(log(dpopx())); }
void FnNRT()  { FnInv(); FnPow(); }

void FnSin()  { callScript(SoSin); }
void FnCos()  { dpush(cos(_to_rad(dpopx()))); }
void FnTan()  { callScript(SoTan); }
void FnASin() { callScript(SoASin); }
void FnACos() { callScript(SoACos); }
void FnATan() { dpush(_to_deg(atan(dpopx()))); }

void FnEq()   { dpush(dpop() == dpop()); }
void FnGt()   { dpush(dpop() < dpop()); }
void FnLt()   { FnGt(); dpush(!dpop()); }
void FnNe()   { FnEq(); dpush(!dpop()); }
void FnIf()   { cl++; if (!dpop()) conditionSeek(); }
void FnElse() { cl--; conditionSeek(); }
void FnThen() { cl--; }

void FnStm()
{
	if (RTCRead()
		&& dpop(rtc_seconds, 0, 59)
		&& dpop(rtc_minutes, 0, 59)
		&& dpop(rtc_hours, 0, 23))
	setupRTC();
}
void FnSdt()
{
	if (RTCRead()
		&& dpop(rtc_year, 0, 99)
		&& dpop(rtc_month, 1, 12)
		&& dpop(rtc_date, 1, 31))
	setupRTC();
}

void FnInt()  { dpush(s32(dpopx())); } // TODO
void FnEnd()  { apop(); }

////////////////////////////////////////////////////////////////////////////////
// Operations Execution
////////////////////////////////////////////////////////////////////////////////

typedef void (*OpHandler)();
const OpHandler opHandlers[] PROGMEM = 
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
	/* 0F */ &FnFunc, // Shift

	/* 10 */ &FnLast,
	/* 11 */ &FnRcl,
	/* 12 */ &FnSto,
	/* 13 */ &FnSub,
	/* 14 */ &FnCst,
	/* 15 */ &FnMAdd,
	/* 16 */ &FnMul,
	/* 17 */ &FnTrig, // Menu
	/* 18 */ &FnProg, // Menu
	/* 19 */ &FnDiv,
	/* 1A */ &FnSwap,
	/* 1B */ &FnAdd,
	/* 1C */ &FnAClr,
	/* 1D */ &FnRotD,
	/* 1E */ &FnRotU,
	/* 1F */ &FnMath, // Menu

	/* 20 */ &FnSqr,
	/* 21 */ &FnSqrt,
	/* 22 */ &FnInv,
	/* 23 */ &FnPw10,
	/* 24 */ &FnLg,
	/* 25 */ &FnPow,
	/* 26 */ &FnExp,
	/* 27 */ &OpLn,
	/* 28 */ &OpNop,  // TODO

	/* 10 */ &FnSin,
	/* 10 */ &FnCos,
	/* 10 */ &FnTan,
	/* 10 */ &FnASin,
	/* 10 */ &FnACos,
	/* 10 */ &FnATan,
	/* 10 */ &FnNop,  // TODO
	/* 10 */ &FnNop,  // TODO
	/* 10 */ &FnNop,  // TODO
	/* 10 */ &FnNop,  // TODO
	/* 10 */ &FnNop,  // TODO
	/* 10 */ &FnNop,  // TODO

	/* 10 */ &FnNop,  // TODO
	/* 10 */ &FnNop,  // TODO
	/* 10 */ &FnNop,  // TODO
	/* 10 */ &FnEq,
	/* 10 */ &FnNe,
	/* 10 */ &FnNop,  // TODO
	/* 10 */ &FnGt,
	/* 10 */ &FnLt,
	/* 10 */ &FnNop,  // TODO
	/* 10 */ &FnIf,
	/* 10 */ &FnElse,
	/* 10 */ &FnThen,

	/* 10 */ &FnNop,  // TODO
	/* 10 */ &FnStm,
	/* 10 */ &FnSdt,

	/* 10 */ &FnEnd,  // Last Operation
};

void ExecuteOperation(Operation op)
{
	isFunc = false;
	isMenu = false;
	isEdit = false;
	isLast = isNewNum;
	
	OpHandler opHandler = (OpHandler)pgm_read_word(&opHandlers[op]);
	opHandler();

	// TODO: save op to program

	isPushed = (op == OpDup);
	if (!isEdit)
	{
		isNewNum = true;
		decimals = 0;
	}
}
