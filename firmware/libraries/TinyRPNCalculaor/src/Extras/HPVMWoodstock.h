#pragma once

namespace HPVM
{
	// -----------------------------------------------------------------------------
	// HP Calculators Virtual Machine
	// -----------------------------------------------------------------------------

	// clock parameters
	#define HPVM_CLOCK_RATE       (180) // kHz
	#define HPVM_BIT_TIME_WIDTH   (1000 / HPVM_CLOCK_RATE) // uS
	#define HPVM_BITS_PER_CYCLE   (14 * 4)
	#define HPVM_CYCLE_TIME_WIDTH (HPVM_BITS_PER_CYCLE * HPVM_BIT_TIME_WIDTH) // uS
	#define HPVM_CYCLES_PER_SEC   (1000000 / HPVM_CYCLE_TIME_WIDTH)

	// public interface
	void Operation(uint8_t op);
	bool Idling();    // true if idling
	bool Error();     // true if error occured
	bool Cycle();     // true if display updated
	char Display[15]; // display state

	// HP21 key operation
	enum
	{
		// OpCLR  = 0x00, OpEXP  = 0x02, OpLN   = 0x03, OpLOG  = 0x04, OpPOW  = 0x06, OpRCL  = 0x08,
		// OpSTO  = 0x0A, OpROT  = 0x0B, OpSWAP = 0x0C, OpINV  = 0x0E, OpNUM6 = 0x12, OpNUM5 = 0x13,
		// OpNUM4 = 0x14, OpADD  = 0x16, OpNUM3 = 0x1A, OpNUM2 = 0x1B, OpNUM1 = 0x1C, OpMUL  = 0x1E,
		// OpPI   = 0x22, OpDOT  = 0x23, OpNUM0 = 0x24, OpDIV  = 0x26, OpTAN  = 0x28, OpCOS  = 0x2A,
		// OpSIN  = 0x2B, OpARC  = 0x2C, OpSQRT = 0x2E, OpNUM9 = 0x32, OpNUM8 = 0x33, OpNUM7 = 0x34,
		// OpSUB  = 0x36, OpCLX  = 0x38, OpEEX  = 0x3A, OpCHS  = 0x3B, OpPUSH = 0x3E, OpNONE = 0xFF,
	};

	// HP21 firmware ROM (1024 words)
	const uint8_t rom_l[] PROGMEM =
	{
		// TODO
	};

	const uint8_t rom_h[] PROGMEM =
	{
		// TODO
	};

	// cpu defines
	typedef uint8_t digit;
	typedef digit reg[16];
	#define hpvm_iterate_word(a)  for (uint8_t i =  0; i <  14; ++i) { a; }
	#define hpvm_iterate_field(a) for (uint8_t i = ff; i <= fl; ++i) { a; }
	namespace fld { enum { P = 0, M, X, W, WP, MS, XS, S }; }

	// display defines
	#define HPVM_DIGIT 0x00
	#define HPVM_SPACE 0x10
	#define HPVM_DASH  0x11
	#define HPVM_DOT   0x12

	// registers
	reg A;
	reg B;
	reg C; // X
	reg D; // Y
	reg E; // Z
	reg F; // T
	reg M1;
	reg M2;

	// state
	uint16_t pc, stack[2];
	uint8_t  bank, del_rom, del_flg;
	uint8_t  p, f, decimal, carry;
	uint8_t  s[16];

	uint8_t  ff, fl, prev_carry;
	uint8_t  disp_enable, disp_update;
	uint8_t  idling, error;

	// basic math
	digit alu_add(digit x, digit y)
	{
		int8_t res = x + y + carry;
		if (res > 9) { res -= 10; carry = 1; } else carry = 0;
		return digit(res);
	}

	digit alu_sub(digit x, digit y)
	{
		int8_t res = x - y - carry;
		if (res < 0) { res += 10; carry = 1; } else carry = 0;
		return digit(res);
	}

	void reg_clr(reg r)
	{
		hpvm_iterate_field(r[i] = 0);
	}

	void reg_add(reg r, reg x, reg y)
	{
		hpvm_iterate_field(r[i] = alu_add(x[i], y[i]));
	}

	void reg_sub(reg r, reg x, reg y)
	{
		hpvm_iterate_field(r[i] = alu_sub(x[i], y[i]));
	}

	void reg_inc(reg r)
	{
		carry = 1;
		hpvm_iterate_field(r[i] = alu_add(r[i], 0));
	}

	void reg_dec(reg r)
	{
		carry = 1;
		hpvm_iterate_field(r[i] = alu_sub(r[i], 0));
	}

	void reg_shr(reg r)
	{
		for (uint8_t i = ff; i < fl; i++) r[i] = r[i + 1];
		r[fl] = 0;
	}

