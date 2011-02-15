#pragma once
#ifndef _RAY_H_
#define _RAY_H_

#include "Vector.h"

class Plane;
class AABB;

class Ray
{
public:
	Ray(const Vector3f& origin, const Vector3f& direction);
	float Intersects(const Plane& plane, bool twoSided = false) const;
	float Intersects(const AABB& aabb) const;
	bool ClosestPoint(const Ray& ray, float& point) const;

	Vector3f _origin;
	Vector3f _direction;
};

#endif