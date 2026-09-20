#pragma once

#include <stdint.h>

// AVR/Arduino supplies these through <avr/pgmspace.h>.  The fallbacks make the
// header easy to compile in a desktop test harness as well.
#if defined(__AVR__)
#include <avr/pgmspace.h>
#else
#ifndef PROGMEM
#define PROGMEM
#endif
#ifndef pgm_read_byte
#define pgm_read_byte(p) (*(const uint8_t *)(p))
#endif
#endif

/*
 * HPVMWoodstock.h
 * -----------------------------------------------------------------------------
 * Virtual machine for the Hewlett-Packard Woodstock ACT architecture.
 *
 * Design goals:
 *   - independent from HPVMClassic.h;
 *   - sufficient for HP-21;
 *   - implements the common Woodstock ACT core rather than being hard-coded to
 *     one HP-21 ROM image;
 *   - keeps the implementation suitable for a small AVR, and follows the style
 *     used by HPVMClassic.h (header-only namespace, byte/nibble registers,
 *     packed 10-bit ROM fetch, one Cycle() per microinstruction word time).
 *
 * IMPORTANT MODEL BOUNDARIES
 * -----------------------------------------------------------------------------
 * The base ACT architecture covers HP-21/22/25/27 very naturally.  Later ACT
 * machines (HP-67/97, 19C/29C, etc.) added model/processor-revision details such
 * as ROM banking, peripheral opcodes, and an undocumented P-wrap behaviour.
 * Optional hooks are provided below, but those later extensions are deliberately
 * not silently guessed in the common core.
 *
 * This header includes the 1024-word HP-21 firmware ROM by default, packed in
 * the same form as HPVMClassic.h.  The ACT core itself is not HP-21-specific:
 * define HPVM_WOODSTOCK_EXTERNAL_ROM before including this file and provide
 * rom_l[]/rom_h[] from a different Woodstock model to reuse the same VM.
 *
 * ROM packing:
 *     rom_l[a]                   = opcode[7:0]
 *     rom_h[a >> 2], bits 0..1   = opcode[9:8] for address a&3 == 0
 *                     bits 2..3  = opcode[9:8] for address a&3 == 1
 *                     bits 4..5  = opcode[9:8] for address a&3 == 2
 *                     bits 6..7  = opcode[9:8] for address a&3 == 3
 *
 * Architecture references used for this implementation:
 *   - Hewlett-Packard Journal, November 1975, "Inside the New Pocket Calculators"
 *   - Jacques Laporte, "HP-21, HP-25 : Hardware analysis"
 *     https://archived.hpcalc.org/laporte/Woodstock/ws_HW_main.htm
 *   - Jacques Laporte, "HP-25 : Firmware analysis"
 *     https://archived.hpcalc.org/laporte/Woodstock/ws_FW_main.htm
 *   - Eric Smith, Nonpareil Woodstock processor model (reference implementation)
 *     https://github.com/brouhaha/nonpareil
 *
 * The default HP-21 ROM below was supplied from x11-calc's model file:
 *   https://github.com/mike632t/x11-calc/blob/stable/src/x11-calc-21.c
 * It contains exactly 1024 ten-bit microinstructions and is repacked losslessly
 * into rom_l[1024] + rom_h[256].
 */

// -----------------------------------------------------------------------------
// Optional model configuration
// -----------------------------------------------------------------------------

// Number of external 56-bit RAM registers visible to ACT data instructions.
// HP-21 has no external RAM chip, therefore zero is the correct default.
// HP-25 uses 16 registers.  Larger ACT models can select a larger value.
#ifndef HPVM_WOODSTOCK_RAM_SIZE
#define HPVM_WOODSTOCK_RAM_SIZE 0
#endif

// Number of 4K-word ROM banks represented by rom_l/rom_h.
// 1 = ordinary Woodstock address space (HP-21/22/25/27).
// 2 = later banked machines.  If enabled, bank 0 occupies addresses 0..4095
// and bank 1 follows at 4096..8191 in the supplied packed ROM arrays.  A model
// with only some pages banked should duplicate its bank-0 page in bank 1.
#ifndef HPVM_WOODSTOCK_ROM_BANKS
#define HPVM_WOODSTOCK_ROM_BANKS 1
#endif

#if !defined(HPVM_WOODSTOCK_EXTERNAL_ROM) && (HPVM_WOODSTOCK_ROM_BANKS > 1)
#error "The built-in HP-21 ROM is not banked; use HPVM_WOODSTOCK_EXTERNAL_ROM for banked models."
#endif

// Model/peripheral-specific miscellaneous instructions are delegated here.
// Examples include printer/card-reader opcodes of later Woodstock machines.
// The macro executes inside namespace HPVM and receives the full 10-bit opcode.
#ifndef HPVM_WOODSTOCK_EXTENSION
#define HPVM_WOODSTOCK_EXTENSION(opcode) do { (void)(opcode); } while (0)
#endif

// Idling and error are firmware states, not ACT hardware flags.  A generic CPU
// cannot know them without knowledge of the selected calculator ROM.  A model
// adapter may define these expressions before including this file; they are
// evaluated inside namespace HPVM and may inspect pc, last_pc, status, etc.
#ifndef HPVM_WOODSTOCK_IDLE_CONDITION
#define HPVM_WOODSTOCK_IDLE_CONDITION false
#endif
#ifndef HPVM_WOODSTOCK_ERROR_CONDITION
#define HPVM_WOODSTOCK_ERROR_CONDITION false
#endif

// Nominal HP-21 ACT clock from the November 1975 HP Journal specification.
// Other Woodstock models/individual calculators may differ, so a model adapter
// may override this before including the header.
#ifndef HPVM_WOODSTOCK_CLOCK_KHZ
#define HPVM_WOODSTOCK_CLOCK_KHZ 200UL
#endif

namespace HPVM
{
	// -------------------------------------------------------------------------
	// Woodstock ACT Virtual Machine
	// -------------------------------------------------------------------------

	// One ACT word time is 56 bit-times (14 digits * 4 bits).  The HP-21
	// specification gives a nominal 200 kHz clock, hence a nominal 280 us word
	// time.  HPVM_WOODSTOCK_CLOCK_KHZ remains overrideable for another model.
	#define HPVM_CLOCK_RATE       (HPVM_WOODSTOCK_CLOCK_KHZ) // kHz
	#define HPVM_BITS_PER_CYCLE   (14UL * 4UL)
	#define HPVM_CYCLE_TIME_WIDTH ((HPVM_BITS_PER_CYCLE * 1000UL) / HPVM_CLOCK_RATE) // us; HP-21 ~= 280
	#define HPVM_CYCLES_PER_SEC   (1000000UL / HPVM_CYCLE_TIME_WIDTH)

	// -------------------------------------------------------------------------
	// Public interface
	// -------------------------------------------------------------------------
	void Reset();
	void KeyDown(uint8_t token);
	void KeyUp();
	void ExternalF1(bool state);
	void ExternalF2(bool state);
	void ExternalF1ConditionalS0(bool state);
	void ExternalF2ConditionalS0(bool state);
	void PulseF1();
	void PulseF2();
	bool FlagOut();
	bool Idling();
	bool Error();
	bool Cycle();
	uint16_t Address();
	uint16_t LastAddress();
	bool Status(uint8_t n);

	/*
	 * HP-21 MODEL ADAPTER: RAW KEYBOARD TOKENS AND ANGLE SWITCH
	 * -------------------------------------------------------------------------
	 * These constants are intentionally separate from the ACT instruction-set
	 * implementation.  A key token is a property of a calculator's physical
	 * keyboard matrix, not a universal Woodstock function number or ROM address.
	 *
	 * The values below are the first argument passed to h_button_create() in
	 * x11-calc's HP-21 model.  They are written in octal, as in the historical
	 * sources.  KeyDown() accepts exactly this raw 8-bit token.  KeyUp() releases
	 * the key but leaves the ACT token latch intact.
	 *
	 * HP-21 has 30 keys plus two slide switches.  ON/OFF is not a keyboard token:
	 * the host should stop scheduling Cycle() while the calculator is off and
	 * call Reset() when modelling a fresh power-up.  DEG/RAD is also not a key.
	 * x11-calc models RAD by asserting status S3; the common ACT hardware feeds
	 * S3 from external flag F2, so SetAngleMode() routes the switch through F2
	 * rather than modifying the status word directly.
	 *
	 * Source for tokens/switch layout:
	 *   https://github.com/mike632t/x11-calc/blob/stable/src/x11-calc-21.c
	 */
	namespace HP21
	{
		enum KeyToken : uint8_t
		{
			KEY_RECIPROCAL = 00264, // 1/x
			KEY_SIN        = 00263,
			KEY_COS        = 00262,
			KEY_TAN        = 00261,
			KEY_F          = 00260,

