#pragma once
#ifndef _RAY_H_
#define _RAY_H_

#include "Vector.h"

class Ray
{
public:
	Ray(const Vector3f& origin, const Vector3f& direction);

	Vector3f _origin;
	Vector3f _direction;
};

#endif