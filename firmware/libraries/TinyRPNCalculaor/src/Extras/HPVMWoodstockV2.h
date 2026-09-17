#pragma once

// HP-21 / Woodstock ACT, for the same single-translation-unit build as
// HPVMClassic.h. Include ONE of the two VM headers in the application.
// Requires u08, s08, u16, PROGMEM, DATAMEM and pointer-based MEM reads.
// HP21ROM.h supplies rom_l[256] and rom_h[1024], with no namespace wrapper.
// Generate it from your HP-21 ROM with pack_hp21.py (see HPVMWoodstock.md).
// The keyboard wait address below is for HP ROM/anode driver 1818-0129.

namespace HPVM
{
	#define HPVM_CLOCK_RATE       (200) // kHz
	#define HPVM_BIT_TIME_WIDTH   (1000 / HPVM_CLOCK_RATE) // uS
	#define HPVM_BITS_PER_CYCLE   (14 * 4)
	#define HPVM_CYCLE_TIME_WIDTH (HPVM_BITS_PER_CYCLE * HPVM_BIT_TIME_WIDTH)
	#define HPVM_CYCLES_PER_SEC   (1000000 / HPVM_CYCLE_TIME_WIDTH)

	void Operation(u08 op); // one key/operation; call when Idling() is true
	void Radians(bool enabled); // false = DEG (default), true = RAD
	bool Idling();
	bool Error();
	bool Cycle(); // one ACT word time; true if Display was refreshed
	char Display[15]; // glyph codes, NOT a null-terminated string

	// Physical HP-21 keys. Values are hardware key addresses, in hexadecimal.
	enum
	{
		OpINV  = 0xB4, OpSIN  = 0xB3, OpCOS  = 0xB2, OpTAN  = 0xB1, OpSHIFT = 0xB0,
		OpSWAP = 0x44, OpROT  = 0x43, OpEXP  = 0x42, OpSTO  = 0x41, OpRCL   = 0x40,
		OpPUSH = 0xD4, OpCHS  = 0xD2, OpEEX  = 0xD1, OpCLX  = 0xD0,
		OpSUB  = 0x64, OpNUM7 = 0x63, OpNUM8 = 0x62, OpNUM9 = 0x61,
		OpADD  = 0xA4, OpNUM4 = 0xA3, OpNUM5 = 0xA2, OpNUM6 = 0xA1,
		OpMUL  = 0x74, OpNUM1 = 0x73, OpNUM2 = 0x72, OpNUM3 = 0x71,
		OpDIV  = 0x94, OpNUM0 = 0x93, OpDOT  = 0x92, OpDSP  = 0x91,
		OpARC  = OpSHIFT, OpNONE = 0xFF,

		// Convenience operations: VM sends SHIFT, then the physical key.
		// These values deliberately occupy unused hardware key addresses.
		OpCLR = 0, OpPOW, OpLN, OpLOG, OpSQRT, OpPI, Op10X,
		OpASIN, OpACOS, OpATAN, OpTOREC, OpTOPOL,
		OpMSUB, OpMADD, OpMMUL, OpMDIV, OpSHIFT_COUNT
	};

	#include "HP21ROM.h"
	static_assert(sizeof(rom_l) == 256, "HP-21 needs 256 packed low-bit bytes");
	static_assert(sizeof(rom_h) == 1024, "HP-21 needs 1024 high-bit bytes");

	using bcd = u08;
	using reg = bcd[14];

	// High nibble = last digit, low nibble = first digit; F means P.
	const u08 field_bounds[] DATAMEM =
	{
		0xFF, 0xF0, 0x22, 0x20, 0xDD, 0xC3, 0xD0, 0xD3
	}; // P, WP, XS, X, S, M, W, MS

