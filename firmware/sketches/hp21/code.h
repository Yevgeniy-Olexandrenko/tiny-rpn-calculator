#pragma once

// -----------------------------------------------------------------------------
// Working mode and keyboard input, shared by all modes
// -----------------------------------------------------------------------------

enum { MODE_RTC, MODE_CALC };
u08 mode;

u08 newKey;
u08 oldkey;

// slow mode (RTC) redraws every 512 ms only, so the mcu sleeps
// most of the time, while the keys are still read every frame
u08 slowPhase; // TMR::Millis / 512 at the last redraw, 0xFF forces one

b08 slowRedraw()
{
	u08 phase = u08(TMR::Millis >> 9);
	if (phase == slowPhase) return false;
	slowPhase = phase;
	return true;
}

// -----------------------------------------------------------------------------
// RTC mode
// -----------------------------------------------------------------------------

u08 battery;

void renderRTCMode()
{
	LCD::Clear();

	TXT::SetFont(digits7x32);
	TXT::PrintChar(RTC_TIME_COLON, RTC_POS_COLON_HM, 0);
	TXT::PrintChar(RTC_TIME_COLON, RTC_POS_COLON_MS, 0);
	TXT::PrintSegBCD(RTC::Hours,   RTC_POS_HOURS, 0);
	TXT::PrintSegBCD(RTC::Minutes, RTC_POS_MINUTES, 0);
	TXT::PrintSegBCD(RTC::Seconds, RTC_POS_SECONDS, 0);

	TXT::SetFont(menu5x8);
	u08 month = BCD::Decode(RTC::Month);
	TXT::PrintString(DPSTR(strMonth), month - 1, RTC_POS_MONTH, 0);
	TXT::PrintBCD(RTC::Date, RTC_POS_DATE, 0);
	TXT::PrintBCD(RTC::Year, RTC_POS_YEAR, 0);
	TXT::PrintChar(RTC_DATE_SLASH, RTC_POS_SLASH, 0);

	TXT::SetScale(TXT::x2, TXT::x1);
	TXT::PrintString(D(RTC_INFO_HP_LABEL), RTC_POS_INFO, 2);
	// blink the last bar while charging, toggling it every 512 ms
	u08 bars = battery;
	if (bars && (TMR::Millis & 0x200) && PWR::Charging()) --bars;
	for (u08 i = bars, x = RTC_POS_INFO; i > 0; --i)
	{ 
		TXT::PrintChar(RTC_BATTERY_LEVEL, x, 1);
		x += TXT::char_dx;
	}

	LCD::Flip();
}

// returns false if a key is pressed to leave the mode
b08 updateRTCMode()
{
	if (newKey != KBD::NONE) return false;
	if (slowRedraw())
	{
		RTC::ReadTimeDate();
		renderRTCMode();
	}
	return true;
}

void enterRTCMode()
{
	battery = (u08)((PWR::Level() + 10U) / 20U); // 0..100% -> 0..5 bars

	LCD::TurnOn();
	TMR::Start(TMR::TIMEOUT_64MS);

	oldkey    = KBD::Read();
	slowPhase = 0xFF;
	mode      = MODE_RTC;
}

NOINLINE
void setupAndEnterRTCMode()
{
	RTC::WriteTimeDate();
	enterRTCMode();
}

// -----------------------------------------------------------------------------
// Calculator mode
// -----------------------------------------------------------------------------

b08 isFunc;
b08 isMenu;

Menu menu;
u08  select;

u16 cycles;
u08 hidden[sizeof(HPVM::Display)];
u08 lastOp = KEY_NOP; // nothing to repeat before the first menu operation

void enterMenu(u08 type)
{
	isMenu = true;
	MEM::ProgCopy(&menu, &menus[type], sizeof(Menu));
	select = 0;
}

void dropStack(u08 n)
{
	using namespace HPVM;
	// roll down via ROM to end number entry and refresh display,
	// then restore T to get a drop: C, Y, Z <- Y, Z, T
	do
	{
		OpAndWait(OpROT_);
		reg_move(T, Z, 0, 13, COPY);
	}
	while (--n);
}

