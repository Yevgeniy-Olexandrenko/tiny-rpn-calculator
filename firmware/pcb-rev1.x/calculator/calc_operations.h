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

b08 isDeg = true;

enum { MENU_MATH_OPS, MENU_TRIG_OPS, MENU_PROG_OPS, MENU_SETS_OPS };

void enterMenu(u08 type);
void setupAndSwitchToRTCMode();

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
	OpExp,  OpLn,   OpNrt,
	
	// trig menu operatons
	OpSin,  OpCos,  OpTan,
	OpASin, OpACos, OpATan,
	OpC2F,  OpC30,  OpC31,
	OpC32,  OpC33,  OpC34,

	// prog menu operations
	OpC35,  OpC36,  OpC37,
	OpEq,   OpNe,   OpC3A,
	OpGt,   OpLt,   OpC3D,
	OpIf,   OpElse, OpThen,

	// sets menu operations
	OpSwDR, OpTime, OpDate,

	OpEnd,
	FUNC_OPS = OpLast,
	MATH_OPS = OpSqr,
	TRIG_OPS = OpSin,
	PROG_OPS = OpC35,
	SETS_OPS = OpSwDR,
};

enum /* Scripted Operations */
{
	SoMAdd, SoCos, SoTan, SoACos, SoATan, SoPow,
};

const Operation scripts[] PROGMEM =
{
	OpEnd,

	// [ok] SoMAdd:
	OpDup, OpRotD, OpRcl, OpAdd, OpSwap, OpSto, OpEnd,

	// [  ] SoCos:  COS = sin(x+90)
	OpNum9, OpNum0, OpAdd, OpSin, OpEnd,

	// [  ] SoTan:  TAN = sin/cos
	OpDup, OpSin, OpSwap, OpCos, OpDiv, OpEnd,

	// [  ] SoACos: ACOS = ???
	OpASin, OpNeg, OpNum9, OpNum0, OpAdd, OpEnd,

	// [  ] SoATan: ATAN = ???
	OpDup, OpDup, OpDup, OpMul, OpNum1, OpAdd, OpSqrt, OpInv, OpMul, OpASin, OpEnd,

	// [ok] SoPow:  POW y^x = exp(x*ln(y))
	OpSwap, OpLn, OpMul, OpExp, OpEnd,
};

/* Forward declarations */
void FnNop();

void FnNum0(); void FnNum1(); void FnNum2(); void FnNum3(); void FnNum4(); void FnNum5(); void FnNum6(); void FnNum7();
void FnNum8(); void FnNum9(); void FnDot();  void FnDup();  void FnDrop(); void FnNeg();  void FnEExp(); void FnFunc();

void FnLast(); void FnRcl();  void FnSto();  void FnSub();  void FnCst();  void FnMAdd(); void FnMul();  void FnTrig();
void FnProg(); void FnDiv();  void FnSwap(); void FnAdd();  void FnAClr(); void FnRotD(); void FnRotU(); void FnMath();

void FnSqr();  void FnSqrt(); void FnInv();
void FnPw10(); void FnLg();   void FnPow();
void FnExp();  void FnLn();   void FnNrt();

void FnSin();  void FnCos();  void FnTan();
void FnASin(); void FnACos(); void FnATan();

void FnD2R();
void FnR2D();
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
// Basic Internal Math
////////////////////////////////////////////////////////////////////////////////

enum { EXP, SIN, ASIN };
f32 taylorExpSinASin(f32 f, u08 op)
{
	f32 result, frac, ff = f * f;
	result = frac = ((op == EXP) ? 1 : f);

	for (u16 i = 1; i < 129; i++)
	{
		u16 i2 = 2 * i;
		u16 i2p = i2 + 1;
		u16 i2m = i2 - 1;
		u16 i2m2 = i2m * i2m;
		f32 ffi2i2p = ff / (i2 * i2p);

		switch(op)
		{
			case EXP: frac *= f / i; break;
			case SIN: frac *= -ffi2i2p; break;
			default:  frac *= ffi2i2p * i2m2; break;
		}
		result += frac;
	}
	return result;
}