	// Woodstock uses different operand encodings for setting and testing P.
	const u08 p_set[] DATAMEM =
	{
		14, 4, 7, 8, 11, 2, 10, 12, 1, 3, 13, 6, 0, 9, 5, 14
	};
	const u08 p_test[] DATAMEM =
	{
		4, 8, 12, 2, 9, 1, 6, 3, 1, 13, 5, 0, 11, 10, 7, 4
	};
	const u08 shifted_keys[] DATAMEM =
	{
		OpCLX, OpINV, OpEXP, OpSTO, OpCHS, OpEEX, OpRCL,
		OpSIN, OpCOS, OpTAN, OpSWAP, OpROT,
		OpSUB, OpADD, OpMUL, OpDIV
	};

	enum { ADD = 0, SUB = 1, COPY = 0, SWAP = 1 };
	#define HPVM_DIGIT 0x00
	#define HPVM_SPACE 0x10
	#define HPVM_DASH  0x11
	#define HPVM_DOT   0x12

	reg A, B, C, D, E, F, M1, M2; // C=X, D=Y, E=Z, F=T
	u16 pc, ret_pc[2];
	u08 sp, carry, p, ff, fl, s[16], f;
	u08 base = 10;
	u08 then_goto, del_rom = 0xFF;
	u08 key_pc, pending_key = OpNONE, next_key = OpNONE;
	u08 disp_enable, idling, error, radians;

	bcd alu(const bcd x, const bcd y, const u08 sub)
	{
		s08 res;
		if (sub)
		{
			res = x - y - carry;
			carry = (res < 0);
			if (carry) res += base;
		}
		else
		{
			res = x + y + carry;
			carry = (res >= base);
			if (carry) res -= base;
		}
		return u08(res) & 0x0F;
	}

	void reg_clr(reg r)
	{
		for (u08 i = ff; i <= fl; ++i) r[i] = 0;
	}

	void reg_math(reg r, const reg x, const reg y, const u08 sub)
	{
		for (u08 i = ff; i <= fl; ++i) r[i] = alu(x[i], y[i], sub);
	}

	void reg_math(reg r, const u08 sub)
	{
		carry = 1;
		for (u08 i = ff; i <= fl; ++i) r[i] = alu(r[i], 0, sub);
	}

	void reg_shr(reg r)
	{
		if (ff > fl) return;
		for (u08 i = ff; i < fl; ++i) r[i] = r[i + 1];
		r[fl] = 0;
	}

	void reg_shl(reg r)
	{
		if (ff > fl) return;
		for (u08 i = fl; i > ff; --i) r[i] = r[i - 1];
		r[ff] = 0;
	}

	void reg_move(reg x, reg y, const u08 first, const u08 last, const u08 swap)
	{
		for (u08 i = first; i <= last; ++i)
		{
			bcd tmp = x[i];
			x[i] = y[i];
			if (swap) y[i] = tmp;
		}
	}

	void reg_move(reg x, reg y, const u08 swap)
	{
		reg_move(x, y, ff, fl, swap);
	}

	bool reg_nonzero(const reg r)
	{
		u08 bits = 0;
		for (u08 i = ff; i <= fl; ++i) bits |= r[i];
		return bits != 0;
	}

	void Operation(u08 op)
	{
		if (op == OpNONE) return;
		idling = error = 0;
		next_key = OpNONE;
		if (op < OpSHIFT_COUNT)
		{
			next_key = MEM::DataRead(shifted_keys + op);
			op = OpSHIFT;
		}
		pending_key = op;
	}

	void Radians(bool enabled) { radians = enabled; }
	bool Idling() { return idling != 0; }
	bool Error() { return error && Idling(); }

	void render_display()
	{
		u08 d = 0;
		error = 0;
		if (disp_enable)
		{
			for (s08 i = 13; i >= 2 && d < sizeof(Display); --i)
			{
				u08 a = A[i], b = B[i];
				if (b & 2)
					Display[d++] = (a >= 2 && (a & 7) != 7) ? HPVM_DASH : HPVM_SPACE;
				else
				{
					// Keep the Classic glyph set: Error is shown as five dashes.
					error |= (a == 0x0E); // E in the HP-21 character generator
					Display[d++] = a < 10 ? HPVM_DIGIT + a :
						(a == 0x0F ? HPVM_SPACE : HPVM_DASH);
				}
				if ((b & 1) && d < sizeof(Display)) Display[d++] = HPVM_DOT;
			}
		}
		while (d < sizeof(Display)) Display[d++] = HPVM_SPACE;
	}

