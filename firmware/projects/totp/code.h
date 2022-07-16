

// -----------------------------------------------------------------------------
// Enter point and main cycle
// -----------------------------------------------------------------------------

u08 hmacKey[] = { 0x4d, 0x79, 0x4c, 0x65, 0x67, 0x6f, 0x44, 0x6f, 0x6f, 0x72 };

TOTP totp(hmacKey, 10);

int main() 
{
    u32 timestamp = RTC::GetTimestamp();
    char* newCode = totp.GetCodeAsString(timestamp);

    return 0;
}