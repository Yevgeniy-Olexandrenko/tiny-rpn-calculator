// -----------------------------------------------------------------------------
// Power Management
// -----------------------------------------------------------------------------

namespace PWR
{
	const u16 BAT_FULL  = 4100; // mV
	const u16 BAT_EMPTY = 3500; // mV

	void saving(u08 mode)
	{
		// Block interrupts while preparing the MCU for sleep,
		// keeping any wake-up event pending until SLEEP is executed.
		cli();

		// Stop clocks of unused peripherals and select sleep mode.
		power_all_disable();
		set_sleep_mode(mode);
		sleep_enable();

		// AVR executes the instruction immediately following SEI
		// before servicing any pending interrupt, so SLEEP cannot
		// miss an interrupt that occurred during sleep preparation.
		sei();
		sleep_cpu();

		// Avoid accidental sleeping on any later SLEEP instruction.
		sleep_disable();
	}

	NOINLINE
	u16 Voltage()
	{
		// 20 mV resolution avoids expensive 32-bit division.
		return (56265U / ADC::Read(ADC::VCC).val) * 20U;
	}

	u08 Level()
	{
		u16 voltage = Voltage();
		if (voltage <= BAT_EMPTY) return 0;
		if (voltage >= BAT_FULL ) return 100;

		// Scale in 20 mV steps to keep the calculation 16-bit.
		return ((voltage - BAT_EMPTY) * 5U) / ((BAT_FULL - BAT_EMPTY) / 20U);
	}

	void Idle()
	{
		saving(SLEEP_MODE_IDLE);
	}

	void Down()
	{
		do saving(SLEEP_MODE_PWR_DOWN);
		while (Voltage() <= BAT_EMPTY);
	}
}