// -----------------------------------------------------------------------------
// Desktop test of HPVMWoodstock.h + HP21Model.h on the original HP-21 ROM.
// The library headers are compiled for the PC with small stubs for AVR.
//
// hp21test                      run all checks, exit code = number of failures
// hp21test "<keys>" ["<keys>"]  trace execution of the first key sequence,
//                               after the optional second one is typed
// Keys are separated by spaces, see the ops table below, e.g. "2 ENTER 3 +".
// -----------------------------------------------------------------------------

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <sstream>
#include <map>

using b08 = bool;
using u08 = uint8_t; using u16 = uint16_t; using u32 = uint32_t;
using s08 = int8_t;  using s16 = int16_t;  using s32 = int32_t;
union w16 { u16 val; struct { u08 lsb, msb; }; };
#define PROGMEM
#define DATAMEM
#define NOINLINE
static inline u08 __builtin_avr_swap(u08 x) { return u08((x << 4) | (x >> 4)); }
class __FlashStringHelper; class __DataStringHelper;

namespace MEM
{
	u08  ProgRead(const u08* p) { return *p; }
	void ProgCopy(void* d, const void* s, size_t n) { memcpy(d, s, n); }
	u08  DataRead(const u08* p) { return *p; }
	void DataCopy(void* d, const void* s, size_t n) { memcpy(d, s, n); }
}
namespace LCD
{
	void BeginWrite(u08, u08) {}
	void Write(u08, u08) {}
	void EndWrite() {}
	void EndWrite(u08, u08) {}
}
#include "TinyRPNCalculator/TXT.h"
#include "Extras/HP21Model.h"
#include "Extras/HPVMWoodstock.h"

u16 x11_word(int addr);

static long total_cycles = 0;

std::string seg2str(const u08* segs, int n)
{
	std::string s;
	for (int i = 0; i < n; ++i)
	{
		u08 g = segs[i] & 0x7F;
		char c = '?';
		switch (g)
		{
			case TXT::SEG_0: c = '0'; break; case TXT::SEG_1: c = '1'; break;
			case TXT::SEG_2: c = '2'; break; case TXT::SEG_3: c = '3'; break;
			case TXT::SEG_4: c = '4'; break; case TXT::SEG_5: c = '5'; break;
			case TXT::SEG_6: c = '6'; break; case TXT::SEG_7: c = '7'; break;
			case TXT::SEG_8: c = '8'; break; case TXT::SEG_9: c = '9'; break;
			case TXT::SEG_r: c = 'r'; break; case TXT::SEG_H: c = 'H'; break;
			case TXT::SEG_o: c = 'o'; break; case TXT::SEG_P: c = 'P'; break;
			case TXT::SEG_E: c = 'E'; break; case TXT::SEG_SPACE: c = ' '; break;
			case TXT::SEG_DASH: c = '-'; break;
		}
		s += c;
		if (segs[i] & TXT::SEG_DOT) s += '.';
	}
	return s;
}

std::string reg2str(const HPVM::reg r)
{
	std::string s;
	for (int i = 13; i >= 0; --i) s += "0123456789ABCDEF"[r[i] & 15];
	return s;
}

bool run_to_idle(long limit = 200000)
{
	long n = 0;
	do { HPVM::Cycle(); ++n; } while (!HPVM::Idling() && n < limit);
	total_cycles += n;
	return n < limit;
}

