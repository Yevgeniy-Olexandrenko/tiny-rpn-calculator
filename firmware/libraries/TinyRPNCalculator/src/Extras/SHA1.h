#pragma once

// -----------------------------------------------------------------------------
// SHA1
// -----------------------------------------------------------------------------

#define SHA1_BSIZE 64
#define SHA1_HSIZE 20

const uint8_t init_state[] PROGMEM =
{
    0x01, 0x23, 0x45, 0x67, // H0
    0x89, 0xab, 0xcd, 0xef, // H1
    0xfe, 0xdc, 0xba, 0x98, // H2
    0x76, 0x54, 0x32, 0x10, // H3
    0xf0, 0xe1, 0xd2, 0xc3  // H4
};

class SHA1
{
    // -----------------------------------------------------------------------------
    // Private constants
    // -----------------------------------------------------------------------------

    enum 
    {
        K00 = 0x5a827999,
        K20 = 0x6ed9eba1,
        K40 = 0x8f1bbcdc,
        K60 = 0xca62c1d6
    };

    enum
    {
        HMAC_IPAD = 0x36,
        HMAC_OPAD = 0x5c
    };

    // -----------------------------------------------------------------------------
    // Private variables
    // -----------------------------------------------------------------------------

    union Buffer {
        uint8_t  b[SHA1_BSIZE];
        uint32_t w[SHA1_BSIZE / sizeof(uint32_t)];
    };

    union State {
        uint8_t  b[SHA1_HSIZE];
        uint32_t w[SHA1_HSIZE / sizeof(uint32_t)];
    };

    Buffer   buffer;
    State    state;

    uint8_t  buff_offset;
    uint32_t byte_count;

    uint8_t  key_buffer[SHA1_BSIZE];
    uint8_t  inner_hash[SHA1_HSIZE];

    // -----------------------------------------------------------------------------
    // Private functions
    // -----------------------------------------------------------------------------

    uint32_t rol32(uint32_t number, uint8_t bits) 
    {
        return ((number << bits) | (number >> (32 - bits)));
    }

    void hash_block() 
    {
        uint32_t a = state.w[0];
        uint32_t b = state.w[1];
        uint32_t c = state.w[2];
        uint32_t d = state.w[3];
        uint32_t e = state.w[4];
        uint32_t t;

        for (uint8_t i = 0; i < 80; i++)
        {
            if (i >= 16)
            {
                t = buffer.w[(i + 13) & 15]
                  ^ buffer.w[(i + 8) & 15]
                  ^ buffer.w[(i + 2) & 15]
                  ^ buffer.w[i & 15];
                buffer.w[i & 15] = rol32(t, 1);
            }

            if (i < 20) 
                t = (d ^ (b & (c ^ d))) + K00;
            else if (i < 40) 
                t = (b ^ c ^ d) + K20;
            else if (i < 60) 
                t = ((b & c) | (d & (b | c))) +K40;
            else 
                t = (b ^ c ^ d) + K60;

            t += rol32(a, 5) + e + buffer.w[i & 15];
            e = d;
            d = c;
            c = rol32(b, 30);
            b = a;
            a = t;
        }

        state.w[0] += a;
        state.w[1] += b;
        state.w[2] += c;
        state.w[3] += d;
        state.w[4] += e;
    }

    void add_uncounted(uint8_t data) 
    {
        buffer.b[buff_offset ^ 3] = data;
        buff_offset++;

        if (buff_offset == SHA1_BSIZE) 
        {
            hash_block();
            buff_offset = 0;
        }
    }

    // -----------------------------------------------------------------------------
    // Public functions
    // -----------------------------------------------------------------------------

public:

    void Init()
    {
        memcpy_P(state.b, init_state, SHA1_HSIZE);
        buff_offset = 0;
        byte_count = 0;
    }

    uint8_t* Result()
    {
        // SHA-1 padding (fips180-2 §5.1.1) to complete the last block. 
        // Pad with 0x80 followed by 0x00 until the end of the block
        add_uncounted(0x80);
        while (buff_offset != SHA1_BSIZE - 8) add_uncounted(0x00);

        // Append length in the last 8 bytes
        add_uncounted(0); // We're only using 32 bit lengths
        add_uncounted(0); // But SHA-1 supports 64 bit lengths
        add_uncounted(0); // So zero pad the top bits
        add_uncounted(byte_count >> 29); // Shifting to multiply by 8
        add_uncounted(byte_count >> 21); // as SHA-1 supports bitstreams
        add_uncounted(byte_count >> 13); // as well as byte.
        add_uncounted(byte_count >> 5);
        add_uncounted(byte_count << 3);

        // Swap byte order back
        for (int i = 0; i < 5; i++)
        {
            uint32_t a, b;
            a = state.w[i];
            b = a << 24;
            b |= (a << 8) & 0x00ff0000;
            b |= (a >> 8) & 0x0000ff00;
            b |= a >> 24;
            state.w[i] = b;
        }

        // Return pointer to hash (20 characters)
        return state.b;
    }

    void InitHmac(const uint8_t* key, uint16_t key_length)
    {
        memset(key_buffer, 0, SHA1_BSIZE);
        if (key_length > SHA1_BSIZE)
        {
            // Hash long keys
            Init();
            Write(key, key_length);
            memcpy(key_buffer, Result(), SHA1_HSIZE);
        }
        else
        {
            // Block length keys are used as is
            memcpy(key_buffer, key, key_length);
        }

        // Start inner hash
        Init();
        for (uint8_t i = 0; i < SHA1_BSIZE; i++)
        {
            Write(key_buffer[i] ^ HMAC_IPAD);
        }
    }

    uint8_t* ResultHmac()
    {
        // Complete inner hash
        memcpy(inner_hash, Result(), SHA1_HSIZE);

        // Calculate outer hash
        Init();
        for (uint8_t i = 0; i < SHA1_BSIZE; i++)
        {
            Write(key_buffer[i] ^ HMAC_OPAD);
        }
        Write(inner_hash, SHA1_HSIZE);
        return Result();
    }

    void Write(uint8_t data) 
    {
        byte_count++;
        add_uncounted(data);
    }

    void Write(const uint8_t* buffer, uint16_t size)
    {
        while (size--) Write(*buffer++);
    }
};