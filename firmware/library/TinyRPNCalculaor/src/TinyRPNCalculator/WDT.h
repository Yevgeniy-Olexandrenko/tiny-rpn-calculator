// -----------------------------------------------------------------------------
// Watch Dog Timer
// -----------------------------------------------------------------------------

namespace WDT
{
	enum
	{
		MODE_DISABLED = 0x00, // disabled
		MODE_RES      = 0x08, // to reset the CPU if there is a timeout
		MODE_INT      = 0x40, // timeout will cause an interrupt
		MODE_INT_RES  = 0x48, // first time-out interrupt, the second time out - reset
	};

	enum
	{
		TIMEOUT_16MS  = 0x00, // (16 ± 1.6) ms
		TIMEOUT_32MS  = 0x01, // (32 ± 3.2) ms
		TIMEOUT_64MS  = 0x02, // (64 ± 6.4) ms
		TIMEOUT_125MS = 0x03, // (128 ± 12.8) ms
		TIMEOUT_250MS = 0x04, // (256 ± 25.6) ms
		TIMEOUT_500MS = 0x05, // (512 ± 51.2) ms
		TIMEOUT_1S    = 0x06, // (1024 ± 102.4) ms
		TIMEOUT_2S    = 0x07, // (2048 ± 204.8) ms
		TIMEOUT_4S    = 0x08, // (4096 ± 409.6) ms
		TIMEOUT_8S    = 0x09, // (8192 ± 819.2) ms
	};

	void Init(u08 mode, u08 prescaler)
	{
		// does not change global interrupts enable flag
		u08 wdtr = mode | ((prescaler > 7) ? 0x20 | (prescaler - 8) : prescaler);
		u08 sreg = SREG;
		cli();
		WDTCR = _BV(WDCE) | _BV(WDE);
		WDTCR = wdtr;
		SREG  = sreg;
	}
}