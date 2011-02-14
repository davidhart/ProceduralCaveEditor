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
};

#endif