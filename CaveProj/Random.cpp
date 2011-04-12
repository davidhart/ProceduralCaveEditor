#include "Random.h"
#include <ctime>

Random::Random()
{
	Seed();
}

void Random::Seed()
{
	_seed = (int)time(NULL);
}