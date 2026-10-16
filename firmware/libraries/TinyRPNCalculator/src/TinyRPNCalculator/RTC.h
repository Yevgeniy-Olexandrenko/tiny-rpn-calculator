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
	constexpr u08 I2C_ADDR = 0x68;

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

	const u08 days_per_month[] DATAMEM =
	{
		31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
	};

	u08 Seconds = BCD::Encode(BUILD_SEC);   // 0 - 59
	u08 Minutes = BCD::Encode(BUILD_MIN);   // 0 - 59
	u08 Hours   = BCD::Encode(BUILD_HOUR);  // 0 - 23
	u08 Date    = BCD::Encode(BUILD_DAY);   // 1 - 31
	u08 Month   = BCD::Encode(BUILD_MONTH); // 1 - 12
	u08 Year    = BCD::Encode(BUILD_YEAR);  // 0 - 99

	// supported years are 2000-2099,
	// so every fourth year is leap
	u08 is_leap_year(u08 year)
	{
		return !(year & 3);
	}

	NOINLINE
	u08 get_days_in_month(u08 month, u08 leap)
	{
		u08 i = month - 1;
		if (i >= sizeof(days_per_month)) return 0;
		return (i == 1 && leap ? 29 : MEM::DataRead(days_per_month + i));
	}

	// one step of time units conversion, shared to save flash
	NOINLINE
	u32 mul_add_bcd(u32 t, u08 k, u08 bcd)
	{
		return t * k + BCD::Decode(bcd);
	}

	void ReadTimeDate()
	{
		I2C::StartWrite(I2C_ADDR);
		I2C::Write(REG_SECONDS);
		I2C::StartRead(I2C_ADDR);
		I2C::ReadAck(Seconds);
		I2C::ReadAck(Minutes);
		I2C::ReadAck(Hours);
		I2C::ReadAck(Date); // skip day of the week
		I2C::ReadAck(Date); // read day of the month
		I2C::ReadAck(Month);
		I2C::ReadNack(Year);
		I2C::Stop();
		Hours &= 0x3F;
		Month &= 0x1F;
	}

	void WriteTimeDate()
	{
		I2C::StartWrite(I2C_ADDR);
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

	w16 ReadTemperature()
	{
		w16 temp;
		I2C::StartWrite(I2C_ADDR);
		I2C::Write(REG_TEMP_MSB);
		I2C::StartRead(I2C_ADDR);
		I2C::ReadAck (temp.msb); // degrees
		I2C::ReadNack(temp.lsb); // fractional
		I2C::Stop();
		return temp;
	}

	u08 IsLeapYear()
	{
		return is_leap_year(BCD::Decode(Year));
	}

	u08 GetDaysInMonth()
	{
		return get_days_in_month(BCD::Decode(Month), IsLeapYear());
	}

	u32 GetTimestamp(s08 GMTTimeZone)
	{
		u08 y = BCD::Decode(Year);
		u08 m = BCD::Decode(Month);
		u08 d = BCD::Decode(Date);

		// helper returns zero for an invalid month
		u08 leap = is_leap_year(y);
		if (!d || d > get_days_in_month(m, leap)) return 0;

		// days since 1970 fit 16 bits: 10957 days before 2000,
		// then whole years and their leap days, and given month
		u16 days = 10957 + y * 365U + (y + 3) / 4 + d - 1;
		for (u08 i = 1; i < m; ++i)
			days += get_days_in_month(i, leap);

		// compute from days to seconds, time zone shifts hours
		u32 t = mul_add_bcd(days, 24, Hours) - GMTTimeZone;
		t = mul_add_bcd(t, 60, Minutes);
		return mul_add_bcd(t, 60, Seconds);
	}
}