std::map<std::string, u08> ops =
{
	{"1/x", HPVM::OpINV_}, {"SIN", HPVM::OpSIN_}, {"COS", HPVM::OpCOS_}, {"TAN", HPVM::OpTAN_}, {"f", HPVM::OpSHFT},
	{"x<>y", HPVM::OpSWAP}, {"RDN", HPVM::OpROT_}, {"e^x", HPVM::OpEXP_}, {"STO", HPVM::OpSTO_}, {"RCL", HPVM::OpRCL_},
	{"ENTER", HPVM::OpPUSH}, {"CHS", HPVM::OpCHS_}, {"EEX", HPVM::OpEEX_}, {"CLX", HPVM::OpCLX_},
	{"-", HPVM::OpSUB_}, {"7", HPVM::OpNUM7}, {"8", HPVM::OpNUM8}, {"9", HPVM::OpNUM9},
	{"+", HPVM::OpADD_}, {"4", HPVM::OpNUM4}, {"5", HPVM::OpNUM5}, {"6", HPVM::OpNUM6},
	{"*", HPVM::OpMUL_}, {"1", HPVM::OpNUM1}, {"2", HPVM::OpNUM2}, {"3", HPVM::OpNUM3},
	{"/", HPVM::OpDIV_}, {"0", HPVM::OpNUM0}, {".", HPVM::OpDOT_}, {"DSP", HPVM::OpDSP_},
	// shifted operations, sent as one operation with FlSHFT
	{"y^x", HPVM::OpPOW_}, {"ASIN", HPVM::OpASIN}, {"ACOS", HPVM::OpACOS}, {"ATAN", HPVM::OpATAN},
	{">R", HPVM::Op2REC}, {">P", HPVM::Op2POL}, {"LN", HPVM::OpLN__}, {"LOG", HPVM::OpLOG_}, {"10^x", HPVM::Op10X_},
	{"SQRT", HPVM::OpSQRT}, {"PI", HPVM::OpPI__}, {"CLR", HPVM::OpCLR_},
	{"M-", HPVM::OpMSUB}, {"M+", HPVM::OpMADD}, {"M*", HPVM::OpMMUL}, {"M/", HPVM::OpMDIV},
};

// keys: whitespace separated; a multi-digit token is typed digit by digit
std::string keys(const char* seq, bool verbose = false)
{
	std::istringstream in(seq);
	std::string t;
	while (in >> t)
	{
		auto it = ops.find(t);
		if (it == ops.end())
		{
			bool digits = true;
			for (char c : t) if (!((c >= '0' && c <= '9') || c == '.')) digits = false;
			if (digits && t.size() > 1)
			{
				for (char c : t)
				{
					std::string d(1, c);
					HPVM::OpAndWait(ops[d]);
				}
				continue;
			}
			printf("unknown key '%s'\n", t.c_str());
			continue;
		}
		HPVM::Operation(it->second);
		if (!run_to_idle()) printf("  !!! no idle after '%s' (pc=%04X)\n", t.c_str(), HPVM::pc);
		if (verbose)
			printf("  %-6s [%s]  C=%s\n", t.c_str(), seg2str(HPVM::Display, sizeof(HPVM::Display)).c_str(), reg2str(HPVM::C).c_str());
	}
	return seg2str(HPVM::Display, sizeof(HPVM::Display));
}

int fails = 0;
void check(const char* name, const char* seq, const char* expect)
{
	std::string got = keys(seq);
	// trim trailing spaces for comparison
	std::string g = got; while (!g.empty() && g.back() == ' ') g.pop_back();
	std::string e = expect;
	bool ok = (g == e);
	if (!ok) ++fails;
	printf("%s %-22s %-34s -> [%s]%s%s\n", ok ? "ok  " : "FAIL", name, seq, got.c_str(),
		ok ? "" : "  expected ", ok ? "" : expect);
}

void expect(const char* name, const std::string& got, const char* expected)
{
	std::string g = got; while (!g.empty() && g.back() == ' ') g.pop_back();
	bool ok = (g == expected);
	if (!ok) ++fails;
	printf("%s %-22s [%s]%s%s\n", ok ? "ok  " : "FAIL", name, got.c_str(),
		ok ? "" : "  expected ", ok ? "" : expected);
}

std::string sbits()
{
	std::string s;
	for (int i = 0; i < 16; ++i) if (HPVM::s[i]) { s += std::to_string(i); s += ','; }
	return s;
}

