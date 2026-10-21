// -----------------------------------------------------------------------------
// System Initialization and Power Down
// -----------------------------------------------------------------------------

namespace SYS
{
	void Init()
	{
		// after a watchdog reset WDRF forces WDE on, so the watchdog would
		// reset the mcu again: the flag is cleared, TMR::Start() then sets
		// the interrupt mode only
		MCUSR = 0;

		ADC::Init();
		I2C::Init();
		LCD::Init();
		KBD::Init();
	#if PCB_REVISION >= 15
		PWR::Init();
	#endif
		// stop clocks of the peripherals after their setup, ADC::Read()
		// turns the ADC clock on for a conversion, the timers and USI are
		// not used by the library: a sketch can turn them on after this
		power_all_disable();

		GIFR = _BV(PCIF);     // clear pending pin-change interrupt
		set_bit(GIMSK, PCIE); // enable pin-change interrupt
		sei();
	}

	void Wake(u08 t)
	{
		LCD::TurnOn();
		TMR::Start(t);
	}

	void Down()
	{
		TMR::Stop();
		LCD::TurnOff();
		PWR::Down();
	}

	// pin-change interrupt from the 
	// selected pins just wakes up the mcu
	EMPTY_INTERRUPT(PCINT0_vect);
}