b08 getBCD(const HPVM::reg& r, u08& out, u08 min, u08 max)
{
	// positive integer 0..999: mantissa sign, exponent sign and
	// exponent tens are zero, exponent units 0..2, digits of the
	// mantissa after the integer part are zero (no fraction)
	if ((r[13] | r[2] | r[1]) == 0 && r[0] < 3)
	{
		u08 last = 12 - r[0]; // the lowest digit of the integer part
		u16 value = 0;
		for (u08 i = 12; i > 2; --i)
		{
			if (i >= last)
				value = value * 16 + r[i];
			else if (r[i])
				return false;
		}
		if (value >= min && value <= max)
		{
			out = u08(value);
			return true;
		}
	}
	return false;
}

void executeOperation(u08 operation)
{
	isFunc = false;
	isMenu = false;
	// repeat the last menu operation, including extended ones
	if (operation == KEY_LAST) operation = lastOp;

	switch (operation)
	{
		default:
			HPVM::Operation(operation);
			break;

		case KEY_FUNC:
			HPVM::DisplayY(hidden);
			isFunc = true;
			break;

		case KEY_ROTU:
			HPVM::Operation(HPVM::OpROT_);
			HPVM::Operation(HPVM::OpROT_);
			HPVM::Operation(HPVM::OpROT_);
			break;

		// DEG/RAD switch is wired to the F2 flag input
		case PROG_DEG:
			HPVM::F2 = false;
			break;

		case PROG_RAD:
			HPVM::F2 = true;
			break;

		case MENU_MATH:
			enterMenu(MENU_MATH_OPS);
			break;

		case MENU_TRIG:
			enterMenu(MENU_TRIG_OPS);
			break;

		case MENU_PROG:
			enterMenu(MENU_PROG_OPS);
			break;

		case PROG_TIME:
			// stack: X - seconds, Y - minutes, Z - hours, the values are
			// dropped if all are valid, otherwise the error is shown
			RTC::ReadTimeDate();
			if (getBCD(HPVM::C, RTC::Seconds, 0x00, 0x59) &&
			    getBCD(HPVM::Y, RTC::Minutes, 0x00, 0x59) &&
			    getBCD(HPVM::Z, RTC::Hours,   0x00, 0x23))
			{
				dropStack(3);
				setupAndEnterRTCMode();
			}
			else HPVM::Error();
			break;

		case PROG_DATE:
			// stack: X - year, Y - month, Z - date, the date is checked
			// against the days in the month, leap years included
			RTC::ReadTimeDate();
			if (getBCD(HPVM::C, RTC::Year,  0x00, 0x99) &&
			    getBCD(HPVM::Y, RTC::Month, 0x01, 0x12) &&
			    getBCD(HPVM::Z, RTC::Date,  0x01, 0x31) &&
			    BCD::Decode(RTC::Date) <= RTC::GetDaysInMonth())
			{
				dropStack(3);
				setupAndEnterRTCMode();
			}
			else HPVM::Error();
			break;

		case KEY_NOP:
			break;
	}
}

void renderCalcNumber(const u08 * segs, u08 y)
{
	// the dot is a part of the display position, but drawn as a separate char
	for (u08 i = 0, x = 0; i < sizeof(HPVM::Display); ++i)
	{
		u08 seg = segs[i];
		TXT::PrintSeg(seg & ~TXT::SEG_DOT, x, y);
		x += TXT::char_dx;
		if (seg & TXT::SEG_DOT)
		{
			TXT::PrintSeg(TXT::SEG_DOT, x, y);
			x += TXT::char_dx;
		}
	}
}

void renderCalcMenu()
{
	TXT::SetFont(menu5x8);
	TXT::SetScale(TXT::x1, TXT::x2);
	TXT::SetInverse(true);
	for (u08 i = 0; i < MENU_ITEMS_PER_LINE; ++i)
	{
		TXT::PrintString(
			DPSTR(menu.string), select * MENU_ITEMS_PER_LINE + i,
			MENU_POS_ITEM * i, 2);
	}
	TXT::SetInverse(false);
}

