#pragma once

// -----------------------------------------------------------------------------
// Classic Virtual Machine. Include model data before this header.
// -----------------------------------------------------------------------------

namespace HPVM
{
	// clock parameters, one cycle is 14 digits of 4 bits
	constexpr u08 CLOCK_RATE_KHZ = 200;
	constexpr u08 BITS_PER_CYCLE = 14 * 4;
	constexpr u16 CYCLES_PER_SEC = CLOCK_RATE_KHZ * 1000UL / BITS_PER_CYCLE;

	// display render data: character codes (see DIGIT..DOT) of 15 positions
	enum { DIGIT = 0x00, BLANK = 0x10, DASH = 0x11, DOT = 0x12 };
	u08 Display[15];

	// public interface
	void Operation(u08 op);
	void OpAndWait(u08 op);
	void DisplayY(u08 (&disp)[sizeof(Display)]); // Y rendered as Display
	void Error(); // clears the stack, shows the error
	b08 Idling(); // true if idling
	b08 Cycle();  // true if display updated

	// status bits driven by C&T hardware
	constexpr u08 STATUS_BIT_KEY = 0;

	// cpu defines
	using bcd = u08;
	using reg = bcd[14];
	const u08 field_bounds[] DATAMEM =
	{
		0xFF, // P  : p..p
		0xC3, // M  : 3..12
		0x20, // X  : 0..2
		0xD0, // W  : 0..13
		0xF0, // WP : 0..p
		0xD3, // MS : 3..13
		0x22, // XS : 2..2
		0xDD  // S  : 13..13
	};
	enum { ADD  = 0, SUB  = 1 };
	enum { COPY = 0, SWAP = 1 };

	// registers, stored contiguously in this order: all are cleared together,
	// stack registers C..F are moved and rotated by multi-register reg_move()
	// calls, ZZ is always zero, TT holds the discarded result of a test
	enum { rA, rB, rC, rD, rE, rF, rM, rZZ, rTT };
	reg regs[9];
	reg& A = regs[rA];
	reg& B = regs[rB];
	reg& C = regs[rC]; // X
	reg& D = regs[rD]; // Y
	reg& E = regs[rE]; // Z
	reg& F = regs[rF]; // T
	reg& M = regs[rM];

	bcd* reg_ptr(u08 i)
	{
		return regs[0] + u08((i << 4) - (i << 1)); // i * 14
	}

