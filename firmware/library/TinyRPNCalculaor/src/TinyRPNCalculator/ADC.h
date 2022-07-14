// -----------------------------------------------------------------------------
// Analog to Digital Converter (10 bit)
// -----------------------------------------------------------------------------

namespace ADC
{
	enum
	{
		A0_PB5 = 0b0000,
		A1_PB2 = 0b0001,
		A2_PB4 = 0b0010,
		A3_PB3 = 0b0011,
		VCC    = 0b1100,
		TEMP   = 0b1111 | _BV(REFS1),
	};

	void Init()
	{
		ADCSRA = _BV(ADEN)   // enable adc
		       | _BV(ADIE)   // enable interrupt
		       | _BV(ADIF)   // clear interrupt flag
		       | _BV(ADPS2)  // prescaler is 32
		       | _BV(ADPS0); //
	}

	NOINLINE
	w16 Read(u08 channel)
	{
		ADMUX = channel;
		set_sleep_mode(SLEEP_MODE_ADC);
		do {
			sleep_enable();
			sleep_cpu();
			sleep_disable();
		} while (isb_set(ADCSRA, ADSC));

		w16 adc;
		adc.lsb = ADCL;
		adc.msb = ADCH;
		return adc;
	}

	EMPTY_INTERRUPT(ADC_vect);
}