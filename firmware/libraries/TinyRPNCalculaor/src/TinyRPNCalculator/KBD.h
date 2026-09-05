// -----------------------------------------------------------------------------
// One Pin Analog 16-Key Keyboard
// -----------------------------------------------------------------------------

#define KBD_PIN PB4
#define KBD_ADC (ADC::A2_PB4 | _BV(ADLAR))

// GND group
#define KBD_R5    680U
#define KBD_R6   1200U
#define KBD_R7   3300U
#define KBD_R8  12000U

// VCC group
#define KBD_R9   1000U
#define KBD_R11   820U
#define KBD_R12   560U
#define KBD_R13  2200U

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

	constexpr u16 KBD_RG0 = KBD_R5;
	constexpr u16 KBD_RG1 = KBD_R5 + KBD_R6;
	constexpr u16 KBD_RG2 = KBD_R5 + KBD_R6 + KBD_R7;
	constexpr u16 KBD_RG3 = KBD_R5 + KBD_R6 + KBD_R7 + KBD_R8;

	constexpr u16 KBD_RVA = KBD_R13 + KBD_R12 + KBD_R11 + KBD_R9;
	constexpr u16 KBD_RVB = KBD_R13 + KBD_R12 + KBD_R11;
	constexpr u16 KBD_RVC = KBD_R13 + KBD_R12;
	constexpr u16 KBD_RVD = KBD_R13;

	constexpr u08 adcMax(u16 g1, u16 v1, u16 g2, u16 v2)
	{
		return (u08)(
			(
				128ULL * // 256 ADC codes / 2 for midpoint
				(
					(unsigned long long)g1 * (g2 + v2) +
					(unsigned long long)g2 * (g1 + v1)
				) +
				(unsigned long long)(g1 + v1) * (g2 + v2) / 2
			) /
			(
				(unsigned long long)(g1 + v1) * (g2 + v2)
			)
		) - 1;
	}

	const u08 ADC_NONE_MAX = adcMax(0, 1, KBD_RG0, KBD_RVA);
	const u08 adc_max_lut[] PROGMEM =
	{
		adcMax(KBD_RG0, KBD_RVA, KBD_RG0, KBD_RVB), // A0
		adcMax(KBD_RG0, KBD_RVB, KBD_RG0, KBD_RVC), // B0
		adcMax(KBD_RG0, KBD_RVC, KBD_RG0, KBD_RVD), // C0
		adcMax(KBD_RG0, KBD_RVD, KBD_RG1, KBD_RVA), // D0

		adcMax(KBD_RG1, KBD_RVA, KBD_RG1, KBD_RVB), // A1
		adcMax(KBD_RG1, KBD_RVB, KBD_RG1, KBD_RVC), // B1
		adcMax(KBD_RG1, KBD_RVC, KBD_RG1, KBD_RVD), // C1
		adcMax(KBD_RG1, KBD_RVD, KBD_RG2, KBD_RVA), // D1

		adcMax(KBD_RG2, KBD_RVA, KBD_RG2, KBD_RVB), // A2
		adcMax(KBD_RG2, KBD_RVB, KBD_RG2, KBD_RVC), // B2
		adcMax(KBD_RG2, KBD_RVC, KBD_RG2, KBD_RVD), // C2
		adcMax(KBD_RG2, KBD_RVD, KBD_RG3, KBD_RVA), // D2

		adcMax(KBD_RG3, KBD_RVA, KBD_RG3, KBD_RVB), // A3
		adcMax(KBD_RG3, KBD_RVB, KBD_RG3, KBD_RVC), // B3
		adcMax(KBD_RG3, KBD_RVC, KBD_RG3, KBD_RVD), // C3
		255                                         // D3
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