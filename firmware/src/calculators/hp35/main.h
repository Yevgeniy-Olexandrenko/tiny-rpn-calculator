#pragma once

// -----------------------------------------------------------------------------
// Switch between working modes
// -----------------------------------------------------------------------------

b08 calcMode;
u08 battery;

u08 key;
u08 oldkey;

void switchToCalcMode(b08 yes = true)
{
	calcMode = yes;
	FPS::SyncStart();
}

void switchToRTCMode()
{
	LCD::TurnOn();
	battery = (u08)((PWR::Level() * 4 + 50) / 100);
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

void enterMenu(u08 type)
{
	isMenu = true;
	memcpy_P(&menu, &menus[type], sizeof(Menu));
	select = 0;
}

void executeOperationAndWait(u08 operation)
{
	HPVM::Operation(operation);
	do
	{
		for (cycles += HP35_CYCLES_PER_FRAME; cycles > 0; --cycles) 
		{
			HPVM::Cycle();
		}
	}
	while (!HPVM::Idling);
}

b08 extractNumber(u08& out, u08 min, u08 max)
{
	if (HPVM::C[1] == 0 && HPVM::C[0] < 3)
	{
		u16 value = 0;
		for (u08 i = 0; i <= HPVM::C[0]; ++i)
		{
			value *= 10;
			value += HPVM::C[12 - i];
		}
		if (value >= min && value <= max)
		{
			out = u08(value);
			executeOperationAndWait(HPVM::OpROT);
			return true;
		}
	}
	return false;
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

		case FUNC_KEY:
			executeOperationAndWait(HPVM::OpSWAP);
			for (u08 i = 0; i < 15; ++i) hidden[i] = HPVM::Display[i];
			executeOperationAndWait(HPVM::OpSWAP);
			isFunc = true;
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
			if (extractNumber(RTC::Minutes, 0, 59) && extractNumber(RTC::Hours, 0, 23))
			{
				RTC::Seconds = 0;
				setupAndSwitchToRTCMode();
			}
			break;

		case PROG_DATE:
			RTC::ReadTimeDate();
			if (extractNumber(RTC::Month, 1, 12) && extractNumber(RTC::Date, 1, 31))
			{
				setupAndSwitchToRTCMode();
			}
			break;

		case PROG_YEAR:
			RTC::ReadTimeDate();
			if (extractNumber(RTC::Year, 0, 99))
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
		TXT::PrintChar(numStr[i], i * TXT::char_dx, y);
	}
}

void renderCalcMenu()
{
	TXT::SetFont(menu5x8);
	TXT::SetScale(SCALE_X1, SCALE_X2);
	TXT::SetInverse(true);
	for (u08 i = 0; i < MENU_OPS_PER_LINE; ++i)
	{
		TXT::PrintString(FPSTR(menu.string), select * MENU_OPS_PER_LINE + i, 46 * i, 2);
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
			TXT::PrintChar(INFO_FLAG_STORAGE, INFO_FLAG_POSITION, isMenu ? 0 : 2);
			break;
		}
	}

	if (isFunc)
	{
		renderCalcNumber(hidden, 0);
		renderCalcNumber(HPVM::Display, 2);
		TXT::PrintChar(INFO_FLAG_FUNCTION, INFO_FLAG_POSITION, 0);
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
		if (key != KBD::NONE)
		{
			switch(key)
			{
				default: isMenu = false; break;
				case KBD::ROTU: if (select > 0) select--; else select = menu.lastIdx; break;
				case KBD::ROTD: if (select < menu.lastIdx) select++; else select = 0; break;
				case KBD::MATH: enterMenu(MENU_MATH_OPS); break;
				case KBD::TRIG: enterMenu(MENU_TRIG_OPS); break;
				case KBD::PROG: enterMenu(MENU_PROG_OPS); break;
				case KBD::SEL1: case KBD::SEL2: case KBD::SEL3:
					u08 index = select * MENU_OPS_PER_LINE + (key - KBD::SEL1);
					u08 op = pgm_read_byte(menu.opsBase + index);
					executeOperation(op);
					break;
			}
			renderCalcMode();
		}
	}
	else
	{
		if (key != KBD::NONE)
		{
			u08 op = pgm_read_byte(mainOps + (isFunc ? 16 : 0) + key);
			executeOperation(op);
			renderCalcMode();
		}
		
		for (cycles += HP35_CYCLES_PER_FRAME; cycles > 0; --cycles)
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
	TXT::SetFont(menu5x8);
	TXT::SetScale(SCALE_X2, SCALE_X4);

	TXT::PrintChar(':', 20, 0);
	TXT::PrintChar(':', 47, 0);
	TXT::PrintTensOnes(RTC::Hours, 0, 0);
	TXT::PrintTensOnes(RTC::Minutes, 27, 0);
	TXT::PrintTensOnes(RTC::Seconds, 54, 0);

	TXT::SetScale(SCALE_X1, SCALE_X1);
	TXT::PrintString(FPSTR(strMonth), RTC::Month - 1, 79, 0);
	TXT::PrintTensOnes(RTC::Date, 99, 0);
	TXT::PrintTensOnes(RTC::Year, 117, 0);
	TXT::PrintChar('/', 111, 0);

	TXT::SetScale(SCALE_X2, SCALE_X1);
	TXT::PrintString(F("HP35"), 79, 2);
	u08 i = battery;
	while (i) TXT::PrintChar('-', 79 + (--i) * TXT::char_dx, 1);

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
	// init hardware and switch to rtc operation mode
	PCB::Init();
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
		key = KBD::Read();
		if (key != oldkey) oldkey = key; else key = KBD::NONE;
		if (key != KBD::NONE) switchToCalcMode();

		// update current operation mode and idle until next frame
		if (calcMode) updateCalcMode(); else updateRTCMode();
		FPS::SyncWait();
	}
	return 0;
}
