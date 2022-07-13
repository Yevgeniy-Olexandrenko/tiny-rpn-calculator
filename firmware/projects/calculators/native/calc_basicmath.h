#pragma once

#include <math.h>

////////////////////////////////////////////////////////////////////////////////
// Basic Math Helpers
////////////////////////////////////////////////////////////////////////////////

template<typename T> static uint8_t _ones(const T & x) { return (uint8_t)(x % 10); }
template<typename T> static uint8_t _tens(const T & x) { return (uint8_t)((x / 10) % 10); }

template<typename T> static T _abs(const T & x) { return x < 0 ? -x : x; }

enum { EXP, SIN, ASIN };
NOINLINE f32 taylorExpSinASin(f32 f, u08 op)
{
	f32 result, frac, ff = f * f;
	result = frac = ((op == EXP) ? 1 : f);

	for (u16 i = 1; i < 129; i++)
	{
		u16 i2 = 2 * i;
		u16 i2p = i2 + 1;
		u16 i2m = i2 - 1;
		u16 i2m2 = i2m * i2m;
		f32 ffi2i2p = ff / (i2 * i2p);

		switch(op)
		{
			case EXP: frac *= f / i; break;
			case SIN: frac *= -ffi2i2p; break;
			default:  frac *= ffi2i2p * i2m2; break;
		}
		result += frac;
	}
	return result;
}

f32 _log (f32 f) { return log(f); }
f32 _exp (f32 f) { return taylorExpSinASin(f, EXP);  }
f32 _sin (f32 f) { return taylorExpSinASin(f, SIN);  }
f32 _asin(f32 f) { return taylorExpSinASin(f, ASIN); }

NOINLINE f32 _p10 (s08 e) 
{ 
	f32 f = 1.f;
	if (e > 0) 
		while (e--) f *= 10.f;
	else 
		while (e++) f /= 10.f;
	return f;
}

#define _to_rad(x) ((x) * (M_PI / 180.f))
#define _to_deg(x) ((x) * (180.f / M_PI))
