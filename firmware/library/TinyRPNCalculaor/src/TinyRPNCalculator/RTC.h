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

	b08 IsLeapYear()
	{
		u16 y = 2000 + BCD::Decode(Year);
		return ((y % 4 == 0) && (y % 100 != 0) || (y % 400 == 0));
	}

	u08 GetDaysInMonth()
	{
		u08 m = BCD::Decode(Month) - 1;
		return pgm_read_byte(&days_per_month[m]);
	}

	#define SECS_PER_MIN  (60UL)
	#define SECS_PER_HOUR (3600UL)
	#define SECS_PER_DAY  (SECS_PER_HOUR * 24UL)
	#define LEAP_YEAR(Y)  ( ((1970+(Y))>0) && !((1970+(Y))%4) && ( ((1970+(Y))%100) || !((1970+(Y))%400) ) )

	u32 GetTimestamp()
	{
		// // One revolution of the Earth is not 365 days but accurately
		// // 365.2422 days. It is leap year that adjusts this decimal
		// // fraction.
		// u32 t = (2000 + BCD::Decode(Year) - 1970) * 3652422 / 10000;

		// // Compute from days to seconds.
		// u08 m = BCD::Decode(Month) - 1;
		// for (u08 i = 0; i < m; pgm_read_byte(&days_per_month[i++]));
		// t = 24 * (t + BCD::Decode(Date));
		// t = 60 * (t + BCD::Decode(Hours));
		// t = 60 * (t + BCD::Decode(Minutes));
		// t += BCD::Decode(Seconds);

		// // Year 2000 is a special leap year, so 1 day must
		// // be added if date is greater than 29/02/2000.
		// if (t > 951847199) t += 86400;

		// // Checks if, in case of a leap year, the date is before or
		// // past the 29th of februray. If no, the leap day hasn't been
		// // yet reached so we have to subtract a day.
		// if (IsLeapYear() && m < 2) t -= 86400;

		// // Because years start at day 0, not day 1.
		// return (t - 86400);

		int i;
		uint32_t seconds;
		u16 year = ((2000 + BCD::Decode(Year)) - 1970);
		u08 month = BCD::Decode(Month);

		// seconds from 1970 till 1 jan 00:00:00 of the given year
		seconds= year*(SECS_PER_DAY * 365);
		for (i = 0; i < year; i++) 
		{
			if (LEAP_YEAR(i)) 
			{
				seconds += SECS_PER_DAY;   // add extra days for leap years
			}
		}

		// add days for this year, months start from 1
		for (i = 1; i < month; i++) 
		{
			if ( (i == 2) && LEAP_YEAR(year)) 
			{
				seconds += SECS_PER_DAY * 29;
			}
			else
			{
				seconds += SECS_PER_DAY * pgm_read_byte(&days_per_month[i-1]);  //monthDay array starts from 0
			}
		}
		seconds+= (BCD::Decode(Date)-1) * SECS_PER_DAY;
		seconds+= BCD::Decode(Hours) * SECS_PER_HOUR;
		seconds+= BCD::Decode(Minutes) * SECS_PER_MIN;
		seconds+= BCD::Decode(Seconds);
		return seconds; 
	}
}