			KEY_EXCHANGE_XY = 00104, // x <-> y
			KEY_ROLL_DOWN   = 00103, // R down
			KEY_EXP         = 00102, // e^x
			KEY_STO         = 00101,
			KEY_RCL         = 00100,

			KEY_ENTER = 00324,
			KEY_CHS   = 00322,
			KEY_EEX   = 00321,
			KEY_CLX   = 00320,

			KEY_SUB = 00144,
			KEY_7   = 00143,
			KEY_8   = 00142,
			KEY_9   = 00141,

			KEY_ADD = 00244,
			KEY_4   = 00243,
			KEY_5   = 00242,
			KEY_6   = 00241,

			KEY_MUL = 00164,
			KEY_1   = 00163,
			KEY_2   = 00162,
			KEY_3   = 00161,

			KEY_DIV = 00224,
			KEY_0   = 00223,
			KEY_DOT = 00222,
			KEY_DSP = 00221
		};

		enum : uint8_t { KEY_COUNT = 30 };

		enum AngleMode : uint8_t
		{
			ANGLE_DEG = 0,
			ANGLE_RAD = 1
		};

		inline void SetAngleMode(AngleMode mode)
		{
			ExternalF2(mode == ANGLE_RAD);
		}
	}

	/*
	 * DISPLAY INTERFACE
	 * -------------------------------------------------------------------------
	 * Woodstock does NOT present a ready ASCII string to the display driver.
	 * During each 56-bit ACT word time exactly one display position is scanned.
	 * A[] supplies a 4-bit character-generator address and B[] supplies display
	 * control bits.  In normal Woodstock mode the twelve physical positions are
	 * sourced from register digits 13..2.
	 *
	 * Display[] therefore contains *display character codes*, not ASCII:
	 *   0x00..0x0f : address into the calculator model's character generator
	 *   HPVM_SPACE : forced blank generated by the ACT/B-mask logic
	 *   HPVM_DASH  : forced minus sign generated by the ACT/B-mask logic
	 *
	 * The HP-21 adapter normally maps 0..9 to decimal glyphs and maps the
	 * remaining codes according to the HP-21 display character ROM.  In
	 * particular firmware often uses 0x0f for a blanked digit.
	 *
	 * Decimal points are integral to a physical Woodstock digit, so they are
	 * exposed separately in DisplayDots: bit n belongs to Display[n].  This is
	 * deliberately different from HPVMClassic, where a decimal point was emitted
	 * as an extra character between LED positions.
	 *
	 * Cycle() returns true only when a complete scan frame has finished AND the
	 * exposed display state changed.  This avoids asking an OLED adapter to redraw
	 * an unchanged display roughly 250+ times per second.
	 */
	#define HPVM_SPACE              0x10
	#define HPVM_DASH               0x11
	#define HPVM_TWF_MANTISSA_SIGN  0x12
	#define HPVM_TWF_EXPONENT_SIGN  0x13

	/*
	 * Normal Woodstock mode exposes 12 scanned positions.  RESET TWF changes
	 * the hardware to a 14-digit scan and inserts one additional, separately
	 * driven mantissa-sign position, so the normalized output has 15 positions.
	 *
	 * The two HPVM_TWF_* values deliberately do not pretend to be ordinary
	 * character-generator addresses.  In 14-digit mode the ACT/anode-driver
	 * hardware derives two displays from the character that would normally be
	 * emitted at the exponent-sign position:
	 *
	 *   HPVM_TWF_MANTISSA_SIGN (Display[0])
	 *       render '-' iff the E segment of DisplayTWFSource is lit.
	 *
	 *   HPVM_TWF_EXPONENT_SIGN (normally Display[12])
	 *       render only the G segment of DisplayTWFSource.  DisplayDots still
	 *       supplies the decimal-point segment for this position.
	 *
	 * This keeps the ACT VM independent of a model's character-generator ROM.
	 * The HP-21/Tiny-RPN display adapter can resolve the two special values from
	 * its own glyph/segment table.  Normal 12-position HP-21 display operation
	 * never needs this special handling.
	 */
	uint8_t  Display[15];
	uint16_t DisplayDots;
	uint8_t  DisplaySize;      // 12 normally; 15 after RESET TWF (14 digits + sign)
	uint8_t  DisplayTWFSource; // source character for the two HPVM_TWF_* entries

