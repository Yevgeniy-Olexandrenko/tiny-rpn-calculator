#pragma once

// -----------------------------------------------------------------------------
// Switch between working modes
// -----------------------------------------------------------------------------

b08 rtcMode;
b08 totpMode;
u08 battery;

u08 newKey;
u08 oldkey;

void enterCalcMode()
{
	if (newKey != KBD::NONE)
	{
		TMR::Start(TMR::TIMEOUT_64MS);
		if (rtcMode)
		{
			newKey  = KBD::NONE;
			rtcMode = false;

			void renderCalcMode();
			renderCalcMode();
		}
	}
}

void enterRTCMode()
{
	battery = (u08)((PWR::Level() + 10U) / 20U); // 0..100% -> 0..5 bars

	LCD::TurnOn();
	TMR::Start(TMR::TIMEOUT_64MS);

	oldkey   = KBD::Read();
	rtcMode  = true;
	totpMode = false;
}

NOINLINE void setupAndEnterRTCMode()
{
	RTC::WriteTimeDate();
	enterRTCMode();
}

// -----------------------------------------------------------------------------
// Calculator operations execution
// -----------------------------------------------------------------------------

b08 isFunc;
b08 isMenu;

Menu menu;
u08  select;

u16 cycles;
u08 hidden[15];
u08 lastOp = HPVM::OpCLR_;

// RTC keeps local time, RAM survives sleeping and any reset
// rewrites RTC with the build time anyway, so no need to store
s08 gmtZone = TOTP_GMT_ZONE;

void enterMenu(u08 type)
{
	isMenu = true;
	MEM::ProgCopy(&menu, &menus[type], sizeof(Menu));
	select = 0;
}

void dropStackTop()
{
	using namespace HPVM;
	// roll down via ROM to end number entry and refresh display,
	// then restore T to get a drop: C, D, E <- D, E, F
	OpAndWait(OpROT_);
	reg_move(F, E, 0, 13, COPY);
}

