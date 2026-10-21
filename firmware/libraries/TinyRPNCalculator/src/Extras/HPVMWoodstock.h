#pragma once

// -----------------------------------------------------------------------------
// Woodstock Virtual Machine. Include model data before this header.
// -----------------------------------------------------------------------------

namespace HPVM
{
	// clock parameters, one cycle is 14 digits of 4 bits
	constexpr u08 CLOCK_RATE_KHZ = 185;
	constexpr u08 BITS_PER_CYCLE = 14 * 4;
	constexpr u16 CYCLES_PER_SEC = CLOCK_RATE_KHZ * 1000UL / BITS_PER_CYCLE;

	// display render data: 7-segment patterns (see TXT::SEG_*) of 12 positions
	u08 Display[12];

	// public interface
	void Operation(u08 op); // key press, the previous one is finished first
	void OpAndWait(u08 op); // key press and wait for its result
	void WaitIdle(); // runs the ROM until it waits for a key
	void DisplayY(u08 (&disp)[sizeof(Display)]); // Y rendered as Display
	void Error(); // clears the stack, shows the error
	void Reset(); // power-on state, the ROM starts again
	b08 Idling(); // true if idling
	b08 Cycle();  // true if display updated

	// external flag inputs, sampled every cycle
	b08 F1; // false sets S5 (HP-21: battery is ok)
	b08 F2; // true sets S3 (HP-21: RAD angle mode)

	// status bits driven by ACT hardware
	constexpr u08 STATUS_BIT_F2  = 3;
	constexpr u08 STATUS_BIT_F1  = 5;
	constexpr u08 STATUS_BIT_KEY = 15;

	// cpu defines
	using bcd = u08;
	using reg = bcd[14];
	const u08 field_bounds[] DATAMEM =
	{
		0xFF, // P  : p..p
		0xF0, // WP : 0..p
		0x22, // XS : 2..2
		0x20, // X  : 0..2
		0xDD, // S  : 13..13
		0xC3, // M  : 3..12
		0xD0, // W  : 0..13
		0xD3  // MS : 3..13
	};
	enum { ADD  = 0, SUB  = 1 };
	enum { COPY = 0, SWAP = 1 };

	// operands of the P set and P test instructions are encoded differently
	const u08 p_set_map[]  DATAMEM = { 0xE, 0x4, 0x7, 0x8, 0xB, 0x2, 0xA, 0xC, 0x1, 0x3, 0xD, 0x6, 0x0, 0x9, 0x5, 0xE };
	const u08 p_test_map[] DATAMEM = { 0x4, 0x8, 0xC, 0x2, 0x9, 0x1, 0x6, 0x3, 0x1, 0xD, 0x5, 0x0, 0xB, 0xA, 0x7, 0x4 };

	// registers, stored contiguously in this order: A..T are cleared together,
	// stack registers C..T are moved and rotated by multi-register reg_move()
	// calls, ZZ is always zero, TT holds the discarded result of a test
	enum { rA, rB, rC, rY, rZ, rT, rM1, rM2, rZZ, rTT };
	reg regs[10];
	reg& A  = regs[rA];
	reg& B  = regs[rB];
	reg& C  = regs[rC]; // X
	reg& Y  = regs[rY];
	reg& Z  = regs[rZ];
	reg& T  = regs[rT];
	reg& M1 = regs[rM1];
	reg& M2 = regs[rM2];

	bcd* reg_ptr(u08 i)
	{
		return regs[0] + u08((i << 4) - (i << 1)); // i * 14
	}

