
// Firmware parts
#define SUPPORT_RTC 1

// Firmware configuration
#define DEBUG_ON_R1_0  1
#define FONT_IN_EEMEM  0
#define ALLOW_NOINLINE 0

// Defines depending on firmware configuration
#undef NOINLINE
#if ALLOW_NOINLINE
#define NOINLINE __attribute__ ((noinline))
#else
#define NOINLINE
#endif

// Includes
#include "lib_hardware.h"
#include "lib_print.h"
#include "lib_hp35vm.h"
#include "mainloop.h"
