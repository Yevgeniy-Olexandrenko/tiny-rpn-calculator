// -----------------------------------------------------------------------------
// DS3231M Real Time Clock on I2C Bus
// -----------------------------------------------------------------------------

// figure out build date and time (Example __DATE__ : "Jul 27 2012" and __TIME__ : "21:06:19")
#define COMPUTE_BUILD_YEAR ((__DATE__[9] - '0') *   10 + (__DATE__[10] - '0'))
#define COMPUTE_BUILD_DAY  (((__DATE__[4] >= '0') ? (__DATE__[4] - '0') * 10 : 0) + (__DATE__[5] - '0'))
#define BUILD_MONTH_IS_JAN (__DATE__[0] == 'J' && __DATE__[1] == 'a' && __DATE__[2] == 'n')
#define BUILD_MONTH_IS_FEB (__DATE__[0] == 'F')
#define BUILD_MONTH_IS_MAR (__DATE__[0] == 'M' && __DATE__[1] == 'a' && __DATE__[2] == 'r')
#define BUILD_MONTH_IS_APR (__DATE__[0] == 'A' && __DATE__[1] == 'p')
#define BUILD_MONTH_IS_MAY (__DATE__[0] == 'M' && __DATE__[1] == 'a' && __DATE__[2] == 'y')
#define BUILD_MONTH_IS_JUN (__DATE__[0] == 'J' && __DATE__[1] == 'u' && __DATE__[2] == 'n')
#define BUILD_MONTH_IS_JUL (__DATE__[0] == 'J' && __DATE__[1] == 'u' && __DATE__[2] == 'l')
#define BUILD_MONTH_IS_AUG (__DATE__[0] == 'A' && __DATE__[1] == 'u')
#define BUILD_MONTH_IS_SEP (__DATE__[0] == 'S')
#define BUILD_MONTH_IS_OCT (__DATE__[0] == 'O')
#define BUILD_MONTH_IS_NOV (__DATE__[0] == 'N')
#define BUILD_MONTH_IS_DEC (__DATE__[0] == 'D')
#define COMPUTE_BUILD_MONTH \
		( \
			(BUILD_MONTH_IS_JAN) ?  1 : \
			(BUILD_MONTH_IS_FEB) ?  2 : \
			(BUILD_MONTH_IS_MAR) ?  3 : \
			(BUILD_MONTH_IS_APR) ?  4 : \
			(BUILD_MONTH_IS_MAY) ?  5 : \
			(BUILD_MONTH_IS_JUN) ?  6 : \
			(BUILD_MONTH_IS_JUL) ?  7 : \
			(BUILD_MONTH_IS_AUG) ?  8 : \
			(BUILD_MONTH_IS_SEP) ?  9 : \
			(BUILD_MONTH_IS_OCT) ? 10 : \
			(BUILD_MONTH_IS_NOV) ? 11 : \
			(BUILD_MONTH_IS_DEC) ? 12 : \
			/* error default */  99 \
		)
#define COMPUTE_BUILD_HOUR ((__TIME__[0] - '0') * 10 + __TIME__[1] - '0')
#define COMPUTE_BUILD_MIN  ((__TIME__[3] - '0') * 10 + __TIME__[4] - '0')
#define COMPUTE_BUILD_SEC  ((__TIME__[6] - '0') * 10 + __TIME__[7] - '0')

#define BUILD_DATE_IS_BAD  (__DATE__[0] == '?')
#define BUILD_YEAR         ((BUILD_DATE_IS_BAD) ? 0 : COMPUTE_BUILD_YEAR )
#define BUILD_MONTH        ((BUILD_DATE_IS_BAD) ? 1 : COMPUTE_BUILD_MONTH)
#define BUILD_DAY          ((BUILD_DATE_IS_BAD) ? 1 : COMPUTE_BUILD_DAY  )

#define BUILD_TIME_IS_BAD  (__TIME__[0] == '?')
#define BUILD_HOUR         ((BUILD_TIME_IS_BAD) ? 99 :  COMPUTE_BUILD_HOUR)
#define BUILD_MIN          ((BUILD_TIME_IS_BAD) ? 99 :  COMPUTE_BUILD_MIN )
#define BUILD_SEC          ((BUILD_TIME_IS_BAD) ? 99 :  COMPUTE_BUILD_SEC )

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

	// MSB degrees, LSB fractional
	w16 ReadTemperature()
	{
		w16 temp;
		if (I2C::StartWrite(I2C_ADDR))
		{
			I2C::Write(REG_TEMP_MSB);
			I2C::StartRead(I2C_ADDR);
			temp.msb = I2C::ReadAck();
			temp.lsb = I2C::ReadNack();
			I2C::Stop();
		}
		return temp;
	}

	b08 IsLeapYear()
	{
		u16 year = 2000 + BCD::Decode(Year);
		return ((year % 4 == 0) && (year % 100 != 0) || (year % 400 == 0));
	}

	u08 GetDaysInMonth()
	{
		u08 i = BCD::Decode(Month) - 1;
		return pgm_read_byte(days_per_month + i);
	}

	u32 GetTimestamp()
	{
		// One revolution of the Earth is not 365 days but accurately
		// 365.2422 days. It is leap year that adjusts this decimal
		// fraction.
		u32 ts = (2000 + BCD::Decode(Year) - 1970) * 3652422 / 10000;

		// Compute from days to seconds.
		u08 month = BCD::Decode(Month) - 1;
		for (u08 i = 0; i < month; ++i) 
			pgm_read_byte(days_per_month + i);
		ts = 24 * (ts + BCD::Decode(Date));
		ts = 60 * (ts + BCD::Decode(Hours));
		ts = 60 * (ts + BCD::Decode(Minutes));
		ts += BCD::Decode(Seconds);

		// Year 2000 is a special leap year, so 1 day must
		// be added if date is greater than 29/02/2000.
		if (ts > 951847199) ts += 86400;

		// Checks if, in case of a leap year, the date is before or past
		// the 29th of februray. If no, the leap day hasn't been yet
		// reached so we have to subtract a day.
		if (IsLeapYear() && month < 2) ts -= 86400;

		// Because years start at day 0.0, not day 1.
		return (ts - 86400);
	}
}