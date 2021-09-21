#pragma once

// -----------------------------------------------------------------------------
// HP35 Calculator Virtual Machine
// -----------------------------------------------------------------------------

// clock parameters
#define HP35_CLOCK_RATE       (800 / 4) // kHz
#define HP35_BIT_TIME_WIDTH   (1000 / HP35_CLOCK_RATE) // uS
#define HP35_BITS_PER_CYCLE   (14 * 4)
#define HP35_CYCLE_TIME_WIDTH (HP35_BITS_PER_CYCLE * HP35_BIT_TIME_WIDTH) // uS
#define HP35_CYCLES_PER_SEC   (1000000 / HP35_CYCLE_TIME_WIDTH)

// public interface
#define HP35_NONE 0xFF // no key
#define HP35_NUM0 0x24 // 
#define HP35_NUM1 0x1C // 
#define HP35_NUM2 0x1B // 
#define HP35_NUM3 0x1A // 
#define HP35_NUM4 0x14 // 
#define HP35_NUM5 0x13 // 
#define HP35_NUM6 0x12 // 
#define HP35_NUM7 0x34 // 
#define HP35_NUM8 0x33 // 
#define HP35_NUM9 0x32 // 
#define HP35_PUSH 0x3E // ENTER
#define HP35_INV  0x0E // 1/x
#define HP35_SIN  0x2B // 
#define HP35_COS  0x2A // 
#define HP35_TAN  0x28 // 
#define HP35_PI   0x22 // 
#define HP35_CHS  0x3B // 
#define HP35_EEX  0x3A // 
#define HP35_SWAP 0x0C // X<->Y
#define HP35_ROT  0x0B // Rv
#define HP35_STO  0x0A // 
#define HP35_RCL  0x08 // 
#define HP35_POW  0x06 // X^Y
#define HP35_LOG  0x04 // LOG
#define HP35_LN   0x03 // LN
#define HP35_EXP  0x02 // e^x
#define HP35_SQRT 0x2E // SQRT
#define HP35_ARC  0x2C // 
#define HP35_ADD  0x16 // +
#define HP35_SUB  0x36 // -
#define HP35_DIV  0x26 // /
#define HP35_MUL  0x1E // *
#define HP35_DOT  0x23 // .
#define HP35_CLR  0x00 // 
#define HP35_CLX  0x38 // 

uint8_t HP35_Display[15]; // output
uint8_t HP35_Error;       // output

void HP35_Operation(uint8_t key);
bool HP35_Cycle();

