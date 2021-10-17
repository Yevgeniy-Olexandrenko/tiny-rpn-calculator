#pragma once

// -----------------------------------------------------------------------------
// Text fonts (menu, big digits, small digits)
// -----------------------------------------------------------------------------

const u08 menu5x8_bytes[] PROGMEM =
{
    // character '/' (0x2F)
    0x40, 0x20, 0x10, 0x08, 0x04,

    // character '0' (0x30)
    0x7E, 0x42, 0x42, 0x42, 0x7E,

    // character '1' (0x31)
    0x00, 0x02, 0x7E, 0x00, 0x00,

    // character '2' (0x32)
    0x72, 0x52, 0x52, 0x52, 0x5E,

    // character '3' (0x33)
    0x42, 0x4A, 0x4A, 0x4A, 0x7E,

    // character '4' (0x34)
    0x1E, 0x10, 0x10, 0x10, 0x7E,

    // character '5' (0x35)
    0x4E, 0x4A, 0x4A, 0x4A, 0x7A,

    // character '6' (0x36)
    0x7E, 0x4A, 0x4A, 0x4A, 0x7A,

    // character '7' (0x37)
    0x06, 0x02, 0x02, 0x02, 0x7E,

    // character '8' (0x38)
    0x7E, 0x4A, 0x4A, 0x4A, 0x7E,

    // character '9' (0x39)
    0x5E, 0x52, 0x52, 0x52, 0x7E,

    // character ':' (0x3A)
    0x00, 0x6C, 0x10, 0x6C, 0x00,

    // character ';' (0x3B)
    0x00, 0x5C, 0x50, 0x7C, 0x00,

    // character '<' (0x3C)
    0x36, 0x08, 0x36, 0x00, 0x00,

    // character '=' (0x3D)
    0x08, 0x08, 0x08, 0x08, 0x08,

    // character '>' (0x3E)
    0x2E, 0x28, 0x3E, 0x00, 0x00,

    // character '?' (0x3F)
    0x10, 0x20, 0x7E, 0x02, 0x02,

    // character '@' (0x40)
    0x00, 0x7C, 0x54, 0x5C, 0x00,

    // character 'A' (0x41)
    0x7E, 0x12, 0x12, 0x12, 0x7E,

    // character 'B' (0x42)
    0x7E, 0x4A, 0x4A, 0x4E, 0x78,

    // character 'C' (0x43)
    0x7E, 0x42, 0x42, 0x42, 0x42,

    // character 'D' (0x44)
    0x7E, 0x42, 0x42, 0x42, 0x3C,

    // character 'E' (0x45)
    0x7E, 0x4A, 0x4A, 0x4A, 0x42,

    // character 'F' (0x46)
    0x7E, 0x0A, 0x0A, 0x0A, 0x02,

    // character 'G' (0x47)
    0x7E, 0x42, 0x4A, 0x4A, 0x7A,

    // character 'H' (0x48)
    0x7E, 0x08, 0x08, 0x08, 0x7E,

    // character 'I' (0x49)
    0x42, 0x42, 0x7E, 0x42, 0x42,

    // character 'J' (0x4A)
    0x70, 0x40, 0x40, 0x40, 0x7E,

    // character 'K' (0x4B)
    0x7E, 0x08, 0x08, 0x14, 0x62,

    // character 'L' (0x4C)
    0x7E, 0x40, 0x40, 0x40, 0x40,

    // character 'M' (0x4D)
    0x7E, 0x02, 0x1E, 0x02, 0x7C,

    // character 'N' (0x4E)
    0x7E, 0x02, 0x02, 0x02, 0x7C,

    // character 'O' (0x4F)
    0x7E, 0x42, 0x42, 0x42, 0x7E,

    // character 'P' (0x50)
    0x7E, 0x12, 0x12, 0x12, 0x1E,

    // character 'Q' (0x51)
    0x7E, 0x42, 0x52, 0x62, 0x7E,

    // character 'R' (0x52)
    0x7E, 0x12, 0x12, 0x32, 0x5E,

    // character 'S' (0x53)
    0x4E, 0x4A, 0x4A, 0x4A, 0x7A,

    // character 'T' (0x54)
    0x02, 0x02, 0x7E, 0x02, 0x02,

    // character 'U' (0x55)
    0x7E, 0x40, 0x40, 0x40, 0x7E,

    // character 'V' (0x56)
    0x1E, 0x20, 0x40, 0x20, 0x1E,

    // character 'W' (0x57)
    0x7E, 0x20, 0x10, 0x20, 0x7E,

    // character 'X' (0x58)
    0x62, 0x14, 0x08, 0x14, 0x62,

    // character 'Y' (0x59)
    0x4E, 0x48, 0x48, 0x48, 0x7E,

    // character 'Z' (0x5A)
    0x62, 0x52, 0x4A, 0x4A, 0x46,
};