	// arithmetic operations, 2 bytes each:
	// hi: kind (7..6), sub/swap (5), result reg (4..1), carry in (0)
	// lo: operand x reg (7..4), operand y reg (3..0)
	// tests write the result to TT, the next word is a then-goto address
	enum { MATH = 0x00, MOVE = 0x40, SHL = 0x80, SHR = 0xC0 };
	#define OP(kind, ci, sw, r, x, y) u16(((kind) | (sw) << 5 | (r) << 1 | (ci)) << 8 | (x) << 4 | (y))
	const u16 math_ops[] DATAMEM =
	{
		OP(MOVE, 0, COPY, rA,  rZZ, 0  ), // 0x00 0 -> A[f]
		OP(MOVE, 0, COPY, rB,  rZZ, 0  ), // 0x01 0 -> B[f]
		OP(MOVE, 0, SWAP, rA,  rB,  0  ), // 0x02 A EXCHANGE B[f]
		OP(MOVE, 0, COPY, rB,  rA,  0  ), // 0x03 A -> B[f]
		OP(MOVE, 0, SWAP, rA,  rC,  0  ), // 0x04 A EXCHANGE C[f]
		OP(MOVE, 0, COPY, rA,  rC,  0  ), // 0x05 C -> A[f]
		OP(MOVE, 0, COPY, rC,  rB,  0  ), // 0x06 B -> C[f]
		OP(MOVE, 0, SWAP, rB,  rC,  0  ), // 0x07 B EXCHANGE C[f]
		OP(MOVE, 0, COPY, rC,  rZZ, 0  ), // 0x08 0 -> C[f]
		OP(MATH, 0, ADD,  rA,  rA,  rB ), // 0x09 A + B -> A[f]
		OP(MATH, 0, ADD,  rA,  rA,  rC ), // 0x0A A + C -> A[f]
		OP(MATH, 0, ADD,  rC,  rC,  rC ), // 0x0B C + C -> C[f]
		OP(MATH, 0, ADD,  rC,  rA,  rC ), // 0x0C A + C -> C[f]
		OP(MATH, 1, ADD,  rA,  rA,  rZZ), // 0x0D A + 1 -> A[f]
		OP(SHL,  0, 0,    rA,  0,   0  ), // 0x0E SHIFT LEFT A[f]
		OP(MATH, 1, ADD,  rC,  rC,  rZZ), // 0x0F C + 1 -> C[f]
		OP(MATH, 0, SUB,  rA,  rA,  rB ), // 0x10 A - B -> A[f]
		OP(MATH, 0, SUB,  rC,  rA,  rC ), // 0x11 A - C -> C[f]
		OP(MATH, 1, SUB,  rA,  rA,  rZZ), // 0x12 A - 1 -> A[f]
		OP(MATH, 1, SUB,  rC,  rC,  rZZ), // 0x13 C - 1 -> C[f]
		OP(MATH, 0, SUB,  rC,  rZZ, rC ), // 0x14 0 - C -> C[f]
		OP(MATH, 1, SUB,  rC,  rZZ, rC ), // 0x15 0 - C - 1 -> C[f]
		OP(MATH, 0, SUB,  rTT, rZZ, rB ), // 0x16 IF B[f] = 0
		OP(MATH, 0, SUB,  rTT, rZZ, rC ), // 0x17 IF C[f] = 0
		OP(MATH, 0, SUB,  rTT, rA,  rC ), // 0x18 IF A >= C[f]
		OP(MATH, 0, SUB,  rTT, rA,  rB ), // 0x19 IF A >= B[f]
		OP(MATH, 1, SUB,  rTT, rA,  rZZ), // 0x1A IF A[f] # 0
		OP(MATH, 1, SUB,  rTT, rC,  rZZ), // 0x1B IF C[f] # 0
		OP(MATH, 0, SUB,  rA,  rA,  rC ), // 0x1C A - C -> A[f]
		OP(SHR,  0, 0,    rA,  0,   0  ), // 0x1D SHIFT RIGHT A[f]
		OP(SHR,  0, 0,    rB,  0,   0  ), // 0x1E SHIFT RIGHT B[f]
		OP(SHR,  0, 0,    rC,  0,   0  ), // 0x1F SHIFT RIGHT C[f]
	};
	#undef OP

	// state
	u16 pc, ret_pc[2];
	u08 sp, key_pc, key_new, del_rom;
	u08 carry, p, ff, fl, f, s[16];
	u08 base = 10;
	b08 disp_enable, key_hit, then_goto;

