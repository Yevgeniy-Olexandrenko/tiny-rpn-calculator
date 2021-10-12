#pragma once

// -----------------------------------------------------------------------------
// Text fonts
// -----------------------------------------------------------------------------

const uint8_t font5x8[] PROGMEM =
{
//	-------------------------------
	0b00100010, // 2A *
	0b00010100,
	0b00001000,
	0b00010100,
	0b00100010,
//	-------------------------------
	0b00001000, // 2B +
	0b00001000,
	0b00111110,
	0b00001000,
	0b00001000,
//	-------------------------------
	0b00010000, // 2C , (squareroot)
	0b00100000,
	0b01111111,
	0b00000001,
	0b00000001,
//	-------------------------------
	0b00001000, // 2D -
	0b00001000,
	0b00001000,
	0b00001000,
	0b00001000,
//	-------------------------------
	0b00000000, // 2E .
	0b01100000,
	0b01100000,
	0b00000000,
	0b00000000,
//	-------------------------------
	0b00100000, // 2F /
	0b00010000,
	0b00001000,
	0b00000100,
	0b00000010,
//	-------------------------------
	0b01111111, // 30 0
	0b01000001,
	0b01000001,
	0b01000001,
	0b01111111,
//	-------------------------------
	0b00000000, // 31 1
	0b00000000,
	0b00000010,
	0b01111111,
	0b00000000,
//	-------------------------------
	0b01111001, // 32 2
	0b01001001,
	0b01001001,
	0b01001001,
	0b01001111,
//	-------------------------------
	0b01000001, // 33 3
	0b01001001,
	0b01001001,
	0b01001001,
	0b01111111,
//	-------------------------------
	0b00001111, // 34 4
	0b00001000,
	0b00001000,
	0b00001000,
	0b01111111,
//	-------------------------------
	0b01001111, // 35 5
	0b01001001,
	0b01001001,
	0b01001001,
	0b01111001,
//	-------------------------------
	0b01111111, // 36 6
	0b01001001,
	0b01001001,
	0b01001001,
	0b01111001,
//	-------------------------------
	0b00000011, // 37 7
	0b00000001,
	0b00000001,
	0b00000001,
	0b01111111,
//	-------------------------------
	0b01111111, // 38 8
	0b01001001,
	0b01001001,
	0b01001001,
	0b01111111,
//	-------------------------------
	0b01001111, // 39 9
	0b01001001,
	0b01001001,
	0b01001001,
	0b01111111,
//	-------------------------------
	0b00000000, // 3A :
	0b00110110,
	0b00110110,
	0b00000000,
	0b00000000,
//	-------------------------------
	0b00000000, // 3B ; (raised x)
	0b00011011,
	0b00000100,
	0b00011011,
	0b00000000,
//	-------------------------------
	0b00000000, // 3C < (play)
	0b01111111,
	0b00111110,
	0b00011100,
	0b00001000,
//	-------------------------------
	0b00000100, // 3D = (shift sign)
	0b01111110,
	0b01111111,
	0b01111110,
	0b00000100,
//	-------------------------------
	0b00001000, // 3E > (arrow to right)
	0b00001000,
	0b00111110,
	0b00011100,
	0b00001000,
//	-------------------------------
	0b00000000, // 3F ? (raised 2)
	0b00011101,
	0b00010101,
	0b00010111,
	0b00000000,
//	-------------------------------
	0b00011100, // 40 @ (record)
	0b00111110,
	0b00111110,
	0b00111110,
	0b00011100,
//	-------------------------------
	0b01111111, // 41 A
	0b00001001,
	0b00001001,
	0b00001001,
	0b01111111,
//	-------------------------------
	0b01111111, // 42 B
	0b01001001,
	0b01001001,
	0b01001111,
	0b01111000,
//	-------------------------------
	0b01111111, // 43 C
	0b01000001,
	0b01000001,
	0b01000001,
	0b01000000,
//	-------------------------------
	0b01111111, // 44 D
	0b01000001,
	0b01000001,
	0b01000001,
	0b00111110,
//	-------------------------------
	0b01111111, // 45 E
	0b01001001,
	0b01001001,
	0b01001001,
	0b01000001,
//	-------------------------------
	0b01111111, // 46 F
	0b00001001,
	0b00001001,
	0b00001001,
	0b00000001,
//	-------------------------------
	0b01111111, // 47 G
	0b01000001,
	0b01001001,
	0b01001001,
	0b01111001,
//	-------------------------------
	0b01111111, // 48 H
	0b00001000,
	0b00001000,
	0b00001000,
	0b01111111,
//	-------------------------------
	0b00000000, // 49 I
	0b01000001,
	0b01111111,
	0b01000001,
	0b00000000,
//	-------------------------------
	0b01000000, // 4A J
	0b01000000,
	0b01000000,
	0b01000000,
	0b01111111,
//	-------------------------------
	0b01111111, // 4B K
	0b00001000,
	0b00001000,
	0b00001111,
	0b01111000,
//	-------------------------------
	0b01111111, // 4C L
	0b01000000,
	0b01000000,
	0b01000000,
	0b01000000,
//	-------------------------------
	0b01111111, // 4D M
	0b00000100,
	0b00001000,
	0b00000100,
	0b01111111,
//	-------------------------------
	0b01111111, // 4E N
	0b00000100,
	0b00001000,
	0b00010000,
	0b01111111,
//	-------------------------------
	0b01111111, // 4F O
	0b01000001,
	0b01000001,
	0b01000001,
	0b01111111,
//	-------------------------------
	0b01111111, // 50 P
	0b00001001,
	0b00001001,
	0b00001001,
	0b00001111,
//	-------------------------------
	0b01111111, // 51 Q
	0b01000001,
	0b01110001,
	0b01000001,
	0b01111111,
//	-------------------------------
	0b01111111, // 52 R
	0b00001001,
	0b00011001,
	0b00101001,
	0b01001111,
//	-------------------------------
	0b01001111, // 53 S
	0b01001001,
	0b01001001,
	0b01001001,
	0b01111000,
//	-------------------------------
	0b00000001, // 54 T
	0b00000001,
	0b01111111,
	0b00000001,
	0b00000001,
//	-------------------------------
	0b01111111, // 55 U
	0b01000000,
	0b01000000,
	0b01000000,
	0b01111111,
//	-------------------------------
	0b00011111, // 56 V
	0b00100000,
	0b01000000,
	0b00100000,
	0b00011111,
//	-------------------------------
	0b01111111, // 57 W
	0b01000000,
	0b01110000,
	0b01000000,
	0b01111111,
//	-------------------------------
	0b01100011, // 58 X
	0b00010100,
	0b00001000,
	0b00010100,
	0b01100011,
//	-------------------------------
	0b00000011, // 59 Y
	0b00000100,
	0b01111000,
	0b00000100,
	0b00000011,
//	-------------------------------
	0b01110001, // 5A Z
	0b01010001,
	0b01011101,
	0b01000101,
	0b01000111,
//	-------------------------------
};