void renderCalcMode()
{
	LCD::Clear();
	TXT::SetFont(digits7x16);

	u08 flagsY = isMenu ? 0 : 2;
	for (u08 i = 0; i < sizeof(HPVM::M1); ++i)
	{
		if (HPVM::M1[i])
		{
			TXT::PrintChar(FLAG_STORAGE, FLAG_POS, flagsY);
			break;
		}
	}
	if (HPVM::F2) TXT::PrintChar(FLAG_RADIANS, FLAG_POS_2, flagsY);

	if (isFunc)
	{
		renderCalcNumber(hidden, 0);
		renderCalcNumber(HPVM::Display, 2);
		TXT::PrintChar(FLAG_FUNCTION, FLAG_POS, 0);
	}
	else if (isMenu)
	{
		renderCalcNumber(HPVM::Display, 0);
		renderCalcMenu();
	}
	else
	{
		TXT::SetFont(digits7x32);
		renderCalcNumber(HPVM::Display, 0);
	}
	
	LCD::Flip();
}

void updateCalcMode()
{
	if (isMenu)
	{
		if (newKey != KBD::NONE)
		{
			switch(newKey)
			{
				default: isMenu = false; break;
				case KBD::ROTU: if (select > 0) select--; else select = menu.lastIdx; break;
				case KBD::ROTD: if (select < menu.lastIdx) select++; else select = 0; break;
				case KBD::MATH: enterMenu(MENU_MATH_OPS); break;
				case KBD::TRIG: enterMenu(MENU_TRIG_OPS); break;
				case KBD::PROG: enterMenu(MENU_PROG_OPS); break;
				case KBD::SEL1: case KBD::SEL2: case KBD::SEL3:
					u08 index = select * MENU_ITEMS_PER_LINE + (newKey - KBD::SEL1);
					u08 op = MEM::DataRead(menu.opsBase + index);
					executeOperation(lastOp = op);
					break;
			}
			renderCalcMode();
		}
	}
	else
	{
		if (newKey != KBD::NONE)
		{
			u08 op = MEM::DataRead(mainOps + (isFunc ? 16 : 0) + newKey);
			executeOperation(op);
			renderCalcMode();
		}
		
		// local counter, unused cycles are carried to the next frame
		u16 n = cycles + HPVM_CYCLES_PER_FRAME;
		for (; n > 0; --n)
		{
			if (HPVM::Cycle())
			{
				renderCalcMode();
				break;
			}
		}
		cycles = n;
	}
}

void enterCalcMode()
{
	mode = MODE_CALC;
	renderCalcMode();
}
// -----------------------------------------------------------------------------
// Enter point and main cycle
// -----------------------------------------------------------------------------

int main() 
{
	// init system hardware
	SYS::Init();

	// switch to rtc operation mode on reset
	setupAndEnterRTCMode();

	while (true)
	{
		// get time passed since last operation mode switch
		u16 timeout = TMR::Millis;

		// handle display brightness change
		LCD::Brightness(mode == MODE_RTC || timeout >= DIMOUT_MILLIS ? 0x00 : 0xFF);

		// handle power down condition
		if (timeout >= POWEROFF_MILLIS)
		{
			// power down and go to sleeping
			SYS::Down();

			// power up an switch to rtc operation mode
			enterRTCMode();
		}

		// read key press, any key restarts the timeouts
		newKey = KBD::Read();
		if (newKey != oldkey) oldkey = newKey; else newKey = KBD::NONE;
		if (newKey != KBD::NONE) TMR::Start(TMR::TIMEOUT_64MS);

		// update current operation mode, switch it on a leaving key
		switch (mode)
		{
			case MODE_RTC:
				// any key goes to calculator
				if (!updateRTCMode()) enterCalcMode();
				break;

			default:
				updateCalcMode();
				break;
		}

		// idle until next frame
		TMR::Sync();
	}
	return 0;
}
