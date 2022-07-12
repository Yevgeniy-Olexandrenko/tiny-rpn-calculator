// -----------------------------------------------------------------------------
// Packed Binary-Coded Decimals
// -----------------------------------------------------------------------------

// compile time implementations
#define BCD_DECODE(data) ((data / 16 * 10) + (data % 16))
#define BCD_ENCODE(data) ((data / 10 * 16) + (data % 10))

// execute time implementations
#if ENABLE_OPT_ASSEMBLER && defined(BCD_ASM_IMPL)
	extern "C" u08 BCD_Decode(u08 data);
	extern "C" u08 BCD_Encode(u08 data);
#else
	u08 BCD_Decode(u08 data)
	{
		return BCD_DECODE(data);
	}

	u08 BCD_Encode(u08 data)
	{
		return BCD_ENCODE(data);
	}
#endif