	// -------------------------------------------------------------------------
	// Packed firmware ROM
	// -------------------------------------------------------------------------
	#ifndef HPVM_WOODSTOCK_EXTERNAL_ROM
		/*
		 * HP-21 firmware ROM
		 * -------------------------------------------------------------------------
		 * 1024 ten-bit microinstructions (four 256-word Woodstock ROM pages).
		 *
		 * Source image supplied by the user from x11-calc:
		 *   https://github.com/mike632t/x11-calc/blob/stable/src/x11-calc-21.c
		 *
		 * The original octal words are repacked here exactly like HPVMClassic:
		 *   rom_l[] : low 8 bits, one byte per microinstruction
		 *   rom_h[] : high 2 bits, four microinstructions packed into one byte
		 *
		 * Flash cost: 1024 + 256 = 1280 bytes.
		 *
		 * Define HPVM_WOODSTOCK_EXTERNAL_ROM before including this header to use a
		 * different Woodstock calculator ROM while keeping the same ACT VM.
		 */
		const uint8_t rom_l[] PROGMEM =
		{
			0xBA, 0xBA, 0xC8, 0x08, 0x1A, 0x8C, 0x48, 0xC8, 0x9D, 0x76, 0x0C, 0x1A, 0x3A, 0x25, 0x88, 0x5C,
			0x50, 0x0D, 0x4A, 0x4A, 0xB4, 0xBA, 0xF3, 0x06, 0xA5, 0xCD, 0x3C, 0xC6, 0x7C, 0xC6, 0x7C, 0xA6,
			0xA6, 0x9A, 0xAE, 0xEA, 0x27, 0x0A, 0x8E, 0xF9, 0x9E, 0x7C, 0x9D, 0xA4, 0x06, 0x63, 0x11, 0x9A,
			0x98, 0x98, 0x98, 0x18, 0x98, 0x9A, 0x36, 0x48, 0x77, 0x54, 0x6D, 0xD6, 0xB7, 0x31, 0x4E, 0x6F,
			0x9C, 0x43, 0xC8, 0x84, 0x1A, 0xFC, 0x31, 0x12, 0x7A, 0x1A, 0xB2, 0xBA, 0xB2, 0xB2, 0x5A, 0x10,
			0x4E, 0x0D, 0x4A, 0xA2, 0x31, 0x4E, 0x3C, 0x42, 0xAE, 0x4E, 0x47, 0x0E, 0x4E, 0x4C, 0x5C, 0xDE,
			0xD1, 0x01, 0x94, 0xDC, 0xF9, 0x10, 0x3C, 0xA2, 0x73, 0xC6, 0xD0, 0x6C, 0x67, 0x90, 0x90, 0x9A,
			0xBA, 0x66, 0x8E, 0x08, 0x56, 0xE5, 0x1E, 0x9E, 0x8C, 0xCD, 0xE4, 0x78, 0x8B, 0xE4, 0x86, 0x90,
			0x4E, 0xF7, 0x1B, 0xFC, 0xAE, 0x03, 0x1A, 0xA6, 0x56, 0x43, 0xB2, 0xBE, 0xAE, 0x54, 0x90, 0x90,
			0x6E, 0x10, 0x4C, 0xCD, 0xE4, 0x93, 0x4C, 0x54, 0x9A, 0x44, 0xC8, 0x1B, 0x42, 0xEC, 0x39, 0x44,
			0x0E, 0xBB, 0xB6, 0xAE, 0x8B, 0x0E, 0x88, 0xFC, 0x90, 0x4E, 0xA3, 0x06, 0x56, 0x3D, 0x7C, 0x19,
			0x4B, 0x48, 0x44, 0x1F, 0x31, 0x4E, 0x8E, 0xEA, 0xBB, 0x8E, 0x6A, 0x8E, 0xA5, 0x77, 0xBC, 0x90,
			0xEE, 0xC2, 0xBF, 0xFC, 0x6E, 0x62, 0xD0, 0x90, 0x94, 0xC4, 0xD6, 0x13, 0x06, 0x10, 0x46, 0x08,
			0x10, 0xD6, 0x67, 0x54, 0xBC, 0x9C, 0xF2, 0x7C, 0x82, 0xA2, 0x82, 0x7B, 0x76, 0xE1, 0x11, 0xE2,
			0xA2, 0x7C, 0xC6, 0x17, 0xE3, 0xF9, 0xDF, 0x60, 0x00, 0x7C, 0xC6, 0xC6, 0x26, 0x42, 0xA2, 0xA2,
			0x42, 0x10, 0xB2, 0x4C, 0xB2, 0x88, 0xCC, 0xBC, 0x90, 0xEC, 0xF8, 0xD4, 0xF6, 0xF0, 0xDC, 0xFE,
			0xC8, 0x3C, 0x02, 0x10, 0x20, 0x84, 0xCC, 0x4D, 0x5C, 0x92, 0x9C, 0x17, 0xB2, 0x92, 0xE0, 0x54,
			0x77, 0x48, 0x6F, 0x44, 0x7C, 0x10, 0x54, 0x97, 0x84, 0x10, 0x08, 0xC8, 0x9A, 0x10, 0xDC, 0x9B,
			0x1A, 0xA2, 0xF6, 0xC3, 0x88, 0x71, 0xC8, 0x6D, 0xA1, 0x17, 0xFC, 0x3A, 0xAA, 0xAA, 0xEA, 0xEA,
			0x0E, 0x33, 0x9A, 0x96, 0xF6, 0x37, 0x9A, 0xF6, 0x0E, 0xB8, 0xDA, 0xAE, 0xDA, 0xB8, 0xE3, 0x20,
			0x9B, 0xE3, 0xEF, 0x3F, 0x5C, 0x5F, 0x4D, 0xC4, 0x4C, 0x69, 0xCB, 0xFC, 0x6A, 0xB3, 0x1A, 0x10,
			0xE0, 0x1A, 0x3C, 0xD8, 0x18, 0x58, 0xD8, 0x58, 0x18, 0x58, 0x98, 0xD8, 0x58, 0xFC, 0x10, 0x69,
			0x6F, 0xA2, 0xA2, 0x83, 0xC1, 0xB2, 0xA9, 0x5C, 0x9B, 0xA1, 0x08, 0x6F, 0x06, 0x66, 0x0A, 0x10,
			0xA2, 0xA2, 0xA2, 0x63, 0xC1, 0x6D, 0x9F, 0xC4, 0xF2, 0x7B, 0x04, 0x88, 0xC8, 0x88, 0x56, 0x82,
			0xF6, 0x9B, 0xFC, 0x71, 0xA1, 0x88, 0x69, 0x69, 0x69, 0x08, 0xA9, 0x4A, 0x4A, 0x4A, 0xBC, 0xA1,
			0x03, 0x20, 0x4F, 0x10, 0xFC, 0xC1, 0xE7, 0xFD, 0x45, 0x7A, 0x7A, 0x84, 0x34, 0x1B, 0x08, 0xAB,
			0xA2, 0xA2, 0xA2, 0xC3, 0xC1, 0x9B, 0x04, 0x54, 0x4B, 0xFD, 0x48, 0x6F, 0x5C, 0x04, 0x08, 0x6F,
			0x20, 0x1B, 0x84, 0x17, 0x54, 0xF6, 0x1A, 0xFC, 0xA2, 0x04, 0x71, 0x9F, 0x4D, 0x41, 0xDC, 0x9B,
			0xC8, 0x88, 0x1B, 0x88, 0x9A, 0x88, 0x6D, 0xA1, 0x1C, 0xCB, 0xB2, 0x08, 0xC8, 0xC8, 0x6D, 0x6F,
			0xB3, 0x5B, 0x20, 0xA0, 0x54, 0xF3, 0xC8, 0x13, 0xA2, 0x10, 0xBA, 0xA0, 0x2E, 0xA0, 0xFC, 0xE2,
			0xA2, 0xE2, 0x90, 0x24, 0x60, 0x7F, 0x08, 0x10, 0xEA, 0xEF, 0x6E, 0xEA, 0x6A, 0x2F, 0xEE, 0x10,
			0x5C, 0xE6, 0x08, 0xBA, 0x48, 0x10, 0x69, 0x84, 0x5C, 0x9E, 0x84, 0x04, 0x5C, 0x4D, 0x1A, 0x96,
			0xA3, 0xBA, 0x72, 0xA3, 0xFC, 0xF2, 0x7A, 0xED, 0x42, 0x17, 0x46, 0x32, 0xEF, 0xBC, 0xBD, 0xFC,
			0xE1, 0x7C, 0xDD, 0x61, 0xBC, 0xDD, 0x01, 0x3C, 0xDD, 0x91, 0xDD, 0xC1, 0xDD, 0xCD, 0x9A, 0x3A,
			0xCA, 0x23, 0x3A, 0x5A, 0x90, 0xDA, 0x6C, 0x24, 0x9A, 0xF2, 0x2C, 0xB6, 0xEE, 0x3C, 0xF9, 0x94,
			0xDA, 0x9C, 0x9B, 0xCD, 0x75, 0x6B, 0xE0, 0xBA, 0xFA, 0xEF, 0x3A, 0x72, 0xEB, 0x9A, 0xDE, 0x9A,
			0xBF, 0x9A, 0x7A, 0xB6, 0x6A, 0x83, 0xEA, 0xBA, 0xEE, 0x1F, 0xA7, 0xC9, 0x71, 0xC9, 0x05, 0xC1,
			0x3C, 0xD9, 0x91, 0xBC, 0xD9, 0x01, 0x7C, 0xD9, 0x61, 0xFC, 0xD9, 0xD9, 0xD9, 0xFC, 0x06, 0xBC,
			0xFA, 0x9A, 0x98, 0x53, 0x7C, 0xD8, 0x58, 0x18, 0x58, 0xD8, 0x58, 0x18, 0x18, 0x58, 0x58, 0xE4,
			0x8A, 0x5B, 0xBA, 0x1A, 0xFC, 0x98, 0xD8, 0x18, 0x98, 0x58, 0x13, 0x52, 0xB2, 0xFE, 0xC6, 0x1B,
			0xBC, 0xD8, 0xD8, 0x18, 0x18, 0x58, 0x18, 0x64, 0x8B, 0x58, 0xD8, 0x5B, 0xED, 0xA2, 0x7A, 0x72,
			0x33, 0xA6, 0x9A, 0xDE, 0x9A, 0x52, 0x3B, 0x5A, 0xA2, 0x15, 0x60, 0x1C, 0xA0, 0xAE, 0x4A, 0xF8,
			0x1E, 0x6F, 0x3E, 0xDA, 0x6E, 0x7B, 0xBA, 0x06, 0x8E, 0xF2, 0xAE, 0x5A, 0x1A, 0xBA, 0xBA, 0xFA,
			0x1A, 0x76, 0x1C, 0xB8, 0x18, 0xF6, 0xEF, 0x98, 0x6C, 0xB7, 0xFA, 0xFA, 0x10, 0x3A, 0x62, 0xF7,
			0xBA, 0xD0, 0x6C, 0xBE, 0xEE, 0x12, 0xFC, 0x3A, 0x42, 0xCF, 0xDA, 0x6E, 0x5A, 0xC8, 0x1A, 0x6E,
			0x3A, 0x52, 0xC0, 0x96, 0xBA, 0x3A, 0xFC, 0x10, 0xBC, 0x07, 0x08, 0x33, 0x7C, 0x8E, 0x32, 0x87,
			0x92, 0x3A, 0x56, 0x1A, 0xAC, 0xBE, 0x76, 0xED, 0x14, 0x2E, 0xC6, 0x56, 0xEA, 0xE6, 0x96, 0xE0,
			0x98, 0x58, 0xD8, 0x58, 0x18, 0xD8, 0x58, 0xAF, 0xB6, 0x9B, 0xA6, 0x52, 0xEB, 0x12, 0x3A, 0x10,
			0x5A, 0xB9, 0x31, 0xB9, 0x31, 0x9C, 0x08, 0x9A, 0x9C, 0x14, 0xF2, 0x0D, 0x04, 0x12, 0x6D, 0xB1,
			0x69, 0x9D, 0x45, 0x31, 0x6D, 0x5C, 0x1E, 0x1A, 0xA2, 0x76, 0x96, 0x6E, 0xC6, 0xEA, 0x1B, 0xA6,
			0xEE, 0x7F, 0xBA, 0xDA, 0xB1, 0xFA, 0x6F, 0x1A, 0xA2, 0x60, 0x90, 0x3E, 0x73, 0x20, 0xB1, 0x9A,
			0xE2, 0x33, 0x9A, 0xBA, 0xCE, 0x13, 0xA0, 0xDA, 0xDE, 0xFA, 0xF3, 0xF2, 0x1A, 0xEF, 0x3A, 0xA0,
			0xA0, 0xE2, 0x9A, 0x07, 0x5A, 0xDA, 0x90, 0xE6, 0xEC, 0x42, 0x02, 0xCF, 0x9A, 0xC8, 0x9A, 0x10,
			0x60, 0xFA, 0x7C, 0x7F, 0xC6, 0x52, 0x53, 0xF2, 0x46, 0x86, 0x5A, 0x7A, 0x86, 0x57, 0x31, 0xBA,
			0x86, 0x5A, 0xC6, 0xB1, 0xB2, 0xB2, 0x97, 0x1A, 0x2E, 0xBE, 0xBD, 0x62, 0x31, 0x9A, 0x7C, 0xFD,
			0xFC, 0xD9, 0xFC, 0xD9, 0x7C, 0x18, 0xBC, 0xD9, 0xE1, 0xD9, 0x41, 0xDA, 0xD9, 0xDA, 0x11, 0x41,
			0x7A, 0x94, 0x88, 0x1C, 0x8D, 0x7A, 0x9A, 0xB2, 0x9A, 0xB2, 0xA5, 0x41, 0x7A, 0x9A, 0xD4, 0x98,
			0x9A, 0x6D, 0x1A, 0x62, 0xEE, 0x5C, 0x9B, 0x69, 0xD4, 0x5F, 0x60, 0xD4, 0xA1, 0x6D, 0x41, 0x7A,
			0x69, 0x41, 0x7A, 0x7A, 0x7A, 0x01, 0x41, 0xBC, 0xDD, 0xE1, 0xFC, 0xE1, 0x7C, 0x18, 0xFC, 0xDD,
			0x7C, 0xDD, 0xDD, 0x5A, 0xFA, 0xBC, 0x58, 0x53, 0x6A, 0x6A, 0x0E, 0x92, 0x52, 0xBF, 0xA0, 0x36,
			0xC3, 0xB2, 0x5A, 0x1A, 0xA0, 0xC6, 0xC6, 0x72, 0x17, 0x86, 0x06, 0xE6, 0xDA, 0x52, 0x1F, 0x86,
			0x31, 0xD2, 0x00, 0xDA, 0x86, 0xB1, 0xC6, 0x72, 0xF2, 0x33, 0x60, 0x60, 0x2E, 0x7F, 0x62, 0xF2,
			0xEC, 0x2A, 0x8E, 0x0E, 0x62, 0xE7, 0xBA, 0xFA, 0xEE, 0x0E, 0xFC, 0x1F, 0x88, 0xC8, 0x10, 0xE2,
			0x1E, 0xBF, 0x3E, 0xDE, 0x90, 0xEC, 0xF0, 0xCF, 0xFC, 0x18, 0x98, 0x58, 0x98, 0x18, 0x58, 0x10
		};

