#pragma once
#ifndef _PLANE_H_
#define _PLANE_H_

#include "Vector.h"

class Plane
{
public:
	Plane(const Vector3f& origin, const Vector3f& normal);

	Vector3f _origin;
	Vector3f _normal;
};

#endif