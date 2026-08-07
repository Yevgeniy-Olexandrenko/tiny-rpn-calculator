// -----------------------------------------------------------------------------
// Packed Binary-Coded Decimals
// -----------------------------------------------------------------------------

// compile time implementations
#define BCD_Decode(data) ((data / 16 * 10) + (data % 16))
#define BCD_Encode(data) ((data / 10 * 16) + (data % 10))

// runtime time implementations
namespace BCD
{
	u08 Decode(u08 data)
	{
		return BCD_Decode(data);
	}

	u08 Encode(u08 data)
	{
		return BCD_Encode(data);
	}
}