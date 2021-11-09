#pragma once

// -----------------------------------------------------------------------------
// Switch between working modes
// -----------------------------------------------------------------------------

b08 calcMode;
u08 battery;

u08 newKey;
u08 oldkey;

void switchToCalcMode(b08 yes = true)
{
	calcMode = yes;
	FPS::SyncStart(FPS::TIMEOUT_15_FPS);
}

void switchToRTCMode()
{
	LCD::TurnOn();
	battery = (u08)((PWR::Level() * 5 + 50) / 100);
	switchToCalcMode(false);
	oldkey = KBD::Read();
}

NOINLINE void setupAndSwitchToRTCMode()
{
	RTC::WriteTimeDate();
	switchToRTCMode();
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
u08 lastOp = HPVM::OpNONE;

void enterMenu(u08 type)
{
	isMenu = true;
	memcpy_P(&menu, &menus[type], sizeof(Menu));
	select = 0;
}

void dropStackTop()
{
	hpvm_iterate_word(
		HPVM::C[i] = HPVM::D[i];
		HPVM::D[i] = HPVM::E[i];
		HPVM::E[i] = HPVM::F[i];
	);
}

b08 getBCDFromStack(u08& out, u08 min, u08 max)
{
	if (HPVM::C[1] == 0 && HPVM::C[0] < 3)
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

void executeOperationAndWait(u08 operation)
{
	HPVM::Operation(operation);
	do HPVM::Cycle(); while (!HPVM::Idling());
}

void executeOperation(u08 operation)
{
	isFunc = false;
	isMenu = false;

	switch (operation)
	{
		default:
			HPVM::Operation(operation);
			break;

		case KEY_FUNC:
			executeOperationAndWait(HPVM::OpSWAP);
			for (u08 i = 0; i < 15; ++i) hidden[i] = HPVM::Display[i];
			executeOperationAndWait(HPVM::OpSWAP);
			isFunc = true;
			break;

		case KEY_LAST:
			HPVM::Operation(lastOp);
			break;

		case KEY_ROTU:
			executeOperationAndWait(HPVM::OpROT);
			executeOperationAndWait(HPVM::OpROT);
			HPVM::Operation(HPVM::OpROT);
			break;

		case KEY_MADD:
			executeOperationAndWait(HPVM::OpRCL);
			executeOperationAndWait(HPVM::OpSWAP);
			executeOperationAndWait(HPVM::OpSTO);
			executeOperationAndWait(HPVM::OpADD);
			executeOperationAndWait(HPVM::OpRCL);
			executeOperationAndWait(HPVM::OpSWAP);
			executeOperationAndWait(HPVM::OpSTO);
			HPVM::Operation(HPVM::OpROT);
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
		
		case TRIG_ASIN:
			executeOperationAndWait(HPVM::OpARC);
			HPVM::Operation(HPVM::OpSIN);
			break;

		case TRIG_ACOS:
			executeOperationAndWait(HPVM::OpARC);
			HPVM::Operation(HPVM::OpCOS);
			break;
		
		case TRIG_ATAN:
			executeOperationAndWait(HPVM::OpARC);
			HPVM::Operation(HPVM::OpTAN);
			break;

		case PROG_TIME:
			RTC::ReadTimeDate();
			if (getBCDFromStack(RTC::Minutes, 0x00, 0x59) && 
			    getBCDFromStack(RTC::Hours,   0x00, 0x23))
			{
				RTC::Seconds = 0x00;
				setupAndSwitchToRTCMode();
			}
			break;

		case PROG_DATE:
			RTC::ReadTimeDate();
			if (getBCDFromStack(RTC::Month, 0x01, 0x12) &&
			    getBCDFromStack(RTC::Date,  0x01, 0x31))
			{
				setupAndSwitchToRTCMode();
			}
			break;

		case PROG_YEAR:
			RTC::ReadTimeDate();
			if (getBCDFromStack(RTC::Year, 0x00, 0x99))
			{
				setupAndSwitchToRTCMode();
			}
			break;
	}
}

// -----------------------------------------------------------------------------
// Calculator mode
// -----------------------------------------------------------------------------

void renderCalcNumber(const u08 * numStr, u08 y)
{
	for (u08 i = 0; i < 15; ++i)
	{
		if (i == 12) TXT::SetFont(digits7x16);

		u08 ch = numStr[i], seg = TXT::SEG_SPACE;
		if (ch == HPVM_DASH) seg = TXT::SEG_DASH;
		else if (ch == HPVM_DOT) seg = TXT::SEG_DOT;
		else if (ch != HPVM_SPACE) seg = TXT::NumToSeg(ch);

		TXT::PrintSeg(seg, i * TXT::char_dx, y);
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
			FPSTR(menu.string), select * MENU_ITEMS_PER_LINE + i,
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
					u08 op = pgm_read_byte(menu.opsBase + index);
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
			u08 op = pgm_read_byte(mainOps + (isFunc ? 16 : 0) + newKey);
			executeOperation(op);
			renderCalcMode();
		}
		
		for (cycles += HPVM_CYCLES_PER_FRAME; cycles > 0; --cycles)
		{
			if (HPVM::Cycle()) 
			{
				renderCalcMode();
				break;
			}
		}
	}
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
	TXT::PrintSegBCD(RTC::Hours, RTC_POS_HOURS, 0);
	TXT::PrintSegBCD(RTC::Minutes, RTC_POS_MINUTES, 0);
	TXT::PrintSegBCD(RTC::Seconds, RTC_POS_SECONDS, 0);

	TXT::SetFont(menu5x8);
	u08 monthIndex = BCD_Decode(RTC::Month) - 1;
	TXT::PrintString(FPSTR(strMonth), monthIndex, RTC_POS_MONTH, 0);
	TXT::PrintBCD(RTC::Date, RTC_POS_DATE, 0);
	TXT::PrintBCD(RTC::Year, RTC_POS_YEAR, 0);
	TXT::PrintChar(RTC_DATE_SLASH, RTC_POS_SLASH, 0);

	TXT::SetScale(TXT::x2, TXT::x1);
	TXT::PrintString(F(RTC_INFO_HP_LABEL), RTC_POS_INFO, 2);
	for (u08 i = 0; i < battery; ++i) 
		TXT::PrintChar(RTC_BATTERY_LEVEL, RTC_POS_INFO + i * TXT::char_dx, 1);

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
	// switch to rtc operation mode on reset
	setupAndSwitchToRTCMode();

	while (true)
	{
		// get time passed since last operation mode switch
		uint16_t timePassedMs = FPS::SyncMillis();

		// handle display brightness change
		LCD::Brightness(calcMode && timePassedMs < DIMOUT_MILLIS ? 0xFF : 0x00);

		// handle power down condition
		if (timePassedMs >= POWEROFF_MILLIS)
		{
			// power down and go to sleeping
			FPS::SyncStop();
			LCD::TurnOff();
			PWR::Down();

			// power up an switch to rtc operation mode
			switchToRTCMode();
		}

		// read key press and switch to calculator operation mode
		newKey = KBD::Read();
		if (newKey != oldkey) oldkey = newKey; else newKey = KBD::NONE;
		if (newKey != KBD::NONE) switchToCalcMode();

		// update current operation mode and idle until next frame
		if (calcMode) updateCalcMode(); else updateRTCMode();
		FPS::SyncWait();
	}
	return 0;
}
