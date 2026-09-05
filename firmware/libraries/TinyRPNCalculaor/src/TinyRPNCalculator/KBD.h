// -----------------------------------------------------------------------------
// One Pin Analog 16-Key Keyboard
// -----------------------------------------------------------------------------

#define KBD_PIN PB4
#define KBD_ADC (ADC::A2_PB4 | _BV(ADLAR))

namespace KBD
{
	enum
	{
		NONE = 0xFF,

		FUNC = 0x0F, NUM7 = 0x07, NUM8 = 0x08, NUM9 = 0x09,
		EEX  = 0x0E, NUM4 = 0x04, NUM5 = 0x05, NUM6 = 0x06,
		CHS  = 0x0D, NUM1 = 0x01, NUM2 = 0x02, NUM3 = 0x03,
		CLR  = 0x0C, NUM0 = 0x00, DOT  = 0x0A, DUP  = 0x0B,

		ROTU = EEX,  ROTD = CHS,
		SEL1 = NUM1, SEL2 = NUM2, SEL3 = NUM3,
		MATH = FUNC, TRIG = NUM7, PROG = NUM8,
		CONS = NUM4, SETS = DOT
	};

	const u08 ADC_NONE_MAX = 16;
	const u08 adc_max_lut[] PROGMEM =
	{
		36,  45,  55,  66,  // A0 B0 C0 D0
		80,  95,  110, 126, // A1 B1 C1 D1
		143, 158, 172, 190, // A2 B2 C2 D2
		206, 215, 223, 255  // A3 B3 C3 D3
	};

	const u08 code_lut[] PROGMEM = 
	{
		FUNC, NUM7, NUM8, NUM9, // A0 B0 C0 D0
		EEX,  NUM4, NUM5, NUM6, // A1 B1 C1 D1
		CHS,  NUM1, NUM2, NUM3, // A2 B2 C2 D2
		CLR,  NUM0, DOT,  DUP   // A3 B3 C3 D3
	};

	u08 key;

	void Init()
	{
		clr_bit(DDRB,  KBD_PIN); // select pin as input
		clr_bit(PORTB, KBD_PIN); // disable pull-up resistor
		set_bit(PCMSK, KBD_PIN); // select pin as interrupt source
		GIFR = _BV(PCIF);        // clear pending pin-change interrupt
		set_bit(GIMSK, PCIE);    // enable pin-change interrupt
		key = NONE;
	}

	NOINLINE
	u08 Read()
	{
		u08 adc = ADC::Read(KBD_ADC).msb;
		if (adc <= ADC_NONE_MAX)
		{
			key = NONE;
			return key;
		}
		if (key != NONE) return key;

		u08 i = 0;
		while (adc > pgm_read_byte(adc_max_lut + i)) ++i;
		key = pgm_read_byte(code_lut + i);
		return key;
	}

	// just interrupt sleeping
	EMPTY_INTERRUPT(PCINT0_vect);
}