#pragma once

#define DEBUG_ON_R1_0   1
#define FONT_IN_EEMEM   0
#define ENABLE_NOINLINE 0

#define FW_HARDWARE_TESTER   0
#define FW_CALCULATOR_NATIVE 1
#define FW_CALCULATOR_HP35   2

#include "firmware-config.h"

#if FIRMWARE == FW_HARDWARE_TESTER
    #include "src/lib_hardware.h"
    #include "src/lib_print.h"
    #include "src/tester/mainloop.h"

#elif FIRMWARE == FW_CALCULATOR_NATIVE
// TODO

#elif FIRMWARE == FW_CALCULATOR_HP35
    #include "src/lib_hardware.h"
//    #include "src/lib_print.h"
    #include "src/lib_hp35vm.h"
    #include "src/calculators/hp35/mainloop.h"

#endif
