// -----------------------------------------------------------------------------
// Watch: shows the time, the date and the temperature of the real-time clock.
// A reset sets the clock to the build time of the sketch. Without a key push
// for 10 seconds the device powers down, a key push powers it up again.
// -----------------------------------------------------------------------------

#define PCB_REVISION 15 // 15 - PCB rev 1.5

#include <TinyRPNCalculator.h>

#define POWEROFF_MILLIS 10000
#define REDRAW_FRAMES   8 // 512 ms at 64 ms per frame

void render()
{
	LCD::Clear();
	TXT::SetFont(FNT::Font5x8);

	// time at scale x2 in pages 0..1, the RTC keeps it in BCD
	TXT::SetScale(TXT::x2, TXT::x2);
	TXT::PrintBCD(RTC::Hours, 0, 0);
	TXT::PrintChar(':', 22, 0);
	TXT::PrintBCD(RTC::Minutes, 33, 0);
	TXT::PrintChar(':', 55, 0);
	TXT::PrintBCD(RTC::Seconds, 66, 0);

	// date as YYYY-MM-DD in page 3
	TXT::SetScale(TXT::x1, TXT::x1);
	TXT::PrintString(F("20"), 0, 3);
	TXT::PrintBCD(RTC::Year, 12, 3);
	TXT::PrintChar('-', 24, 3);
	TXT::PrintBCD(RTC::Month, 30, 3);
	TXT::PrintChar('-', 42, 3);
	TXT::PrintBCD(RTC::Date, 48, 3);

	// temperature in whole degrees, the fraction is ignored
	s08 temp = s08(RTC::ReadTemperature().msb);
	if (temp < 0) TXT::PrintChar('-', 90, 3);
	TXT::PrintBCD(BCD::Encode(temp < 0 ? -temp : temp), 96, 3);
	TXT::PrintChar('C', 114, 3);

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
	RTC::WriteTimeDate();
	powerUp();

	u08 frame = 0;
	while (true)
	{
		// a key push restarts the timer, thus TMR::Millis is the time without keys
		if (KBD::Read() != KBD::NONE) TMR::Start(TMR::TIMEOUT_64MS);

		// power down, the pin change of a key push wakes the MCU up
		if (TMR::Millis >= POWEROFF_MILLIS)
		{
			SYS::Down();
			powerUp();
			frame = 0;
		}

		// read the clock and draw every REDRAW_FRAMES
		if (frame == 0)
		{
			RTC::ReadTimeDate();
			render();
		}
		if (++frame == REDRAW_FRAMES) frame = 0;

		// idle until the next frame
		TMR::Sync();
	}
	return 0;
}
