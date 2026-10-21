// -----------------------------------------------------------------------------
// Power Management
// -----------------------------------------------------------------------------

#if PCB_REVISION >= 15
#define PWR_CHRG PB3
#endif

namespace PWR
{
	const u16 BAT_MAX      = 4200; // mV
	const u16 BAT_FULL     = 4100; // mV
	const u16 BAT_EMPTY    = 3500; // mV
	const u08 UNIT_VOLTAGE = 20;   // mV

	static_assert(BAT_MAX   % UNIT_VOLTAGE == 0, "BAT_MAX   must match voltage resolution");
	static_assert(BAT_FULL  % UNIT_VOLTAGE == 0, "BAT_FULL  must match voltage resolution");
	static_assert(BAT_EMPTY % UNIT_VOLTAGE == 0, "BAT_EMPTY must match voltage resolution");
	static_assert(BAT_MAX  >= BAT_FULL,          "Invalid battery voltage range");
	static_assert(BAT_FULL  > BAT_EMPTY,         "Invalid battery voltage range");

	const u16 BAT_MAX_UNITS   = BAT_MAX   / UNIT_VOLTAGE;
	const u16 BAT_FULL_UNITS  = BAT_FULL  / UNIT_VOLTAGE;
	const u16 BAT_EMPTY_UNITS = BAT_EMPTY / UNIT_VOLTAGE;

	void saving(u08 mode)
	{
		// Block interrupts while preparing the MCU for sleep,
		// keeping any wake-up event pending until SLEEP is executed.
		cli();

		// Select sleep mode, clocks of unused peripherals are already
		// stopped by SYS::Init(), so a timer of the sketch keeps running.
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

#if PCB_REVISION >= 15
	void Init()
	{
		// Charger status output is open-drain: pulled low while charging,
		// released when charge is complete or there is no USB power.
		clr_bit(DDRB,  PWR_CHRG); // select pin as input
		set_bit(PORTB, PWR_CHRG); // enable pull-up resistor
		set_bit(PCMSK, PWR_CHRG); // select pin as wake-up source
	}

	b08 Charging()
	{
		return isb_clr(PINB, PWR_CHRG);
	}
#else
	b08 Charging()
	{
		// No charger status on these PCBs. VCC above the maximal battery
		// voltage means external power (e.g. ISP programmer), so treat it
		// as charging for debugging.
		return (voltageUnits() > BAT_MAX_UNITS);
	}
#endif

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