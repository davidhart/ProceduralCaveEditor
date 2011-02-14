#include "Plane.h"
#include "Ray.h"

Plane::Plane(const Vector3f& origin, const Vector3f& normal) :
	_origin(origin),
	_normal(normal)
{
}