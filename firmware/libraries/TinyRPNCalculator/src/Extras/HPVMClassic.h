#pragma once

// -----------------------------------------------------------------------------
// Classic Virtual Machine. Include model data before this header.
// -----------------------------------------------------------------------------

namespace HPVM
{
	// clock parameters
	constexpr u08 CLOCK_RATE_KHZ      = 200;
	constexpr u08 BIT_TIME_WIDTH_US   = 1000 / CLOCK_RATE_KHZ;
	constexpr u08 BITS_PER_CYCLE      = 14 * 4;
	constexpr u16 CYCLE_TIME_WIDTH_US = BITS_PER_CYCLE * BIT_TIME_WIDTH_US;
	constexpr u16 CYCLES_PER_SEC      = 1000000 / CYCLE_TIME_WIDTH_US;

	// public interface
	void Operation(u08 op);
	void OpAndWait(u08 op);
	bool Idling(); // true if idling
	bool Cycle();  // true if display updated

	// display render data
	enum { DIGIT = 0x00, BLANK = 0x10, DASH = 0x11, DOT = 0x12 };
	char Display[15];

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

	// registers
	reg A;
	reg B;
	reg C; // X
	reg D; // Y
	reg E; // Z
	reg F; // T
	reg M;

	// state
	u08 rom, pc, ret_pc, key_pc;
	u08 carry, p, ff, fl, s[12];
	u08 disp_enable, bp_idle;

