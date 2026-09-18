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
// Sketch uses 6606 bytes (80%) of program storage space. Maximum is 8192 bytes.
// Global variables use 187 bytes (36%) of dynamic memory, leaving 325 bytes for local variables. Maximum is 512 bytes.
