#pragma once
#ifndef _UTIL_H_
#define _UTIL_H_

#include <Windows.h>
#include <istream>
#include "Random.h"

class Util
{
public:
	template<typename T> inline static bool Between(T value, T min, T max)
	{
		return value >= min && value <= max;
	}

	template<typename T> inline static T Min(T a, T b)
	{
		return a < b ? a : b;
	}

	#define COLOR_ARGB(a,r,g,b) \
		((DWORD)((((a)&0xff)<<24)|(((b)&0xff)<<16)|(((g)&0xff)<<8)|((r)&0xff)))

	inline static DWORD GetR(DWORD argb)
	{
		return argb & 0xff;
	}

	inline static DWORD GetG(DWORD argb)
	{
		return (argb >> 8) & 0xff;
	}
	
	inline static DWORD GetB(DWORD argb)
	{
		return (argb >> 16) & 0xff;
	}

	inline static DWORD GetA(DWORD argb)
	{
		return argb >> 24;
	}

	static bool HexCharToInt(char c, int& i);

	static bool ReadHexColor(std::istream& stream, DWORD& color);

	template<typename T> inline static T Lerp(T a, T b, T f)
	{
		return a * (1 - f) + b * f;
	}

	template<typename T> inline static T Smoothstep(T a, T b, T f)
	{
		float blendfactor = f*f*(3-2*f); // smoothstep (3t^2 - 2t^3)
		return Lerp(a, b, f);
	}

	static Random Rand;
};

#endif