// -----------------------------------------------------------------------------
// Emulator for HP-35 Calculator 
// -----------------------------------------------------------------------------

#define  ENABLE_OPT_DATAMEM  1
#define  ENABLE_OPT_NOINLINE 1
#define  PCB_REVISION        14 // 14 - PCB rev 1.3/1.4, 15 - PCB rev 1.5

#include <TinyRPNCalculator.h>
#include <Extras/HP35Model.h>
#include <Extras/HPVMClassic.h>
#include <Extras/TOTP.h>

#include "data.h"
#include "totp.h"
#include "code.h"

// Reference compilation output (PCB_REVISION 14, with 15 the sketch uses 8170 bytes):
// Sketch uses 8156 bytes (99%) of program storage space. Maximum is 8192 bytes.
// Global variables use 310 bytes (60%) of dynamic memory, leaving 202 bytes for local variables. Maximum is 512 bytes.
// EEPROM data (hp35.ino.eep) uses 512 of 512 bytes, 126 of them is the TOTP keys area.