		const uint8_t rom_h[] PROGMEM =
		{
			0x75, 0x01, 0x4F, 0x34, 0xE8, 0x20, 0x6F, 0xC6, 0x83, 0xA4, 0x30, 0x10, 0xEB, 0x02, 0xC9, 0x4D,
			0x20, 0x35, 0xD0, 0x85, 0x39, 0xE3, 0x38, 0x54, 0x37, 0x67, 0x96, 0x14, 0xC8, 0x03, 0x0F, 0x41,
			0x66, 0x09, 0xD9, 0x41, 0xE8, 0x80, 0x30, 0x86, 0x74, 0x72, 0x29, 0x83, 0x30, 0x83, 0x28, 0x67,
			0x49, 0x4E, 0xD3, 0xE4, 0xDE, 0x0D, 0x08, 0x53, 0x20, 0x19, 0x50, 0x50, 0xE8, 0x30, 0xC9, 0x3C,
			0x0C, 0xC4, 0xDF, 0xC2, 0xA9, 0x79, 0xAB, 0x58, 0x64, 0xDD, 0x1F, 0x55, 0x07, 0x06, 0x7F, 0x0E,
			0x3E, 0x77, 0x63, 0x95, 0x54, 0x86, 0x12, 0x25, 0x96, 0xCB, 0x9D, 0x99, 0xD5, 0x5F, 0x56, 0x79,
			0xD6, 0x37, 0x8B, 0xDE, 0x83, 0x2D, 0x15, 0x90, 0x55, 0xE7, 0x92, 0x83, 0x20, 0x47, 0x79, 0x57,
			0x46, 0xF8, 0xA5, 0xB6, 0x02, 0x23, 0x09, 0x12, 0x12, 0xAC, 0x66, 0x96, 0x17, 0x89, 0x97, 0xCB,
			0xEF, 0xC5, 0x42, 0x21, 0xCC, 0x61, 0xC4, 0x84, 0x2A, 0x95, 0xA8, 0xA5, 0x56, 0xCB, 0x90, 0x10,
			0x02, 0xD9, 0x65, 0xD7, 0x51, 0x38, 0x03, 0x88, 0x90, 0x03, 0xA4, 0x94, 0x4E, 0x01, 0x24, 0x1D,
			0x00, 0x86, 0xCA, 0x87, 0x4E, 0x28, 0x8D, 0xB6, 0x5A, 0x7A, 0x28, 0x3A, 0xA9, 0x65, 0xF9, 0xA6,
			0xA7, 0x11, 0x9B, 0x1B, 0x2D, 0x90, 0x6B, 0xE6, 0x02, 0xB8, 0x82, 0x01, 0x09, 0x45, 0xB9, 0x93,
			0x10, 0x39, 0xED, 0xF5, 0x53, 0x1F, 0x81, 0xFB, 0xF1, 0x13, 0x51, 0x33, 0x2E, 0x4C, 0x43, 0x12,
			0x74, 0xD5, 0x1E, 0x84, 0xC0, 0x5B, 0x04, 0xD7, 0xD0, 0x45, 0xBC, 0x01, 0x02, 0x19, 0x53, 0x70,
			0x79, 0x07, 0x48, 0xC1, 0x9C, 0xFD, 0x05, 0x5F, 0x57, 0x55, 0x0C, 0x29, 0x00, 0xDB, 0xCA, 0xCF,
			0x8B, 0xBC, 0x27, 0x38, 0x79, 0xBC, 0x0C, 0x6D, 0x0E, 0xFF, 0x54, 0x65, 0x5E, 0x39, 0x5A, 0xA4
		};