// firmware ROM (768 words)
const uint8_t hp35_rom_l[] PROGMEM =
{
	0xDD, 0xFF, 0x24, 0x17, 0x44, 0x44, 0x84, 0x10, 0xD1, 0xFB, 0x5F, 0xC3, 0xA8, 0x67, 0xEE, 0xE2,
	0x2E, 0x90, 0xEA, 0xEA, 0xEA, 0x6B, 0x69, 0xA8, 0xA8, 0xFF, 0xEA, 0xEA, 0xEA, 0x30, 0xCC, 0xAA,
	0xA8, 0x43, 0xD3, 0xCC, 0x30, 0x00, 0x83, 0x44, 0x44, 0xBB, 0x44, 0x9F, 0x84, 0x0B, 0x2E, 0x90,
	0x28, 0x6F, 0xEA, 0xEA, 0xEA, 0x4B, 0x67, 0xA8, 0x71, 0x77, 0xCB, 0xCE, 0xC4, 0xDB, 0x28, 0x34,
	0xCE, 0x75, 0x2E, 0xFA, 0x16, 0x6A, 0x83, 0xBA, 0x9B, 0x36, 0x4C, 0x9B, 0x1C, 0xEA, 0x02, 0x33,
	0xC4, 0xD6, 0xA6, 0x14, 0x1F, 0x7D, 0x77, 0xD2, 0x72, 0xDA, 0x8A, 0x77, 0xCE, 0x34, 0x8E, 0x0C,
	0x2A, 0x8A, 0xBA, 0xA3, 0xAA, 0x7A, 0x5F, 0x4C, 0xAA, 0x14, 0x0B, 0x2A, 0x2A, 0xDD, 0x0A, 0xCE,
	0x2C, 0x27, 0xB2, 0xEB, 0xD1, 0x90, 0x14, 0xDB, 0xB2, 0xFA, 0x8E, 0xBA, 0xFF, 0xDA, 0xAA, 0x4C,
	0x16, 0x6A, 0x7E, 0x3B, 0x76, 0x03, 0xCA, 0xDD, 0xD6, 0x9E, 0x2C, 0x4F, 0x8E, 0xEE, 0x4C, 0x12,
	0x3C, 0xA2, 0x3F, 0xAE, 0xEC, 0xE7, 0xCA, 0x84, 0xEB, 0xFE, 0xA8, 0x2E, 0xFA, 0xFA, 0xFA, 0xFA,
	0x4A, 0x8F, 0xAE, 0xA6, 0xA6, 0x9F, 0xAE, 0x26, 0x4A, 0xFB, 0x8E, 0xEA, 0x0E, 0xFB, 0xA3, 0xF6,
	0xD4, 0xD3, 0x7E, 0xFE, 0xD4, 0xDF, 0x28, 0xC4, 0xCE, 0x6E, 0xBE, 0xFE, 0x2E, 0x30, 0x90, 0x71,
	0x44, 0x77, 0xCE, 0x9E, 0x24, 0x3F, 0xFA, 0x04, 0x54, 0x37, 0xEA, 0x1B, 0x28, 0x14, 0x1F, 0x24,
	0x1C, 0x2C, 0x43, 0x28, 0x14, 0x33, 0x0E, 0x64, 0xD0, 0x28, 0xAE, 0x75, 0xC4, 0xDD, 0xBD, 0x2B,
	0xD6, 0x1C, 0xAC, 0x17, 0x0C, 0xEE, 0xF6, 0xE2, 0xE2, 0x8C, 0x3C, 0x62, 0xBF, 0x02, 0xAB, 0xE2,
	0x2E, 0x30, 0x04, 0xD4, 0x73, 0xBF, 0xFE, 0xA4, 0x0F, 0x94, 0xF3, 0x1C, 0x92, 0xE9, 0xA8, 0x6F,
	0xCF, 0x2E, 0xA1, 0xA8, 0xA1, 0xA8, 0x54, 0x27, 0xAE, 0x54, 0x4B, 0xDE, 0x99, 0x28, 0x95, 0x61,
	0x95, 0xA8, 0x99, 0x94, 0x6B, 0xEE, 0xE2, 0x26, 0xA6, 0x6A, 0x92, 0xBA, 0x67, 0xD2, 0xEA, 0x77,
	0xCE, 0x8E, 0x28, 0x2E, 0x07, 0x2E, 0x0C, 0x7B, 0x28, 0xAE, 0xA2, 0xB7, 0xAE, 0x8E, 0x8A, 0x2F,
	0x8E, 0x54, 0x97, 0x94, 0xB7, 0x54, 0x57, 0xFE, 0xBE, 0x37, 0x92, 0x7E, 0xEB, 0xFE, 0x32, 0xD2,
	0x2E, 0x2E, 0x52, 0xEF, 0xA8, 0xCE, 0xB2, 0x2E, 0x12, 0x28, 0xFE, 0xFE, 0x8F, 0xCE, 0x2A, 0xD6,
	0xC9, 0x62, 0xA8, 0xAE, 0x0C, 0x91, 0x8C, 0x6D, 0x0C, 0x6D, 0x8C, 0x18, 0x8C, 0x6D, 0x39, 0x6D,
	0x31, 0x0E, 0x6D, 0x8E, 0x2D, 0x31, 0xAE, 0x99, 0x54, 0xB3, 0xFE, 0x61, 0x64, 0xCE, 0x62, 0xEA,
	0x54, 0x97, 0x99, 0x31, 0xAE, 0x95, 0x31, 0xAE, 0xAE, 0x55, 0xAE, 0xAD, 0x31, 0x8C, 0x71, 0x39,
	0x0C, 0x75, 0x8C, 0x18, 0x8C, 0x71, 0x0C, 0x71, 0x71, 0x2E, 0x4E, 0x4C, 0x58, 0xEF, 0x8C, 0x18,
	0x98, 0x58, 0x98, 0x18, 0x58, 0x54, 0x6B, 0x30, 0xEE, 0xE2, 0x10, 0x10, 0x0E, 0x96, 0x2E, 0x87,
	0xFE, 0x0E, 0x83, 0x8E, 0x10, 0x10, 0x4A, 0x10, 0xE2, 0x4E, 0xA3, 0xCE, 0x0E, 0x1C, 0x52, 0x2C,
	0xA7, 0xB7, 0xE2, 0x16, 0xCB, 0x96, 0x16, 0x1C, 0x2C, 0xCF, 0xB7, 0x1C, 0x96, 0x6F, 0x10, 0x7A,
	0x7A, 0xEA, 0x5E, 0x7E, 0x1B, 0x10, 0x06, 0x2B, 0xFE, 0x2E, 0x0E, 0x10, 0xCE, 0xCC, 0xD8, 0x18,
	0x58, 0xD8, 0x58, 0x18, 0x58, 0x98, 0xD8, 0x58, 0x0C, 0x30, 0x10, 0x8A, 0x7B, 0x62, 0xFE, 0x2C,
	0xEF, 0xAA, 0xEA, 0x62, 0x9B, 0xCE, 0x4E, 0x2A, 0xCA, 0x0C, 0xBB, 0x10, 0x92, 0x92, 0x7E, 0xB3,
	0xD2, 0x12, 0x32, 0x8E, 0x7E, 0xBB, 0xB2, 0xA8, 0x1E, 0x07, 0x0E, 0xB2, 0x28, 0x92, 0x7E, 0x3E,
	0x10, 0x3E, 0xFE, 0x56, 0x12, 0x4B, 0xA8, 0x99, 0x8E, 0x14, 0x0B, 0xEE, 0x46, 0x03, 0xCE, 0x7E,
	0x03, 0xFE, 0x2E, 0x59, 0x62, 0x47, 0x32, 0x9E, 0x07, 0xCC, 0xB5, 0x0C, 0x75, 0x4C, 0x71, 0xF9,
	0x8C, 0x71, 0xF5, 0xCC, 0x71, 0x7D, 0x71, 0xE5, 0x71, 0xD9, 0xAE, 0x4E, 0x1A, 0xBF, 0x4E, 0x2E,
	0x1C, 0x0E, 0x6C, 0xC3, 0xAE, 0xBE, 0xE3, 0xE6, 0xEA, 0xCC, 0x15, 0x54, 0x1B, 0x54, 0x53, 0xD9,
	0x9D, 0x53, 0xD9, 0xB1, 0xE5, 0xCC, 0x6D, 0x7D, 0x8C, 0x6D, 0xF5, 0x4C, 0x6D, 0xF9, 0x0C, 0x6D,
	0x6D, 0x6D, 0x8C, 0xF2, 0x4C, 0x2E, 0xAE, 0x98, 0x3B, 0x94, 0x7B, 0xEA, 0x7A, 0x0B, 0x16, 0x67,
	0x96, 0x0E, 0x6A, 0x73, 0xCE, 0xD2, 0xAA, 0xBE, 0xB3, 0x2E, 0x0E, 0xEE, 0xCE, 0x2E, 0xCE, 0x66,
	0x94, 0xDB, 0x18, 0xE6, 0xE7, 0x98, 0x6C, 0xD7, 0x4E, 0x4E, 0x94, 0x53, 0x30, 0xCC, 0xD8, 0xD8,
	0x18, 0x18, 0x58, 0x18, 0x58, 0xAB, 0x59, 0xE2, 0x2E, 0x7E, 0x1B, 0xD2, 0xAE, 0x16, 0xAE, 0x7E,
	0x23, 0x2E, 0xE2, 0x31, 0x90, 0xD2, 0x7E, 0x57, 0xFE, 0x8E, 0x30, 0x90, 0xCE, 0x2E, 0x83, 0x8E,
	0x7E, 0x7F, 0xAE, 0x16, 0xAE, 0xB7, 0xCC, 0xCA, 0x5E, 0xAF, 0xBE, 0x26, 0xEE, 0x2C, 0x17, 0x66,
	0xDB, 0x54, 0x03, 0x92, 0x66, 0xFA, 0x32, 0xA6, 0x90, 0x24, 0x98, 0x58, 0xD8, 0x58, 0x18, 0xD8,
	0x58, 0x9B, 0xE6, 0x93, 0x8E, 0x62, 0x13, 0xCE, 0x3C, 0x6C, 0x17, 0xEA, 0xFE, 0x0C, 0x2E, 0x62,
	0x5B, 0x0E, 0x6A, 0x6E, 0x3F, 0xCE, 0x2A, 0x8E, 0x7E, 0x1F, 0xA6, 0x8E, 0x2E, 0x0C, 0xEB, 0x4C,
	0xD8, 0x58, 0x18, 0x58, 0xD8, 0x58, 0x18, 0x18, 0x58, 0x58, 0xD8, 0x77, 0xAE, 0x2E, 0x86, 0xBA,
	0x7B, 0xFA, 0xCE, 0xEA, 0xCB, 0x9F, 0xCE, 0x0C, 0x98, 0xD8, 0x18, 0x98, 0x58, 0x07, 0x4C, 0xFB
};

