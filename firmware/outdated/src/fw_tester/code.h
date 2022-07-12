#pragma once

#define DIMOUT_MILLIS   10000 // Time before display dim out
#define POWEROFF_MILLIS 20000 // Time before power off


u16 adcVal;
u16 adcMin;
u16 adcMax;

u16 adcValCap;
u16 adcMinCap;
u16 adcMaxCap;

u16 adcBuf[5];
u16 adcPtr;

u08 keyVal;
u08 keyOld;
u08 keyBuf[19];
u08 keyPtr;


const char strKey[] PROGMEM = "\01" "0123456789DPCSEF";

void initKeyboardTest()
{
	for (u08 i = 0; i < 19; ++i)
	{
		keyBuf[i] = KBD::NONE;
	}
	keyPtr = 0;
}

u08 readKeyboardKey()
{
	adcMin = 110;
	adcVal = ADC::Read(KBD_ADC);

	for (u08 i = 0; i < 16; ++i)
	{
		adcMax = pgm_read_word(&KBD::adc[i]);
		if (adcVal > adcMin && adcVal < adcMax) 
		{
			return pgm_read_byte(&KBD::code[i]);
		}
		adcMin = adcMax;
	}
	return KBD::NONE;
}

void printAdc(u16 adc, u08 pos, u08 y)
{
	PrintCharAt('0' + (adc / 100), pos * dx, y);
	PrintTensOnesAt(adc % 100, (pos + 1) * dx, y);
}

void updateKeyboardTest()
{
	keyVal = readKeyboardKey();

	if (adcMin == adcMax)
	{
		adcMin = adcMax = 0;
	}

	if (keyVal != keyOld)
	{
		keyOld = keyVal;
		keyBuf[keyPtr++] = keyVal;
		keyPtr %= 19;

		if (keyVal != KBD::NONE)
		{
			adcBuf[adcPtr++] = adcVal;
			adcPtr %= 5;
		}

		adcMinCap = adcMin;
		adcMaxCap = adcMax;
		adcValCap = adcVal;
	}

	LCD::Clear();
	PrintCharSize(CHAR_SIZE_S, CHAR_SIZE_S);
	PrintStringAt(FPSTR("@-KEYBOARD-@"), 0, 0);

	for (u08 i = 0; i < 19; ++i)
	{
		u08 key = keyBuf[(keyPtr + i) % 19];
		if (key == KBD::NONE)
			PrintCharAt('-', i * dx, 1);
		else
			PrintStringAt(FPSTR(strKey), key, i * dx, 1);
	}

	u16 adcAvgCap = (adcMinCap + adcMaxCap) / 2;
	s16 adcDelCap = (adcValCap - adcAvgCap);

	printAdc(adcMinCap, 0, 2);
	PrintCharAt('-', 3 * dx, 2);
	printAdc(adcMaxCap, 4, 2);
	printAdc(adcAvgCap, 8, 2);
	if (adcDelCap < 0)
	{
		PrintCharAt('-', 11 * dx, 2);
		printAdc(-adcDelCap, 12, 2);
	}
	else
	{
		PrintCharAt('+', 11 * dx, 2);
		printAdc(adcDelCap, 12, 2);
	}
	PrintCharAt('>', 15 * dx, 2);
	printAdc(adcValCap, 16, 2);

	for (u08 i = 0; i < 5; ++i)
	{
		u16 adc = adcBuf[(adcPtr + i) % 5];
		printAdc(adc, i * 4, 3);
	}

	LCD::Flip();
}

void powerDown()
{
	FPS::SyncStop();
	LCD::TurnOff();
	PWR::Down();
	LCD::TurnOn();
 	FPS::SyncStart();
}

void keepAlive()
{
	FPS::SyncWait();
	u16 timePassedMs = FPS::SyncMillis();
	
	LCD::Brightness(timePassedMs < DIMOUT_MILLIS ? 0xFF : 0x00);
	if (timePassedMs >= POWEROFF_MILLIS) powerDown();
		
	if (KBD::Read() != KBD::NONE) FPS::SyncStart();
}

void init()
{
	PCB::Init();
	initKeyboardTest();
}

int main() 
{
	init();
	powerDown();
	while (true)
	{
		keepAlive();
		updateKeyboardTest();
	}
	return 0;
}
