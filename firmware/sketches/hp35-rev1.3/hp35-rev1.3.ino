// -----------------------------------------------------------------------------
// Emulator for HP-35 Calculator 
// -----------------------------------------------------------------------------

#define  PCB_REV 13
#define  ENABLE_OPT_DATAMEM  1
#define  ENABLE_OPT_NOINLINE 1

#include <TinyRPNCalculator.h>
#include <Extras/HPVMClassicV2.h>

#include "data.h"
#include "code.h"

// Reference compilation output:
// Sketch uses 7296 bytes (89%) of program storage space. Maximum is 8192 bytes.
// Global variables use 204 bytes (39%) of dynamic memory, leaving 308 bytes for local variables. Maximum is 512 bytes.

// New reference compilation output:
// Sketch uses 7140 bytes (87%) of program storage space. Maximum is 8192 bytes.
// Global variables use 202 bytes (39%) of dynamic memory, leaving 310 bytes for local variables. Maximum is 512 bytes.