		// Guard against an accidental edit that silently breaks the 10-bit packing.
		static_assert(sizeof(rom_l) == 1024, "HP-21 rom_l must contain 1024 low bytes");
		static_assert(sizeof(rom_h) ==  256, "HP-21 rom_h must contain 256 packed high-bit bytes");
	#else
		extern const uint8_t rom_l[] PROGMEM;
		extern const uint8_t rom_h[] PROGMEM;
	#endif

	// -------------------------------------------------------------------------
	// ACT registers
	// -------------------------------------------------------------------------
	// ACT contains eight 56-bit serial registers: exactly 14 four-bit digits.
	// A C++ byte is used per digit for cheap AVR indexing; the upper nibble is
	// unused.  Keeping the exact architectural length also saves 16 bytes of SRAM
	// versus the old 16-byte-per-register placeholder representation.
	enum : uint8_t { WORD_DIGITS = 14, WORD_MSD = 13 };
	using digit = uint8_t;
	using reg   = digit[WORD_DIGITS];

	reg A;
	reg B;
	reg C;  // X stack level / normalized working value
	reg Y;
	reg Z;
	reg T;
	reg M1; // internal ACT scratch register
	reg M2; // internal ACT scratch register

	// Separate four-bit F register (not to be confused with Classic's F=T reg).
	uint8_t f;

	// Pointer and arithmetic field boundaries.
	uint8_t p, ff, fl;

	// Woodstock has sixteen status bits.  Keeping them as a uint16_t matches the
	// hardware more closely and saves 14 bytes of SRAM versus uint8_t s[16].
	uint16_t status;

	// Decimal/binary ALU mode and the usual carry pipeline.
	uint8_t decimal, carry, prev_carry;

	/*
	 * PROGRAM CONTROL STATE
	 * -------------------------------------------------------------------------
	 * Woodstock has a 12-bit microprogram address (0x000..0xfff), unlike the
	 * Classic rom+8-bit-PC split.  Ordinary GOTO/JSB still replace only the low
	 * eight address bits.  SELECT ROM replaces address bits 11..8.
	 *
	 * The return stack is exactly two deep.  ret_sp is a one-bit circular index:
	 * JSB writes ret_stack[ret_sp] then toggles ret_sp; RETURN toggles first and
	 * reads the entry.  This reproduces the two-level hardware stack, including
	 * its wraparound behaviour on excessive nesting.
	 */
	uint16_t pc;
	uint16_t last_pc;
	uint16_t ret_stack[2];
	uint8_t  ret_sp;

	/*
	 * THEN-GOTO IS THE BIGGEST CONTROL-FLOW DIFFERENCE FROM CLASSIC
	 * -------------------------------------------------------------------------
	 * Arithmetic/status/P tests do not directly contain a branch address.  They
	 * put ACT into a one-word "then goto" state.  The *next 10-bit ROM word is
	 * consumed as a literal low-10-bit branch address*, not decoded as an opcode.
	 * If the test succeeds (represented by previous carry == 0), that literal is
	 * merged into the current 12-bit PC.  This is why a Woodstock disassembler
	 * must know instruction state, not just the 10-bit word at an address.
	 */
	uint8_t then_goto;

	/*
	 * DELAYED ROM SELECT
	 * -------------------------------------------------------------------------
	 * DELAYED ROM n also takes effect one word later.  Crucially, the pending
	 * selection is snapshotted at the beginning of Cycle(), the current word is
	 * executed normally, and only afterwards are PC bits 11..8 replaced.  A new
	 * delayed-select issued by that current word is therefore for the NEXT cycle.
	 */
	uint8_t delayed_rom;
	uint8_t delayed_rom_pending;

	#if HPVM_WOODSTOCK_ROM_BANKS > 1
	uint8_t bank;
	#endif

	/*
	 * Keyboard token is the raw ACT hardware token (not an HP-21 function code).
	 * 0xff means that no token has ever been supplied.  Releasing the key clears
	 * only key_down; the token latch itself remains, as in the ACT.
	 *
	 * Real ACT hardware can also build the token from KA..KE scanner lines, with
	 * a second set conditionally enabled by S0.  Tiny RPN has no reason to emulate
	 * those pins: its model adapter performs that electrical scan externally and
	 * passes the resulting architectural token to KeyDown().
	 */
	uint8_t key_token;
	uint8_t key_down;

	/*
	 * Raw external ACT flag inputs.  Besides the ordinary F1/F2 levels, ACT has
	 * pulse inputs and versions gated by S0.  At the end of every word time:
	 *
	 *   S5 |= !(F1 | F1_pulse | (S0 && F1_cond_S0))
	 *   S3 |=  (F2 | F2_pulse | (S0 && F2_cond_S0))
	 *
	 * Pulses last one ACT word time.  S0 is also the hardware F0 output; FlagOut()
	 * exposes its current level to a model adapter without coupling the VM to any
	 * particular peripheral implementation.
	 */
	uint8_t ext_f1, ext_f2;
	uint8_t ext_f1_cond_s0, ext_f2_cond_s0;
	uint8_t ext_f1_pulse, ext_f2_pulse;

	// Display scan state.
	uint8_t disp_enable;
	uint8_t disp_14;
	uint8_t disp_scan;
	uint8_t disp_pos;
	uint8_t disp_changed;

	// Optional external data memory.  HP-21 compiles this section out completely.
	#if HPVM_WOODSTOCK_RAM_SIZE > 0
	using ram_reg = digit[WORD_DIGITS];
	ram_reg RAM[HPVM_WOODSTOCK_RAM_SIZE];
	#endif
	uint8_t ram_addr;

	// -------------------------------------------------------------------------
	// Small ROM tables
	// -------------------------------------------------------------------------
	// Woodstock's P operand encoding is non-linear.  These two tables are ACT
	// instruction-set maps, not HP-21-specific data.
	const uint8_t p_set_map[16] PROGMEM =
	{
		14, 4, 7, 8, 11, 2, 10, 12, 1, 3, 13, 6, 0, 9, 5, 14
	};
	const uint8_t p_test_map[16] PROGMEM =
	{
		4, 8, 12, 2, 9, 1, 6, 3, 1, 13, 5, 0, 11, 10, 7, 4
	};

	// Fixed field boundaries.  P and WP are handled separately because they use p.
	// Encoding: high nibble = first digit, low nibble = last digit.
	// field 2 XS=2..2, 3 X=0..2, 4 S=13..13, 5 M=3..12,
	//       6 W =0..13, 7 MS=3..13.
	const uint8_t field_bounds[6] PROGMEM = { 0x22, 0x02, 0xDD, 0x3C, 0x0D, 0x3D };

	// -------------------------------------------------------------------------
	// Basic register / ALU primitives
	// -------------------------------------------------------------------------
	__attribute__((noinline)) digit alu(digit x, digit y, uint8_t sub)
	{
		int8_t r;
		uint8_t base = decimal ? 10 : 16;
		if (sub)
		{
			r = int8_t(x) - int8_t(y) - int8_t(carry);
			if (r < 0) { r += base; carry = 1; }
			else carry = 0;
		}
		else
		{
			r = int8_t(x) + int8_t(y) + int8_t(carry);
			if (r >= base) { r -= base; carry = 1; }
			else carry = 0;
		}
		return digit(r);
	}

	__attribute__((noinline)) void reg_clr(reg r, uint8_t first, uint8_t last)
	{
		for (uint8_t i = first; i <= last; ++i) r[i] = 0;
	}

	__attribute__((noinline)) void reg_move(reg dst, reg src, uint8_t swap, uint8_t first, uint8_t last)
	{
		for (uint8_t i = first; i <= last; ++i)
		{
			digit t = dst[i];
			dst[i] = src[i];
			if (swap) src[i] = t;
		}
	}

	__attribute__((noinline)) void reg_math(reg dst, reg x, digit *y, uint8_t sub)
	{
		/*
		 * dst may be null for arithmetic comparison instructions.  The real ACT
		 * still performs the subtraction so carry is produced, but discards every
		 * result digit.  This is used by IF A>=C and IF A>=B.
		 */
		for (uint8_t i = ff; i <= fl; ++i)
		{
			digit r = alu(x ? x[i] : 0, y ? y[i] : 0, sub);
			if (dst) dst[i] = r;
		}
	}

