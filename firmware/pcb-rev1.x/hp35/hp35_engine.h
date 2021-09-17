#pragma once

namespace hp35
{
    #define HP35_NUM0 0x24
    #define HP35_NUM1 0x1C
    #define HP35_NUM2 0x1B
    #define HP35_NUM3 0x1A
    #define HP35_NUM4 0x14
    #define HP35_NUM5 0x13
    #define HP35_NUM6 0x12
    #define HP35_NUM7 0x34
    #define HP35_NUM8 0x33
    #define HP35_NUM9 0x32
    #define HP35_PUSH 0x3E // ENTER
    #define HP35_INV  0x0E // 1/x
    #define HP35_SIN  0x2B
    #define HP35_COS  0x2A
    #define HP35_TAN  0x28
    #define HP35_PI   0x22
    #define HP35_CHS  0x3B
    #define HP35_EEX  0x3A
    #define HP35_SWAP 0x0C // X<->Y
    #define HP35_ROT  0x0B // Rv
    #define HP35_STO  0x0A
    #define HP35_RCL  0x08
    #define HP35_POW  0x06  // X^Y
    #define HP35_LOG  0x04
    #define HP35_LN   0x03
    #define HP35_EXP  0x02  // e^x
    #define HP35_SQRT 0x2E
    #define HP35_ARC  0x2C
    #define HP35_ADD  0x16 // +
    #define HP35_SUB  0x36 // -
    #define HP35_DIV  0x26 // /
    #define HP35_MUL  0x1E // *
    #define HP35_DOT  0x23 // .
    #define HP35_CLR  0x00
    #define HP35_CLX  0x38