	// arithmetic operations, 2 bytes each:
	// hi: kind (7..6), sub/swap (5), result reg (4..1), carry in (0)
	// lo: operand x reg (7..4), operand y reg (3..0)
	// tests write the result to TT, the next GO TO checks the carry
	enum { MATH = 0x00, MOVE = 0x40, SHL = 0x80, SHR = 0xC0 };
	#define OP(kind, ci, sw, r, x, y) u16(((kind) | (sw) << 5 | (r) << 1 | (ci)) << 8 | (x) << 4 | (y))
	const u16 math_ops[] DATAMEM =
	{
		OP(MATH, 0, SUB,  rTT, rZZ, rB ), // 0x00 IF B[f] = 0
		OP(MOVE, 0, COPY, rB,  rZZ, 0  ), // 0x01 0 -> B[f]
		OP(MATH, 0, SUB,  rTT, rA,  rC ), // 0x02 IF A >= C[f]
		OP(MATH, 1, SUB,  rTT, rC,  rZZ), // 0x03 IF C[f] >= 1
		OP(MOVE, 0, COPY, rC,  rB,  0  ), // 0x04 B -> C[f]
		OP(MATH, 0, SUB,  rC,  rZZ, rC ), // 0x05 0 - C -> C[f]
		OP(MOVE, 0, COPY, rC,  rZZ, 0  ), // 0x06 0 -> C[f]
		OP(MATH, 1, SUB,  rC,  rZZ, rC ), // 0x07 0 - C - 1 -> C[f]
		OP(SHL,  0, 0,    rA,  0,   0  ), // 0x08 SHIFT LEFT A[f]
		OP(MOVE, 0, COPY, rB,  rA,  0  ), // 0x09 A -> B[f]
		OP(MATH, 0, SUB,  rC,  rA,  rC ), // 0x0A A - C -> C[f]
		OP(MATH, 1, SUB,  rC,  rC,  rZZ), // 0x0B C - 1 -> C[f]
		OP(MOVE, 0, COPY, rA,  rC,  0  ), // 0x0C C -> A[f]
		OP(MATH, 0, SUB,  rTT, rZZ, rC ), // 0x0D IF C[f] = 0
		OP(MATH, 0, ADD,  rC,  rA,  rC ), // 0x0E A + C -> C[f]
		OP(MATH, 1, ADD,  rC,  rC,  rZZ), // 0x0F C + 1 -> C[f]
		OP(MATH, 0, SUB,  rTT, rA,  rB ), // 0x10 IF A >= B[f]
		OP(MOVE, 0, SWAP, rB,  rC,  0  ), // 0x11 B EXCHANGE C[f]
		OP(SHR,  0, 0,    rC,  0,   0  ), // 0x12 SHIFT RIGHT C[f]
		OP(MATH, 1, SUB,  rTT, rA,  rZZ), // 0x13 IF A[f] >= 1
		OP(SHR,  0, 0,    rB,  0,   0  ), // 0x14 SHIFT RIGHT B[f]
		OP(MATH, 0, ADD,  rC,  rC,  rC ), // 0x15 C + C -> C[f]
		OP(SHR,  0, 0,    rA,  0,   0  ), // 0x16 SHIFT RIGHT A[f]
		OP(MOVE, 0, COPY, rA,  rZZ, 0  ), // 0x17 0 -> A[f]
		OP(MATH, 0, SUB,  rA,  rA,  rB ), // 0x18 A - B -> A[f]
		OP(MOVE, 0, SWAP, rA,  rB,  0  ), // 0x19 A EXCHANGE B[f]
		OP(MATH, 0, SUB,  rA,  rA,  rC ), // 0x1A A - C -> A[f]
		OP(MATH, 1, SUB,  rA,  rA,  rZZ), // 0x1B A - 1 -> A[f]
		OP(MATH, 0, ADD,  rA,  rA,  rB ), // 0x1C A + B -> A[f]
		OP(MOVE, 0, SWAP, rA,  rC,  0  ), // 0x1D A EXCHANGE C[f]
		OP(MATH, 0, ADD,  rA,  rA,  rC ), // 0x1E A + C -> A[f]
		OP(MATH, 1, ADD,  rA,  rA,  rZZ), // 0x1F A + 1 -> A[f]
	};
	#undef OP

	// state
	u08 rom, pc, ret_pc, key_pc;
	u08 carry, p, ff, fl, s[12];
	b08 disp_enable, bp_idle;

	// basic math, decimal only, digits are 4-bit wide
	bcd alu(const bcd x, const bcd y, const u08 sub)
	{
		s08 res;
		if (sub)
		{
			res = x - y - carry;
			if (res < 0) { res += 10; carry = 1; } else carry = 0;
		}
		else
		{
			res = x + y + carry;
			if (res >= 10) { res -= 10; carry = 1; } else carry = 0;
		}
		return bcd(res);
	}

