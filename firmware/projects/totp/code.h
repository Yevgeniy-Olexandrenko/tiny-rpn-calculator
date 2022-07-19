// -----------------------------------------------------------------------------
// Enter point and main cycle
// -----------------------------------------------------------------------------

#define DISPLAY_TIMESTAMP 0

u08  hmacKey[] = { 0xD5, 0xCA, 0xB3, 0x0A, 0x4D, 0xC2, 0x1F, 0x0C, 0x05, 0xFB };
TOTP totp(hmacKey, 10);

s32 timezone = +3 * 3600;

int main() 
{
    LCD::TurnOn();
    RTC::WriteTimeDate();
    FPS::SyncStart(FPS::TIMEOUT_15_FPS);

    while(true)
    {
        RTC::ReadTimeDate();
        u32 timestamp = RTC::GetTimestamp() - timezone;
        char* newCode = totp.GetCodeAsString(timestamp);

        LCD::Clear();
        TXT::SetFont(FNT::Font5x8);
#if DISPLAY_TIMESTAMP
        XT::SetScale(TXT::x1, TXT::x2);
        for (u08 x = 128 - 6; timestamp > 0; x -= 6)
        {
            char ch = '0' + timestamp % 10;
            timestamp /= 10;
            TXT::PrintChar(ch, x, 0);
        }
#else
        u08 seconds = BCD::Encode(30 - timestamp % 30);
        TXT::SetScale(TXT::x2, TXT::x4);
        TXT::PrintString(newCode, 0, 0);
        TXT::SetScale(TXT::x2, TXT::x2);
        TXT::PrintBCD(seconds, 128 - (5 * 2 + 1) * 2, 0);
#endif
        LCD::Flip();

        FPS::SyncWait();
    }

    return 0;
}