    // HP35-ROM, 768 words
    const uint8_t rom[] PROGMEM =
    { 
        0, 221, 2, 255, 2, 36,  0, 23,  1, 68,  2, 68,  0, 132, 1, 16,  2, 209, 3, 251, 0, 95,  0, 195, 1, 168, 3, 103, 2, 238, 3, 226,
        0, 46,  0, 144, 3, 234, 3, 234, 3, 234, 0, 107, 2, 105, 0, 168, 2, 168, 0, 255, 3, 234, 3, 234, 3, 234, 0, 48,  0, 204, 0, 170,
        1, 168, 0, 67,  1, 211, 0, 204, 0, 48,  0, 0,   0, 131, 1, 68,  0, 68,  0, 187, 2, 68,  0, 159, 2, 132, 3, 11,  0, 46,  0, 144,
        3, 40,  3, 111, 3, 234, 3, 234, 3, 234, 0, 75,  2, 103, 3, 168, 1, 113, 3, 119, 3, 203, 2, 206, 0, 196, 1, 219, 1, 40,  0, 52,
        2, 206, 3, 117, 1, 46,  2, 250, 1, 22,  3, 106, 3, 131, 1, 186, 3, 155, 3, 54,  3, 76,  3, 155, 0, 28,  1, 234, 0, 2,   1, 51,
        2, 196, 2, 214, 3, 166, 1, 20,  2, 31,  1, 125, 3, 119, 0, 210, 1, 114, 0, 218, 3, 138, 1, 119, 0, 206, 0, 52,  1, 142, 3, 12,
        1, 42,  1, 138, 1, 186, 1, 163, 0, 170, 1, 122, 1, 95,  1, 76,  3, 170, 1, 20,  1, 11,  3, 42,  0, 42,  3, 221, 1, 10,  2, 206,
        3, 44,  2, 39,  3, 178, 1, 235, 2, 209, 0, 144, 1, 20,  3, 219, 3, 178, 0, 250, 1, 142, 1, 186, 1, 255, 0, 218, 0, 170, 3, 76,
        1, 22,  1, 106, 2, 126, 1, 59,  2, 118, 2, 3,   0, 202, 3, 221, 2, 214, 1, 158, 3, 44,  2, 79,  0, 142, 1, 238, 0, 76,  1, 18,
        0, 60,  1, 162, 2, 63,  3, 174, 0, 236, 3, 231, 0, 202, 1, 132, 1, 235, 0, 254, 1, 168, 0, 46,  3, 250, 3, 250, 1, 250, 1, 250,
        0, 74,  2, 143, 3, 174, 3, 166, 1, 166, 2, 159, 3, 174, 2, 38,  0, 74,  2, 251, 2, 142, 3, 234, 0, 14,  2, 251, 2, 163, 2, 246,
        0, 212, 2, 211, 3, 126, 0, 254, 1, 212, 2, 223, 1, 40,  1, 196, 0, 206, 1, 110, 0, 190, 1, 254, 2, 46,  0, 48,  0, 144, 1, 113,
        1, 68,  3, 119, 2, 206, 1, 158, 2, 36,  3, 63,  1, 250, 2, 4,   1, 84,  3, 55,  1, 234, 3, 27,  0, 40,  0, 20,  3, 31,  0, 36,
        0, 28,  3, 44,  3, 67,  2, 40,  2, 20,  3, 51,  1, 14,  1, 100, 0, 208, 1, 40,  3, 174, 1, 117, 1, 196, 3, 221, 2, 189, 2, 43,
        2, 214, 0, 28,  0, 172, 1, 23,  3, 12,  2, 238, 2, 246, 3, 226, 3, 226, 0, 140, 0, 60,  3, 98,  3, 191, 0, 2,   3, 171, 3, 226,
        3, 46,  0, 48,  1, 4,   2, 212, 0, 115, 1, 191, 0, 254, 2, 164, 3, 15,  1, 148, 3, 243, 0, 28,  2, 146, 1, 233, 2, 168, 3, 111,
        3, 207, 3, 46,  0, 161, 1, 168, 0, 161, 1, 168, 2, 84,  0, 39,  3, 174, 1, 84,  0, 75,  0, 222, 2, 153, 1, 40,  2, 149, 2, 97,
        0, 149, 1, 168, 2, 153, 2, 148, 3, 107, 2, 238, 3, 226, 1, 38,  3, 166, 1, 106, 2, 146, 1, 186, 0, 103, 2, 210, 1, 234, 0, 119,
        2, 206, 2, 142, 1, 40,  2, 46,  1, 7,   2, 46,  1, 12,  3, 123, 1, 40,  3, 174, 1, 162, 0, 183, 0, 174, 1, 142, 0, 138, 3, 47,
        1, 142, 0, 84,  0, 151, 2, 148, 1, 183, 1, 84,  0, 87,  0, 254, 3, 190, 0, 55,  2, 146, 3, 126, 0, 235, 1, 254, 3, 50,  1, 210,
        3, 46,  1, 46,  3, 82,  0, 239, 1, 168, 2, 206, 3, 178, 3, 46,  1, 18,  1, 40,  3, 254, 3, 254, 0, 143, 0, 206, 0, 42,  2, 214,
        2, 201, 1, 98,  1, 168, 3, 174, 1, 12,  2, 145, 1, 140, 2, 109, 2, 12,  2, 109, 0, 140, 2, 24,  2, 140, 2, 109, 2, 57,  2, 109,
        3, 49,  1, 14,  2, 109, 0, 142, 3, 45,  3, 49,  2, 174, 2, 153, 2, 84,  1, 179, 0, 254, 2, 97,  0, 100, 0, 206, 1, 98,  1, 234,
        0, 84,  2, 151, 2, 153, 3, 49,  2, 174, 2, 149, 3, 49,  2, 174, 2, 174, 2, 85,  2, 174, 3, 173, 3, 49,  2, 140, 2, 113, 2, 57,
        2, 12,  2, 117, 0, 140, 2, 24,  1, 140, 2, 113, 1, 12,  2, 113, 2, 113, 3, 46,  2, 78,  3, 76,  1, 88,  3, 239, 1, 140, 2, 24,
        1, 152, 1, 88,  0, 152, 1, 24,  2, 88,  0, 84,  3, 107, 0, 48,  2, 238, 3, 226, 0, 16,  1, 16,  1, 14,  2, 150, 2, 46,  2, 135,
        1, 254, 3, 14,  2, 131, 3, 142, 1, 16,  1, 16,  1, 74,  1, 16,  1, 226, 3, 78,  2, 163, 3, 206, 1, 14,  0, 28,  2, 82,  0, 44,
        2, 167, 0, 183, 1, 226, 3, 22,  2, 203, 3, 150, 1, 22,  0, 28,  0, 44,  2, 207, 0, 183, 0, 28,  3, 150, 3, 111, 0, 16,  1, 122,
        1, 122, 2, 234, 3, 94,  2, 126, 3, 27,  1, 16,  2, 6,   3, 43,  0, 254, 3, 46,  3, 14,  1, 16,  0, 206, 2, 204, 1, 216, 2, 24,
        1, 88,  0, 216, 2, 88,  2, 24,  0, 88,  1, 152, 0, 216, 1, 88,  3, 12,  0, 48,  0, 16,  3, 138, 3, 123, 1, 98,  1, 254, 0, 44,
        2, 239, 3, 170, 2, 234, 0, 98,  3, 155, 2, 206, 2, 78,  2, 42,  0, 202, 3, 12,  2, 187, 1, 16,  2, 146, 2, 146, 1, 126, 3, 179,
        1, 210, 3, 18,  2, 50,  0, 142, 3, 126, 3, 187, 3, 178, 1, 168, 0, 30,  0, 7,   1, 14,  3, 178, 1, 40,  2, 146, 1, 126, 2, 62,
        0, 16,  3, 62,  3, 254, 2, 86,  1, 18,  0, 75,  1, 168, 2, 153, 1, 142, 2, 20,  1, 11,  2, 238, 3, 70,  0, 3,   2, 206, 1, 126,
        0, 3,   1, 254, 1, 46,  2, 89,  3, 98,  0, 71,  3, 50,  3, 158, 0, 7,   1, 204, 1, 181, 2, 12,  2, 117, 2, 76,  2, 113, 3, 249,
        2, 140, 2, 113, 1, 245, 2, 204, 2, 113, 3, 125, 2, 113, 2, 229, 2, 113, 3, 217, 3, 174, 1, 78,  0, 26,  0, 191, 1, 78,  3, 46,
        0, 28,  1, 14,  0, 108, 0, 195, 3, 174, 1, 190, 0, 227, 0, 230, 1, 234, 2, 204, 3, 21,  2, 84,  0, 27,  1, 84,  2, 83,  3, 217,
        2, 157, 2, 83,  3, 217, 3, 177, 2, 229, 2, 204, 2, 109, 3, 125, 2, 140, 2, 109, 1, 245, 2, 76,  2, 109, 3, 249, 2, 12,  2, 109,
        2, 109, 2, 109, 1, 140, 2, 242, 3, 76,  2, 46,  3, 174, 1, 152, 2, 59,  0, 148, 1, 123, 3, 234, 2, 122, 3, 11,  3, 22,  1, 103,
        3, 150, 1, 14,  1, 106, 1, 115, 2, 206, 0, 210, 3, 170, 1, 190, 1, 179, 3, 46,  3, 14,  0, 238, 2, 206, 2, 46,  0, 206, 1, 102,
        0, 148, 1, 219, 1, 24,  1, 230, 1, 231, 1, 152, 0, 108, 1, 215, 2, 78,  2, 78,  0, 148, 2, 83,  0, 48,  1, 204, 0, 216, 0, 216,
        0, 24,  2, 24,  1, 88,  0, 24,  2, 88,  3, 171, 2, 89,  3, 226, 1, 46,  1, 126, 2, 27,  2, 210, 3, 174, 1, 22,  3, 174, 3, 126,
        2, 35,  3, 46,  3, 226, 3, 49,  0, 144, 2, 210, 3, 126, 2, 87,  2, 254, 3, 142, 0, 48,  0, 144, 2, 206, 2, 46,  2, 131, 3, 142,
        1, 126, 2, 127, 3, 174, 1, 22,  3, 174, 1, 183, 0, 204, 1, 202, 1, 94,  2, 175, 0, 190, 3, 38,  2, 238, 3, 44,  3, 23,  0, 102,
        2, 219, 0, 84,  0, 3,   0, 146, 3, 102, 1, 250, 2, 50,  3, 166, 0, 144, 2, 36,  1, 152, 2, 88,  0, 216, 0, 88,  1, 24,  1, 216,
        0, 88,  3, 155, 3, 230, 1, 147, 3, 142, 1, 98,  3, 19,  2, 206, 0, 60,  3, 108, 3, 23,  1, 234, 2, 254, 3, 12,  0, 46,  2, 98,
        3, 91,  1, 14,  1, 106, 2, 110, 3, 63,  0, 206, 1, 42,  3, 142, 2, 126, 3, 31,  3, 166, 1, 142, 0, 46,  3, 12,  1, 235, 2, 76,
        0, 216, 0, 88,  0, 24,  0, 88,  1, 216, 2, 88,  2, 24,  0, 24,  1, 88,  1, 88,  0, 216, 3, 119, 3, 174, 1, 46,  1, 134, 2, 186,
        1, 123, 1, 250, 2, 206, 1, 234, 3, 203, 1, 159, 0, 206, 3, 12,  0, 152, 0, 216, 0, 24,  0, 152, 1, 88,  2,  7,  1, 76,  1, 251
    };

