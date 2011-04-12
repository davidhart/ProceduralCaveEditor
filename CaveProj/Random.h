#pragma once
#ifndef _RANDOM_H_
#define _RANDOM_H_

// Implementation: http://software.intel.com/en-us/articles/fast-random-number-generator-on-the-intel-pentiumr-4-processor/

class Random
{
public:
	Random();
	void Seed();
	inline void Seed(int seed)
	{
		_seed = seed;
	}
	inline int Next()
	{
		_seed = (214013*_seed+2531011);
		return (_seed>>16)&0x7FFF;
	}

private:
	int _seed;
};

#endif