const uint8_t hp35_rom_h[] PROGMEM =
{
	0x28, 0x49, 0x0E, 0xED, 0xF0, 0x23, 0xF2, 0x03, 0x11, 0x40, 0x20, 0x0E, 0xFF, 0xE3, 0xBD, 0x14,
	0x9E, 0x7D, 0xFF, 0x44, 0x7A, 0x36, 0x71, 0xD0, 0x55, 0x54, 0xD7, 0x9C, 0x7B, 0xD2, 0x53, 0xC1,
	0x65, 0xCA, 0xB6, 0x44, 0xE4, 0x4C, 0x11, 0x5F, 0xF8, 0xB9, 0xE8, 0xA8, 0x38, 0x59, 0x44, 0x42,
	0x6D, 0x9E, 0xDD, 0x30, 0xBC, 0x5E, 0x74, 0xAD, 0x42, 0xEB, 0xC3, 0xF3, 0x93, 0x84, 0x37, 0xE6,
	0x4F, 0x24, 0x07, 0xA6, 0xA4, 0x7B, 0x67, 0x18, 0x9A, 0xD9, 0x1D, 0xC4, 0x81, 0x05, 0xE3, 0x74,
	0x37, 0xF9, 0xF5, 0x80, 0xD6, 0x99, 0x8A, 0xAA, 0x27, 0xAF, 0x86, 0x50, 0xE8, 0xBA, 0xEA, 0xAB,
	0x8A, 0x99, 0xEE, 0x9D, 0x45, 0x32, 0x4E, 0xA9, 0xED, 0x55, 0xED, 0x21, 0xD2, 0x1E, 0x08, 0x4F,
	0xB9, 0xE7, 0x7C, 0x98, 0xA1, 0x44, 0xC3, 0x17, 0x2E, 0xAB, 0x6C, 0xDA, 0x2D, 0x7F, 0xD0, 0x99,
	0xBC, 0x91, 0x99, 0x63, 0x94, 0xF3, 0x94, 0xEA, 0x9A, 0xAE, 0x7E, 0xD0, 0x04, 0x07, 0xB9, 0xE4,
	0xFA, 0xEA, 0x9A, 0xAE, 0x9A, 0x7B, 0xD2, 0x7E, 0x57, 0x72, 0x3D, 0x4A, 0x54, 0x45, 0x8A, 0x04,
	0x18, 0xEE, 0xA5, 0xF7, 0xFE, 0xB8, 0x0E, 0xEA, 0x79, 0x47, 0xC9, 0x3E, 0x02, 0xE7, 0x98, 0x50,
	0x7C, 0xB7, 0x7C, 0x8E, 0x97, 0xD3, 0x7E, 0x9C, 0x00, 0x29, 0xC5, 0x97, 0x65, 0xC7, 0x00, 0x59
};

