#pragma once

// -----------------------------------------------------------------------------
// Woodstock Virtual Machine. Include model data before this header.
// -----------------------------------------------------------------------------

namespace HPVM
{
	// clock parameters
	constexpr u08 CLOCK_RATE_KHZ      = 185;
	constexpr u08 BIT_TIME_WIDTH_US   = 1000 / CLOCK_RATE_KHZ;
	constexpr u08 BITS_PER_CYCLE      = 14 * 4;
	constexpr u16 CYCLE_TIME_WIDTH_US = BITS_PER_CYCLE * BIT_TIME_WIDTH_US;
	constexpr u16 CYCLES_PER_SEC      = 1000000 / CYCLE_TIME_WIDTH_US;

	// public interface
	void Operation(u08 op);
	void OpAndWait(u08 op);
	bool Idling(); // true if idling
	bool Cycle();  // true if display updated

	// TODO

}
