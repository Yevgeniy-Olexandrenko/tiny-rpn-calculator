// -----------------------------------------------------------------------------
// Desktop test of HPVMClassic.h + HP35Model.h on the original HP-35 ROM.
// The library headers are compiled for the PC with small stubs for AVR.
//
// hp35test           run all checks, exit code = number of failures
// hp35test "<keys>"  type the keys and show the display and the stack
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

namespace MEM
{
	u08  ProgRead(const u08* p) { return *p; }
	void ProgCopy(void* d, const void* s, size_t n) { memcpy(d, s, n); }
	u08  DataRead(const u08* p) { return *p; }
	void DataCopy(void* d, const void* s, size_t n) { memcpy(d, s, n); }
}
#include "Extras/HP35Model.h"
#include "Extras/HPVMClassic.h"

static long total_cycles = 0;

// character codes of the display: DIGIT + digit, BLANK, DASH, DOT
std::string disp2str(const u08* disp, int n)
{
	std::string s;
	for (int i = 0; i < n; ++i)
	{
		u08 c = disp[i];
		if (c == HPVM::BLANK) s += ' ';
		else if (c == HPVM::DASH) s += '-';
		else if (c == HPVM::DOT) s += '.';
		else s += "0123456789ABCDEF"[c & 15];
	}
	return s;
}

std::string display()
{
	return disp2str(HPVM::Display, sizeof(HPVM::Display));
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
	{"x^y", HPVM::OpPOW_}, {"LOG", HPVM::OpLOG_}, {"LN", HPVM::OpLN__}, {"e^x", HPVM::OpEXP_}, {"CLR", HPVM::OpCLR_},
	{"SQRT", HPVM::OpSQRT}, {"ARC", HPVM::OpARC_}, {"SIN", HPVM::OpSIN_}, {"COS", HPVM::OpCOS_}, {"TAN", HPVM::OpTAN_},
	{"1/x", HPVM::OpINV_}, {"x<>y", HPVM::OpSWAP}, {"RDN", HPVM::OpROT_}, {"STO", HPVM::OpSTO_}, {"RCL", HPVM::OpRCL_},
	{"ENTER", HPVM::OpPUSH}, {"CHS", HPVM::OpCHS_}, {"EEX", HPVM::OpEEX_}, {"CLX", HPVM::OpCLX_},
	{"-", HPVM::OpSUB_}, {"7", HPVM::OpNUM7}, {"8", HPVM::OpNUM8}, {"9", HPVM::OpNUM9},
	{"+", HPVM::OpADD_}, {"4", HPVM::OpNUM4}, {"5", HPVM::OpNUM5}, {"6", HPVM::OpNUM6},
	{"*", HPVM::OpMUL_}, {"1", HPVM::OpNUM1}, {"2", HPVM::OpNUM2}, {"3", HPVM::OpNUM3},
	{"/", HPVM::OpDIV_}, {"0", HPVM::OpNUM0}, {".", HPVM::OpDOT_}, {"PI", HPVM::OpPI__},
	{"ASIN", HPVM::OpASIN}, {"ACOS", HPVM::OpACOS}, {"ATAN", HPVM::OpATAN},
};

// keys: whitespace separated; a multi-digit token is typed digit by digit
std::string keys(const char* seq)
{
	std::istringstream in(seq);
	std::string t;
	while (in >> t)
	{
		auto it = ops.find(t);
		if (it == ops.end())
		{
			bool digits = !t.empty();
			for (char c : t) if (!((c >= '0' && c <= '9') || c == '.')) digits = false;
			if (!digits)
			{
				printf("unknown key '%s'\n", t.c_str());
				continue;
			}
			for (char c : t) HPVM::OpAndWait(ops[std::string(1, c)]);
			continue;
		}
		HPVM::Operation(it->second);
		if (!run_to_idle()) printf("  !!! no idle after '%s' (rom=%u pc=%03o)\n", t.c_str(), HPVM::rom, HPVM::pc);
	}
	return display();
}

int fails = 0;

void expect(const char* name, const std::string& got, const char* expected)
{
	std::string g = got; while (!g.empty() && g.back() == ' ') g.pop_back();
	bool ok = (g == expected);
	if (!ok) ++fails;
	printf("%s %-24s [%s]%s%s\n", ok ? "ok  " : "FAIL", name, got.c_str(),
		ok ? "" : "  expected ", ok ? "" : expected);
}

void check(const char* name, const char* seq, const char* expected)
{
	std::string got = keys(seq);
	std::string g = got; while (!g.empty() && g.back() == ' ') g.pop_back();
	bool ok = (g == expected);
	if (!ok) ++fails;
	printf("%s %-24s %-28s -> [%s]%s%s\n", ok ? "ok  " : "FAIL", name, seq, got.c_str(),
		ok ? "" : "  expected ", ok ? "" : expected);
}

// display updates while the ROM is idle: the error flashes, a result does not
int flashes(long cycles)
{
	int n = 0;
	for (long i = 0; i < cycles; ++i) if (HPVM::Cycle()) ++n;
	return n;
}

