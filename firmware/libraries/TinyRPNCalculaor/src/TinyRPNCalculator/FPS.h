// -----------------------------------------------------------------------------
// Frames per Second Synchronization
// -----------------------------------------------------------------------------

namespace FPS
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
		TIMEOUT_15_FPS = WDTO_60MS,
		TIMEOUT_30_FPS = WDTO_30MS,
		TIMEOUT_60_FPS = WDTO_15MS,
	};

	u08 timeout;
	volatile b08 waiting;
	volatile u16 counter;

	void wdt_init(u08 mode, u08 prescaler)
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
	void SyncStart(u08 t)
	{
		wdt_init(WDT_MODE_INT, t);
		timeout = (16 << t);
		counter = 0;
	}

	void SyncStop()
	{
		wdt_init(WDT_MODE_DISABLED, 0);
	}

	void SyncWait()
	{
		waiting = true;
		while (waiting) PWR::Idle();
	}

	u16 SyncMillis()
	{
		return (counter * timeout);
	}

	ISR(WDT_vect)
	{
		waiting = false;
		counter++;
	}
}