	bool Cycle()
	{
		u08 disp_update = 0;

		// HP-21's wait2 loop, octal 0376. Inject a latched key event only
		// after the ROM has finished its release/debounce loop. No timer
		// or key-release call is required from the existing application.
		if (!then_goto && pc == 0x00FE && !s[15])
		{
			if (pending_key != OpNONE)
			{
				key_pc = pending_key;
				pending_key = next_key;
				next_key = OpNONE;
				s[15] = 1;
				idling = 0;
			}
			else if (!idling)
			{
				idling = 1;
				disp_update = 1;
			}
		}

		const u08 op_type = (MEM::ProgRead(rom_l + (pc >> 2)) >> ((pc & 3) << 1)) & 3;
		const u08 op_code = MEM::ProgRead(rom_h + pc);
		const u08 prev_carry = carry;
		const u08 prev_del_rom = del_rom;
		carry = 0;
		del_rom = 0xFF;
		pc = (pc + 1) & 0x0FFF;

		if (then_goto)
		{
			// The entire following 10-bit word is an address, not an opcode.
			then_goto = 0;
			if (!prev_carry) pc = (pc & 0x0C00) | (u16(op_code) << 2) | op_type;
		}
		else if (op_type == 0)
		{
			const u08 n = op_code >> 4;
			switch (op_code)
			{
				case 0x02: // clear A, B, C, stack; retain M1, M2 and f
					for (u08 i = 0; i < 14; ++i)
						A[i] = B[i] = C[i] = D[i] = E[i] = F[i] = 0;
					break;
				case 0x12: // clear status, except S1, S2, S5 and S15
					for (u08 i = 0; i < 16; ++i)
						if (i != 1 && i != 2 && i != 5 && i != 15) s[i] = 0;
					break;
				case 0x22: // display toggle
					disp_enable ^= 1;
					disp_update = 1;
					break;
				case 0x32: // display off
					disp_update |= disp_enable;
					disp_enable = 0;
					break;
				case 0x42: reg_move(C, M1, 0, 13, SWAP); break;
				case 0x52: reg_move(C, M1, 0, 13, COPY); break;
				case 0x62: reg_move(C, M2, 0, 13, SWAP); break;
				case 0x72: reg_move(C, M2, 0, 13, COPY); break;
				case 0x82: // stack -> A; T remains unchanged
					reg_move(A, D, 0, 13, COPY);
					reg_move(D, E, 0, 13, COPY);
					reg_move(E, F, 0, 13, COPY);
					break;
				case 0x92: // down rotate; leave A, B, M1 and M2 untouched
					reg_move(C, D, 0, 13, SWAP);
					reg_move(D, E, 0, 13, SWAP);
					reg_move(E, F, 0, 13, SWAP);
					break;
				case 0xA2: reg_move(A, D, 0, 13, COPY); break;
				case 0xB2: // C -> stack
					reg_move(F, E, 0, 13, COPY);
					reg_move(E, D, 0, 13, COPY);
					reg_move(D, C, 0, 13, COPY);
					break;
				case 0xC2: base = 10; break;
				case 0xE2: A[0] = f; break;
				case 0xF2:
				{
					u08 tmp = A[0]; A[0] = f; f = tmp;
					break;
				}
				case 0x04: pc = (pc & 0x0F00) | key_pc; break;
				case 0x24: pc = (pc & 0x0F00) | (A[2] << 4) | A[1]; break;
				case 0x44: base = 16; break;
				case 0x64: p = p ? p - 1 : 13; break;
				case 0x74: if (++p >= 14) p = 0; break;
				case 0x84: sp ^= 1; pc = ret_pc[sp]; break;
				default:
					switch (op_code & 0x0F)
					{
						case 0x1: s[n] = 1; break;
						case 0x3: s[n] = 0; break;
						case 0x5: carry = !s[n]; then_goto = 1; break;
						case 0x6:
							if (p < 14) C[p] = n;
							p = p ? p - 1 : 13;
							break;
						case 0x7: carry = s[n]; then_goto = 1; break;
						case 0x8: pc = (u16(n) << 8) | u08(pc); break;
						case 0x9: carry = (p != MEM::DataRead(p_test + n)); then_goto = 1; break;
						case 0xB: carry = (p == MEM::DataRead(p_test + n)); then_goto = 1; break;
						case 0xD: del_rom = n; break;
						case 0xF: p = MEM::DataRead(p_set + n); break;
						// NOP, HI I'M WOODSTOCK, and instructions absent from HP-21.
					}
			}
		}
		else if (op_type == 1)
		{
			ret_pc[sp] = pc;
			sp ^= 1;
			pc = (pc & 0x0F00) | op_code;
		}
		else if (op_type == 2)
		{
			fl = MEM::DataRead(field_bounds + (op_code & 7));
			ff = fl & 0x0F; fl >>= 4;
			if (ff == 0x0F) ff = p;
			if (fl == 0x0F) fl = p < 14 ? p : 13;

			switch (op_code >> 3)
			{
				case 0x00: reg_clr(A); break;
				case 0x01: reg_clr(B); break;
				case 0x02: reg_move(A, B, SWAP); break;
				case 0x03: reg_move(B, A, COPY); break;
				case 0x04: reg_move(A, C, SWAP); break;
				case 0x05: reg_move(A, C, COPY); break;
				case 0x06: reg_move(C, B, COPY); break;
				case 0x07: reg_move(B, C, SWAP); break;
				case 0x08: reg_clr(C); break;
				case 0x09: reg_math(A, A, B, ADD); break;
				case 0x0A: reg_math(A, A, C, ADD); break;
				case 0x0B: reg_math(C, C, C, ADD); break;
				case 0x0C: reg_math(C, A, C, ADD); break;
				case 0x0D: reg_math(A, ADD); break;
				case 0x0E: reg_shl(A); break;
				case 0x0F: reg_math(C, ADD); break;
				case 0x10: reg_math(A, A, B, SUB); break;
				case 0x11: reg_math(C, A, C, SUB); break;
				case 0x12: reg_math(A, SUB); break;
				case 0x13: reg_math(C, SUB); break;
				case 0x14: case 0x15:
					carry = (op_code >> 3) & 1;
					for (u08 i = ff; i <= fl; ++i) C[i] = alu(0, C[i], SUB);
					break;
				case 0x16: carry = reg_nonzero(B); then_goto = 1; break;
				case 0x17: carry = reg_nonzero(C); then_goto = 1; break;
				case 0x18:
					for (u08 i = ff; i <= fl; ++i) alu(A[i], C[i], SUB);
					then_goto = 1;
					break;
				case 0x19:
					for (u08 i = ff; i <= fl; ++i) alu(A[i], B[i], SUB);
					then_goto = 1;
					break;
				case 0x1A: carry = !reg_nonzero(A); then_goto = 1; break;
				case 0x1B: carry = !reg_nonzero(C); then_goto = 1; break;
				case 0x1C: reg_math(A, A, C, SUB); break;
				case 0x1D: reg_shr(A); break;
				case 0x1E: reg_shr(B); break;
				case 0x1F: reg_shr(C); break;
			}
		}
		else if (!prev_carry) pc = (pc & 0x0F00) | op_code;

		// Delayed ROM selection takes effect AFTER the following word.
		if (prev_del_rom != 0xFF) pc = (u16(prev_del_rom) << 8) | u08(pc);
		s[5] = 1; // battery OK
		s[3] |= radians; // RAD is a hardware-set, software-cleared status bit

		if (disp_update) render_display();
		return disp_update != 0;
	}
}
