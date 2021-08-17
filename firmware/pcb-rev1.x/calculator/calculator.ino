// Firmware configuration
#define DEBUG_ON_R1_0  1
#define FONT_IN_EEMEM  0
#define SOFTWARE_I2C   0
#define ALLOW_NOINLINE 0

#if ALLOW_NOINLINE
#undef  NOINLINE
#define NOINLINE __attribute__ ((noinline))
#endif

#include "pcb_support.h"
#include "calc_basicmath.h"
#include "calc_printing.h"
#include "calc_operations.h"
#include "calc_mainloop.h"
