#include "Ray.h"
#include "Plane.h"
#include "AABB.h"
#include "Util.h"
#include "Math.h"

Ray::Ray(const Vector3f& origin, const Vector3f& direction) :
	_origin(origin),
	_direction(direction)
{
}

float Ray::Intersects(const Plane& plane, bool twoSided) const
{
	// http://www.siggraph.org/education/materials/HyperGraph/raytrace/rayplane_intersection.htm

	float Vd = plane._normal.Dot(_direction);

	if (Vd >= 0 && !twoSided) // plane is pointing away from the ray
		return -1;

	float planeD = -plane._origin.Dot(plane._normal); // distance from plane to origin

	float V0 = -(plane._normal.Dot(_origin) + planeD);

	float t = V0 / Vd;

	if (t < 0) // intersection is behind origin
		return -1;

	return t;
}

float Ray::Intersects(const AABB& aabb) const
{
	float distance = -1;
	
	// bottom
	Plane p (aabb._min, Vector3f(0, -1, 0));
	float t = Intersects(p, false);
	if (t > 0 && (distance < 0 || t < distance))
	{
		Vector3f pt = _origin + t * _direction;

		if (Util::Between(pt.x, aabb._min.x, aabb._max.x) &&
			Util::Between(pt.z, aabb._min.z, aabb._max.z))
		{
			distance = t;
		}
	}

	// top
	p._origin = aabb._max;
	p._normal = Vector3f(0, 1, 0);
	t = Intersects(p, false);
	if (t > 0 && (distance < 0 || t < distance))
	{
		Vector3f pt = _origin + t * _direction;

		if (Util::Between(pt.x, aabb._min.x, aabb._max.x) &&
			Util::Between(pt.z, aabb._min.z, aabb._max.z))
		{
			distance = t;
		}
	}

	// left
	p._origin = aabb._min;
	p._normal = Vector3f(-1, 0, 0);
	t = Intersects(p, false);
	if (t > 0 && (distance < 0 || t < distance))
	{
		Vector3f pt = _origin + t * _direction;

		if (Util::Between(pt.y, aabb._min.y, aabb._max.y) &&
			Util::Between(pt.z, aabb._min.z, aabb._max.z))
		{
			distance = t;
		}
	}

	// right
	p._origin = aabb._max;
	p._normal = Vector3f(1, 0, 0);
	t = Intersects(p, false);
	if (t > 0 && (distance < 0 || t < distance))
	{
		Vector3f pt = _origin + t * _direction;

		if (Util::Between(pt.y, aabb._min.y, aabb._max.y) &&
			Util::Between(pt.z, aabb._min.z, aabb._max.z))
		{
			distance = t;
		}
	}

	// back
	p._origin = aabb._min;
	p._normal = Vector3f(0, 0, -1);
	t = Intersects(p, false);
	if (t > 0 && (distance < 0 || t < distance))
	{
		Vector3f pt = _origin + t * _direction;

		if (Util::Between(pt.y, aabb._min.y, aabb._max.y) &&
			Util::Between(pt.x, aabb._min.x, aabb._max.x))
		{
			distance = t;
		}
	}

	// front
	p._origin = aabb._max;
	p._normal = Vector3f(0, 0, 1);
	t = Intersects(p, false);
	if (t > 0 && (distance < 0 || t < distance))
	{
		Vector3f pt = _origin + t * _direction;

		if (Util::Between(pt.y, aabb._min.y, aabb._max.y) &&
			Util::Between(pt.x, aabb._min.x, aabb._max.x))
		{
			distance = t;
		}
	}

	return distance;
}

float Ray::Intersects(const Vector3f& spherePosition, float radius) const
{
	float a = _direction.Dot(_direction);
	float b = 2 * _direction.Dot(_origin - spherePosition);
	float c = _origin.Dot(_origin) +
			  spherePosition.Dot(spherePosition) -
			  2 * _origin.Dot(spherePosition) - radius * radius;

	float discriminant = pow(b, 2) - 4 * a * c;

	if (discriminant >= 0)
	{
		float t = Util::Min((-b + sqrt(discriminant)) / (2 * a), (-b - sqrt(discriminant)) / (2 * a)); 

		if (t >= 0)
			return t;
	}

	return -1;
}

bool Ray::ClosestPoint(const Ray& ray, float& point) const
{
	// Find closest point on ray 1 to ray 2
	// pa = p1 + t1 * d1
	// pa: closest point
	// returns t1

	// Distance between two rays
	Vector3f d12 = _origin - ray._origin; 
	
	float d12d2 = d12.Dot(ray._direction);
	float d2d1 = ray._direction.Dot(_direction);
	float d12d1 = d12.Dot(_direction);
	float d2d2 = ray._direction.Dot(ray._direction);
	float d1d1 = _direction.Dot(_direction);

	float denom = d1d1 * d2d2 - d2d1 * d2d1;

	if (abs(denom) < Math::EPSILON)
		return false;

	float numer = d12d2 * d2d1 - d12d1 * d2d2;

	point = numer/denom;

	return true;
}