const TXT::Font menu5x8 PROGMEM =
{
    .bytesInRow  = 0x05,
    .rowsOfBytes = 0x01,
    .asciiFirst  = 0x2F,
    .asciiLast   = 0x5A,
    .bytes = menu5x8_bytes // 220 bytes
};

const uint8_t digits7x32_bytes[] PROGMEM =
{
    // character '-' (0x2D)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    // character '.' (0x2E)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x1C, 0x1C, 0x1C, 0x00, 0x00,

    // character '/' (0x2F)
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,

    // character '0' (0x30)
    0xFC, 0x02, 0x02, 0x02, 0x02, 0x02, 0xFC,
    0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7F,
    0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,
    0x1F, 0x20, 0x20, 0x20, 0x20, 0x20, 0x1F,

    // character '1' (0x31)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFC,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7F,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F,

    // character '2' (0x32)
    0x00, 0x02, 0x02, 0x02, 0x02, 0x02, 0xFC,
    0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x7F,
    0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x1F, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00,

    // character '3' (0x33)
    0x00, 0x02, 0x02, 0x02, 0x02, 0x02, 0xFC,
    0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x7F,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x20, 0x20, 0x20, 0x20, 0x20, 0x1F,

    // character '4' (0x34)
    0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFC,
    0x7F, 0x80, 0x80, 0x80, 0x80, 0x80, 0x7F,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F,

    // character '5' (0x35)
    0xFC, 0x02, 0x02, 0x02, 0x02, 0x02, 0x00,
    0x7F, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x20, 0x20, 0x20, 0x20, 0x20, 0x1F,

    // character '6' (0x36)
    0xFC, 0x02, 0x02, 0x02, 0x02, 0x02, 0x00,
    0x7F, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00,
    0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,
    0x1F, 0x20, 0x20, 0x20, 0x20, 0x20, 0x1F,

    // character '7' (0x37)
    0x00, 0x02, 0x02, 0x02, 0x02, 0x02, 0xFC,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7F,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F,

    // character '8' (0x38)
    0xFC, 0x02, 0x02, 0x02, 0x02, 0x02, 0xFC,
    0x7F, 0x80, 0x80, 0x80, 0x80, 0x80, 0x7F,
    0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,
    0x1F, 0x20, 0x20, 0x20, 0x20, 0x20, 0x1F,

    // character '9' (0x39)
    0xFC, 0x02, 0x02, 0x02, 0x02, 0x02, 0xFC,
    0x7F, 0x80, 0x80, 0x80, 0x80, 0x80, 0x7F,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x20, 0x20, 0x20, 0x20, 0x20, 0x1F,

    // character ':' (0x3A)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x07, 0x07, 0x07, 0x00, 0x00,
    0x00, 0x00, 0x70, 0x70, 0x70, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const TXT::Font digits7x32 PROGMEM =
{
    .bytesInRow  = 0x07,
    .rowsOfBytes = 0x04,
    .asciiFirst  = 0x2D,
    .asciiLast   = 0x3A,
    .bytes = digits7x32_bytes // 392 bytes
};

const u08 digits7x16_bytes[] PROGMEM =
{
    // character '-' (0x2D)
    0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    // character '.' (0x2E)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x18, 0x18, 0x18, 0x00, 0x00,

    // character '/' (0x2F)
    0xFE, 0x02, 0x02, 0x72, 0x72, 0xF2, 0xFE,
    0x3F, 0x20, 0x20, 0x3E, 0x3E, 0x3F, 0x3F,

    // character '0' (0x30)
    0x7C, 0x02, 0x02, 0x02, 0x02, 0x02, 0x7C,
    0x1F, 0x20, 0x20, 0x20, 0x20, 0x20, 0x1F,

    // character '1' (0x31)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7C,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F,

    // character '2' (0x32)
    0x00, 0x82, 0x82, 0x82, 0x82, 0x82, 0x7C,
    0x1F, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00,

    // character '3' (0x33)
    0x00, 0x82, 0x82, 0x82, 0x82, 0x82, 0x7C,
    0x00, 0x20, 0x20, 0x20, 0x20, 0x20, 0x1F,

    // character '4' (0x34)
    0x7C, 0x80, 0x80, 0x80, 0x80, 0x80, 0x7C,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F,

    // character '5' (0x35)
    0x7C, 0x82, 0x82, 0x82, 0x82, 0x82, 0x00,
    0x00, 0x20, 0x20, 0x20, 0x20, 0x20, 0x1F,

    // character '6' (0x36)
    0x7C, 0x82, 0x82, 0x82, 0x82, 0x82, 0x00,
    0x1F, 0x20, 0x20, 0x20, 0x20, 0x20, 0x1F,

    // character '7' (0x37)
    0x00, 0x02, 0x02, 0x02, 0x02, 0x02, 0x7C,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F,

    // character '8' (0x38)
    0x7C, 0x82, 0x82, 0x82, 0x82, 0x82, 0x7C,
    0x1F, 0x20, 0x20, 0x20, 0x20, 0x20, 0x1F,

    // character '9' (0x39)
    0x7C, 0x82, 0x82, 0x82, 0x82, 0x82, 0x7C,
    0x00, 0x20, 0x20, 0x20, 0x20, 0x20, 0x1F,

    // character ':' (0x3A)
    0xFE, 0x86, 0x02, 0x32, 0x32, 0x72, 0xFE,
    0x3F, 0x27, 0x27, 0x27, 0x20, 0x30, 0x3F,
};

const TXT::Font digits7x16 PROGMEM =
{
    .bytesInRow  = 0x07,
    .rowsOfBytes = 0x02,
    .asciiFirst  = 0x2D,
    .asciiLast   = 0x3A,
    .bytes = digits7x16_bytes // 196 bytes
};

// -----------------------------------------------------------------------------
// HP35 Operations (basic + extended)
// -----------------------------------------------------------------------------

#define KEY_FUNC  (HPVM::OpNONE -  1)
#define KEY_LAST  (HPVM::OpNONE -  2) //
#define KEY_ROTU  (HPVM::OpNONE -  3) //
#define KEY_MADD  (HPVM::OpNONE -  4) //
#define MENU_MATH (HPVM::OpNONE -  5)
#define MENU_TRIG (HPVM::OpNONE -  6)
#define MENU_PROG (HPVM::OpNONE -  7)
#define TRIG_ASIN (HPVM::OpNONE -  8)
#define TRIG_ACOS (HPVM::OpNONE -  9)
#define TRIG_ATAN (HPVM::OpNONE - 10)
#define PROG_TIME (HPVM::OpNONE - 11)
#define PROG_DATE (HPVM::OpNONE - 12)
#define PROG_YEAR (HPVM::OpNONE - 13)

const u08 mainOps[] PROGMEM =
{
	HPVM::OpNUM0, HPVM::OpNUM1, HPVM::OpNUM2, HPVM::OpNUM3, HPVM::OpNUM4, HPVM::OpNUM5, HPVM::OpNUM6, HPVM::OpNUM7,
	HPVM::OpNUM8, HPVM::OpNUM9, HPVM::OpDOT,  HPVM::OpPUSH, HPVM::OpCLX,  HPVM::OpCHS,  HPVM::OpEEX,  KEY_FUNC,

	KEY_LAST,     HPVM::OpRCL,  HPVM::OpSTO,  HPVM::OpSUB,  HPVM::OpPI,   KEY_MADD,     HPVM::OpMUL,  MENU_TRIG,
	MENU_PROG,    HPVM::OpDIV,  HPVM::OpSWAP, HPVM::OpADD,  HPVM::OpCLR,  HPVM::OpROT,  KEY_ROTU,     MENU_MATH
};

const u08 mathOps[] PROGMEM =
{
	HPVM::OpPOW, HPVM::OpSQRT, HPVM::OpINV,
	HPVM::OpLOG, HPVM::OpLN,   HPVM::OpEXP
};

const u08 trigOps[] PROGMEM =
{
	HPVM::OpSIN, HPVM::OpCOS,  HPVM::OpTAN,
	TRIG_ASIN,   TRIG_ACOS,    TRIG_ATAN
};

const u08 progOps[] PROGMEM =
{
	PROG_TIME, PROG_DATE, PROG_YEAR
};

// -----------------------------------------------------------------------------
// String arrays
// -----------------------------------------------------------------------------

const u08 strMonth[] PROGMEM = 
	"\03"
	"JAN" "FEB" "MAR" "APR" "MAY" "JUN"
	"JUL" "AUG" "SEP" "OCT" "NOV" "DEC";

const u08 strMenuMath[] PROGMEM = 
	"\06"
	" :>   " " ?:   " " 1/:  "  // X^Y, SQRT(X), 1/X
	" LOG  " " LN   " " @<   "; // LOG(X), LN(X), e^X

const u08 strMenuTrig[] PROGMEM = 
	"\06"
	" SIN  " " COS  " " TAN  "  // Sine, Cosine, Tangent
	" ASIN " " ACOS " " ATAN "; // Inverse sine, Inverse cosine, Inverse tangent

const u08 strMenuProg[] PROGMEM =
	"\06"
	" TIME " " DATE " " YEAR ";

// -----------------------------------------------------------------------------
// Menu structures
// -----------------------------------------------------------------------------

#define MENU_OPS_PER_LINE 3

struct Menu
{
	u08 lastIdx;
	const u08 * string;
	const u08 * opsBase;
};

const Menu menus[] PROGMEM =
{
	{ sizeof(mathOps) / MENU_OPS_PER_LINE - 1, strMenuMath, mathOps },
	{ sizeof(trigOps) / MENU_OPS_PER_LINE - 1, strMenuTrig, trigOps },
	{ sizeof(progOps) / MENU_OPS_PER_LINE - 1, strMenuProg, progOps },
};

enum { MENU_MATH_OPS, MENU_TRIG_OPS, MENU_PROG_OPS };

// -----------------------------------------------------------------------------
// Defines
// -----------------------------------------------------------------------------

#define CALC_FRAMES_PER_SEC   (15)
#define HP35_CYCLES_PER_FRAME (HP35_CYCLES_PER_SEC / CALC_FRAMES_PER_SEC)

#define INFO_FLAG_WIDTH       (7)
#define INFO_FLAG_POSITION    (128 - INFO_FLAG_WIDTH)
#define INFO_FLAG_FUNCTION    '/'
#define INFO_FLAG_STORAGE     ':'

#define DIMOUT_MILLIS         (10000)
#define POWEROFF_MILLIS       (20000)
