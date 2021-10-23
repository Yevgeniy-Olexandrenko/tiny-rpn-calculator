#pragma once

#include "firmware-config.h"

// -----------------------------------------------------------------------------
// Hardware Tester
// -----------------------------------------------------------------------------

#if defined(FW_HARDWARE_TESTER)
    #include "src/lib_hardware.h"
    #include "src/tester/data.h"
    #include "src/tester/main.h"

// -----------------------------------------------------------------------------
// Native Calculator
// -----------------------------------------------------------------------------

#elif defined(FW_CALCULATOR_NATIVE)
    #include "src/lib_hardware.h"

// -----------------------------------------------------------------------------
// Emulator for HP-35 Calculator 
// -----------------------------------------------------------------------------

#elif defined(FW_CALCULATOR_HP35)
    //#define HPVM_HP45
    #include "src/lib_hardware.h"
    #include "src/lib_hpvm_classic.h"
    #include "src/calculators/hp35/data.h"
    #include "src/calculators/hp35/main.h"

#endif
