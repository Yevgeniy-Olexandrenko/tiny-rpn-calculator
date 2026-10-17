#pragma once

// -----------------------------------------------------------------------------
// SHA-1 hash (FIPS 180-2) and HMAC-SHA1 (RFC 2104)
// Message length is limited to 65535 bytes, HMAC key length to 64 bytes.
// -----------------------------------------------------------------------------

namespace SHA1
{
	constexpr u08 BLOCK_SIZE = 64;
	constexpr u08 HASH_SIZE  = 20;

	constexpr u32 K00 = 0x5a827999;
	constexpr u32 K20 = 0x6ed9eba1;
	constexpr u32 K40 = 0x8f1bbcdc;
	constexpr u32 K60 = 0xca62c1d6;

	constexpr u08 HMAC_IPAD = 0x36;
	constexpr u08 HMAC_OPAD = 0x5c;

	const u08 init_state[] PROGMEM =
	{
		0x01, 0x23, 0x45, 0x67, // H0
		0x89, 0xab, 0xcd, 0xef, // H1
		0xfe, 0xdc, 0xba, 0x98, // H2
		0x76, 0x54, 0x32, 0x10, // H3
		0xf0, 0xe1, 0xd2, 0xc3  // H4
	};

	u08 offset; // write position in the block
	u16 length; // message length in bytes

	const u08* hmac_key;
	u08 hmac_key_len;

	// words are stored in the MCU's little endian order
	union { u08 b[HASH_SIZE];  u32 w[HASH_SIZE  / 4]; } state;
	union { u08 b[BLOCK_SIZE]; u32 w[BLOCK_SIZE / 4]; } block;

	// bit by bit rotation is slow, but much smaller on AVR
	NOINLINE
	u32 rol32(u32 x, u08 bits)
	{
		while (bits--) x = (x << 1) | (x >> 31);
		return x;
	}

	void hash_block()
	{
		// working variables a, b, c, d, e
		union { u08 b[HASH_SIZE]; u32 w[HASH_SIZE / 4]; } v;
		memcpy(v.w, state.w, HASH_SIZE);

		for (u08 i = 0; i < 80; ++i)
		{
			// message schedule is computed in place in the block,
			// bitwise operations are done byte by byte: smaller on AVR
			u08 wi = 4 * (i & 15);
			if (i >= 16)
			{
				u08 w2  = 4 * ((i + 2) & 15);
				u08 w8  = 4 * ((i + 8) & 15);
				u08 w13 = 4 * ((i + 13) & 15);
				for (u08 j = 0; j < 4; ++j)
				{
					block.b[wi + j] ^= block.b[w2 + j] ^ block.b[w8 + j] ^ block.b[w13 + j];
				}
				block.w[wi / 4] = rol32(block.w[wi / 4], 1);
			}

			union { u08 b[4]; u32 w; } f;
			for (u08 j = 0; j < 4; ++j)
			{
				u08 b = v.b[4 + j], c = v.b[8 + j], d = v.b[12 + j];
				if (i < 20)
					f.b[j] = d ^ (b & (c ^ d));
				else if (i < 40 || i >= 60)
					f.b[j] = b ^ c ^ d;
				else
					f.b[j] = (b & c) | (d & (b | c));
			}

			u32 t = f.w + rol32(v.w[0], 5) + v.w[4] + block.w[wi / 4];
			if (i < 20)
				t += K00;
			else if (i < 40)
				t += K20;
			else if (i < 60)
				t += K40;
			else
				t += K60;

			// e = d, d = c, c = b <<< 30, b = a, a = t
			memmove(v.w + 1, v.w, 4 * sizeof(u32));
			v.w[2] = rol32(v.w[2], 30);
			v.w[0] = t;
		}

		for (u08 i = 0; i < HASH_SIZE / 4; ++i) state.w[i] += v.w[i];
	}

	void add_byte(u08 data)
	{
		// message bytes are big endian within the words
		block.b[offset ^ 3] = data;
		if (++offset == BLOCK_SIZE)
		{
			hash_block();
			offset = 0;
		}
	}

	NOINLINE
	void Init()
	{
		MEM::ProgCopy(state.b, init_state, HASH_SIZE);
		offset = 0;
		length = 0;
	}

	void Write(u08 data)
	{
		length++;
		add_byte(data);
	}

	void Write(const u08* data, u08 size)
	{
		while (size--) Write(*data++);
	}

	// returns pointer to the 20 bytes of hash, valid until next Init()
	const u08* Result()
	{
		// pad with 0x80 and zeros, finish the block with the message
		// length in bits as 64-bit big endian (high 5 bytes are zero)
		u16 len = length;
		add_byte(0x80);
		while (offset != BLOCK_SIZE - 8) add_byte(0x00);
		for (u08 i = 0; i < 5; ++i) add_byte(0x00);
		add_byte(u08(len >> 13));
		add_byte(u08(len >> 5));
		add_byte(u08(len << 3));

		// convert words to big endian byte order
		for (u08 i = 0; i < HASH_SIZE; i += 4)
		{
			u08 t;
			t = state.b[i + 0]; state.b[i + 0] = state.b[i + 3]; state.b[i + 3] = t;
			t = state.b[i + 1]; state.b[i + 1] = state.b[i + 2]; state.b[i + 2] = t;
		}
		return state.b;
	}

	void write_key_pad(u08 pad)
	{
		// key padded with zeros to the block size
		for (u08 i = 0; i < BLOCK_SIZE; ++i)
		{
			Write((i < hmac_key_len ? hmac_key[i] : 0) ^ pad);
		}
	}

	// key stays in caller's RAM until ResultHmac(), its length is up to 64
	void InitHmac(const u08* key, u08 keyLength)
	{
		hmac_key = key;
		hmac_key_len = keyLength;

		// start inner hash
		Init();
		write_key_pad(HMAC_IPAD);
	}

	// returns pointer to the 20 bytes of HMAC, valid until next Init()
	const u08* ResultHmac()
	{
		// complete inner hash
		u08 inner_hash[HASH_SIZE];
		memcpy(inner_hash, Result(), HASH_SIZE);

		// calculate outer hash
		Init();
		write_key_pad(HMAC_OPAD);
		Write(inner_hash, HASH_SIZE);
		return Result();
	}
}