	// basic math, decimal or binary, digits are 4-bit wide
	bcd alu(const bcd x, const bcd y, const u08 sub)
	{
		s08 res;
		if (sub)
		{
			res = x - y - carry;
			if (res < 0) { res += base; carry = 1; } else carry = 0;
		}
		else
		{
			res = x + y + carry;
			if (res >= base) { res -= base; carry = 1; } else carry = 0;
		}
		return bcd(res) & 0x0F;
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

	// P register wraps around 13 <-> 0
	void dec_p()
	{
		p = (p ? p - 1 : 13);
	}

	void WaitIdle()
	{
		while (!Idling()) Cycle();
	}

	// one-shot key event abstraction: the key is latched and delivered to
	// the ROM in its idle loop, a key pressed while the ROM is busy waits
	// until the previous one is done, so no key is lost; unlike the original
	// ACT, S15 is not reasserted while a key is held, but stays set until
	// the ROM clears it, that is enough for the ROM to dispatch the same
	// key again (HP-21 does it on number entry)
	void Operation(u08 op)
	{
		// handle shift key as a separate key press
		if (op & FlSHFT)
		{
			Operation(OpSHFT);
			op ^= FlSHFT;
		}

		// finish the previous key and handle operation key
		WaitIdle();
		key_new = MEM::DataRead(key_tokens + op);
		key_hit = true;
	}

	void OpAndWait(u08 op)
	{
		Operation(op);
		WaitIdle();
	}

	// renders Y the same way as Display, by x<>y twice (the model provides
	// OpSWAP and STATUS_BIT_LIFT); x<>y ends the number entry and enables
	// the stack lift, the lift is restored so that after ENTER or CLX the
	// next operation still replaces X
	void DisplayY(u08 (&disp)[sizeof(Display)])
	{
		WaitIdle();
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
		WaitIdle();
		for (u08 i = rC * sizeof(reg); i < (rC + 4) * sizeof(reg); ++i) ((bcd*)regs)[i] = 0;
		Operation(OpINV_);
	}

	// the state after power-on: the ROM starts again from address 0 with all
	// registers (the memory too) and the status clear, as the original loses
	// them without power, and runs until it waits for a key, so the display
	// is drawn; the ROM sets the rest before use (the return stack, the key
	// code, the field); F1 and F2 are inputs (HP-21: a switch), kept
	NOINLINE
	void Reset()
	{
		memset(regs, 0, sizeof(regs));
		memset(s, 0, sizeof(s));
		pc = 0;
		del_rom = carry = p = f = 0;
		base = 10;
		disp_enable = key_hit = then_goto = false;
		WaitIdle();
	}

	b08 Idling()
	{
		return (pc == BP_IDLE && !s[STATUS_BIT_KEY] && !key_hit);
	}

	b08 Cycle()
	{
		// deliver latched key in idle state
		if (pc == BP_IDLE && !s[STATUS_BIT_KEY] && key_hit)
		{
			key_pc = key_new;
			s[STATUS_BIT_KEY] = 1;
			key_hit = false;
		}

		// fetch and decode ROM
		u08 op_type = MEM::ProgRead(rom_l + (pc >> 2));
		if (pc & 0x02) op_type = __builtin_avr_swap(op_type); // >> 4
		if (pc & 0x01) op_type >>= 2;
		op_type &= 0x03;
		u08 op_code = MEM::ProgRead(rom_h + pc);

		u08 disp_update = 0;
		u08 prev_carry = carry;
		u08 prev_del_rom = del_rom;
		carry = 0;
		del_rom = 0;
		pc++;

		// Then-goto address word follows a test instruction
		if (then_goto)
		{
			// (THEN) GO TO addr, 10-bit address
			then_goto = 0;
			if (!prev_carry)
			{
				pc = (pc & 0x0C00) | (u16(op_code) << 2) | op_type;
			}
		}

		// Type 00: Misc Instructions
		// op_code bits 7..4 hold n or select the instruction of a group,
		// not implemented, unused by HP-21: KEYS -> A, A -> ROM ADDRESS,
		// RESET TWF, CIRCULATE A LEFT and the data storage instructions;
		// NO OPERATION and HI I'M WOODSTOCK are decoded as no operation
		else if (op_type == 0x00)
		{
			u08 nnnn = __builtin_avr_swap(op_code) & 0x0F; // >> 4
			switch(op_code & 0x0F)
			{
				case 0x01: // 1 -> Sn
					s[nnnn] = 1;
					break;
				case 0x02:
					switch(nnnn)
					{
						case 0x00: // CLEAR REGISTERS
							// M1, M2, F and P are kept
							for (u08 i = 0; i < rM1 * sizeof(reg); ++i) ((bcd*)regs)[i] = 0;
							break;
						case 0x01: // CLEAR STATUS
							// S1, S2, S5 and S15 are kept
							for (u08 i = 0; i < sizeof(s); ++i)
							{
								if (i != 1 && i != 2 && i != 5 && i != 15) s[i] = 0;
							}
							break;
						case 0x02: // DISPLAY TOGGLE
							disp_enable = !disp_enable;
							disp_update = 1;
							break;
						case 0x03: // DISPLAY OFF
							if (disp_enable)
							{
								disp_enable = 0;
								disp_update = 1;
							}
							break;
						case 0x04: // M1 EXCHANGE C
							reg_move(C, M1, 0, 13, SWAP);
							break;
						case 0x05: // M1 -> C
							reg_move(C, M1, 0, 13, COPY);
							break;
						case 0x06: // M2 EXCHANGE C
							reg_move(C, M2, 0, 13, SWAP);
							break;
						case 0x07: // M2 -> C
							reg_move(C, M2, 0, 13, COPY);
							break;
						case 0x08: // STACK -> A
							reg_move(A, Y, 0, 13, COPY);
							reg_move(Y, Z, 0, 27, COPY); // Y, Z <- Z, T
							break;
						case 0x09: // DOWN ROTATE
							reg_move(C, Y, 0, 41, SWAP); // C <- Y <- Z <- T <- C
							break;
						case 0x0A: // Y -> A
							reg_move(A, Y, 0, 13, COPY);
							break;
						case 0x0B: // C -> STACK
							reg_move(T, Z, 0, 13, COPY);
							reg_move(Z, Y, 0, 13, COPY);
							reg_move(Y, C, 0, 13, COPY);
							break;
						case 0x0C: // DECIMAL
							base = 10;
							break;
						case 0x0E: // F -> A[0]
							A[0] = f;
							break;
						case 0x0F: // F EXCHANGE A[0]
							{ bcd t = A[0]; A[0] = f; f = t; }
							break;
					}
					break;
				case 0x03: // 0 -> Sn
					s[nnnn] = 0;
					break;
				case 0x04:
					switch(nnnn)
					{
						case 0x00: // KEYS -> ROM ADDRESS
							pc = (pc & 0x0F00) | key_pc;
							break;
						case 0x04: // BINARY
							base = 16;
							break;
						case 0x06: // P - 1 -> P
							dec_p();
							break;
						case 0x07: // P + 1 -> P
							p = (p < 13 ? p + 1 : 0);
							break;
						case 0x08: // RETURN
							sp ^= 1;
							pc = ret_pc[sp];
							break;
					}
					break;
				case 0x05: // IF 1 = Sn
					carry = !s[nnnn];
					then_goto = 1;
					break;
				case 0x06: // LOAD CONSTANT n
					if (p < sizeof(reg)) C[p] = nnnn;
					dec_p();
					break;
				case 0x07: // IF 0 = Sn
					carry = s[nnnn];
					then_goto = 1;
					break;
				case 0x08: // SELECT ROM n
					pc = (u16(nnnn) << 8) | (pc & 0xFF);
					break;
				case 0x09: // IF P = n
					carry = (p != MEM::DataRead(p_test_map + nnnn));
					then_goto = 1;
					break;
				case 0x0B: // IF P # n
					carry = (p == MEM::DataRead(p_test_map + nnnn));
					then_goto = 1;
					break;
				case 0x0D: // DELAYED SELECT ROM n
					del_rom = 0x10 | nnnn; // 0x10 marks the request
					break;
				case 0x0F: // n -> P
					p = MEM::DataRead(p_set_map + nnnn);
					break;
			}
		}

		// Type 01: Subroutine Call
		else if (op_type == 0x01)
		{
			// JSB addr, within the current ROM
			ret_pc[sp] = pc;
			sp ^= 1;
			pc = (pc & 0x0F00) | op_code;
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
			u08 ri = (hi >> 1) & 0x0F;
			bcd* r = reg_ptr(ri);
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
			then_goto = (ri == rTT);
		}

		// Type 11: Branching Instructions
		else // op_type == 0x03
		{
			// GO TO addr, within the current ROM, if no carry
			if (!prev_carry)
			{
				pc = (pc & 0x0F00) | op_code;
			}
		}

		// delayed rom select takes effect after the next instruction
		if (prev_del_rom)
		{
			pc = (u16(prev_del_rom & 0x0F) << 8) | (pc & 0xFF);
		}

		// external flags
		if (!F1) s[STATUS_BIT_F1] = 1;
		if ( F2) s[STATUS_BIT_F2] = 1;

		// display update
		if (disp_update)
		{
			// A/B digits 13..2: B bit 1 selects a sign instead of
			// the character from A, B bit 0 lights the dot
			u08* dp = Display;
			for (u08 i = sizeof(reg); i-- > 2;)
			{
				u08 seg = TXT::SEG_SPACE;
				if (disp_enable)
				{
					bcd a = A[i], b = B[i];
					if (b & 0x02)
					{
						if (a >= 2 && (a & 0x07) != 7) seg = TXT::SEG_DASH;
					}
					else
						seg = MEM::DataRead(char_gen + a);
					if (b & 0x01)
						seg |= TXT::SEG_DOT;
				}
				*dp++ = seg;
			}
			return true;
		}
		return false;
	}
}