	__attribute__((noinline)) void reg_test_nonzero(reg r)
	{
		for (uint8_t i = ff; i <= fl; ++i)
			carry |= (r[i] != 0);
	}

	__attribute__((noinline)) void reg_test_zero(reg r)
	{
		carry = 1;
		for (uint8_t i = ff; i <= fl; ++i)
			carry &= (r[i] == 0);
	}

	__attribute__((noinline)) void reg_shr(reg r)
	{
		for (uint8_t i = ff; i < fl; ++i) r[i] = r[i + 1];
		if (ff <= fl) r[fl] = 0;
	}

	void reg_shl(reg r)
	{
		if (ff > fl) return;
		for (int8_t i = fl; i > ff; --i) r[i] = r[i - 1];
		r[ff] = 0;
	}

	void circulate_a_left()
	{
		digit t = A[WORD_MSD];
		for (int8_t i = WORD_MSD; i > 0; --i) A[i] = A[i - 1];
		A[0] = t;
	}

	// -------------------------------------------------------------------------
	// Status helpers
	// -------------------------------------------------------------------------
	bool Status(uint8_t n)
	{
		return n < 16 && (status & (uint16_t(1) << n));
	}

	void set_status(uint8_t n, uint8_t value)
	{
		uint16_t mask = uint16_t(1) << n;
		if (value) status |= mask;
		else status &= ~mask;
	}

	// -------------------------------------------------------------------------
	// External RAM helpers
	// -------------------------------------------------------------------------
	__attribute__((noinline)) void data_to_c()
	{
		#if HPVM_WOODSTOCK_RAM_SIZE > 0
		if (ram_addr < HPVM_WOODSTOCK_RAM_SIZE)
		{
			for (uint8_t i = 0; i < WORD_DIGITS; ++i) C[i] = RAM[ram_addr][i];
			return;
		}
		#endif
		// A non-existent selected data register reads as zero in the reference
		// simulators.  This also gives sensible behaviour for the RAM-less HP-21.
		reg_clr(C, 0, WORD_MSD);
	}

	__attribute__((noinline)) void c_to_data()
	{
		#if HPVM_WOODSTOCK_RAM_SIZE > 0
		if (ram_addr < HPVM_WOODSTOCK_RAM_SIZE)
			for (uint8_t i = 0; i < WORD_DIGITS; ++i) RAM[ram_addr][i] = C[i];
		#endif
	}

	void clear_data_regs()
	{
		#if HPVM_WOODSTOCK_RAM_SIZE > 0
		uint16_t base = ram_addr & 0xF0;
		for (uint16_t r = base; r < base + 16; ++r)
			if (r < HPVM_WOODSTOCK_RAM_SIZE)
				for (uint8_t i = 0; i < WORD_DIGITS; ++i) RAM[r][i] = 0;
		#endif
	}

	// -------------------------------------------------------------------------
	// Display scanner
	// -------------------------------------------------------------------------
	bool display_cycle()
	{
		/*
		 * Normal Woodstock scans A/B digits 13..2 into twelve display positions.
		 * RESET TWF extends the scan through A/B digit 0 and reserves an extra
		 * leftmost position for the mantissa sign.  Thus 14-digit scan mode exposes
		 * fifteen normalized positions: one special sign + fourteen scanned digits.
		 */
		uint8_t right = disp_14 ? 0 : 2;
		uint8_t size  = disp_14 ? 15 : 12;
		if (DisplaySize != size)
		{
			DisplaySize = size;
			disp_changed = 1;
		}

		// In TWF mode position zero is not sourced by A[13].  It is a separate
		// mantissa-sign position, so the first scanned digit starts at Display[1].
		if (disp_14 && disp_pos == 0) ++disp_pos;

		uint8_t code = HPVM_SPACE;
		uint8_t dot = 0;
		if (disp_enable)
		{
			uint8_t a = A[disp_scan];
			uint8_t b = B[disp_scan];
			if (b & 2)
			{
				// B bit 1 requests a sign/blank rather than the normal character.
				code = ((a >= 2) && ((a & 7) != 7)) ? HPVM_DASH : HPVM_SPACE;
			}
			else code = a & 0x0F;
			dot = b & 1;

			/*
			 * In 14-digit mode, when the exponent-sign source reaches display
			 * position 12, the anode driver steals its E segment for the separate
			 * mantissa sign and leaves only its G segment at the exponent-sign digit.
			 * Character-to-segment decoding is model-specific, so expose two special
			 * output codes plus the original source character instead of guessing it.
			 */
			if (disp_14 && disp_pos == 12)
			{
				if (DisplayTWFSource != code)
				{
					DisplayTWFSource = code;
					disp_changed = 1;
				}
				if (Display[0] != HPVM_TWF_MANTISSA_SIGN)
				{
					Display[0] = HPVM_TWF_MANTISSA_SIGN;
					disp_changed = 1;
				}
				DisplayDots &= ~uint16_t(1); // the separate mantissa-sign slot has no DP
				code = HPVM_TWF_EXPONENT_SIGN;
			}
		}
		else if (disp_14 && disp_pos == 12)
		{
			// With the display disabled both special TWF-derived positions are blank.
			if (DisplayTWFSource != HPVM_SPACE)
			{
				DisplayTWFSource = HPVM_SPACE;
				disp_changed = 1;
			}
			if (Display[0] != HPVM_SPACE)
			{
				Display[0] = HPVM_SPACE;
				disp_changed = 1;
			}
			DisplayDots &= ~uint16_t(1);
		}

		if (Display[disp_pos] != code)
		{
			Display[disp_pos] = code;
			disp_changed = 1;
		}

		uint16_t mask = uint16_t(1) << disp_pos;
		uint8_t old_dot = (DisplayDots & mask) != 0;
		if (dot) DisplayDots |= mask;
		else DisplayDots &= ~mask;
		if (old_dot != dot) disp_changed = 1;

		++disp_pos;
		if (disp_scan == right)
		{
			// Clear any positions that are outside the newly completed frame.
			for (uint8_t i = size; i < 15; ++i)
			{
				if (Display[i] != HPVM_SPACE)
				{
					Display[i] = HPVM_SPACE;
					disp_changed = 1;
				}
				DisplayDots &= ~(uint16_t(1) << i);
			}
			disp_scan = WORD_MSD;
			disp_pos = 0;
			uint8_t changed = disp_changed;
			disp_changed = 0;
			return changed;
		}
		--disp_scan;
		return false;
	}

	// -------------------------------------------------------------------------
	// Public state / I/O helpers
	// -------------------------------------------------------------------------
	void KeyDown(uint8_t token)
	{
		key_token = token;
		key_down = 1;
	}

	void KeyUp()
	{
		key_down = 0;
	}

	void ExternalF1(bool state) { ext_f1 = state; }
	void ExternalF2(bool state) { ext_f2 = state; }
	void ExternalF1ConditionalS0(bool state) { ext_f1_cond_s0 = state; }
	void ExternalF2ConditionalS0(bool state) { ext_f2_cond_s0 = state; }
	void PulseF1() { ext_f1_pulse = 1; }
	void PulseF2() { ext_f2_pulse = 1; }
	bool FlagOut() { return Status(0); }

	uint16_t Address()     { return pc & 0x0FFF; }
	uint16_t LastAddress() { return last_pc & 0x0FFF; }

	bool Idling() { return HPVM_WOODSTOCK_IDLE_CONDITION; }
	bool Error()  { return HPVM_WOODSTOCK_ERROR_CONDITION; }

