// -----------------------------------------------------------------------------
// One Pin Analog 16-Key Keyboard
// -----------------------------------------------------------------------------

#if PCB_REV == 10
	#define KBD_PIN PB3
	#define KBD_ADC ADC::A3_PB3
#else
	#define KBD_PIN PB4
	#define KBD_ADC ADC::A2_PB4
#endif

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

	const u16 adc_lut[] PROGMEM =
	{
		147, 182, 221, 269, // A0 B0 C0 D0
		324, 383, 442, 505, // A1 B1 C1 D1
		573, 635, 692, 762, // A2 B2 C2 D2
		827, 863, 893, 913  // A3 B3 C3 D3
	};

	const u08 code_lut[] PROGMEM = 
	{
		FUNC, NUM7, NUM8, NUM9, // A0 B0 C0 D0
		EEX,  NUM4, NUM5, NUM6, // A1 B1 C1 D1
		CHS,  NUM1, NUM2, NUM3, // A2 B2 C2 D2
		CLR,  NUM0, DOT,  DUP   // A3 B3 C3 D3
	};

	u08 key;

	NOINLINE
	u08 read_raw_key()
	{
		w16 adc = ADC::Read(KBD_ADC);
		if (adc.val > 110)
		{
			for (u08 i = 0; i < 16; ++i)
			{
				u16 adcMax = pgm_read_word(&adc_lut[i]);
				if (adc.val < adcMax) return pgm_read_byte(&code_lut[i]);
			}
		}
		return NONE;
	}

	void Init()
	{
		clr_bit(DDRB,  KBD_PIN); // chose pin as input
		clr_bit(PORTB, KBD_PIN); // disable pull-up resistor
		set_bit(PCMSK, KBD_PIN); // chose pin as interrupt source
		set_bit(GIMSK, PCIE);    // enable pin change interruptions
		set_bit(GIFR,  PCIF);    // clear the interruption flag
		key = NONE;
	}

	u08 Read()
	{
		u08 raw = read_raw_key();
		if (raw == NONE && key != NONE) key = NONE;
		if (raw != NONE && key == NONE) key = raw;
		return key;
	}

	// just interrupt sleeping
	EMPTY_INTERRUPT(PCINT0_vect);
}