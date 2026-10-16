// -----------------------------------------------------------------------------
// Emulator for HP-21 Calculator
// -----------------------------------------------------------------------------

#define  ENABLE_OPT_DATAMEM  1
#define  ENABLE_OPT_NOINLINE 1

#include <TinyRPNCalculator.h>
#include <Extras/HP21Model.h>
#include <Extras/HPVMWoodstock.h>

#include "data.h"
#include "code.h"

// Reference compilation output:
// Sketch uses 6486 bytes (79%) of program storage space. Maximum is 8192 bytes.
// Global variables use 237 bytes (46%) of dynamic memory, leaving 275 bytes for local variables. Maximum is 512 bytes.
