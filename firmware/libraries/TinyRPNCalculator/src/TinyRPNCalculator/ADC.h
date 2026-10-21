// -----------------------------------------------------------------------------
// Analog to Digital Converter (8/10 bit)
// -----------------------------------------------------------------------------

namespace ADC
{
	enum
	{
		A0_PB5 = 0x00,
		A1_PB2 = 0x01,
		A2_PB4 = 0x02,
		A3_PB3 = 0x03,
		A0_PB5_8BIT = A0_PB5 | _BV(ADLAR),
		A1_PB2_8BIT = A1_PB2 | _BV(ADLAR),
		A2_PB4_8BIT = A2_PB4 | _BV(ADLAR),
		A3_PB3_8BIT = A3_PB3 | _BV(ADLAR),
		VCC  = 0x0C,
		TEMP = 0x0F | _BV(REFS1),
	};

	void Init()
	{
		ADCSRA = _BV(ADIE)   // enable interrupt
		       | _BV(ADIF)   // clear interrupt flag
		       | _BV(ADPS2)  // prescaler 128 gives
		       | _BV(ADPS1)  // 125 kHz ADC clock
		       | _BV(ADPS0);
		power_adc_disable();
	}

	NOINLINE
	w16 Read(u08 channel)
	{
		// Restore the ADC peripheral clock before accessing the ADC hardware.
		power_adc_enable();

		// Select the input channel and voltage reference before enabling the ADC.
		ADMUX = channel;

		// Enable the ADC. Since it was previously disabled, the next conversion
		// will use the extended first-conversion startup sequence.
		set_bit(ADCSRA, ADEN);

		// ADC Noise Reduction sleep starts the conversion automatically
		// when the CPU enters sleep, so ADSC must not be set manually here.
		set_sleep_mode(SLEEP_MODE_ADC);

		// Enable sleep immediately before executing the SLEEP instruction.
		sleep_enable();

		// Enter ADC Noise Reduction sleep. Normally the ADC interrupt wakes
		// the CPU when the conversion is complete.
		sleep_cpu();

		// Prevent any later accidental SLEEP instruction from putting
		// the CPU to sleep again.
		sleep_disable();

		// Another interrupt (for example PCINT or WDT) may wake the CPU
		// before the ADC conversion has finished. In that rare case,
		// wait for the current conversion to complete without starting
		// another one.
		while (isb_set(ADCSRA, ADSC));

		// ADCL must be read before ADCH to obtain a consistent 10-bit result.
		w16 adc;
		adc.lsb = ADCL;
		adc.msb = ADCH;

		// Disable the ADC analog circuitry when it is not being used.
		clr_bit(ADCSRA, ADEN);

		// Disable the ADC peripheral clock as well to minimize idle power.
		power_adc_disable();
		return adc;
	}

	EMPTY_INTERRUPT(ADC_vect);
}