	/*
	 * Reset follows the ACT reset sequence used by the reference implementation,
	 * rather than "memset absolutely all VM state".  A/B/C/Y/Z/T, status and the
	 * control state are reset; M1/M2/F and the two return-stack storage words are
	 * not explicitly cleared by the hardware reset sequence.  ret_sp=0 resets the
	 * stack position; the storage words themselves are intentionally left untouched.
	 *
	 * External pins are not CPU storage.  In particular a physical DEG/RAD switch
	 * must survive a CPU reset, so F2 and the S0-gated external levels are left as
	 * supplied by the host.  Nonpareil explicitly forces ordinary F1 low on a
	 * Woodstock reset (the battery-OK condition), which we reproduce here.  Pulse
	 * latches are host-side one-word events and are discarded on Reset().
	 */
	void Reset()
	{
		reg_clr(A, 0, WORD_MSD); reg_clr(B, 0, WORD_MSD); reg_clr(C, 0, WORD_MSD);
		reg_clr(Y, 0, WORD_MSD); reg_clr(Z, 0, WORD_MSD); reg_clr(T, 0, WORD_MSD);
		status = 0;
		p = ff = fl = 0;
		decimal = 1;
		carry = prev_carry = 0;
		pc = last_pc = 0;
		ret_sp = 0;
		then_goto = 0;
		delayed_rom = delayed_rom_pending = 0;
		#if HPVM_WOODSTOCK_ROM_BANKS > 1
		bank = 0;
		#endif
		key_token = 0xFF;
		key_down = 0;
		ext_f1 = 0; // Woodstock reset convention: force ordinary F1 low (battery OK)
		ext_f1_pulse = ext_f2_pulse = 0;
		disp_enable = 0;
		disp_14 = 0;
		disp_scan = WORD_MSD;
		disp_pos = 0;
		disp_changed = 1;
		DisplayDots = 0;
		DisplaySize = 12;
		DisplayTWFSource = HPVM_SPACE;
		for (uint8_t i = 0; i < 15; ++i) Display[i] = HPVM_SPACE;
	}

	// -------------------------------------------------------------------------
	// 10-bit ROM fetch
	// -------------------------------------------------------------------------
	/*
	 * ACT has a 12-bit architectural address space (0x000..0xfff), but the
	 * built-in HP-21 image contains one 1024-word ROM/display chip and therefore
	 * only addresses 0x000..0x3ff.  Correct HP-21 firmware never fetches outside
	 * that range.  We deliberately do not mask the architectural PC down to ten
	 * bits here: an external model may supply the full 4096-word ACT space.
	 */
	uint16_t fetch_word(uint16_t address)
	{
		#if HPVM_WOODSTOCK_ROM_BANKS > 1
		uint16_t physical = (uint16_t(bank) << 12) | (address & 0x0FFF);
		#else
		uint16_t physical = address & 0x0FFF;
		#endif
		uint8_t lo = pgm_read_byte(rom_l + physical);
		uint8_t hi = (pgm_read_byte(rom_h + (physical >> 2)) >> ((physical & 3) << 1)) & 3;
		return uint16_t(lo) | (uint16_t(hi) << 8);
	}

	// -------------------------------------------------------------------------
	// Arithmetic instructions (type ..10)
	// -------------------------------------------------------------------------
	void arithmetic(uint16_t opcode)
	{
		uint8_t field = (opcode >> 2) & 7;
		if (field == 0) // P
		{
			ff = p;
			fl = p < 14 ? p : 0; // p==14 makes the field empty
		}
		else if (field == 1) // WP
		{
			ff = 0;
			fl = p < 14 ? p : 13;
		}
		else
		{
			uint8_t bounds = pgm_read_byte(field_bounds + field - 2);
			ff = bounds >> 4;
			fl = bounds & 0x0F;
		}

		carry = 0;
		switch (opcode >> 5)
		{
			case 0x00: // 0 -> A[f]
				reg_clr(A, ff, fl); break;
			case 0x01: // 0 -> B[f]
				reg_clr(B, ff, fl); break;
			case 0x02: // A <-> B[f]
				reg_move(A, B, 1, ff, fl); break;
			case 0x03: // A -> B[f]
				reg_move(B, A, 0, ff, fl); break;
			case 0x04: // A <-> C[f]
				reg_move(A, C, 1, ff, fl); break;
			case 0x05: // C -> A[f]
				reg_move(A, C, 0, ff, fl); break;
			case 0x06: // B -> C[f]
				reg_move(C, B, 0, ff, fl); break;
			case 0x07: // B <-> C[f]
				reg_move(B, C, 1, ff, fl); break;
			case 0x08: // 0 -> C[f]
				reg_clr(C, ff, fl); break;
			case 0x09: // A + B -> A[f]
				reg_math(A, A, B, 0); break;
			case 0x0A: // A + C -> A[f]
				reg_math(A, A, C, 0); break;
			case 0x0B: // C + C -> C[f]
				reg_math(C, C, C, 0); break;
			case 0x0C: // A + C -> C[f]
				reg_math(C, A, C, 0); break;
			case 0x0D: // A + 1 -> A[f]
				carry = 1; reg_math(A, A, 0, 0); break;
			case 0x0E: // shift left A[f]
				reg_shl(A); break;
			case 0x0F: // C + 1 -> C[f]
				carry = 1; reg_math(C, C, 0, 0); break;
			case 0x10: // A - B -> A[f]
				reg_math(A, A, B, 1); break;
			case 0x11: // A - C -> C[f]
				reg_math(C, A, C, 1); break;
			case 0x12: // A - 1 -> A[f]
				carry = 1; reg_math(A, A, 0, 1); break;
			case 0x13: // C - 1 -> C[f]
				carry = 1; reg_math(C, C, 0, 1); break;
			case 0x14: // 0 - C -> C[f]
				reg_math(C, 0, C, 1); break;
			case 0x15: // 0 - C - 1 -> C[f]
				carry = 1; reg_math(C, 0, C, 1); break;
			case 0x16: // IF B[f] = 0 ; next word is THEN-GOTO literal
				then_goto = 1; reg_test_nonzero(B); break;
			case 0x17: // IF C[f] = 0
				then_goto = 1; reg_test_nonzero(C); break;
			case 0x18: // IF A >= C[f]
				then_goto = 1; reg_math(0, A, C, 1); break;
			case 0x19: // IF A >= B[f]
				then_goto = 1; reg_math(0, A, B, 1); break;
			case 0x1A: // IF A[f] != 0
				then_goto = 1; reg_test_zero(A); break;
			case 0x1B: // IF C[f] != 0
				then_goto = 1; reg_test_zero(C); break;
			case 0x1C: // A - C -> A[f]
				reg_math(A, A, C, 1); break;
			case 0x1D: // shift right A[f]
				reg_shr(A); break;
			case 0x1E: // shift right B[f]
				reg_shr(B); break;
			case 0x1F: // shift right C[f]
				reg_shr(C); break;
		}
	}

