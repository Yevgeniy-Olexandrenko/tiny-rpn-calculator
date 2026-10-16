// -----------------------------------------------------------------------------
// Emulator for HP-35 Calculator 
// -----------------------------------------------------------------------------

#define  ENABLE_OPT_DATAMEM  1
#define  ENABLE_OPT_NOINLINE 1

#include <TinyRPNCalculator.h>
#include <Extras/HP35Model.h>
#include <Extras/HPVMClassic.h>
#include <Extras/TOTP.h>

#include "data.h"
#include "totp.h"
#include "code.h"

// Reference compilation output:
// Sketch uses 8102 bytes (98%) of program storage space. Maximum is 8192 bytes.
// Global variables use 226 bytes (44%) of dynamic memory, leaving 286 bytes for local variables. Maximum is 512 bytes.
// EEPROM data (hp35.ino.eep) uses 512 of 512 bytes, 126 of them is the TOTP keys area.