int main(int argc, char** argv)
{
	if (!run_to_idle()) printf("!!! no idle after power on\n");
	if (argc > 1)
	{
		keys(argv[1]);
		printf("[%s] X=%s Y=%s Z=%s T=%s M=%s\n", display().c_str(), reg2str(HPVM::C).c_str(), reg2str(HPVM::D).c_str(),
			reg2str(HPVM::E).c_str(), reg2str(HPVM::F).c_str(), reg2str(HPVM::M).c_str());
		return 0;
	}
	printf("CYCLES_PER_SEC = %u\n", HPVM::CYCLES_PER_SEC);
	std::string boot = display(); // power-on display, without the trailing blanks
	while (!boot.empty() && boot.back() == ' ') boot.pop_back();
	printf("power on: [%s] cycles=%ld\n", display().c_str(), total_cycles);

	// operations and display formats, the values of the user manual
	check("entry",           "CLR 1 2 3 . 4 5",          " 123.45");
	check("enter add",       "CLR 2 ENTER 3 +",          " 5.");
	check("sub",             "CLR 2 ENTER 3 -",          "-1.");
	check("div sci",         "CLR 1 ENTER 3 0 0 /",      " 3.333333333-03");
	check("cos 60",          "CLR 6 0 COS",              " .5000000001");
	check("x^y",             "CLR 7 ENTER 2 x^y",        " 128."); // X to the power Y
	check("pi",              "CLR PI",                   " 3.141592654");
	check("overflow",        "CLR 9 EEX 9 9 ENTER *",    " 9.999999999 99");
	check("chs",             "CLR 5 CHS",                "-5.");

	// error: 1/0 flashes 0., the ROM stays idle, the next key works
	keys("CLR 0 1/x");
	expect("error shown", display(), " 0.");
	bool idle = HPVM::Idling();
	int n = flashes(3000);
	expect("error flashes", n >= 2 && idle ? "yes" : "no", "yes");
	check("after error",     "CLX 2",                    " 2.");

	// X register during digit entry (used by the sketch to read integers)
	keys("CLR 1 2");
	expect("entry 12 C", reg2str(HPVM::C), "01200000000001");

	// scripted sequences of the hp35 sketch
	keys("CLR 1 ENTER 2 ENTER 3 ENTER 4 RDN RDN RDN"); // KEY_ROTU
	expect("roll up X", reg2str(HPVM::C), "01000000000000");
	expect("roll up Y", reg2str(HPVM::D), "04000000000000");
	keys("CLR 1 ENTER 2 ENTER 3 ENTER 4 RDN"); // dropStack()
	HPVM::reg_move(HPVM::F, HPVM::E, 0, 13, HPVM::COPY);
	expect("drop X", reg2str(HPVM::C), "03000000000000");
	expect("drop T", reg2str(HPVM::F), "01000000000000");

	u08 hidden[sizeof(HPVM::Display)];
	keys("CLR 1 2 ENTER 3 0"); // KEY_FUNC
	HPVM::DisplayY(hidden);
	expect("func hidden", disp2str(hidden, sizeof(hidden)), " 12.");
	expect("func X", display(), " 30.");
	keys("CLR 3 ENTER"); // KEY_FUNC keeps the stack lift disabled
	HPVM::DisplayY(hidden);
	keys("PI");
	expect("func lift Y", reg2str(HPVM::D), "03000000000000");
	expect("func lift Z", reg2str(HPVM::E), "00000000000000");

	keys("CLR 5 STO 9 ENTER 1 2"); // invalid TIME, DATE or ZONE values
	HPVM::Error();
	run_to_idle();
	expect("error() shown", display(), " 0.");
	expect("error() clears Y", reg2str(HPVM::D), "00000000000000");
	expect("error() keeps M", reg2str(HPVM::M), "05000000000000");
	check("after error()",   "3 ENTER RCL",              " 5.");

	// DisplayY() and Error() while the ROM is still busy with a key
	keys("CLR 3 0");
	HPVM::Operation(HPVM::OpSIN_);
	for (int i = 0; i < 50; ++i) HPVM::Cycle();
	HPVM::Error();
	run_to_idle();
	expect("busy error shown", display(), " 0.");
	expect("busy error clears Y", reg2str(HPVM::D), "00000000000000");
	keys("CLR 3");
	HPVM::Operation(HPVM::OpPUSH);
	for (int i = 0; i < 5; ++i) HPVM::Cycle();
	HPVM::DisplayY(hidden);
	keys("PI");
	expect("busy func lift Y", reg2str(HPVM::D), "03000000000000");
	expect("busy func lift Z", reg2str(HPVM::E), "00000000000000");

	// keys pressed while the ROM is busy are not lost, a shifted one too
	keys("CLR 3 0");
	HPVM::Operation(HPVM::OpSIN_);
	for (int i = 0; i < 50; ++i) HPVM::Cycle();
	HPVM::Operation(HPVM::OpNUM2);
	HPVM::Operation(HPVM::OpSQRT);
	run_to_idle();
	expect("two keys X", display(), " 1.414213562");
	expect("two keys Y", reg2str(HPVM::D), "05000000000999");

	// Reset(): the power-on state, also while the ROM is busy with a key
	keys("CLR 5 STO 2 ENTER 3");
	HPVM::Operation(HPVM::OpSIN_);
	for (int i = 0; i < 50; ++i) HPVM::Cycle();
	HPVM::Reset();
	run_to_idle();
	expect("reset display", display(), boot.c_str());
	expect("reset clears M", reg2str(HPVM::M), "00000000000000");
	expect("reset clears Y", reg2str(HPVM::D), "00000000000000");
	check("after reset", "2 ENTER 3 +", " 5.");

	printf("%d failures\n", fails);
	return fails;
}
