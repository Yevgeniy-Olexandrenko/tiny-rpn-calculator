// -----------------------------------------------------------------------------
// Time-based one-time password for Two-factor Authentication
// -----------------------------------------------------------------------------

#define  PCB_REV 13
#define  ENABLE_OPT_NOINLINE 0
#include <TinyRPNCalculator.h>
#include <Extras/TOTP.h>

#include "data.h"
#include "code.h"

// Sketch uses 4224 bytes (51%) of program storage space. Maximum is 8192 bytes.
// Global variables use 215 bytes (41%) of dynamic memory, leaving 297 bytes for local variables. Maximum is 512 bytes.
