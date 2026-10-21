// -----------------------------------------------------------------------------
// HP35: a simple HP-35 calculator. The virtual machine runs the original ROM,
// the sketch maps the keys to HP-35 operations and shows the display.
// The F key selects the second function of the next key:
//
//   F 0  square root     F 1  x<>y          F 2  roll down    F 3  subtract
//   F 4  pi              F 5  1/x           F 6  multiply     F 7  sine
//   F 8  cosine          F 9  divide        F .  log          F ^  add
//   F C  clear all       F +- ln            F E  tangent      F F  y^x
//
// Without a key push for 60 seconds the device powers down, a key push powers
// it up again with the same stack.
// -----------------------------------------------------------------------------

#define PCB_REVISION 15 // 15 - PCB rev 1.5

#include <TinyRPNCalculator.h>
#include <Extras/HP35Model.h>
#include <Extras/HPVMClassic.h>

#define POWEROFF_MILLIS  60000
#define FRAMES_PER_SEC   15 // about 64 ms per frame, TMR::TIMEOUT_64MS
#define CYCLES_PER_FRAME (HPVM::CYCLES_PER_SEC / FRAMES_PER_SEC)

// HP-35 operation for each key code (KBD::NUM0..KBD::FUNC), then with F
const u08 key_ops[] PROGMEM =
{
	HPVM::OpNUM0, HPVM::OpNUM1, HPVM::OpNUM2, HPVM::OpNUM3,
	HPVM::OpNUM4, HPVM::OpNUM5, HPVM::OpNUM6, HPVM::OpNUM7,
	HPVM::OpNUM8, HPVM::OpNUM9, HPVM::OpDOT_, HPVM::OpPUSH,
	HPVM::OpCLX_, HPVM::OpCHS_, HPVM::OpEEX_, 0xFF, // F

	HPVM::OpSQRT, HPVM::OpSWAP, HPVM::OpROT_, HPVM::OpSUB_,
	HPVM::OpPI__, HPVM::OpINV_, HPVM::OpMUL_, HPVM::OpSIN_,
	HPVM::OpCOS_, HPVM::OpDIV_, HPVM::OpLOG_, HPVM::OpADD_,
	HPVM::OpCLR_, HPVM::OpLN__, HPVM::OpTAN_, HPVM::OpPOW_
};

b08 shift;
u16 cycles; // VM cycles left from the previous frame

void render()
{
	LCD::Clear();
	TXT::SetFont(FNT::Font5x8);

	// 15 positions of the HP-35 display at scale x1 by x2 in pages 1..2
	TXT::SetScale(TXT::x1, TXT::x2);
	for (u08 i = 0, x = 0; i < sizeof(HPVM::Display); ++i)
	{
		u08 c = HPVM::Display[i];
		if (c == HPVM::BLANK) c = ' ';
		else if (c == HPVM::DASH) c = '-';
		else if (c == HPVM::DOT) c = '.';
		else c += '0';
		TXT::PrintChar(c, x, 1);
		TXT::NextPos(x);
	}

	// shift indicator
	TXT::SetScale(TXT::x1, TXT::x1);
	if (shift) TXT::PrintChar('F', 122, 0);

	LCD::Flip();
}

void powerUp()
{
	LCD::TurnOn();
	TMR::Start(TMR::TIMEOUT_64MS);
}

int main()
{
	SYS::Init();
	powerUp();

	u08 last = KBD::NONE;
	while (true)
	{
		// a key push restarts the timer, thus TMR::Millis is the time without keys
		u08 key = KBD::Read();
		if (key != KBD::NONE) TMR::Start(TMR::TIMEOUT_64MS);

		// power down, the pin change of a key push wakes the MCU up
		if (TMR::Millis >= POWEROFF_MILLIS)
		{
			SYS::Down();
			powerUp();
			render();
		}

		// a new key push gives an operation to the VM, F changes the shift
		if (key != last && key != KBD::NONE)
		{
			u08 op = MEM::ProgRead(key_ops + (shift ? 16 : 0) + key);
			if (op == 0xFF)
				shift = true;
			else
			{
				shift = false;
				HPVM::Operation(op);
			}
			render();
		}
		last = key;

		// run the ROM in real time, draw at most once per frame when it
		// changes the display, the rest of the cycles runs in the next frame
		u16 n = cycles + CYCLES_PER_FRAME;
		for (; n > 0; --n)
		{
			if (HPVM::Cycle())
			{
				render();
				break;
			}
		}
		cycles = n;

		// idle until the next frame
		TMR::Sync();
	}
	return 0;
}