// defines
typedef uint8_t hp35_nibble;
typedef hp35_nibble * hp35_register;
#define hp35_iterate_word(a) for (uint8_t i = 0; i < 14; ++i) { a; }
#define hp35_iterate_field(a) for (uint8_t i = hp35_ff; i <= hp35_fl; ++i) { a; }

// registers
hp35_nibble hp35_A[16]; // A register
hp35_nibble hp35_B[16]; // B register
hp35_nibble hp35_C[16]; // C register (X)
hp35_nibble hp35_D[16]; // D register (Y)
hp35_nibble hp35_E[16]; // E register (Z)
hp35_nibble hp35_F[16]; // F register (T)
hp35_nibble hp35_M[16]; // M Scratchpad

// flags
uint8_t hp35_s[12];                         // Status
uint8_t hp35_p, hp35_pc, hp35_ret;          // Pointer
uint8_t hp35_rom_offset;                    // ROM offset
uint8_t hp35_ff, hp35_fl;                   // Register loop boundaries
uint8_t hp35_carry, hp35_carry_alu;         // Carry bits
uint8_t hp35_key_in, hp35_key_pc;           // Key variables
uint8_t hp35_disp_enable, hp35_disp_update; // Display control

// basic math
hp35_nibble hp35_nib_add(hp35_nibble x, hp35_nibble y)
{
	int8_t res = x + y + hp35_carry;
	if (res > 9)
	{
		res -= 10;
		hp35_carry = 1;
	}
	else
		hp35_carry = 0;
	return hp35_nibble(res);
}

