// -----------------------------------------------------------------------------
// Watch-Dog based Software Timer
// -----------------------------------------------------------------------------

namespace TMR
{
	enum
	{
		TIMEOUT_16MS  = WDTO_15MS,
		TIMEOUT_32MS  = WDTO_30MS,
		TIMEOUT_64MS  = WDTO_60MS,
		TIMEOUT_125MS = WDTO_120MS
	};

	volatile u08 Timeout;
	volatile u16 Millis;
	volatile b08 wait;

	NOINLINE
	void wdt_setup(u08 wdtr)
	{
		const u08 sreg = SREG;
		cli();
		WDTCR = _BV(WDCE) | _BV(WDE);
		WDTCR = wdtr;
		SREG = sreg;
	}

	NOINLINE
	void Start(u08 t)
	{
		Millis = 0;
		Timeout = (16U << t);
		wdt_setup(_BV(WDIE) | t);
	}

	void Stop()
	{
		wdt_setup(0);
	}

	void Sync()
	{
		wait = true;
		while (wait) PWR::Idle();
	}

	ISR(WDT_vect)
	{
		Millis += Timeout;
		wait = false;
	}
}