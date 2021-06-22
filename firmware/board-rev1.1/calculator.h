
bool isFunc;
bool isMenu;

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
	KEY_PUSH = KEY_D3,
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
	OpNum0 = 0x00, OpNum1, OpNum2, OpNum3, OpNum4, OpNum5, OpNum6, OpNum7,
	OpNum8, OpNum9, OpDot, OpPush, OpClr, OpChSg, OpEnEx, OpFunc,

	// basic functions
	OpConst = 0x10, OpRcl, OpSto, OpSub, OpFunc3, OpFung4, OpMul, OpFunc1,
	OpFunc2, OpDiv, OpSwap, OpAdd, OpPwr, OpRotD, OpRotU, OpMenu,

	// menu functions
	// TODO
} Operation;

void enterNumber(uint8_t number)
{
	//
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
	//
}

void OpPush()
{
	//
}

void OpClr()
{
	//
}

void OpChSg()
{
	//
}

void OpEnExp()
{
	//
}

void OpFunc()
{
	//
}

void OpConst()
{
	//
}
 void OpRcl()
 {
	 //
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