	void reg_shl(reg r)
	{
		for (int8_t i = fl; i > ff; i--) r[i] = r[i - 1];
		r[ff] = 0;
	}

	void reg_copy(reg x, reg y)
	{
		hpvm_iterate_field(x[i] = y[i]);
	}

	void reg_swap(reg x, reg y)
	{
		digit t;
		hpvm_iterate_field(t = x[i]; x[i] = y[i]; y[i] = t);
	}

	// implementation
	void Operation(uint8_t op)
	{
		idling = error = 0;
		key_pc = op;
		s[0] = 1;
	}

	bool Idling()
	{
		return (idling && s[8]);
	}

	bool Error()
	{
		return (error && Idling());
	}

	bool Cycle()
	{
		// handling state change breakpoints
		if (rom == 0x00)
		{
			if (pc == 0xC5) idling = 1;
			if (pc == 0xBF) error  = 1;
		}

		// fetch ROM
#if defined(HPVM_HP45)
		uint16_t addr_l  = (group << 11 | rom << 8 | pc);
#else
		uint16_t addr_l  = (rom << 8 | pc);
#endif
		uint8_t  addr_h  = (addr_l >> 2);
		uint8_t  shift   = (addr_l & 0x03) << 1;
		uint8_t  fetch_l = pgm_read_byte(rom_l + addr_l);
		uint8_t  fetch_h = pgm_read_byte(rom_h + addr_h) >> shift & 0x03;

		prev_carry = carry;
		carry = 0;
		pc++;

		// operation decode
		uint8_t op_type = (fetch_l & 0x03);
		uint8_t op_code = (fetch_l >> 2 | fetch_h << 6);

		// Type 00: Misc Instructions
		if (op_type == 0x00)
		{
			switch(op_code)
			{
				case 0b00000000: // NO OPERATION
					break;
				case 0b00110100: // KEY -> ROM ADDRESS
					pc = key_pc;
					break;
				case 0b00000111: // P – 1 -> P
					p -= 0x01; p &= 0x0F;
					break;
				case 0b00001010: // DISPLAY TOGGLE
					disp_enable = !disp_enable; disp_update = 1;
					break;
				case 0b00101010: // C EXCHANGE M
					hpvm_iterate_word(digit t = C[i]; C[i] = M[i]; M[i] = t);
					break;
				case 0b01001010: // C -> STACK
					hpvm_iterate_word(F[i] = E[i]; E[i] = D[i]; D[i] = C[i]);
					break;
				case 0b01101010: // STACK -> A
					hpvm_iterate_word(A[i] = D[i]; D[i] = E[i]; E[i] = F[i]);
					break;
				case 0b10001010: // DISPLAY OFF
					if (disp_enable) { disp_enable = 0; disp_update = 1; }
					break;
				case 0b10101010: // M -> C
					hpvm_iterate_word(C[i] = M[i]);
					break;
				case 0b11001010: // DOWN ROTATE
					hpvm_iterate_word(digit t = C[i]; C[i] = D[i]; D[i] = E[i]; E[i] = F[i]; F[i] = t);
					break;
				case 0b11101010: // CLEAR REGISTERS
					hpvm_iterate_word(A[i] = B[i] = C[i] = D[i] = E[i] = F[i] = M[i] = 0);
					break;
				case 0b00001100: // RETURN
					pc = ret_pc;
					break;
				case 0b00001101: // CLEAR STATUS
					for (uint8_t i = 0; i < 12; i++) s[i] = 0;
					break;
#if defined(HPVM_HP45)
				case 0b10001101: // DELAYED GROUP SELECT 0 or 1
				case 0b10101101:
					del_group = ((op_code >> 5) & 0x01);
					break;
#endif
				case 0b00001111: // P + 1 -> P
					p += 0x01; p &= 0x0F;
					break;
				default:
					uint8_t nnnn = op_code >> 4;
					switch(op_code & 0x0F)
					{
						case 0b0001: // 1 -> Sn
							s[nnnn] = 1;
							break;
						case 0b0011: // n -> P
							p = nnnn;
							break;
						case 0b0100: // ROM SELECT n
							rom = (nnnn >> 1);
#if defined(HPVM_HP45)
							group = del_group;
							del_rom = rom;
#endif
							break;
						case 0b0101: // IF Sn = 0
							carry = s[nnnn];
							break;
						case 0b0110: // n -> C
							C[p] = nnnn; p -= 0x01; p &= 0x0F;
							break;
						case 0b1001: // 0 -> Sn
							s[nnnn] = 0;
							break;
						case 0b1011: // IF p # n
							carry = (p == nnnn);
							break;
#if defined(HPVM_HP45)
						case 0b1101: // DELAYED ROM SELECT n
							del_rom = (nnnn >> 1);
							break;
#endif
					}
			}
		}

		// Type 01: Branching Instructions
		else if (op_type == 0x01)
		{
			// JSB addr
			ret_pc = pc;
			pc = op_code;
#if defined(HPVM_HP45)
			rom = del_rom;
			group = del_group;
#endif
		}

		// Type 10: Arithmetic Instructions
		else if (op_type == 0x02)
		{
			// get register boundaries
			switch (op_code & 0x07)
			{
				case fld::P : ff = fl = p; break;
				case fld::M : ff = 3; fl = 12; break;
				case fld::X : ff = 0; fl = 2;  break;
				case fld::W : ff = 0; fl = 13; break;
				case fld::WP: ff = 0; fl = p;  break;
				case fld::MS: ff = 3; fl = 13; break;
				case fld::XS: ff = fl = 2;  break;
				case fld::S : ff = fl = 13; break;
			}

			// process opcode
			carry = 0;
			switch(op_code >> 3)
			{
				case 0b00000: // IF B[f] = 0
					hpvm_iterate_field(carry |= (B[i] != 0));
					break;
				case 0b00001: // 0 -> B[f]
					reg_clr(B);
					break;
				case 0b00010: // IF A >= C[f]
					hpvm_iterate_field(alu_sub(A[i], C[i]));
					break;
				case 0b00011: // IF C[f] >= 1
					carry = 1; hpvm_iterate_field(carry &= (C[i] == 0));
					break;
				case 0b00100: // B -> C[f]
					reg_copy(C, B);
					break;
				case 0b00101: // 0 – C -> C[f]
					hpvm_iterate_field(C[i] = alu_sub(0, C[i]));
					break;
				case 0b00110: // 0 -> C[f]
					reg_clr(C);
					break;
				case 0b00111: // 0 – C – 1 -> C[f]
					carry = 1; hpvm_iterate_field(C[i] = alu_sub(0, C[i]));
					break;
				case 0b01000: // SHIFT LEFT A[f]
					reg_shl(A);
					break;
				case 0b01001: // A -> B[f]
					reg_copy(B, A);
					break;
				case 0b01010: // A – C -> C[f]
					reg_sub(C, A, C);
					break;
				case 0b01011: // C – 1 -> C[f]
					reg_dec(C);
					break;
				case 0b01100: // C -> A[f]
					reg_copy(A, C);
					break;
				case 0b01101: // IF C[f] = 0
					hpvm_iterate_field(carry |= (C[i] != 0));
					break;
				case 0b01110: // A + C -> C[f]
					reg_add(C, A, C);
					break;
				case 0b01111: // C + 1 -> C[f]
					reg_inc(C);
					break;
				case 0b10000: // IF A >= B[f]
					hpvm_iterate_field(alu_sub(A[i], B[i]));
					break;
				case 0b10001: // B EXCHANGE C[f]
					reg_swap(B, C);
					break;
				case 0b10010: // SHIFT RIGHT C[f]
					reg_shr(C);
					break;
				case 0b10011: // IF A[f] >= 1
					carry = 1; hpvm_iterate_field(carry &= (A[i] == 0));
					break;
				case 0b10100: // SHIFT RIGHT B[f]
					reg_shr(B);
					break;
				case 0b10101: // C + C -> C[f]
					reg_add(C, C, C);
					break;
				case 0b10110: // SHIFT RIGHT A[f]
					reg_shr(A);
					break;
				case 0b10111: // 0 -> A[f]
					reg_clr(A);
					break;
				case 0b11000: // A – B -> A[f]
					reg_sub(A, A, B);
					break;
				case 0b11001: // A EXCHANGE B[f]
					reg_swap(A, B);
					break;
				case 0b11010: // A – C -> A[f]
					reg_sub(A, A, C);
					break;
				case 0b11011: // A – 1 -> A[f]
					reg_dec(A);
					break;
				case 0b11100: // A + B -> A[f]
					reg_add(A, A, B);
					break;
				case 0b11101: // A EXCHANGE C[f]
					reg_swap(A, C);
					break;
				case 0b11110: // A + C -> A[f]
					reg_add(A, A, C);
					break;
				case 0b11111: // A + 1 -> A[f]
					reg_inc(A);
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
#if defined(HPVM_HP45)
				rom = del_rom;
				group = del_group;
#endif
			}
		}

		// display update
		if (disp_update)
		{
			disp_update = 0;
			for (int8_t d = 0, i = 13; i >= 0; --i)
			{
				if (disp_enable)
				{
					if (B[i] == 9)
						Display[d++] = HPVM_SPACE;
					else if (i == 2 || i == 13)
						Display[d++] = (A[i] == 9 ? HPVM_DASH : HPVM_SPACE);
					else
						Display[d++] = HPVM_DIGIT + A[i];
					if (B[i] == 2)
						Display[d++] = HPVM_DOT;
				}
				else
					Display[d++] = HPVM_SPACE;
			}
			return true;
		}
		return false;
	}
}
