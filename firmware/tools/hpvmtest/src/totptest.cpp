// -----------------------------------------------------------------------------
// Desktop test of SHA1.h + TOTP.h on the reference vectors of
// FIPS 180-2 (SHA-1), RFC 2202 (HMAC-SHA1), RFC 4226 (HOTP), RFC 6238 (TOTP).
// The library headers are compiled for the PC with small stubs for AVR.
//
// totptest  run all checks, exit code = number of failures
// -----------------------------------------------------------------------------

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <string>

using b08 = bool;
using u08 = uint8_t; using u16 = uint16_t; using u32 = uint32_t;
using s08 = int8_t;  using s16 = int16_t;  using s32 = int32_t;
union w16 { u16 val; struct { u08 lsb, msb; }; };
#define PROGMEM
#define DATAMEM
#define NOINLINE

namespace MEM
{
	void ProgCopy(void* d, const void* s, size_t n) { memcpy(d, s, n); }
	u08  DataRead(const u08* p) { return *p; }
	void DataCopy(void* d, const void* s, size_t n) { memcpy(d, s, n); }
}
namespace I2C
{
	void StartWrite(u08) {}
	void StartRead(u08) {}
	void Write(u08) {}
	void ReadAck(u08&) {}
	void ReadNack(u08&) {}
	void Stop() {}
}
#include "TinyRPNCalculator/BCD.h"
#include "TinyRPNCalculator/RTC.h"
#include "Extras/TOTP.h"

// test keys area of the hp35 sketch
#include "../../../sketches/hp35/totp.h"

int fails = 0;

void expect(const char* name, const std::string& got, const std::string& want)
{
	bool ok = (got == want);
	printf("%s %-24s %s", ok ? "ok  " : "FAIL", name, got.c_str());
	if (!ok) printf(" (want %s)", want.c_str());
	printf("\n");
	if (!ok) ++fails;
}

std::string hex(const u08* data, int size)
{
	std::string s;
	for (int i = 0; i < size; ++i)
	{
		s += "0123456789abcdef"[data[i] >> 4];
		s += "0123456789abcdef"[data[i] & 15];
	}
	return s;
}

std::string sha1(const std::string& msg, int repeat = 1)
{
	SHA1::Init();
	while (repeat--)
	{
		for (char c : msg) SHA1::Write(u08(c));
	}
	return hex(SHA1::Result(), SHA1::HASH_SIZE);
}

std::string hmac(const std::string& key, const std::string& msg)
{
	SHA1::InitHmac((const u08*)key.data(), u08(key.size()));
	SHA1::Write((const u08*)msg.data(), u08(msg.size()));
	return hex(SHA1::ResultHmac(), SHA1::HASH_SIZE);
}

std::string code(const TOTP::Key& key, u32 time)
{
	char s[8];
	u32 c = TOTP::GetCode(key, time / TOTP::PERIOD);
	snprintf(s, sizeof(s), "%06u", c);
	return s;
}

std::string code(const std::string& data, u32 time)
{
	TOTP::Key key{ u08(data.size()) };
	memcpy(key.data, data.data(), data.size());
	return code(key, time);
}

// reads the record from the area, returns "label:hex key" or "none"
std::string record(const u08* area, u08 index)
{
	TOTP::Key key;
	if (!TOTP::ReadKey(area, index, key)) return "none";
	return std::string(key.label, TOTP::LABEL_SIZE) + ":" + hex(key.data, key.length);
}