	// carry is used as input and holds the result carry/borrow
	void reg_math(reg r, const reg x, const reg y, const u08 sub)
	{
		for (u08 i = ff; i <= fl; ++i) r[i] = alu(x[i], y[i], sub);
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
			bcd t = x[i]; x[i] = y[i];
			if (swap) y[i] = t;
		}
	}

	void reg_move(reg x, reg y, const u08 swap)
	{
		reg_move(x, y, ff, fl, swap);
	}

	// P register wraps around 15 <-> 0
	void dec_p()
	{
		p = (p - 1) & 0x0F;
	}

	// one-shot key event abstraction: the key is delivered to the ROM
	// at once, so a key pressed while the ROM is busy replaces the
	// previous one; unlike the original C&T, S0 is not reasserted
	// while a key is held, but stays set until the ROM clears it
	void Operation(u08 op)
	{
		// handle shift key
		if (op & FlSHFT)
		{
			OpAndWait(OpSHFT);
			op ^= FlSHFT;
		}

		// handle operation key
		key_pc = MEM::DataRead(key_tokens + op);
		s[STATUS_BIT_KEY] = 1;
		bp_idle = 0;
	}

	void OpAndWait(u08 op)
	{
		Operation(op);
		do Cycle(); while (!Idling());
	}

	// renders Y the same way as Display, by x<>y twice (the model provides
	// OpSWAP and STATUS_BIT_LIFT); x<>y ends the number entry and enables
	// the stack lift, the lift is restored so that after ENTER or CLX the
	// next operation still replaces X
	void DisplayY(u08 (&disp)[sizeof(Display)])
	{
		u08 lift = s[STATUS_BIT_LIFT];
		OpAndWait(OpSWAP);
		memcpy(disp, Display, sizeof(Display));
		OpAndWait(OpSWAP);
		s[STATUS_BIT_LIFT] = lift;
	}

	// puts the calculator into the error state the way its ROM shows it:
	// the stack registers X..T are cleared and 1/x of zero is computed
	// (the model provides OpINV_), memory registers are kept
	void Error()
	{
		for (u08 i = rC * sizeof(reg); i < (rC + 4) * sizeof(reg); ++i) ((bcd*)regs)[i] = 0;
		Operation(OpINV_);
	}

	b08 Idling()
	{
		return (bp_idle && s[STATUS_BIT_IDLE]);
	}

	b08 Cycle()
	{
		// handle idle state breakpoint
		if (rom == (BP_IDLE >> 8) && pc == u08(BP_IDLE)) bp_idle = 1;

		// fetch and decode ROM
		u16 addr_h  = (u16(rom) << 8 | pc);
		u08 addr_l  = (addr_h >> 2);
		u08 op_type = MEM::ProgRead(rom_l + addr_l);
		if (pc & 0x02) op_type = __builtin_avr_swap(op_type); // >> 4
		if (pc & 0x01) op_type >>= 2;
		op_type &= 0x03;
		u08 op_code = MEM::ProgRead(rom_h + addr_h);

		u08 disp_update = 0;
		u08 prev_carry = carry;
		carry = 0;
		pc++;

		// Type 00: Misc Instructions
		// op_code is decoded as a whole, or bits 7..4 hold n
		// and bits 3..0 select the instruction of a group
		if (op_type == 0x00)
		{
			switch(op_code)
			{
				case 0x00: // NO OPERATION
					break;
				case 0x34: // KEY -> ROM ADDRESS
					pc = key_pc;
					break;
				case 0x07: // P - 1 -> P
					dec_p();
					break;
				case 0x0A: // DISPLAY TOGGLE
					disp_enable = !disp_enable;
					disp_update = 1;
					break;
				case 0x2A: // C EXCHANGE M
					reg_move(C, M, 0, 13, SWAP);
					break;
				case 0x4A: // C -> STACK
					reg_move(F, E, 0, 13, COPY);
					reg_move(E, D, 0, 13, COPY);
					reg_move(D, C, 0, 13, COPY);
					break;
				case 0x6A: // STACK -> A
					reg_move(A, D, 0, 13, COPY);
					reg_move(D, E, 0, 27, COPY); // D, E <- E, F
					break;
				case 0x8A: // DISPLAY OFF
					if (disp_enable)
					{
						disp_enable = 0;
						disp_update = 1;
					}
					break;
				case 0xAA: // M -> C
					reg_move(C, M, 0, 13, COPY);
					break;
				case 0xCA: // DOWN ROTATE
					reg_move(C, D, 0, 41, SWAP); // C <- D <- E <- F <- C
					break;
				case 0xEA: // CLEAR REGISTERS
					for (u08 i = 0; i < sizeof(regs); ++i) ((bcd*)regs)[i] = 0;
					break;
				case 0x0C: // RETURN
					pc = ret_pc;
					break;
				case 0x0D: // CLEAR STATUS
					for (u08 i = 0; i < sizeof(s); ++i) s[i] = 0;
					break;
				case 0x0F: // P + 1 -> P
					p = (p + 1) & 0x0F;
					break;
				default:
					u08 nnnn = __builtin_avr_swap(op_code) & 0x0F; // >> 4
					switch(op_code & 0x0F)
					{
						case 0x01: // 1 -> Sn
							s[nnnn] = 1;
							break;
						case 0x03: // n -> P
							p = nnnn;
							break;
						case 0x04: // ROM SELECT n
							rom = (nnnn >> 1);
							break;
						case 0x05: // IF Sn = 0
							carry = s[nnnn];
							break;
						case 0x06: // n -> C
							if (p < sizeof(reg)) C[p] = nnnn;
							dec_p();
							break;
						case 0x09: // 0 -> Sn
							s[nnnn] = 0;
							break;
						case 0x0B: // IF P # n
							carry = (p == nnnn);
							break;
					}
			}
		}

		// Type 01: Subroutine Call
		else if (op_type == 0x01)
		{
			// JSB addr, within the current ROM
			ret_pc = pc;
			pc = op_code;
		}

		// Type 10: Arithmetic Instructions
		else if (op_type == 0x02)
		{
			// get register boundaries
			fl = MEM::DataRead(field_bounds + (op_code & 0x07));
			ff = fl & 0x0F;
			fl = __builtin_avr_swap(fl) & 0x0F; // >> 4
			if (ff == 0x0F) ff = p;
			if (fl == 0x0F) fl = (p < sizeof(reg) ? p : sizeof(reg) - 1);

			// decode and process operation from the table
			const u08* opp = (const u08*)(math_ops + (op_code >> 3));
			u08 lo = MEM::DataRead(opp);
			u08 hi = MEM::DataRead(opp + 1);
			bcd* r = reg_ptr((hi >> 1) & 0x0F);
			bcd* x = reg_ptr(__builtin_avr_swap(lo) & 0x0F); // >> 4
			bcd* y = reg_ptr(lo & 0x0F);
			u08 sw = hi & 0x20;
			carry = hi & 0x01;
			switch (hi & 0xC0)
			{
				case MATH: reg_math(r, x, y, sw); break;
				case MOVE: reg_move(r, x, sw); break;
				case SHL:  reg_shl(r); break;
				case SHR:  reg_shr(r); break;
			}
		}

		// Type 11: Branching Instructions
		else // op_type == 0x03
		{
			// (THEN) GO TO addr, within the current ROM, if no carry
			if (!prev_carry)
			{
				pc = op_code;
			}
		}

		// display update
		if (disp_update)
		{
			// A/B digits 13..0: B digit 9 blanks the position, digits 13
			// and 2 show the signs, B digit 2 adds the dot after the digit
			Display[sizeof(Display) - 1] = BLANK;
			u08* dp = Display;
			for (u08 i = sizeof(reg); i-- > 0;)
			{
				if (disp_enable)
				{
					if (B[i] == 9)
						*dp++ = BLANK;
					else if (i == 2 || i == 13)
						*dp++ = (A[i] == 9 ? DASH : BLANK);
					else
						*dp++ = DIGIT + A[i];
					if (B[i] == 2)
						*dp++ = DOT;
				}
				else
					*dp++ = BLANK;
			}
			return true;
		}
		return false;
	}
}