// one line per cycle: address and word (octal, '*' - then-goto address word),
// then the state after the cycle
void trace(const char* seq)
{
	std::istringstream in(seq);
	std::string t;
	while (in >> t)
	{
		printf("=== key %s\n", t.c_str());
		HPVM::Operation(ops[t]);
		long n = 0;
		do
		{
			u16 pc = HPVM::pc;
			u08 tl = (HPVM::rom_l[pc >> 2] >> ((pc & 3) * 2)) & 3;
			u16 w = u16(HPVM::rom_h[pc]) << 2 | tl;
			bool tg = HPVM::then_goto;
			HPVM::Cycle(); ++n;
			printf("%04o %04o%s p=%2u cy=%u s=%-14s A=%s B=%s C=%s\n", pc, w, tg ? "*" : " ", HPVM::p, HPVM::carry,
				sbits().c_str(), reg2str(HPVM::A).c_str(), reg2str(HPVM::B).c_str(), reg2str(HPVM::C).c_str());
		} while (!HPVM::Idling() && n < 3000);
	}
}

int main(int argc, char** argv)
{
	if (argc > 1)
	{
		run_to_idle();
		if (argc > 2) keys(argv[2], true);
		trace(argv[1]);
		return 0;
	}

	// ROM cross check against the x11-calc image, see hp21rom_x11calc.cpp
	int diffs = 0;
	for (int a = 0; a < 1024; ++a)
	{
		u08 t = MEM::ProgRead(HPVM::rom_l + (a >> 2));
		t = (t >> ((a & 3) * 2)) & 3;
		u16 w = u16(HPVM::rom_h[a]) << 2 | t;
		if (w != x11_word(a)) { if (diffs < 10) printf("ROM diff @%04o: %04o vs %04o\n", a, w, x11_word(a)); ++diffs; }
	}
	printf("ROM cross check: %d differences\n", diffs);
	if (diffs) ++fails;
	printf("CYCLES_PER_SEC = %u\n", HPVM::CYCLES_PER_SEC);

	// power on
	if (!run_to_idle()) printf("!!! no idle after power on\n");
	printf("power on: [%s] cycles=%ld f=%u\n", seg2str(HPVM::Display, sizeof(HPVM::Display)).c_str(), total_cycles, HPVM::f);
	std::string boot = seg2str(HPVM::Display, sizeof(HPVM::Display)); // power-on display, without the trailing blanks
	while (!boot.empty() && boot.back() == ' ') boot.pop_back();

	check("entry",       "1 2 3 . 4 5",                 " 123.45");
	check("enter add",   "CLR 2 ENTER 3 +",             " 5.00");
	check("sub",         "CLR 2 ENTER 3 -",             "-1.00");
	check("mul",         "CLR 6 ENTER 7 *",             " 42.00");
	check("div",         "CLR 1 ENTER 3 /",             " 0.33");
	check("chs",         "CLR 5 CHS",                   "-5.");
	check("eex",         "CLR 1 EEX 5",                 " 1.        05");
	check("1/x",         "CLR 4 1/x",                   " 0.25");
	check("sqrt",        "CLR 1 6 SQRT",                " 4.00");
	check("pi",          "CLR PI",                      " 3.14");
	check("y^x",         "CLR 2 ENTER 1 0 y^x",         " 1024.00");
	check("e^x",         "CLR 1 e^x",                   " 2.72");
	check("ln",          "CLR 1 0 LN",                  " 2.30");
	check("log",         "CLR 1 0 0 0 LOG",             " 3.00");
	check("10^x",        "CLR 3 10^x",                  " 1000.00");
	check("x<>y",        "CLR 1 ENTER 2 x<>y",          " 1.00");
	check("rdn",         "CLR 1 ENTER 2 ENTER 3 ENTER 4 RDN", " 3.00");
	check("sin deg",     "CLR 3 0 SIN",                 " 0.50");
	check("cos deg",     "CLR 6 0 COS",                 " 0.50");
	check("tan deg",     "CLR 4 5 TAN",                 " 1.00");
	check("asin deg",    "CLR . 5 ASIN",                " 30.00");
	check("acos deg",    "CLR . 5 ACOS",                " 60.00");
	check("atan deg",    "CLR 1 ATAN",                  " 45.00");
	check(">P",          "CLR 3 ENTER 4 >P",            " 5.00");
	check(">P angle",    "CLR 1 ENTER 1 >P x<>y",       " 45.00");
	check(">P y in Y",   "CLR 1 ENTER 0 >P x<>y",       " 90.00");
	check(">R",          "CLR 3 0 ENTER 2 >R",          " 1.73");
	check(">R y",        "CLR 3 0 ENTER 2 >R x<>y",     " 1.00");
	check("sto rcl",     "CLR 7 STO CLX RCL",           " 7.00");
	check("m+",          "CLR 5 STO 3 M+ RCL",          " 8.00");
	check("m-",          "CLR 5 STO 3 M- RCL",          " 2.00");
	check("m*",          "CLR 5 STO 3 M* RCL",          " 15.00");
	check("m/",          "CLR 6 STO 3 M/ RCL",          " 2.00");
	check("clr keeps M", "CLR 9 STO CLR RCL",           " 9.00");
	check("error",       "CLR 0 1/x",                   " Error");
	check("after error", "CLX 2",                       " 2.");
	check("dsp fix 4",   "CLR 2 DSP . 4",               " 2.0000");
	check("dsp sci 2",   "CLR 1 2 3 4 DSP 2",           " 1.23      03");
	check("dsp fix 2",   "DSP . 2",                     " 1234.00");

	// scripted sequences of the hp21 sketch
	keys("CLR 1 ENTER 2 ENTER 3 ENTER 4 RDN RDN RDN"); // KEY_ROTU
	expect("roll up X", seg2str(HPVM::Display, sizeof(HPVM::Display)), " 1.00");
	expect("roll up Y", reg2str(HPVM::Y), "04000000000000");
	expect("roll up T", reg2str(HPVM::T), "02000000000000");
	keys("CLR 1 ENTER 2 ENTER 3 ENTER 4 RDN"); // dropStackTop()
	HPVM::reg_move(HPVM::T, HPVM::Z, 0, 13, HPVM::COPY);
	expect("drop X", seg2str(HPVM::Display, sizeof(HPVM::Display)), " 3.00");
	expect("drop Z", reg2str(HPVM::Z), "01000000000000");
	expect("drop T", reg2str(HPVM::T), "01000000000000");
	u08 hidden[sizeof(HPVM::Display)];
	keys("CLR 1 2 ENTER 3 0"); // KEY_FUNC
	HPVM::DisplayY(hidden);
	expect("func hidden", seg2str(hidden, sizeof(hidden)), " 12.00");
	expect("func X", seg2str(HPVM::Display, sizeof(HPVM::Display)), " 30.00");
	check("func ends entry", "4 +", " 34.00");
	keys("CLR 3 ENTER"); // KEY_FUNC keeps the stack lift disabled
	HPVM::DisplayY(hidden);
	keys("PI");
	expect("func lift Y", reg2str(HPVM::Y), "03000000000000");
	expect("func lift Z", reg2str(HPVM::Z), "00000000000000");
	keys("CLR 5 STO 9 ENTER 1 2"); // invalid TIME or DATE values
	HPVM::Error();
	run_to_idle();
	expect("error shown", seg2str(HPVM::Display, sizeof(HPVM::Display)), " Error");
	expect("error clears Y", reg2str(HPVM::Y), "00000000000000");
	expect("error keeps M1", reg2str(HPVM::M1), "05000000000000");
	check("after error", "3 ENTER RCL", " 5.00");

	// DisplayY() and Error() while the ROM is still busy with a key
	keys("CLR 3 0");
	HPVM::Operation(HPVM::OpSIN_);
	for (int i = 0; i < 50; ++i) HPVM::Cycle();
	HPVM::Error();
	run_to_idle();
	expect("busy error shown", seg2str(HPVM::Display, sizeof(HPVM::Display)), " Error");
	keys("CLR 3");
	HPVM::Operation(HPVM::OpPUSH);
	for (int i = 0; i < 5; ++i) HPVM::Cycle();
	HPVM::DisplayY(hidden);
	keys("PI");
	expect("busy func lift Y", reg2str(HPVM::Y), "03000000000000");
	expect("busy func lift Z", reg2str(HPVM::Z), "00000000000000");

	// low battery: F1 high keeps S5 clear, ROM lights all dots
	HPVM::F1 = true; HPVM::s[HPVM::STATUS_BIT_F1] = 0;
	check("low battery", "CLR 1 2 ENTER", " 1.20.0. . . . .");
	HPVM::F1 = false;
	check("battery ok", "CLR 1 2 ENTER", " 12.00");

	HPVM::F2 = true;
	check("sin rad",     "CLR 3 0 SIN",                 "-0.99");
	check("pi cos rad",  "CLR PI COS",                  "-1.00");
	HPVM::F2 = false;
	check("sin deg again","CLR 3 0 SIN",                " 0.50");

	// latched key: operation requested while the ROM is busy
	keys("CLR 3 0");
	HPVM::Operation(HPVM::OpSIN_);
	for (int i = 0; i < 50; ++i) HPVM::Cycle();
	HPVM::Operation(HPVM::OpNUM2); // pressed while sin is being computed
	run_to_idle();
	expect("latched key X", seg2str(HPVM::Display, sizeof(HPVM::Display)), " 2.");
	expect("latched key Y", reg2str(HPVM::Y), "05000000000999");

	// two keys pressed while the ROM is busy, the second one is shifted
	keys("CLR 3 0");
	HPVM::Operation(HPVM::OpSIN_);
	for (int i = 0; i < 50; ++i) HPVM::Cycle();
	HPVM::Operation(HPVM::OpNUM2);
	HPVM::Operation(HPVM::OpSQRT);
	run_to_idle();
	expect("two keys X", seg2str(HPVM::Display, sizeof(HPVM::Display)), " 1.41");
	expect("two keys Y", reg2str(HPVM::Y), "05000000000999");

	// X register during digit entry (used by the sketch to read integers)
	keys("CLR 1 2");
	expect("entry 12 C", reg2str(HPVM::C), "01200000000001");
	keys("CLR 1 2 3 . 5");
	expect("entry 123.5 C", reg2str(HPVM::C), "01235000000002");

	// the sketch (renderCalcNumber() of hp21 code.h) finds the exponent by the
	// display only: position 9 blank or a dash, something in positions 10..11;
	// the ROM marks the exponent sign position by B bit 1 (positions 0..11 are
	// A/B digits 13..2, position 9 is B[4]); compare both after every key of
	// all display formats and numbers of different kinds, also during entry
	{
		const char* numbers[] =
		{
			"1", "1 2 3 4 5 6 7 8 9 0", "1 2 3 4 5 6 7 8 9 0 1 2", ". 0 0 0 1 2 3", "0", "CHS 5",
			"1 2 3 4 5 6 7 8 . 9 EEX 2 1", "1 EEX 9 9", "1 EEX 9 9 CHS", "1 . 5 CHS EEX 5 CHS",
			"EEX 3", "9 EEX 9 9 ENTER 1 0 *", "1 EEX 9 9 CHS ENTER 1 0 /", "0 1/x", "1 EEX 9 ENTER",
			"1 EEX 1 0 ENTER", "1 EEX 5 CHS ENTER", "2 SQRT", "PI CHS", "3 ENTER 1 2 /", "1 ENTER 7 /",
		};
		int n = 0, bad = 0, with_exp = 0;
		for (int sci = 0; sci < 2; ++sci)
		{
			for (int digits = 0; digits <= 9; ++digits)
			{
				for (const char* num : numbers)
				{
					std::string dsp = std::string("CLR DSP ") + (sci ? "" : ". ") + char('0' + digits);
					keys(dsp.c_str());
					std::istringstream in(std::string(num) + " ENTER");
					std::string t;
					while (in >> t)
					{
						keys(t.c_str());
						const u08* d = HPVM::Display;
						bool digits_10_11 = (d[10] | d[11]) != 0;
						bool sketch = (d[9] == TXT::SEG_SPACE || d[9] == TXT::SEG_DASH) && digits_10_11;
						bool rom = (HPVM::B[4] & 2) && digits_10_11;
						++n; with_exp += rom;
						if (sketch != rom)
						{
							if (++bad <= 5) printf("  %s, %s, after '%s': [%s] sketch %d, ROM %d\n", dsp.c_str(), num,
								t.c_str(), seg2str(d, sizeof(HPVM::Display)).c_str(), sketch, rom);
						}
					}
				}
			}
		}
		char got[64]; sprintf(got, "%d of %d displays differ", bad, n);
		char exp[64]; sprintf(exp, "0 of %d displays differ", n);
		printf("     %d displays with an exponent, %d without\n", with_exp, n - with_exp);
		expect("exponent rule", got, exp);
	}

	// Constant(): a number in the register layout enters X as a number key
	// would: no stack lift after ENTER, a lift after a number
	{
		static const u08 c[7] PROGMEM = { 0x02, 0x99, 0x79, 0x24, 0x58, 0x00, 0x08 }; // 2.99792458 08
		keys("CLR DSP . 2 3 ENTER"); HPVM::Constant(c);
		expect("constant after ENTER", seg2str(HPVM::Display, sizeof(HPVM::Display)), " 299792458.0");
		expect("constant: Y = 3", reg2str(HPVM::Y), "03000000000000");
		keys("CLR 3"); HPVM::Constant(c);
		expect("constant after 3: Y = 3", reg2str(HPVM::Y), "03000000000000");
		check("constant * 3", "*", " 899377374.0");
		HPVM::Constant(c);
		check("digit after constant", "1", " 1.");
		expect("constant lifted to Y", reg2str(HPVM::Y), "02997924580008");
	}

	// ExchangeX(): X <-> a register, the stack does not move, the number
	// entry ends first, a number entered after it lifts the stack
	{
		HPVM::reg r = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0 }; // 7.
		keys("CLR DSP . 2 1 ENTER 2 ENTER 3 *"); HPVM::ExchangeX(r);
		expect("exchange: X = 7", seg2str(HPVM::Display, sizeof(HPVM::Display)), " 7.00");
		expect("exchange: r = 6", reg2str(r), "06000000000000");
		expect("exchange: Y = 1", reg2str(HPVM::Y), "01000000000000");
		keys("CLR 1 2"); HPVM::ExchangeX(r);
		expect("exchange in entry: X = 6", seg2str(HPVM::Display, sizeof(HPVM::Display)), " 6.00");
		expect("exchange in entry: r = 12", reg2str(r), "01200000000001");
		check("digit after exchange", "5", " 5.");
		expect("exchange lifted to Y", reg2str(HPVM::Y), "06000000000000");
	}

	// Reset(): the power-on state (FIX 2, memory clear), also while the ROM is
	// busy with a key; the F2 input (DEG/RAD switch) is kept
	keys("CLR DSP 4 5 STO 2 ENTER 3");
	HPVM::F2 = true;
	HPVM::Operation(HPVM::OpSIN_);
	for (int i = 0; i < 50; ++i) HPVM::Cycle();
	HPVM::Reset();
	run_to_idle();
	expect("reset display", seg2str(HPVM::Display, sizeof(HPVM::Display)), boot.c_str());
	expect("reset clears M1", reg2str(HPVM::M1), "00000000000000");
	expect("reset clears Y", reg2str(HPVM::Y), "00000000000000");
	expect("reset keeps F2", HPVM::F2 ? "1" : "0", "1");
	check("after reset: FIX 2, RAD", "3 0 SIN", "-0.99");
	HPVM::F2 = false;

	// key timing
	long c0 = total_cycles; keys("CLR 3 0 SIN");
	printf("CLR 3 0 SIN: %ld cycles\n", total_cycles - c0);
	c0 = total_cycles; keys("2 ENTER 1 0 y^x");
	printf("2 ENTER 1 0 y^x: %ld cycles\n", total_cycles - c0);

	printf("%d failures\n", fails);
	return fails;
}
