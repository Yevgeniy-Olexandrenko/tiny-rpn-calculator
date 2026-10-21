// -----------------------------------------------------------------------------
// System Initialization and Power Down
// -----------------------------------------------------------------------------

namespace SYS
{
	void Init()
	{
		ADC::Init();
		I2C::Init();
		LCD::Init();
		KBD::Init();
	#if PCB_REVISION >= 15
		PWR::Init();
	#endif
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
