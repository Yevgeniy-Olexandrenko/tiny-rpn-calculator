// -----------------------------------------------------------------------------
// Packed Binary-Coded Decimals
// -----------------------------------------------------------------------------

namespace BCD
{
	constexpr u08 Decode(u08 data)
	{
		return data - 6 * (data >> 4);
	}

	constexpr u08 Encode(u08 data)
	{
		return data + 6 * (data / 10);
	}
}