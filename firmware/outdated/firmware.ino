#pragma once

#include "firmware.h"

// -----------------------------------------------------------------------------
// Hardware Tester
// -----------------------------------------------------------------------------

#if FW_HARDWARE_TESTER
    #include "src/lib_hardware.h"
    #include "src/fw_tester/data.h"
    #include "src/fw_tester/code.h"
#endif

// -----------------------------------------------------------------------------
// Emulator for HP-35 Calculator 
// -----------------------------------------------------------------------------

#if FW_CALCULATOR_HP35
    #define HPVM_HP35
    #include "src/lib_hardware.h"
    #include "src/lib_hpvm_classic.h"
    #include "src/fw_calc_hp35/data.h"
    #include "src/fw_calc_hp35/code.h"
#endif

// -----------------------------------------------------------------------------
// Time-based one-time password for Two-factor Authentication
// -----------------------------------------------------------------------------

#if FW_TOTP_AUTHENTICATE
    #include "src/lib_hardware.h"
    #include "src/fw_totp/data.h"
    #include "src/fw_totp/code.h"
#endif
