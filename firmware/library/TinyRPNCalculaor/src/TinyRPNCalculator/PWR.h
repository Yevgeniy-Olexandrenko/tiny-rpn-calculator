// -----------------------------------------------------------------------------
// Power Management
// -----------------------------------------------------------------------------

namespace PWR
{
	const u16 BAT_FULL  = 4100; // mV
	const u16 BAT_EMPTY = 3500; // mV

	void saving(u08 mode)
	{
		clr_bit(ADCSRA, ADEN);
		power_all_disable();
		set_sleep_mode(mode);
		sleep_enable();
		sleep_cpu();
		sleep_disable();
		power_adc_enable();
		set_bit(ADCSRA, ADEN);
	}

	NOINLINE u16 Voltage()
	{
		return (1125300L / ADC::Read(ADC::VCC).val);
	}

	u08 Level()
	{
		u16 voltage = Voltage();
		if (voltage <= BAT_EMPTY) return 0;
		if (voltage >= BAT_FULL ) return 100;
		return ((voltage - BAT_EMPTY) / ((BAT_FULL - BAT_EMPTY) / 100));
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