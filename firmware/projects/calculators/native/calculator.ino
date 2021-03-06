// Firmware configuration
#define DEBUG_ON_R1_0  0
#define FONT_IN_EEMEM  0
#define ALLOW_NOINLINE 0

#if ALLOW_NOINLINE
#undef  NOINLINE
#define NOINLINE __attribute__ ((noinline))
#else
#undef  NOINLINE
#define NOINLINE
#endif

#include "pcb_support.h"
#include "calc_basicmath.h"
#include "calc_printing.h"
#include "calc_operations.h"
#include "calc_mainloop.h"
