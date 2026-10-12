// -----------------------------------------------------------------------------
// Emulator for HP-35 Calculator 
// -----------------------------------------------------------------------------

#define  ENABLE_OPT_DATAMEM  1
#define  ENABLE_OPT_NOINLINE 1

#include <TinyRPNCalculator.h>
#include <Extras/HP35Model.h>
#include <Extras/HPVMClassic.h>

#include "data.h"
#include "code.h"

// Reference compilation output:
// Sketch uses 5734 bytes (69%) of program storage space. Maximum is 8192 bytes.
// Global variables use 215 bytes (41%) of dynamic memory, leaving 297 bytes for local variables. Maximum is 512 bytes.
