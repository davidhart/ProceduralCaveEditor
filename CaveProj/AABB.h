#pragma once
#ifndef _AABB_H_
#define _AABB_H_

#include "Vector.h"

class AABB
{
public:
	AABB(const Vector3f& min, const Vector3f& max);

	Vector3f _min;
	Vector3f _max;
};

#endif