// -----------------------------------------------------------------------------
// Frames per Second Synchronization
// -----------------------------------------------------------------------------

namespace FPS
{
	enum
	{
		TIMEOUT_15_FPS = WDT::TIMEOUT_64MS,
		TIMEOUT_30_FPS = WDT::TIMEOUT_32MS,
		TIMEOUT_60_FPS = WDT::TIMEOUT_16MS,
	};

	u08 timeout;
	volatile b08 waiting;
	volatile u16 counter;

	NOINLINE void SyncStart(u08 t)
	{
		WDT::Init(WDT::MODE_INT, t);
		timeout = (16 << t);
		counter = 0;
	}

	void SyncStop()
	{
		WDT::Init(WDT::MODE_DISABLED, 0);
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