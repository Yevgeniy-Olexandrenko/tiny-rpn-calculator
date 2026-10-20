// -----------------------------------------------------------------------------
// Keyboard and power: shows the code of the last pushed key, the battery
// voltage and level, and the charging state. Without a key push for 10 seconds
// the device powers down, a key push powers it up again.
// -----------------------------------------------------------------------------

#define PCB_REVISION 15 // 15 - PCB rev 1.5

#include <TinyRPNCalculator.h>

#define POWEROFF_MILLIS 10000
#define REDRAW_FRAMES   8 // 512 ms at 64 ms per frame

// prints the number as the given count of decimal digits, with leading zeros
void printNumber(u16 n, u08 digits, u08 x, u08 y)
{
	for (x += digits * TXT::char_dx; digits > 0; --digits, n /= 10)
	{
		x -= TXT::char_dx;
		TXT::PrintChar('0' + n % 10, x, y);
	}
}

void render(u08 key)
{
	LCD::Clear();
	TXT::SetFont(FNT::Font5x8);

	TXT::PrintString(F("KEY"), 0, 0);
	if (key == KBD::NONE)
		TXT::PrintString(F("--"), 36, 0);
	else
		printNumber(key, 2, 36, 0);

	TXT::PrintString(F("VCC"), 0, 1);
	printNumber(PWR::Voltage(), 4, 36, 1);
	TXT::PrintString(F("mV"), 66, 1);

	TXT::PrintString(F("BAT"), 0, 2);
	printNumber(PWR::Level(), 3, 36, 2);
	TXT::PrintChar('%', 60, 2);

	if (PWR::Charging()) TXT::PrintString(F("CHARGING"), 0, 3);

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
	u08 frame = 0;
	while (true)
	{
		// a key push restarts the timer, thus TMR::Millis is the time without keys
		u08 key = KBD::Read();
		if (key != KBD::NONE)
		{
			TMR::Start(TMR::TIMEOUT_64MS);
			if (key != last) frame = 0;
			last = key;
		}

		// power down, the pin change of a key push wakes the MCU up
		if (TMR::Millis >= POWEROFF_MILLIS)
		{
			SYS::Down();
			powerUp();
			frame = 0;
		}

		// draw on a new key and every REDRAW_FRAMES
		if (frame == 0) render(last);
		if (++frame == REDRAW_FRAMES) frame = 0;

		// idle until the next frame
		TMR::Sync();
	}
	return 0;
}