	// basic math
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
			if (res > 9) { res -= 10; carry = 1; } else carry = 0;
		}
		return bcd(res);
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

	void reg_move(reg x, reg y, const u08 f, const u08 l, const u08 swap)
	{
		for (u08 i = f; i <= l; ++i)
		{
			bcd t = x[i]; x[i] = y[i];
			if (swap) y[i] = t;
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

	// one-shot key event abstraction, unlike the original
	// C&T, S0 is not reasserted while a key is held
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

	bool Idling()
	{
		return (bp_idle && s[STATUS_BIT_IDLE]);
	}

	bool Cycle()
	{
		// handle idle state breakpoint
		if (rom == BP_IDLE.msb && pc == BP_IDLE.lsb) bp_idle = 1;

		// fetch and decode ROM
		u16 addr_h  = (u16(rom) << 8 | pc);
		u08 addr_l  = (addr_h >> 2);
		u08 shift   = (addr_h & 0x03) << 1;
		u08 op_type = (MEM::ProgRead(rom_l + addr_l) >> shift) & 0x03;
		u08 op_code =  MEM::ProgRead(rom_h + addr_h);

		u08 disp_update = 0;
		u08 prev_carry = carry;
		carry = 0;
		pc++;

		// Type 00: Misc Instructions
		if (op_type == 0x00)
		{
			switch(op_code)
			{
				case 0x00: // NO OPERATION
					break;
				case 0x34: // KEY -> ROM ADDRESS
					pc = key_pc;
					break;
				case 0x07: // P – 1 -> P
					p -= 0x01;
					p &= 0x0F;
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
					reg_move(D, E, 0, 13, COPY);
					reg_move(E, F, 0, 13, COPY);
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
					reg_move(C, D, 0, 13, SWAP);
					reg_move(D, E, 0, 13, SWAP);
					reg_move(E, F, 0, 13, SWAP);
					break;
				case 0xEA: // CLEAR REGISTERS
					for (u08 i = 0; i <= 13; ++i)
						A[i] = B[i] =
						C[i] = D[i] =
						E[i] = F[i] =
						M[i] = 0;
					break;
				case 0x0C: // RETURN
					pc = ret_pc;
					break;
				case 0x0D: // CLEAR STATUS
					for (u08 i = 0; i < sizeof(s); ++i) s[i] = 0;
					break;
				case 0x0F: // P + 1 -> P
					p += 0x01; 
					p &= 0x0F;
					break;
				default:
					u08 nnnn = op_code >> 4;
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
							if (p < 14) C[p] = nnnn;
							p -= 0x01;
							p &= 0x0F;
							break;
						case 0x09: // 0 -> Sn
							s[nnnn] = 0;
							break;
						case 0x0B: // IF p # n
							carry = (p == nnnn);
							break;
					}
			}
		}

		// Type 01: Branching Instructions
		else if (op_type == 0x01)
		{
			// JSB addr
			ret_pc = pc;
			pc = op_code;
		}

		// Type 10: Arithmetic Instructions
		else if (op_type == 0x02)
		{
			// get register boundaries
			fl = MEM::DataRead(field_bounds + (op_code & 0x07));
			ff = fl & 0x0F; fl >>= 4;
			if (ff == 0x0F) ff = p;
			if (fl == 0x0F) fl = (p < 14 ? p : 13);

			// process opcode
			switch(op_code >> 3)
			{
				case 0x00: // IF B[f] = 0
					carry = reg_nonzero(B);
					break;
				case 0x01: // 0 -> B[f]
					reg_clr(B);
					break;
				case 0x02: // IF A >= C[f]
					for (u08 i = ff; i <= fl; ++i) alu(A[i], C[i], SUB);
					break;
				case 0x03: // IF C[f] >= 1
					carry = !reg_nonzero(C);
					break;
				case 0x04: // B -> C[f]
					reg_move(C, B, COPY);
					break;
				case 0x05: // 0 – C -> C[f]
					for (u08 i = ff; i <= fl; ++i) C[i] = alu(0, C[i], SUB);
					break;
				case 0x06: // 0 -> C[f]
					reg_clr(C);
					break;
				case 0x07: // 0 – C – 1 -> C[f]
					carry = 1; 
					for (u08 i = ff; i <= fl; ++i) C[i] = alu(0, C[i], SUB);
					break;
				case 0x08: // SHIFT LEFT A[f]
					reg_shl(A);
					break;
				case 0x09: // A -> B[f]
					reg_move(B, A, COPY);
					break;
				case 0x0A: // A – C -> C[f]
					reg_math(C, A, C, SUB);
					break;
				case 0x0B: // C – 1 -> C[f]
					reg_math(C, SUB);
					break;
				case 0x0C: // C -> A[f]
					reg_move(A, C, COPY);
					break;
				case 0x0D: // IF C[f] = 0
					carry = reg_nonzero(C);
					break;
				case 0x0E: // A + C -> C[f]
					reg_math(C, A, C, ADD);
					break;
				case 0x0F: // C + 1 -> C[f]
					reg_math(C, ADD);
					break;
				case 0x10: // IF A >= B[f]
					for (u08 i = ff; i <= fl; ++i) alu(A[i], B[i], SUB);
					break;
				case 0x11: // B EXCHANGE C[f]
					reg_move(B, C, SWAP);
					break;
				case 0x12: // SHIFT RIGHT C[f]
					reg_shr(C);
					break;
				case 0x13: // IF A[f] >= 1
					carry = !reg_nonzero(A);
					break;
				case 0x14: // SHIFT RIGHT B[f]
					reg_shr(B);
					break;
				case 0x15: // C + C -> C[f]
					reg_math(C, C, C, ADD);
					break;
				case 0x16: // SHIFT RIGHT A[f]
					reg_shr(A);
					break;
				case 0x17: // 0 -> A[f]
					reg_clr(A);
					break;
				case 0x18: // A – B -> A[f]
					reg_math(A, A, B, SUB);
					break;
				case 0x19: // A EXCHANGE B[f]
					reg_move(A, B, SWAP);
					break;
				case 0x1A: // A – C -> A[f]
					reg_math(A, A, C, SUB);
					break;
				case 0x1B: // A – 1 -> A[f]
					reg_math(A, SUB);
					break;
				case 0x1C: // A + B -> A[f]
					reg_math(A, A, B, ADD);
					break;
				case 0x1D: // A EXCHANGE C[f]
					reg_move(A, C, SWAP);
					break;
				case 0x1E: // A + C -> A[f]
					reg_math(A, A, C, ADD);
					break;
				case 0x1F: // A + 1 -> A[f]
					reg_math(A, ADD);
					break;
			}
		}

		// Type 11: Branching Instructions
		else // op_type == 0x03
		{
			// (THEN) GO TO addr
			if (!prev_carry)
			{
				pc = op_code;
			}
		}

		// display update
		if (disp_update)
		{
			Display[14] = BLANK;
			for (s08 d = 0, i = 13; i >= 0; --i)
			{
				if (disp_enable)
				{
					if (B[i] == 9)
						Display[d++] = BLANK;
					else if (i == 2 || i == 13)
						Display[d++] = (A[i] == 9 ? DASH : BLANK);
					else
						Display[d++] = DIGIT + A[i];
					if (B[i] == 2)
						Display[d++] = DOT;
				}
				else
					Display[d++] = BLANK;
			}
			return true;
		}
		return false;
	}
}