b08 getBCDFromStack(u08& out, u08 min, u08 max)
{
	// positive integer 0..999: mantissa sign, exponent sign and
	// exponent tens are zero, exponent units 0..2
	if ((HPVM::C[13] | HPVM::C[2] | HPVM::C[1]) == 0 && HPVM::C[0] < 3)
	{
		u16 value = 0;
		for (u08 i = 0; i <= HPVM::C[0]; ++i)
		{
			value *= 16;
			value += HPVM::C[12 - i];
		}
		if (value >= min && value <= max)
		{
			out = u08(value);
			dropStackTop();
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
			HPVM::OpAndWait(HPVM::OpSWAP);
			memcpy(hidden, HPVM::Display, 15);
			HPVM::OpAndWait(HPVM::OpSWAP);
			isFunc = true;
			break;

		case KEY_ROTU:
			HPVM::OpAndWait(HPVM::OpROT_);
			HPVM::OpAndWait(HPVM::OpROT_);
			HPVM::Operation(HPVM::OpROT_);
			break;

		case KEY_MADD:
			HPVM::OpAndWait(HPVM::OpRCL_);
			HPVM::OpAndWait(HPVM::OpSWAP);
			HPVM::OpAndWait(HPVM::OpSTO_);
			HPVM::OpAndWait(HPVM::OpADD_);
			HPVM::OpAndWait(HPVM::OpRCL_);
			HPVM::OpAndWait(HPVM::OpSWAP);
			HPVM::OpAndWait(HPVM::OpSTO_);
			HPVM::Operation(HPVM::OpROT_);
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
			// stack: X - seconds, Y - minutes, Z - hours
			RTC::ReadTimeDate();
			if (getBCDFromStack(RTC::Seconds, 0x00, 0x59) &&
			    getBCDFromStack(RTC::Minutes, 0x00, 0x59) &&
			    getBCDFromStack(RTC::Hours,   0x00, 0x23))
			{
				setupAndEnterRTCMode();
			}
			break;

		case PROG_DATE:
			// stack: X - year, Y - month, Z - date
			RTC::ReadTimeDate();
			if (getBCDFromStack(RTC::Year,  0x00, 0x99) &&
			    getBCDFromStack(RTC::Month, 0x01, 0x12) &&
			    getBCDFromStack(RTC::Date,  0x01, 0x31))
			{
				setupAndEnterRTCMode();
			}
			break;

		case PROG_TOTP:
			void enterTOTPMode();
			enterTOTPMode();
			break;

		case PROG_ZONE:
		{
			// signed integer, mantissa sign is 9 for negative numbers
			u08 sign = HPVM::C[13];
			u08 zone;
			HPVM::C[13] = 0;
			if (getBCDFromStack(zone, 0x00, 0x14))
			{
				gmtZone = BCD::Decode(zone);
				if (sign) gmtZone = -gmtZone;
			}
			else HPVM::C[13] = sign;
			break;
		}

		case KEY_NOP:
			break;
	}
}

// -----------------------------------------------------------------------------
// Calculator mode
// -----------------------------------------------------------------------------

void renderCalcNumber(const u08 * numStr, u08 y)
{
	for (u08 i = 0, x = 0; i < 15; ++i, x += TXT::char_dx)
	{
		if (i == 12) TXT::SetFont(digits7x16);
		u08 seg = TXT::NumToSeg(numStr[i]);
		TXT::PrintSeg(seg, x, y);
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

	for (u08 i = 0; i < 14; ++i)
	{
		if (HPVM::M[i])
		{
			TXT::PrintChar(FLAG_STORAGE, FLAG_POS, isMenu ? 0 : 2);
			break;
		}
	}

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

// -----------------------------------------------------------------------------
// TOTP mode
// -----------------------------------------------------------------------------

u08 totpIndex;
u08 totpCount;
u32 totpCounter;
u32 totpCode;

void enterTOTPMode()
{
	// count the keys, stay in calculator mode if there are none
	TOTP::Key key;
	for (totpCount = 0; TOTP::ReadKey(totp_keys, totpCount, key); ++totpCount);
	if (totpCount)
	{
		if (totpIndex >= totpCount) totpIndex = 0;
		totpCounter = 0;
		totpMode = true;
	}
}

void renderTOTPMode(const TOTP::Key& key, u08 left)
{
	LCD::Clear();

	// code as two groups of three digits
	TXT::SetFont(digits7x32);
	u32 code = totpCode;
	for (u08 i = 6; i-- > 0;)
	{
		u08 x = TOTP_POS_CODE + (i < 3 ? i : i + 1) * TOTP_CHAR_W;
		TXT::PrintSeg(TXT::NumToSeg(code % 10), x, 0);
		code /= 10;
	}

	// seconds left in place of exponent
	TXT::SetFont(digits7x16);
	TXT::PrintSegBCD(BCD::Encode(left), TOTP_POS_LEFT, 0);

	// key label under seconds, looks like a menu item
	TXT::SetFont(menu5x8);
	TXT::SetScale(TXT::x1, TXT::x2);
	TXT::SetInverse(true);
	for (u08 i = 0, x = TOTP_POS_LABEL; i < TOTP::LABEL_SIZE + 2; ++i, x += TXT::char_dx)
	{
		u08 ch = (i == 0 || i > TOTP::LABEL_SIZE ? ' ' : key.label[i - 1]);
		TXT::PrintChar(ch, x, 2);
	}
	TXT::SetInverse(false);

	LCD::Flip();
}

void updateTOTPMode()
{
	if (newKey != KBD::NONE)
	{
		// select previous or next key, any other key returns to calculator
		switch (newKey)
		{
			default:
				totpMode = false;
				renderCalcMode();
				return;
			case KBD::ROTU:
				totpIndex = (totpIndex ? totpIndex : totpCount) - 1;
				break;
			case KBD::ROTD:
				if (++totpIndex == totpCount) totpIndex = 0;
				break;
		}
		totpCounter = 0;
	}

	TOTP::Key key;
	TOTP::ReadKey(totp_keys, totpIndex, key);

	// recompute the code when its time step or the key changes
	RTC::ReadTimeDate();
	u32 time = RTC::GetTimestamp(gmtZone);
	u32 counter = time / TOTP::PERIOD;
	if (counter != totpCounter)
	{
		totpCounter = counter;
		totpCode = TOTP::GetCode(key, counter);
	}
	renderTOTPMode(key, TOTP::PERIOD - u08(time - counter * TOTP::PERIOD));
}

// -----------------------------------------------------------------------------
// RTC mode
// -----------------------------------------------------------------------------

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

void updateRTCMode()
{
	RTC::ReadTimeDate();
	renderRTCMode();
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
		LCD::Brightness(rtcMode || timeout >= DIMOUT_MILLIS ? 0x00 : 0xFF);

		// handle power down condition
		if (timeout >= POWEROFF_MILLIS)
		{
			// power down and go to sleeping
			SYS::Down();

			// power up an switch to rtc operation mode
			enterRTCMode();
		}

		// read key press and switch to calculator operation mode
		newKey = KBD::Read();
		if (newKey != oldkey) oldkey = newKey; else newKey = KBD::NONE;
		enterCalcMode();

		// update current operation mode and idle until next frame
		if (rtcMode) updateRTCMode();
		else if (totpMode) updateTOTPMode();
		else updateCalcMode();
		TMR::Sync();
	}
	return 0;
}
