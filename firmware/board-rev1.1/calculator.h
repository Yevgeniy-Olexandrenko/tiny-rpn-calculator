
uint16_t keyVoltage;
uint8_t  keyIndex;

int main() 
{
	ADCInit();
	I2CBusInit();
	DisplayInit();
	KeyboardInit();
    sei();

	DisplayTurnOn();
	FrameSyncEnable();
	
	for (;;)
	{
        FrameSyncWait();

        keyVoltage = ADCRead(KEYBOARD_ADC, 1);
        keyIndex = KeyboardRead();

        DisplayClear();
        PrintCharSize(CHAR_SIZE_M, CHAR_SIZE_M);

        //
        PrintCharAt('0' + _huns(keyVoltage), (FONT_WIDTH * cw + 1) * 0, ch * 0);
        PrintCharAt('0' + _tens(keyVoltage), (FONT_WIDTH * cw + 1) * 1, ch * 0);
        PrintCharAt('0' + _ones(keyVoltage), (FONT_WIDTH * cw + 1) * 2, ch * 0);
        //
        PrintCharAt('0' + _huns(keyIndex), (FONT_WIDTH * cw + 1) * 0, ch * 1);
        PrintCharAt('0' + _tens(keyIndex), (FONT_WIDTH * cw + 1) * 1, ch * 1);
        PrintCharAt('0' + _ones(keyIndex), (FONT_WIDTH * cw + 1) * 2, ch * 1);
        //

        DisplayRefresh();
    }
}