    #define SSIZE 12
    #define WSIZE 14
    #define ZERO  0

    static uint8_t a[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // A register
    static uint8_t b[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // B register
    static uint8_t c[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // C register (X)
    static uint8_t d[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // D register (Y)
    static uint8_t e[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // E register (Z)
    static uint8_t f[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // F register (T)
    static uint8_t m[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // M Scratchpad
    static uint8_t t[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // Temporary

    static uint8_t s[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};             // Status
    static uint8_t p = 0, pc = 0, ret = 0;                                   // Pointer
    static uint8_t offset = 0;                                               // ROM offset
    static uint8_t first, last;                                              // Register loop boundaries
    static uint8_t carry = 0, prevCarry = 0;                                 // Carry bits
    static uint8_t fetch_h, fetch_l, op_code = 0;                            // ROM fetch and operation code
    static uint8_t key_code = -1, key_rom = 0;                               // Key variables
    static bool    display_enable = true, update_display = true;             // Display control
    static bool    iserror = false;                                          // True if error

    // Add 2 nibbles
    uint8_t do_add(uint8_t x, uint8_t y)
    {
        int res = x + y + carry; // ???
        if (res > 9)
        {
            res -= 10;
            carry = 1;
        }
        else
            carry = 0;
        return ((uint8_t)res);
    }

    // Substract 2 nibbles
    uint8_t do_sub(uint8_t x, uint8_t y)
    {
        int res = x - y - carry; // ???
        if (res < 0)
        {
            res += 10;
            carry = 1;
        }
        else
            carry = 0;
        return ((uint8_t)res);
    }

    uint8_t fetch_index()
    {
        return (((fetch_h & 0x03) << 2) | ((fetch_l & 0xc0) >> 6));
    }

    // Process key with HP35-engine
    void Execute()
    {
        // Error handling
        if ((pc == 0xBF) & (offset == 0))
        {
            iserror = true;
        }

        // Fetch ROM
        prevCarry = carry;
        carry = 0;
        fetch_h = pgm_read_byte_near(rom + (offset * 256 * 2) + (pc * 2));
        fetch_l = pgm_read_byte_near(rom + (offset * 256 * 2) + (pc * 2) + 1);
        pc++;
        pc %= 256; // ???

        // Process received key
        if (key_code < 0xFF)
        {
            key_rom = key_code;
            key_code = 0xFF;
            s[0] = 1;
        }

        // NOP
        if (fetch_l == 0x00) { /* do nothing */ }

        // Jump subroutine
        if ((fetch_l & 0x03) == 0x01)
        {
            ret = pc;
            pc  = (fetch_l >> 2) & 0x3f;
            pc |= (fetch_h << 6) & 0xc0;
        }

        // Return from subroutine
        if ((fetch_l & 0x7F) == 0x30)
        {
            pc = ret;
        }

        // ROM select
        if ((fetch_l & 0x7F) == 0x10)
        {
            offset  = (fetch_h << 1) & 06;
            offset |= (fetch_l >> 7) & 01;
        }

        // Jump to pc + rom if key is available
        if (fetch_l == 0xD0)
        {
            pc = key_rom;
            s[0] = 0;
        }

        // Set carry due to status
        if ((fetch_l & 0x3f) == 0x14)
        {
            carry = s[fetch_index()];
        }

        // Set s
        if ((fetch_l & 0x3f) == 0x04)
        {
            s[fetch_index()] = 1;
        }

        // Clear s
        if ((fetch_l & 0x3f) == 0x24)
        {
            s[fetch_index()] = 0;
        }

        // Clear stati
        if ((fetch_l & 0x3f) == 0x34)
        {
            for (uint8_t i = 0; i <= 12; i++) s[i] = 0; // ???
        }

        // Set carry
        if ((fetch_l & 0x3f) == 0x2C)
        {
            carry = 0;
            if (p == fetch_index()) carry = 1;
        }

        // Set p
        if ((fetch_l & 0x3f) == 0x0C)
        {
            p = fetch_index();
        }
        if ((fetch_l & 0x3f) == 0x3C)
        {
            p += 0x01;
            p &= 0x0F;
        }
        if ((fetch_l & 0x3f) == 0x1C)
        {
            p -= 0x01;
            p &= 0x0F;
        }

        // Load constant
        if ((fetch_l & 0x3F) == 0x18)
        {
            c[p]  = (fetch_l >> 6);
            c[p] |= (fetch_h << 2);
            p -= 0x01;
            p &= 0x0F;
        }

        // c<->m
        if (((fetch_h & 0x03) == 0x00) && ((fetch_l & 0xef) == 0xA8))
        {
            for (uint8_t i = 0; i < WSIZE; i++)
            {
                int tmp = c[i]; c[i] = m[i]; m[i] = tmp;
            }
        }

        // c to stack
        if (((fetch_h & 0x03) == 0x01) && ((fetch_l & 0xef) == 0x28))
        {
            for (uint8_t i = 0; i < WSIZE; i++)
            {
                f[i] = e[i];
                e[i] = d[i];
                d[i] = c[i];
            }
        }

        // stack to a
        if (((fetch_h & 0x03) == 0x01) && ((fetch_l & 0xef) == 0xA8))
        {
            for (uint8_t i = 0; i < WSIZE; i++)
            {
                a[i] = d[i];
                d[i] = e[i];
                e[i] = f[i];
            }
        }

        // m to c
        if (((fetch_h & 0x03) == 0x02) && ((fetch_l & 0xef) == 0xA8))
        {
            for (uint8_t i = 0; i < WSIZE; i++) c[i] = m[i];
        }

        // Rotate down
        if (((fetch_h & 0x03) == 0x03) && ((fetch_l & 0xef) == 0x28))
        {
            for (uint8_t i = 0; i < WSIZE; i++)
            {
                int tmp = c[i];
                c[i] = d[i];
                d[i] = e[i];
                e[i] = f[i];
                f[i] = tmp;
            }
        }

        // Clear all register
        if (((fetch_h & 0x03) == 0x03) && ((fetch_l & 0xef) == 0xA8))
        {
            for (uint8_t i = 0; i < WSIZE; i++)
                a[i] = b[i] = c[i] = d[i] = e[i] = f[i] = m[i] = 0;
        }

        // No display
        if (((fetch_h & 0x03) == 0x00) && ((fetch_l & 0xef) == 0x28))
        {
            display_enable = false;
            update_display = false;
        }

        // Toggle display
        if (((fetch_h & 0x03) == 0x02) && ((fetch_l & 0xef) == 0x28))
        {
            display_enable = !display_enable;
            if (display_enable) update_display = true;
        }

        // Conditional branch
        if ((fetch_l & 0x03) == 0x03)
        {
            if (prevCarry != 1)
            {
                pc  = (fetch_l & 0xfc) >> 2;
                pc |= (fetch_h & 0x03) << 6;
            }
        }

        // A&R calculations due to opcode
        if ((fetch_l & 0x03) == 0x02)
        {
            op_code  = ((fetch_l >> 5) & 0x07);
            op_code |= ((fetch_h << 3) & 0x18);

            // Get register boundaries first/last
            switch ((fetch_l >> 2) & 0x07)
            {
            case 0: first = last = p; break;
            case 1: first = 3; last = 12; break;
            case 2: first = 0; last = 2; break;
            case 3: first = 0; last = 13; break;
            case 4: first = 0; last = p; break;
            case 5: first = 3; last = 13; break;
            case 6: first = last = 2; break;
            case 7: first = last = 13; break;
            }

            // Process opcode
            carry = 0;
            switch (op_code)
            {
            case 0x0: // 0+B
                for (uint8_t i = first; i <= last; i++)
                    if (b[i] != 0) carry = 1;
                break;

            case 0x01: // 0->B
                for (uint8_t i = first; i <= last; i++)
                    b[i] = 0;
                break;

            case 0x02: //  A-C
                for (uint8_t i = first; i <= last; i++)
                    t[i] = do_sub(a[i], c[i]);
                break;

            case 0x03: // C-1
                carry = 1;
                for (uint8_t i = first; i <= last; i++)
                    if (c[i] != 0) carry = 0;
                break;

            case 0x04: // B->C
                for (uint8_t i = first; i <= last; i++)
                    c[i] = b[i];
                break;

            case 0x05: // 0-C->C
                for (uint8_t i = first; i <= last; i++)
                    c[i] = do_sub(ZERO, c[i]);
                break;

            case 0x06: // 0->C
                for (uint8_t i = first; i <= last; i++)
                    c[i] = 0;
                break;

            case 0x07: // 0-C-1->C
                carry = 1;
                for (uint8_t i = first; i <= last; i++)
                    c[i] = do_sub(ZERO, c[i]);
                break;

            case 0x08: // Sh A Left
                for (int8_t i = last; i > first; i--)
                    a[i] = a[i - 1];
                a[first] = 0;
                break;

            case 0x09: // A->B
                for (uint8_t i = first; i <= last; i++)
                    b[i] = a[i];
                break;

            case 0x0a: // A-C->C
                for (uint8_t i = first; i <= last; i++)
                    c[i] = do_sub(a[i], c[i]);
                break;

            case 0x0b: // C-1->C
                carry = 1;
                for (uint8_t i = first; i <= last; i++)
                    c[i] = do_sub(c[i], ZERO);
                break;

            case 0x0c: // C->A
                for (uint8_t i = first; i <= last; i++)
                    a[i] = c[i];
                break;

            case 0x0d: // 0-C
                for (uint8_t i = first; i <= last; i++)
                    if (c[i] != 0) carry = 1;
                break;

            case 0x0e: // A+C->C
                for (uint8_t i = first; i <= last; i++)
                    c[i] = do_add(a[i], c[i]);
                break;

            case 0x0f: // C+1->C
                carry = 1;
                for (uint8_t i = first; i <= last; i++)
                    c[i] = do_add(c[i], ZERO);
                break;

            case 0x010: // A-B
                for (uint8_t i = first; i <= last; i++)
                    t[i] = do_sub(a[i], b[i]);
                break;

            case 0x011: // B<->C
                for (uint8_t i = first; i <= last; i++)
                {
                    uint8_t tmp = b[i]; b[i] = c[i]; c[i] = tmp;
                }
                break;

            case 0x012: // Sh C Right
                for (uint8_t i = first; i < last; i++)
                    c[i] = c[i + 1];
                c[last] = 0;
                break;

            case 0x013: // A-1
                carry = 1;
                for (uint8_t i = first; i <= last; i++)
                    if (a[i] != 0) carry = 0;
                break;

            case 0x014: // Sh B Right
                for (uint8_t i = first; i < last; i++)
                    b[i] = b[i + 1];
                b[last] = 0;
                break;

            case 0x015: // C+C->A
                for (uint8_t i = first; i <= last; i++)
                    c[i] = do_add(c[i], c[i]);
                break;

            case 0x016: // Sh A Right
                for (uint8_t i = first; i < last; i++)
                    a[i] = a[i + 1];
                a[last] = 0;
                break;

            case 0x017: // 0->A
                for (uint8_t i = first; i <= last; i++)
                    a[i] = 0;
                break;

            case 0x018: // A-B->A
                for (uint8_t i = first; i <= last; i++)
                    a[i] = do_sub(a[i], b[i]);
                break;

            case 0x019: // A<->B
                for (uint8_t i = first; i <= last; i++)
                {
                    uint8_t tmp = a[i]; a[i] = b[i]; b[i] = tmp;
                }
                break;

            case 0x01a: // A-C->A
                for (uint8_t i = first; i <= last; i++)
                    a[i] = do_sub(a[i], c[i]);
                break;

            case 0x01b: // A-1->A
                carry = 1;
                for (uint8_t i = first; i <= last; i++)
                    a[i] = do_sub(a[i], ZERO);
                break;

            case 0x01c: // A+B->A
                for (uint8_t i = first; i <= last; i++)
                    a[i] = do_add(a[i], b[i]);
                break;

            case 0x01d: // C<->A
                for (uint8_t i = first; i <= last; i++)
                {
                    uint8_t tmp = a[i]; a[i] = c[i]; c[i] = tmp;
                }
                break;

            case 0x01e: // A+C->A
                for (uint8_t i = first; i <= last; i++)
                    a[i] = do_add(a[i], c[i]);
                break;

            case 0x01f: // A+1->A
                carry = 1;
                for (uint8_t i = first; i <= last; i++)
                    a[i] = do_add(a[i], ZERO);
                break;
            }
        }

        // Display
        if (display_enable)
        {
            update_display = true;
        }
        else if (update_display)
        {
            printscreen(); // ???
            update_display = false;
        }
    }
}