// -----------------------------------------------------------------------------
// DS3231M Real Time Clock on I2C Bus
// -----------------------------------------------------------------------------

#include "Extras/BuildTime.h"

// Time always stored in 24-hour format!
// Day of the week is not used!
// Century flag is not supported!
// Alarms are not supported!

namespace RTC
{
	const u08 I2C_ADDR = 0x68;

	enum
	{
		REG_SECONDS      = 0x00,
		REG_MINUTES      = 0x01,
		REG_HOURS        = 0x02,
		REG_DAY          = 0x03,
		REG_DATE         = 0x04,
		REG_MONTH        = 0x05,
		REG_YEAR         = 0x06,
		REG_A1_SECONDS   = 0x07,
		REG_A1_MINUTES   = 0x08,
		REG_A1_HOUR      = 0x09,
		REG_A1_DAY_DATE  = 0x0A,
		REG_A2_MINUTES   = 0x0B,
		REG_A2_HOUR      = 0x0C,
		REG_A2_DAY_DATE  = 0x0D,
		REG_CONTROL      = 0x0E,
		REG_STATUS       = 0x0F,
		REG_AGING_OFFSET = 0x10,
		REG_TEMP_MSB     = 0x11,
		REG_TEMP_LSB     = 0x12,
	};

	const u08 days_per_month[] PROGMEM =
	{
		31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
	};

	u08 Seconds = BCD_Encode(BUILD_SEC);   // 0 - 59
	u08 Minutes = BCD_Encode(BUILD_MIN);   // 0 - 59
	u08 Hours   = BCD_Encode(BUILD_HOUR);  // 0 - 23
	u08 Date    = BCD_Encode(BUILD_DAY);   // 1 - 31
	u08 Month   = BCD_Encode(BUILD_MONTH); // 1 - 12
	u08 Year    = BCD_Encode(BUILD_YEAR);  // 0 - 99

	void ReadTimeDate()
	{
		if (I2C::StartWrite(I2C_ADDR))
		{
			I2C::Write(REG_SECONDS);
			I2C::StartRead(I2C_ADDR);
			Seconds = I2C::ReadAck();
			Minutes = I2C::ReadAck();
			Hours   = I2C::ReadAck() & 0x3F;
			I2C::ReadAck();
			Date    = I2C::ReadAck();
			Month   = I2C::ReadAck() & 0x1F;
			Year    = I2C::ReadNack();
			I2C::Stop();
		}
	}

	void WriteTimeDate()
	{
		if (I2C::StartWrite(I2C_ADDR))
		{
			I2C::Write(REG_SECONDS);
			I2C::Write(Seconds);
			I2C::Write(Minutes);
			I2C::Write(Hours);
			I2C::Write(1);
			I2C::Write(Date);
			I2C::Write(Month);
			I2C::Write(Year);
			I2C::Stop();
		}
	}

	w16 ReadTemperature()
	{
		w16 temp;
		if (I2C::StartWrite(I2C_ADDR))
		{
			I2C::Write(REG_TEMP_MSB);
			I2C::StartRead(I2C_ADDR);
			temp.msb = I2C::ReadAck();  // degrees
			temp.lsb = I2C::ReadNack(); // fractional
			I2C::Stop();
		}
		return temp;
	}

	b08 IsLeapYear(u16 year)
	{
		return (!(year % 4) && ((year % 100) || !(year % 400)));
	}

	b08 IsLeapYear()
	{
		return IsLeapYear(2000 + BCD::Decode(Year));
	}

	u32 GetTimestamp(s08 GMTTimeZone)
	{
		u08 y = BCD::Decode(Year) + 2000 - 1970;
		u08 m = BCD::Decode(Month);

		// count days before given year
		u32 t = y * 365;
		for (u08 i = 0; i < y; ++i)
		{
			if (IsLeapYear(1970 + i)) t++;
		}

		// count days before given month
		for (u08 i = 1; i < m; ++i)
		{
			t += pgm_read_byte(&days_per_month[i - 1]);
			if (i == 2 && IsLeapYear(1970 + y)) t++;
		}

		// compute from days to seconds
		t = 24 * (t + BCD::Decode(Date) - 1);
		t = 60 * (t + BCD::Decode(Hours)   );
		t = 60 * (t + BCD::Decode(Minutes) );
		t += BCD::Decode(Seconds);
		t -= 3600 * GMTTimeZone;
		return t;
	}
}