	// -------------------------------------------------------------------------
	// Miscellaneous instructions (type ..00)
	// -------------------------------------------------------------------------
	void misc(uint16_t opcode)
	{
		/*
		 * Fixed opcodes are checked first because several of them occupy encodings
		 * that otherwise look like an argument-bearing family (00070 is the most
		 * obvious example: DATA -> C overrides REGISTER 0 -> C).
		 *
		 * Octal literals are intentional: virtually all historical Woodstock
		 * listings and emulator sources use octal microcode notation.
		 */
		switch (opcode)
		{
			case 00000: // NOP
				return;
			case 00070: // DATA -> C (indirect RAM address)
				data_to_c(); return;
			case 00010: // CLEAR REGISTERS: A,B,C,Y,Z,T only; not M1/M2/F/P
				reg_clr(A,0,WORD_MSD); reg_clr(B,0,WORD_MSD); reg_clr(C,0,WORD_MSD);
				reg_clr(Y,0,WORD_MSD); reg_clr(Z,0,WORD_MSD); reg_clr(T,0,WORD_MSD); return;
			case 00110: // CLEAR STATUS; ACT preserves S1,S2,S5,S15
				status &= 0x8026; return;
			case 00210: // DISPLAY TOGGLE
				disp_enable = !disp_enable; return;
			case 00310: // DISPLAY OFF
				disp_enable = 0; return;
			case 00410: // M1 <-> C
				reg_move(M1,C,1,0,WORD_MSD); return;
			case 00510: // M1 -> C
				reg_move(C,M1,0,0,WORD_MSD); return;
			case 00610: // M2 <-> C
				reg_move(M2,C,1,0,WORD_MSD); return;
			case 00710: // M2 -> C
				reg_move(C,M2,0,0,WORD_MSD); return;
			case 01010: // STACK -> A: A=Y, Y=Z, Z=T; T unchanged
				reg_move(A,Y,0,0,WORD_MSD); reg_move(Y,Z,0,0,WORD_MSD); reg_move(Z,T,0,0,WORD_MSD); return;
			case 01110: // DOWN ROTATE: C<-Y<-Z<-T<-old C
				reg_move(C,Y,1,0,WORD_MSD); reg_move(Y,Z,1,0,WORD_MSD); reg_move(Z,T,1,0,WORD_MSD); return;
			case 01210: // Y -> A
				reg_move(A,Y,0,0,WORD_MSD); return;
			case 01310: // C -> STACK: T=Z, Z=Y, Y=C
				reg_move(T,Z,0,0,WORD_MSD); reg_move(Z,Y,0,0,WORD_MSD); reg_move(Y,C,0,0,WORD_MSD); return;
			case 01410: // DECIMAL arithmetic mode
				decimal = 1; return;
			case 01510: // architecturally unassigned on the common Woodstock ACT
				HPVM_WOODSTOCK_EXTENSION(opcode); return;
			case 01610: // F -> A[0]
				A[0] = f; return;
			case 01710: // F <-> A[0]
				{ digit t = A[0]; A[0] = f; f = t; } return;

			case 00020: // KEYS -> ROM ADDRESS (raw keyboard token)
				// ACT always clears PC[7:0] first.  With no latched key the low byte
				// therefore remains zero rather than retaining the previous address.
				pc &= 0x0F00;
				if (key_token != 0xFF) pc |= key_token;
				return;
			case 00120: // KEYS -> A: token into A[2:1]
				if (key_token == 0xFF) A[2] = A[1] = 0;
				else { A[2] = key_token >> 4; A[1] = key_token & 0x0F; }
				return;
			case 00220: // A -> ROM ADDRESS: computed goto using A[2:1]
				pc = (pc & 0x0F00) | (uint16_t(A[2]) << 4) | A[1]; return;
			case 00320: // RESET TWF: 14 scanned digits + separate mantissa-sign position
				disp_14 = 1; disp_scan = WORD_MSD; disp_pos = 0; disp_changed = 1; return;
			case 00420: // BINARY arithmetic mode (radix 16)
				decimal = 0; return;
			case 00520: // CIRCULATE A LEFT (whole 14-digit register)
				circulate_a_left(); return;
			case 00620: // P - 1 -> P, modulo 14
				p = p ? p - 1 : 13; return;
			case 00720: // P + 1 -> P, modulo 14
				if (++p >= 14) p = 0;
				return;
			case 01020: // RETURN from two-level microcode return stack
				ret_sp ^= 1; pc = ret_stack[ret_sp] & 0x0FFF; return;

			case 01060: // BANK SWITCH, used by later ACT-family machines
				#if HPVM_WOODSTOCK_ROM_BANKS > 1
				bank ^= 1;
				#else
				HPVM_WOODSTOCK_EXTENSION(opcode);
				#endif
				return;
			case 01160: // C -> DATA ADDRESS, address = C[1]:C[0]
				ram_addr = (C[1] << 4) | C[0]; return;
			case 01260: // CLEAR DATA REGISTERS in selected 16-register block
				clear_data_regs(); return;
			case 01360: // C -> DATA (indirect RAM address)
				c_to_data(); return;
			case 01460: // ROM SELFTEST belongs to later Spice, not base Woodstock
				HPVM_WOODSTOCK_EXTENSION(opcode); return;
			case 01760: // NOP in Nonpareil; traditionally labelled "HI I'M WOODSTOCK"
				return;
		}

		// Argument-bearing type-00 families.  The argument is opcode bits 9..6.
		uint8_t n = opcode >> 6;
		switch (opcode & 0x3F)
		{
			case 00004: // 1 -> S[n]
				set_status(n,1); break;
			case 00014: // 0 -> S[n]
				set_status(n,0); break;
			case 00024: // IF S[n] = 1 ; next word is THEN-GOTO literal
				then_goto = 1; carry = !Status(n); break;
			case 00030: // LOAD CONSTANT n -> C[P], then P-- modulo 14
				if (p < 14) C[p] = n;
				p = p ? p - 1 : 13;
				break;
			case 00034: // IF S[n] = 0
				then_goto = 1; carry = Status(n); break;
			case 00040: // SELECT ROM n immediately; preserve low 8 PC bits
				pc = (uint16_t(n) << 8) | (pc & 0x00FF); break;
			case 00044: // IF P = mapped(n)
				then_goto = 1;
				carry = !(p == pgm_read_byte(p_test_map + n));
				break;
			case 00050: // C -> REGISTER n (direct low-nibble RAM selection)
				ram_addr = (ram_addr & 0xF0) | n; c_to_data(); break;
			case 00054: // IF P != mapped(n)
				then_goto = 1;
				carry = (p == pgm_read_byte(p_test_map + n));
				break;
			case 00064: // DELAYED ROM n; applied after the NEXT ROM word
				delayed_rom = n; delayed_rom_pending = 1; break;
			case 00070: // REGISTER n -> C (n=0 encoding was overridden above)
				ram_addr = (ram_addr & 0xF0) | n; data_to_c(); break;
			case 00074: // mapped(n) -> P
				p = pgm_read_byte(p_set_map + n); break;
			default:
				// Printer/card-reader and other model-specific ACT-family opcodes.
				HPVM_WOODSTOCK_EXTENSION(opcode); break;
		}
	}

	// -------------------------------------------------------------------------
	// Execute one 56-bit ACT word time
	// -------------------------------------------------------------------------
	bool Cycle()
	{
		last_pc = pc & 0x0FFF;
		uint16_t opcode = fetch_word(last_pc);

		/*
		 * Save the state that belongs to the PREVIOUS word before changing it.
		 * This ordering is essential for both THEN-GOTO and DELAYED ROM.
		 */
		uint8_t prev_then = then_goto;
		then_goto = 0;

		prev_carry = carry;
		carry = 0;

		uint8_t apply_delayed = delayed_rom_pending;
		uint8_t apply_rom = delayed_rom;
		delayed_rom_pending = 0;

		pc = (pc + 1) & 0x0FFF;

		if (prev_then)
		{
			// The fetched 10-bit word is DATA, not an instruction.  Successful
			// tests are represented by prev_carry == 0.  It replaces bits 9..0,
			// while bits 11..10 remain from the current address context.
			if (!prev_carry) pc = (pc & 0x0C00) | (opcode & 0x03FF);
		}
		else
		{
			switch (opcode & 3)
			{
				case 0: // Miscellaneous
					misc(opcode); break;
				case 1: // JSB: push post-increment address, replace low 8 PC bits
					ret_stack[ret_sp] = pc;
					ret_sp ^= 1;
					pc = (pc & 0x0F00) | (opcode >> 2);
					break;
				case 2: // Arithmetic / test
					arithmetic(opcode); break;
				case 3: // GOTO: conditional on carry from the preceding word
					if (!prev_carry) pc = (pc & 0x0F00) | (opcode >> 2);
					break;
			}
		}

		// A delayed ROM select that was pending BEFORE this cycle takes effect
		// only now, after the current word (instruction or THEN-GOTO literal).
		if (apply_delayed)
			pc = (uint16_t(apply_rom) << 8) | (pc & 0x00FF);

		bool display_updated = display_cycle();

		/*
		 * Hardware-fed status bits are ORed in after every word time.  This matches
		 * the ACT inputs modelled by Nonpareil: ordinary level, one-word pulse, and
		 * an input enabled only while S0/F0 is asserted.  Pulse latches are consumed
		 * here and therefore last for exactly one ACT word time.
		 */
		uint8_t f1_state = ext_f1 | ext_f1_pulse | (Status(0) && ext_f1_cond_s0);
		uint8_t f2_state = ext_f2 | ext_f2_pulse | (Status(0) && ext_f2_cond_s0);
		if (!f1_state) set_status(5,1);
		if (f2_state)  set_status(3,1);
		ext_f1_pulse = ext_f2_pulse = 0;
		if (key_down) set_status(15,1);

		return display_updated;
	}
}
