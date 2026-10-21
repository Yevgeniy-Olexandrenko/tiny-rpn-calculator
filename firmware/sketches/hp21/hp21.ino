// -----------------------------------------------------------------------------
// Emulator for HP-21 Calculator
// -----------------------------------------------------------------------------

#define  ENABLE_OPT_DATAMEM  1
#define  ENABLE_OPT_NOINLINE 1
#define  PCB_REVISION        14 // 14 - PCB rev 1.3/1.4, 15 - PCB rev 1.5

#include <TinyRPNCalculator.h>
#include <Extras/HP21Model.h>
#include <Extras/HPVMWoodstock.h>

#include "data.h"
#include "code.h"

// Reference compilation output (PCB_REVISION 14, with 15 the sketch uses 6714 bytes):
// Sketch uses 6700 bytes (81%) of program storage space. Maximum is 8192 bytes.
// Global variables use 239 bytes (46%) of dynamic memory, leaving 273 bytes for local variables. Maximum is 512 bytes.