hp35_nibble hp35_nib_sub(hp35_nibble x, hp35_nibble y)
{
	int8_t res = x - y - hp35_carry;
	if (res < 0)
	{
		res += 10;
		hp35_carry = 1;
	}
	else
		hp35_carry = 0;
	return hp35_nibble(res);
}

void hp35_reg_clr(hp35_register r)
{
	hp35_iterate_field(r[i] = 0);
}

void hp35_reg_add(hp35_register r, hp35_register x, hp35_register y)
{
	hp35_iterate_field(r[i] = hp35_nib_add(x[i], y[i]));
}

void hp35_reg_sub(hp35_register r, hp35_register x, hp35_register y)
{
	hp35_iterate_field(r[i] = hp35_nib_sub(x[i], y[i]));
}

void hp35_reg_inc(hp35_register r)
{
	hp35_carry = 1;
	hp35_iterate_field(r[i] = hp35_nib_add(r[i], 0));
}

void hp35_reg_dec(hp35_register r)
{
	hp35_carry = 1;
	hp35_iterate_field(r[i] = hp35_nib_sub(r[i], 0));
}

void hp35_reg_shr(hp35_register r)
{
	for (uint8_t i = hp35_ff; i < hp35_fl; i++) r[i] = r[i + 1];
	r[hp35_fl] = 0;
}

void hp35_reg_shl(hp35_register r)
{
	for (int8_t i = hp35_fl; i > hp35_ff; i--) r[i] = r[i - 1];
	r[hp35_ff] = 0;
}

void hp35_reg_copy(hp35_register x, hp35_register y)
{
	hp35_iterate_field(x[i] = y[i]);
}

void hp35_reg_swap(hp35_register x, hp35_register y)
{
	hp35_nibble t;
	hp35_iterate_field(t = x[i]; x[i] = y[i]; y[i] = t);
}

// implementation
void HP35_Operation(uint8_t key)
{
	hp35_key_in = key;
}