const TXT::Font font PROGMEM =
{
	.widthBytes = 5,
	.heightRows = 1,
	.asciiFirst = 0x2A,
	.asciiLast  = 0x5A,
	.data = font5x8
};

#define CHAR_SHIFT '='

// -----------------------------------------------------------------------------
// HP35 Operations (basic + extended)
// -----------------------------------------------------------------------------

#define FUNC_KEY  (HP35::OpNONE - 1)
#define MENU_MATH (HP35::OpNONE - 2)
#define MENU_TRIG (HP35::OpNONE - 3)
#define TRIG_ASIN (HP35::OpNONE - 4)
#define TRIG_ACOS (HP35::OpNONE - 5)
#define TRIG_ATAN (HP35::OpNONE - 6)

const uint8_t mainOps[16 + 16] PROGMEM =
{
	HP35::OpNUM0, HP35::OpNUM1, HP35::OpNUM2, HP35::OpNUM3, HP35::OpNUM4, HP35::OpNUM5, HP35::OpNUM6, HP35::OpNUM7,
	HP35::OpNUM8, HP35::OpNUM9, HP35::OpDOT,  HP35::OpPUSH, HP35::OpCLX,  HP35::OpCHS,  HP35::OpEEX,  FUNC_KEY,

	HP35::OpNONE, HP35::OpRCL,  HP35::OpSTO,  HP35::OpSUB,  HP35::OpPI,   HP35::OpNONE, HP35::OpMUL,  MENU_TRIG,
	HP35::OpNONE, HP35::OpDIV,  HP35::OpSWAP, HP35::OpADD,  HP35::OpCLR,  HP35::OpROT,  HP35::OpNONE, MENU_MATH
};

const uint8_t mathOps[6] PROGMEM =
{
	HP35::OpPOW, HP35::OpSQRT, HP35::OpINV,
	HP35::OpLOG, HP35::OpLN,   HP35::OpEXP
};

const uint8_t trigOps[6] PROGMEM =
{
	HP35::OpSIN, HP35::OpCOS,  HP35::OpTAN,
	TRIG_ASIN,   TRIG_ACOS,    TRIG_ATAN
};

const u08 seqASIN[] PROGMEM = { HP35::OpARC, HP35::OpSIN, HP35::OpNONE };
const u08 seqACOS[] PROGMEM = { HP35::OpARC, HP35::OpCOS, HP35::OpNONE };
const u08 seqATAN[] PROGMEM = { HP35::OpARC, HP35::OpTAN, HP35::OpNONE };

// -----------------------------------------------------------------------------
// String arrays
// -----------------------------------------------------------------------------

const char strMonth[] PROGMEM = 
	"\03"
	"JAN" "FEB" "MAR" "APR" "MAY" "JUN"
	"JUL" "AUG" "SEP" "OCT" "NOV" "DEC";

const char strMenuMath[] PROGMEM = 
	"\03"
	"Y; " "?,X" "1/X"  // X^Y, SQRT(X), 1/X
	"LOG" "LN " "E; "; // LOG(X), LN(X), e^X

const char strMenuTrig[] PROGMEM = 
	"\03"
	"SIN" "COS" "TAN"  // Sine, Cosine, Tangent
	"ASN" "ACS" "ATN"; // Inverse sine, Inverse cosine, Inverse tangent

// -----------------------------------------------------------------------------
// Menu structures
// -----------------------------------------------------------------------------

#define MENU_OPS_PER_LINE 3

struct Menu
{
	u08 lastIdx;
	const char * string;
	const uint8_t * opsBase;
};

const Menu menus[] PROGMEM =
{
	{ sizeof(mathOps) / MENU_OPS_PER_LINE - 1, strMenuMath, mathOps },
	{ sizeof(trigOps) / MENU_OPS_PER_LINE - 1, strMenuTrig, trigOps },
};

enum { MENU_MATH_OPS, MENU_TRIG_OPS };