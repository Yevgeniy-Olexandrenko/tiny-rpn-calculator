#pragma once

#include "SHA1.h"

class TOTP
{
    uint8_t* hmac_key;
    uint16_t hmac_key_len;
    uint8_t  time_step;

    SHA1 sha1;
    char code[7];

public:
    TOTP(uint8_t* hmacKey, uint16_t keyLength)
        : hmac_key(hmacKey)
        , hmac_key_len(keyLength)
        , time_step(30)
    {
    }

    TOTP(uint8_t* hmacKey, uint16_t keyLength, uint8_t timeStep)
        : hmac_key(hmacKey)
        , hmac_key_len(keyLength)
        , time_step(timeStep)
    {
    }

    uint32_t GetCodeAsNumber(uint32_t timeStamp)
    {
        uint32_t steps = (timeStamp / time_step);

        // STEP 0, map the number of steps in a 8-bytes array (counter value)
        uint8_t data[] =
        {
            0x00, 0x00, 0x00, 0x00,
            uint8_t(steps >> 24),
            uint8_t(steps >> 16),
            uint8_t(steps >> 8),
            uint8_t(steps)
        };

        // STEP 1, get the HMAC-SHA1 hash from counter and key
        sha1.InitHmac(hmac_key, hmac_key_len);
        sha1.Write(data, sizeof(data));
        uint8_t* hmac_hash = sha1.ResultHmac();

        // STEP 2, apply dynamic truncation to obtain a 4-bytes string
        uint8_t  hash_offset = hmac_hash[SHA1_HSIZE - 1] & 0x0F; 
        uint32_t code_number = 0;
        for (uint8_t i = 0; i < 4; ++i)
        {
            code_number <<= 8;
            code_number  |= hmac_hash[hash_offset + i];
        }

        // STEP 3, compute the OTP value
        code_number &= 0x7FFFFFFF;
        code_number %= 1000000;
        return code_number;
    }

    const char* GetCodeAsString(uint32_t timeStamp)
    {
        uint32_t code_number = GetCodeAsNumber(timeStamp);
        code[6] = 0;

        for (int8_t i = 5; i >= 0; --i)
        {
            code[i] = '0' + code_number % 10;
            code_number /= 10;
        }
        return code;
    }
};