f32 _log (f32 f) { return log(f); }
f32 _exp (f32 f) { return taylorExpSinASin(f, EXP);  }
f32 _sin (f32 f) { return taylorExpSinASin(f, SIN);  }
f32 _asin(f32 f) { return taylorExpSinASin(f, ASIN); }
f32 _p10 (s08 e) 
{ 
	f32 f = 1.f;
	if (e > 0) 
		while (e--) f *= 10.f;
	else 
		while (e++) f /= 10.f;
	return f;
}

#define _to_rad(x) ((x) * (M_PI / 180.f))
#define _to_deg(x) ((x) * (180.f / M_PI))

////////////////////////////////////////////////////////////////////////////////
// Helpers
////////////////////////////////////////////////////////////////////////////////

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
			case 0: dpush(PWR_Voltage() ); return;
			case 1: dpush(RTC_ReadTemperature()); return;
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
		{ dpush(_p10(decimals++)); FnDiv(); }
	else
		{ FnSwap(); dpush(10); FnMul(); }
	FnAdd();
	isEdit = true;
}

void clearDigit() // TODO: check!
{
	if (decimals) 
	{
		f32 a = _p10(--decimals);
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
void FnMAdd() { callScript(SoMAdd); }
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
void FnSqrt() { dpush(2); FnNrt(); }
void FnInv()  { dpush(1.f / dpopx()); }
void FnPw10() { dpush(_p10(dpopx())); }
void FnLg()   { FnLn(); dpush(M_LOG10E); FnMul(); }
void FnPow()  {	callScript(SoPow); }
void FnExp()  {	dpush(_exp(dpopx())); }
void FnLn()   { dpush(_log(dpopx())); }
void FnNrt()  { FnInv(); FnPow(); }

void FnSin()  { dpush(_sin(_to_rad(dpopx())));  }
void FnCos()  { callScript(SoCos); }
void FnTan()  { callScript(SoTan); }
void FnASin() { dpush(_to_deg(_asin(dpopx()))); }
void FnACos() { callScript(SoACos); }
void FnATan() { callScript(SoATan); }

void FnEq()   { dpush(dpop() == dpop()); }
void FnGt()   { dpush(dpop() < dpop()); }
void FnLt()   { FnGt(); dpush(!dpop()); }
void FnNe()   { FnEq(); dpush(!dpop()); }
void FnIf()   { cl++; if (!dpop()) conditionSeek(); }
void FnElse() { cl--; conditionSeek(); }
void FnThen() { cl--; }

void FnSwDR() { isDeg ^= true; }
void FnTime()
{
	if (RTC_ReadDateAndTime()
		&& dpop(rtc_seconds, 0, 59)
		&& dpop(rtc_minutes, 0, 59)
		&& dpop(rtc_hours, 0, 23))
	setupAndSwitchToRTCMode();
}
void FnDate()
{
	if (RTC_ReadDateAndTime()
		&& dpop(rtc_year, 0, 99)
		&& dpop(rtc_month, 1, 12)
		&& dpop(rtc_date, 1, 31))
	setupAndSwitchToRTCMode();
}

void FnD2R()  { if ( isDeg) dpush(dpop() * PI / 180); }
void FnR2D()  { if (!isDeg) dpush(dpop() * 180 / PI); }
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
	/* 27 */ &FnLn,
	/* 28 */ &FnNrt,

	/* 29 */ &FnSin,
	/* 2A */ &FnCos,
	/* 2B */ &FnTan,
	/* 2C */ &FnASin,
	/* 2D */ &FnACos,
	/* 2E */ &FnATan,
	/* 2F */ &FnNop,  // TODO
	/* 30 */ &FnNop,  // TODO
	/* 31 */ &FnNop,  // TODO
	/* 32 */ &FnNop,  // TODO
	/* 33 */ &FnNop,  // TODO
	/* 34 */ &FnNop,  // TODO

	/* 35 */ &FnNop,  // TODO
	/* 36 */ &FnNop,  // TODO
	/* 37 */ &FnNop,  // TODO
	/* 38 */ &FnEq,
	/* 39 */ &FnNe,
	/* 3A */ &FnNop,  // TODO
	/* 3B */ &FnGt,
	/* 3C */ &FnLt,
	/* 3D */ &FnNop,  // TODO
	/* 3E */ &FnIf,
	/* 3F */ &FnElse,
	/* 40 */ &FnThen,

	/* 41 */ &FnSwDR,
	/* 42 */ &FnTime,
	/* 43 */ &FnDate,

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
