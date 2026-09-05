// -----------------------------------------------------------------------------
// Power Management
// -----------------------------------------------------------------------------

namespace PWR
{
	const u16 BAT_FULL     = 4100; // mV
	const u16 BAT_EMPTY    = 3500; // mV
	const u08 UNIT_VOLTAGE = 20;   // mV

	static_assert(BAT_FULL  % UNIT_VOLTAGE == 0, "BAT_FULL  must match voltage resolution");
	static_assert(BAT_EMPTY % UNIT_VOLTAGE == 0, "BAT_EMPTY must match voltage resolution");
	static_assert(BAT_FULL  > BAT_EMPTY,         "Invalid battery voltage range");

	const u16 BAT_FULL_UNITS  = BAT_FULL  / UNIT_VOLTAGE;
	const u16 BAT_EMPTY_UNITS = BAT_EMPTY / UNIT_VOLTAGE;

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
	u16 voltageUnits()
	{
		// Measure VCC directly in UNIT_VOLTAGE steps,
		// avoiding expensive 32-bit runtime division.
		return ((1125300UL / UNIT_VOLTAGE) / ADC::Read(ADC::VCC).val);
	}

	u16 Voltage()
	{
		return (voltageUnits() * UNIT_VOLTAGE);
	}

	u08 Level()
	{
		u16 voltage = voltageUnits();
		if (voltage <= BAT_EMPTY_UNITS) return 0;
		if (voltage >= BAT_FULL_UNITS ) return 100;
		return ((voltage - BAT_EMPTY_UNITS) * 100U / (BAT_FULL_UNITS - BAT_EMPTY_UNITS));
	}

	void Idle()
	{
		saving(SLEEP_MODE_IDLE);
	}

	void Down()
	{
		do saving(SLEEP_MODE_PWR_DOWN);
		while (voltageUnits() <= BAT_EMPTY_UNITS);
	}
}