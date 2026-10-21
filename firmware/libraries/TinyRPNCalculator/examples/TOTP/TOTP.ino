// -----------------------------------------------------------------------------
// TOTP: shows the 6-digit time-based one-time password (RFC 6238) of one key
// and the seconds until the next code. The key is the RFC 6238 test key, thus
// any TOTP generator with the same key and time gives the same code.
// A reset sets the clock to the build time of the sketch: set GMT_ZONE to the
// time zone of the computer that builds the sketch.
// -----------------------------------------------------------------------------

#define PCB_REVISION 15 // 15 - PCB rev 1.5

#include <TinyRPNCalculator.h>
#include <Extras/TOTP.h>

#define GMT_ZONE        (+3) // hours, the RTC keeps the local time
#define POWEROFF_MILLIS 30000
#define REDRAW_FRAMES   8    // 512 ms at 64 ms per frame

// RFC 6238 test key for HMAC-SHA1, Base32: GEZDGNBVGY3TQOJQGEZDGNBVGY3TQOJQ
const char secret[] PROGMEM = "12345678901234567890";

TOTP::Key key;

// prints the number as the given count of decimal digits, with leading zeros,
// from right to left: x is the position after the last digit
void printNumber(u32 n, u08 digits, u08 x, u08 y)
{
	for (; digits > 0; --digits, n /= 10)
	{
		TXT::PrevPos(x);
		TXT::PrintChar('0' + n % 10, x, y);
	}
}

void render()
{
	LCD::Clear();
	TXT::SetFont(FNT::Font5x8);

	// time stamp is 0 if the date in the RTC is not valid
	u32 time = RTC::GetTimestamp(GMT_ZONE);
	if (time)
	{
		// code at the right edge at scale x2, seconds left in the bottom line
		TXT::SetScale(TXT::x2, TXT::x2);
		printNumber(TOTP::GetCode(key, time / TOTP::PERIOD), 6, LCD::WIDTH, 0);

		TXT::SetScale(TXT::x1, TXT::x1);
		TXT::PrintString(F("SECONDS LEFT"), 0, 3);
		printNumber(TOTP::PERIOD - time % TOTP::PERIOD, 2, LCD::WIDTH, 3);
	}
	else
	{
		TXT::PrintString(F("NO DATE"), 0, 0);
	}

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

	// a key from DATAMEM can be read with TOTP::ReadKey(), refer to TOTP.h
	key.length = sizeof(secret) - 1;
	MEM::ProgCopy(key.data, secret, key.length);

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
