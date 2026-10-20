// -----------------------------------------------------------------------------
// Display: shows text with the built-in font in all scales and in inverse.
// The screens change every 2 seconds.
// -----------------------------------------------------------------------------

#define PCB_REVISION 15 // 15 - PCB rev 1.5

#include <TinyRPNCalculator.h>

#define SCREEN_MILLIS 2000
#define SCREEN_COUNT  3

void renderScreen(u08 screen)
{
	LCD::Clear();
	TXT::SetFont(FNT::Font5x8);

	switch (screen)
	{
		case 0:
			// scale x1: 21 characters in each of 4 lines
			TXT::SetInverse(true);
			TXT::PrintString(F(" TINY RPN CALCULATOR "), 0, 0);
			TXT::SetInverse(false);
			TXT::PrintString(F("Font 5x8, scale x1"), 0, 1);
			TXT::PrintString(F("0123456789 +-*/=.:"), 0, 2);
			TXT::PrintString(F("abcdefghijklmnopqrstu"), 0, 3);
			break;

		case 1:
			// scale x2: 11 characters in each of 2 lines
			TXT::SetScale(TXT::x2, TXT::x2);
			TXT::PrintString(F("Scale x2"), 0, 0);
			TXT::PrintString(F("12:34:56"), 0, 2);
			break;

		case 2:
			// scale x4: 6 characters in 1 line
			TXT::SetScale(TXT::x4, TXT::x4);
			TXT::PrintString(F("x4 RPN"), 0, 0);
			break;
	}

	LCD::Flip();
}

int main()
{
	SYS::Init();
	LCD::TurnOn();
	TMR::Start(TMR::TIMEOUT_64MS);

	u08 screen = 0xFF;
	while (true)
	{
		// TMR::Millis counts from TMR::Start() and overflows after 65 s
		u08 next = (TMR::Millis / SCREEN_MILLIS) % SCREEN_COUNT;
		if (TMR::Millis >= SCREEN_MILLIS * SCREEN_COUNT) TMR::Start(TMR::TIMEOUT_64MS);
		if (next != screen) renderScreen(screen = next);

		// idle until the next frame
		TMR::Sync();
	}
	return 0;
}