int main()
{
	// SHA-1 (FIPS 180-2 examples)
	expect("sha1 empty", sha1(""), "da39a3ee5e6b4b0d3255bfef95601890afd80709");
	expect("sha1 abc", sha1("abc"), "a9993e364706816aba3e25717850c26c9cd0d89d");
	expect("sha1 448 bits", sha1("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"),
		"84983e441c3bd26ebaae4aa1f95129e5e54670f1");
	expect("sha1 896 bits", sha1("abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmn"
		"hijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu"),
		"a49b2446a02c645bf419f995b67091253a04a259");

	// SHA-1 edge cases (reference values from .NET SHA1):
	// padding goes to an extra block, maximum supported length
	expect("sha1 63 bytes", sha1(std::string(63, 'b')),
		"956b36ade6e3e4b7818ced2d611fb95e8abc2bd0");
	expect("sha1 65535 bytes", sha1(std::string(65535, 'a')),
		"e1d0a18d43d3a689af088e156bd0434aa0c831fc");

	// every padding case: hash of hex hashes of 0..128 bytes of 'c'
	std::string all;
	for (int n = 0; n <= 128; ++n) all += sha1(std::string(n, 'c'));
	expect("sha1 lengths 0..128", sha1(all), "06a2fba226bd4800b1fd8ab2a9304d07e08d92be");

	// HMAC-SHA1 (RFC 2202 test cases 1..4, the others use keys over 64 bytes)
	std::string key4;
	for (char c = 1; c <= 25; ++c) key4 += c;
	expect("hmac case 1", hmac(std::string(20, '\x0b'), "Hi There"),
		"b617318655057264e28bc0b6fb378c8ef146be00");
	expect("hmac case 2", hmac("Jefe", "what do ya want for nothing?"),
		"effcdf6ae5eb2fa2d27416d5f184df9c259a7c79");
	expect("hmac case 3", hmac(std::string(20, '\xaa'), std::string(50, '\xdd')),
		"125d7342b9ac11cd91a39af48aa17b4f63f175d3");
	expect("hmac case 4", hmac(key4, std::string(50, '\xcd')),
		"4c9007f4026250c6bc8414f9bf50c86c2d7235da");

	// key of exactly one block length (reference value from .NET HMACSHA1)
	expect("hmac 64-byte key", hmac(std::string(64, '\x0c'), "Test"),
		"0fdd984bc30c14a02f7a792cc2cefe3a31c5f79d");

	// HOTP (RFC 4226 appendix D), counter 0..9
	const std::string key = "12345678901234567890";
	const char* hotp[] =
	{
		"755224", "287082", "359152", "969429", "338314",
		"254676", "287922", "162583", "399871", "520489"
	};
	for (u32 i = 0; i < 10; ++i)
	{
		char name[16];
		snprintf(name, sizeof(name), "hotp %u", i);
		expect(name, code(key, i * TOTP::PERIOD), hotp[i]);
	}

	// TOTP (RFC 6238 appendix B, SHA1, last 6 of 8 digits)
	expect("totp 59",         code(key, 59),         "287082");
	expect("totp 1111111109", code(key, 1111111109), "081804");
	expect("totp 1111111111", code(key, 1111111111), "050471");
	expect("totp 1234567890", code(key, 1234567890), "005924");
	expect("totp 2000000000", code(key, 2000000000), "279037");

	// keys area: end marker, record over the area end and too long key stop reading
	const u08 area[] =
	{
		'T', 'O', 'T', 'P', 22,
		1, 'A', ' ', ' ', ' ', 0xAA,
		2, 'B', 'C', '2', ' ', 0xBB, 0xCC,
		0, 'X', 'X', 'X', 'X',
		1, 'Y', 'Y', 'Y', 'Y', 0x11, // after the area end
	};
	expect("area record 0", record(area, 0), "A   :aa");
	expect("area record 1", record(area, 1), "BC2 :bbcc");
	expect("area end marker", record(area, 2), "none");
	u08 full[sizeof(area)];
	memcpy(full, area, sizeof(area));
	full[18] = 3;
	expect("area record over end", record(full, 2), "none");
	full[4] = sizeof(full);
	expect("area record 2", record(full, 2), "XXXX:015959");
	full[18] = TOTP::KEY_MAX + 1;
	expect("area key too long", record(full, 2), "none");

	// hp35 test keys
	for (u08 i = 0; i < 4; ++i)
	{
		TOTP::Key key;
		char name[16];
		snprintf(name, sizeof(name), "hp35 key %u", i);
		bool ok = TOTP::ReadKey(totp_keys, i, key);
		expect(name, ok ? std::string(key.label, 4) + " " + code(key, 59) : "none",
			std::string("TST") + char('1' + i) + " 287082");
	}
	expect("hp35 key 4", record(totp_keys, 4), "none");

	// RTC::GetTimestamp for every day of 2000..2099 in several time zones,
	// reference is the C runtime's _mkgmtime
	int ts_fails = 0;
	const s08 zones[] = { -12, -5, 0, 3, 14 };
	for (int y = 0; y <= 99; ++y)
	{
		for (int m = 1; m <= 12; ++m)
		{
			RTC::Year = BCD::Encode(y);
			RTC::Month = BCD::Encode(m);
			RTC::Date = BCD::Encode(1);
			for (int d = 1; d <= RTC::GetDaysInMonth(); ++d)
			{
				int hh = (y + d) % 24, mm = (m * 7 + d) % 60, ss = (y * 3 + d) % 60;
				RTC::Date = BCD::Encode(d);
				RTC::Hours = BCD::Encode(hh);
				RTC::Minutes = BCD::Encode(mm);
				RTC::Seconds = BCD::Encode(ss);
				std::tm tm = {};
				tm.tm_year = 100 + y; tm.tm_mon = m - 1; tm.tm_mday = d;
				tm.tm_hour = hh; tm.tm_min = mm; tm.tm_sec = ss;
				long long utc = _mkgmtime(&tm);
				for (s08 tz : zones)
				{
					u32 got = RTC::GetTimestamp(tz);
					if (got != u32(utc - tz * 3600LL) && ts_fails++ < 5)
					{
						printf("FAIL timestamp 20%02d-%02d-%02d %02d:%02d:%02d tz %+d: %u (want %lld)\n",
							y, m, d, hh, mm, ss, tz, got, utc - tz * 3600LL);
					}
				}
			}
		}
	}
	expect("timestamp 2000..2099", ts_fails ? "mismatch" : "match", "match");
	fails += ts_fails ? ts_fails - 1 : 0;

	// invalid dates give zero
	auto ts = [](u08 y, u08 m, u08 d)
	{
		RTC::Year = BCD::Encode(y); RTC::Month = BCD::Encode(m); RTC::Date = BCD::Encode(d);
		return std::to_string(RTC::GetTimestamp(0));
	};
	expect("timestamp 2001-02-29", ts(1, 2, 29), "0");
	expect("timestamp 2024-13-01", ts(24, 13, 1), "0");
	expect("timestamp 2024-00-01", ts(24, 0, 1), "0");
	expect("timestamp 2024-04-00", ts(24, 4, 0), "0");
	expect("timestamp 2024-04-31", ts(24, 4, 31), "0");

	printf("%d failures\n", fails);
	return fails;
}
