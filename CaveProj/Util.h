#pragma once
#ifndef _UTIL_H_
#define _UTIL_H_

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
};

#endif