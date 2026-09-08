// -----------------------------------------------------------------------------
// Watch-Dog based Software Timer
// -----------------------------------------------------------------------------

namespace TMR
{
	enum
	{
		WDT_MODE_DISABLED = 0x00, // disabled
		WDT_MODE_RES      = 0x08, // to reset the CPU if there is a timeout
		WDT_MODE_INT      = 0x40, // timeout will cause an interrupt
		WDT_MODE_INT_RES  = 0x48, // first time-out interrupt, the second time out - reset
	};

	enum
	{
		TIMEOUT_15MS  = WDTO_15MS,
		TIMEOUT_30MS  = WDTO_30MS,
		TIMEOUT_60MS  = WDTO_60MS,
		TIMEOUT_120MS = WDTO_120MS,
		TIMEOUT_250MS = WDTO_250MS
	};

	volatile u08 Timeout;
	volatile u16 Millis;

	void wdt_setup(u08 mode, u08 prescaler)
	{
		// does not change global interrupts enable flag
		u08 wdtr = mode | ((prescaler > 7) ? 0x20 | (prescaler - 8) : prescaler);
		u08 sreg = SREG;
		cli();
		WDTCR = _BV(WDCE) | _BV(WDE);
		WDTCR = wdtr;
		SREG  = sreg;
	}

	NOINLINE
	void Start(u08 t)
	{
		wdt_setup(WDT_MODE_INT, t);
		Timeout = (15U << t);
		Millis = 0;
	}

	void Stop()
	{
		wdt_setup(WDT_MODE_DISABLED, 0);
	}

	void Sync()
	{
		const u16 ms = Millis;
		while (Millis == ms) PWR::Idle();
	}

	ISR(WDT_vect)
	{
		Millis += Timeout;
	}
}