bool HP35_Cycle()
{
	// error handling
	if ((hp35_pc == 0xBF) & (hp35_rom_offset == 0x00))
	{
		HP35_Error = true;
	}

	// process received key
	if (hp35_key_in != HP35_NONE)
	{
		HP35_Error  = false;
		hp35_key_pc = hp35_key_in;
		hp35_key_in = HP35_NONE;
		hp35_s[0] = 1;
	}

	// fetch ROM
	uint16_t addr_l  = (hp35_rom_offset << 8 | hp35_pc);
	uint8_t  addr_h  = (addr_l >> 2);
	uint8_t  shift   = (addr_l & 0x03) << 1;
	uint8_t  fetch_l = pgm_read_byte(hp35_rom_l + addr_l);
	uint8_t  fetch_h = pgm_read_byte(hp35_rom_h + addr_h) >> shift & 0x03;

	hp35_carry_alu = hp35_carry;
	hp35_carry = 0;
	hp35_pc++;

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
				hp35_pc = hp35_key_pc; hp35_s[0] = 0;
				break;
			case 0b00000111: // P – 1 -> P
				hp35_p -= 0x01; hp35_p &= 0x0F;
				break;
			case 0b00001010: // DISPLAY TOGGLE
				hp35_disp_enable = !hp35_disp_enable;
				hp35_disp_update = 1;
				break;
			case 0b00101010: // C EXCHANGE M
				hp35_iterate_word(
					hp35_nibble t = hp35_C[i];
					hp35_C[i] = hp35_M[i];
					hp35_M[i] = t);
				break;
			case 0b01001010: // C -> STACK
				hp35_iterate_word(
					hp35_F[i] = hp35_E[i];
					hp35_E[i] = hp35_D[i];
					hp35_D[i] = hp35_C[i]);
				break;
			case 0b01101010: // STACK -> A
				hp35_iterate_word(
					hp35_A[i] = hp35_D[i];
					hp35_D[i] = hp35_E[i];
					hp35_E[i] = hp35_F[i]);
				break;
			case 0b10001010: // DISPLAY OFF
				if (hp35_disp_enable) 
				{
					hp35_disp_enable = 0;
					hp35_disp_update = 1; 
				}
				break;
			case 0b10101010: // M -> C
				hp35_iterate_word(hp35_C[i] = hp35_M[i]);
				break;
			case 0b11001010: // DOWN ROTATE
				hp35_iterate_word(
					hp35_nibble t = hp35_C[i];
					hp35_C[i] = hp35_D[i];
					hp35_D[i] = hp35_E[i];
					hp35_E[i] = hp35_F[i];
					hp35_F[i] = t);
				break;
			case 0b11101010: // CLEAR REGISTERS
				hp35_iterate_word(
					hp35_A[i] =
					hp35_B[i] =
					hp35_C[i] =
					hp35_D[i] =
					hp35_E[i] =
					hp35_F[i] =
					hp35_M[i] = 0);
				break;
			case 0b00001100: // RETURN
				hp35_pc = hp35_ret;
				break;
			case 0b00001101: // CLEAR STATUS
				for (uint8_t i = 0; i < 12; i++) hp35_s[i] = 0;
				break;
			case 0b00001111: // P + 1 -> P
				hp35_p += 0x01; hp35_p &= 0x0F;
				break;
			default:
				uint8_t nnnn = op_code >> 4;
				switch(op_code & 0x0F)
				{
					case 0b0001: // 1 -> Sn
						hp35_s[nnnn] = 1;
						break;
					case 0b0011: // n -> P
						hp35_p = nnnn;
						break;
					case 0b0100: // ROM SELECT n
						hp35_rom_offset = (nnnn >> 1);
						break;
					case 0b0101: // IF Sn = 0
						hp35_carry = hp35_s[nnnn];
						break;
					case 0b0110: // n -> C
						hp35_C[hp35_p] = nnnn;
						hp35_p -= 0x01;
						hp35_p &= 0x0F;
						break;
					case 0b1001: // 0 -> Sn
						hp35_s[nnnn] = 0;
						break;
					case 0b1011: // IF p # n
						hp35_carry = (hp35_p == nnnn);
						break;
				}
		}
	}

	// Type 01: Branching Instructions
	else if (op_type == 0x01)
	{
		// JSB addr
		hp35_ret = hp35_pc;
		hp35_pc  = op_code;
	}

	// Type 10: Arithmetic Instructions
	else if (op_type == 0x02)
	{
		// Get register boundaries first/last
		switch (op_code & 0x07)
		{
		case 0: hp35_ff = hp35_fl = hp35_p; break;
		case 1: hp35_ff = 3; hp35_fl = 12; break;
		case 2:	hp35_ff = 0; hp35_fl = 2; break;
		case 3: hp35_ff = 0; hp35_fl = 13; break;
		case 4: hp35_ff = 0; hp35_fl = hp35_p; break;
		case 5: hp35_ff = 3; hp35_fl = 13; break;
		case 6: hp35_ff = hp35_fl = 2; break;
		case 7: hp35_ff = hp35_fl = 13; break;
		}

		// Process opcode
		hp35_carry = 0;
		switch(op_code >> 3)
		{
			case 0b00000: // IF B[f] = 0
				hp35_iterate_field(hp35_carry |= (hp35_B[i] != 0));
				break;
			case 0b00001: // 0 -> B[f]
				hp35_reg_clr(hp35_B);
				break;
			case 0b00010: // IF A >= C[f]
				hp35_iterate_field(hp35_nib_sub(hp35_A[i], hp35_C[i]));
				break;
			case 0b00011: // IF C[f] >= 1
				hp35_carry = 1; hp35_iterate_field(hp35_carry &= (hp35_C[i] == 0));
				break;
			case 0b00100: // B -> C[f]
				hp35_reg_copy(hp35_C, hp35_B);
				break;
			case 0b00101: // 0 – C -> C[f]
				hp35_iterate_field(hp35_C[i] = hp35_nib_sub(0, hp35_C[i]));
				break;
			case 0b00110: // 0 -> C[f]
				hp35_reg_clr(hp35_C);
				break;
			case 0b00111: // 0 – C – 1 -> C[f]
				hp35_carry = 1; hp35_iterate_field(hp35_C[i] = hp35_nib_sub(0, hp35_C[i]));
				break;
			case 0b01000: // SHIFT LEFT A[f]
				hp35_reg_shl(hp35_A);
				break;
			case 0b01001: // A -> B[f]
				hp35_reg_copy(hp35_B, hp35_A);
				break;
			case 0b01010: // A – C -> C[f]
				hp35_reg_sub(hp35_C, hp35_A, hp35_C);
				break;
			case 0b01011: // C – 1 -> C[f]
				hp35_reg_dec(hp35_C);
				break;
			case 0b01100: // C -> A[f]
				hp35_reg_copy(hp35_A, hp35_C);
				break;
			case 0b01101: // IF C[f] = 0
				hp35_iterate_field(hp35_carry |= (hp35_C[i] != 0));
				break;
			case 0b01110: // A + C -> C[f]
				hp35_reg_add(hp35_C, hp35_A, hp35_C);
				break;
			case 0b01111: // C + 1 -> C[f]
				hp35_reg_inc(hp35_C);
				break;
			case 0b10000: // IF A >= B[f]
				hp35_iterate_field(hp35_nib_sub(hp35_A[i], hp35_B[i]));
				break;
			case 0b10001: // B EXCHANGE C[f]
				hp35_reg_swap(hp35_B, hp35_C);
				break;
			case 0b10010: // SHIFT RIGHT C[f]
				hp35_reg_shr(hp35_C);
				break;
			case 0b10011: // IF A[f] >= 1
				hp35_carry = 1; hp35_iterate_field(hp35_carry &= (hp35_A[i] == 0));
				break;
			case 0b10100: // SHIFT RIGHT B[f]
				hp35_reg_shr(hp35_B);
				break;
			case 0b10101: // C + C -> C[f]
				hp35_reg_add(hp35_C, hp35_C, hp35_C);
				break;
			case 0b10110: // SHIFT RIGHT A[f]
				hp35_reg_shr(hp35_A);
				break;
			case 0b10111: // 0 -> A[f]
				hp35_reg_clr(hp35_A);
				break;
			case 0b11000: // A – B -> A[f]
				hp35_reg_sub(hp35_A, hp35_A, hp35_B);
				break;
			case 0b11001: // A EXCHANGE B[f]
				hp35_reg_swap(hp35_A, hp35_B);
				break;
			case 0b11010: // A – C -> A[f]
				hp35_reg_sub(hp35_A, hp35_A, hp35_C);
				break;
			case 0b11011: // A – 1 -> A[f]
				hp35_reg_dec(hp35_A);
				break;
			case 0b11100: // A + B -> A[f]
				hp35_reg_add(hp35_A, hp35_A, hp35_B);
				break;
			case 0b11101: // A EXCHANGE C[f]
				hp35_reg_swap(hp35_A, hp35_C);
				break;
			case 0b11110: // A + C -> A[f]
				hp35_reg_add(hp35_A, hp35_A, hp35_C);
				break;
			case 0b11111: // A + 1 -> A[f]
				hp35_reg_inc(hp35_A);
				break;
		}
	}

	// Type 11: Branching Instructions
	else // op_type == 0x03
	{
		// (THEN) GO TO addr
		if (!hp35_carry_alu)
		{
			hp35_pc = op_code;
		}
	}

	// display update
	if (hp35_disp_update)
	{
		hp35_disp_update = 0;
		for (int8_t d = 0, i = 13; i >= 0; --i)
		{
			if (hp35_disp_enable)
			{
				if (hp35_B[i] == 9)
					HP35_Display[d++] = ' ';
				else if (i == 2 || i == 13)
					HP35_Display[d++] = (hp35_A[i] == 9 ? '-' : ' ');
				else
					HP35_Display[d++] = ('0' + hp35_A[i]);
				if (hp35_B[i] == 2)
					HP35_Display[d++] = '.';
			}
			else
				HP35_Display[d++] = ' ';
		}
		return true;
	}
	return false;
}
