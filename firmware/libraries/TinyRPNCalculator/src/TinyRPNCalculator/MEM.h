// -----------------------------------------------------------------------------
// Static Data Memory
// -----------------------------------------------------------------------------

#if ENABLE_OPT_DATAMEM
	#define DATAMEM EEMEM
#else
	#define DATAMEM PROGMEM
#endif

namespace MEM
{
	u08  ProgRead(const u08* src) { return pgm_read_byte(src); }
	void ProgCopy(void* dst, const void* src, size_t size) { memcpy_P(dst, src, size); }

#if ENABLE_OPT_DATAMEM
	u08  DataRead(const u08* src) { return eeprom_read_byte(src); }
	void DataCopy(void* dst, const void* src, size_t size) { eeprom_read_block(dst, src, size); }
#else
	u08  DataRead(const u08* src) { return ProgRead(src); }
	void DataCopy(void* dst, const void* src, size_t size) { ProgCopy(dst, src, size); }
#endif
}