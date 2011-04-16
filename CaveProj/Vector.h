#pragma once

#include <iostream>
#include <cmath>

template <typename T>
class Vector2
{
public:
	Vector2(T x, T y) : x(x), y(y) { }
	Vector2( ) : x(0), y(0) { }

	inline void Normalise()
	{ 
		T length = Length(); 
		x /= length;
		y /= length; 
	}

	inline T Length() const
	{
		return sqrt(x*x+y*y);
	}

	inline const Vector2<T> operator+ (const Vector2<T>& r) const
	{
		return Vector2<T>(x+r.x, y+r.y);
	}

	inline const Vector2<T>& operator+= (const Vector2<T>& r)
	{
		x += r.x;
		y += r.y;
		return *this;
	}

	inline const Vector2<T> operator- (const Vector2<T>& r) const
	{
		return Vector2<T>(x-r.x, y-r.y);
	}

	inline const Vector2<T>& operator-= (const Vector2<T>& r)
	{
		x -= r.x;
		y -= r.y;
		return *this;
	}

	inline const Vector2<T> operator* (const Vector2<T>& r)
	{
		return Vector2<T>(x*r.x, y*r.y);
	}

	inline const Vector2<T> operator* (const T r)
	{
		return Vector2<T>(x*r, y*r);
	}

	inline const Vector2<T>& operator*= (const Vector2<T>& r)
	{
		x *= r.x;
		y *= r.y;
		return *this;
	}
	
	inline const Vector2<T>& operator*= (const T r)
	{
		x *= r;
		y *= r;
		return *this;
	}

	inline const Vector2<T> operator/ (const Vector2<T>& r)
	{
		return Vector2<T>(x/r.x, y/r.y);
	}

	inline const Vector2<T> operator/ (const T r)
	{
		return Vector2<T>(x/r, y/r);
	}

	inline const Vector2<T>& operator/= (const Vector2<T>& r)
	{
		x /= r.x;
		y /= r.y;
		return *this;
	}

	inline const Vector2<T>& operator/= (const T r)
	{
		x /= r;
		y /= r;
		return *this;
	}

	T x;
	T y;
};

inline const Vector2<float> operator* (const Vector2<float>& v, float s)
{
	return Vector2<float>(v.x * s, v.y * s);
}

typedef Vector2<int> Vector2i;
typedef Vector2<float> Vector2f;
typedef Vector2<double> Vector2d;

template <typename T>
class Vector3
{
public:
	Vector3(T x, T y, T z) : x(x), y(y), z(z) { }
	Vector3() : x(0), y(0), z(0) { }

	const Vector3<T> operator+(const Vector3<T>& r) const
	{
		return Vector3<T>(x+r.x, y+r.y, z+r.z);
	}

	const Vector3<T>& operator+= (const Vector3<T>& r)
	{
		x += r.x;
		y += r.y;
		z += r.z;
		return *this;
	}

	const Vector3<T> operator- (const Vector3<T>& r) const
	{
		return Vector3<T>(x - r.x, y - r.y, z - r.z);
	}

	const Vector3<T>& operator-= (const Vector3<T>& r)
	{
		x -= r.x;
		y -= r.y;
		z -= r.z;
		return *this;
	}

	inline const Vector3<T> operator* (T s) const
	{
		return Vector3<T>(x * s, y * s, z * s);
	}

	inline const Vector3<T>& operator*= (T s)
	{
		x *= s;
		y *= s;
		z *= s;
		return *this;
	}

	inline const Vector3<T> operator* (const Vector3<T>& rhs) const
	{
		return Vector3<T>(x * rhs.x, y * rhs.y, z * rhs.z);
	}

	inline const Vector3<T>& operator *= (const Vector3<T>& rhs)
	{
		x *= rhs.x;
		y *= rhs.y;
		z *= rhs.z;
		return *this;
	}

	inline const Vector3<T> operator/ (const Vector3<T>& rhs) const
	{
		return Vector3<T>(x / rhs.x, y / rhs.y, z / rhs.z);
	}

	inline const Vector3<T>& operator/= (const Vector3<T>& rhs)
	{
		x /= rhs.x;
		y /= rhs.y;
		z /= rhs.z;

		return *this;
	}

	inline const Vector3<T> operator/ (T s) const
	{
		return Vector3<T>(x / s, y / s, z / s);
	}

	inline const Vector3<T>& operator/= (T s)
	{
		x /= s;
		y /= s;
		z /= s;
		return *this;
	}

	inline T Dot(const Vector3<T>&rhs) const
	{
		return x*rhs.x + y*rhs.y + z*rhs.z;
	}

	inline T Length() const
	{
		return sqrt(x*x + y*y + z*z);
	}

	inline void Normalize()
	{
		float length = Length();
		x /= length;
		y /= length;
		z /= length;
	}

	T x;
	T y;
	T z;
};


template <typename T> inline const Vector3<T> operator* (T s, const Vector3<T>& v)
{
	return Vector3<T>(v.x * s, v.y * s, v.z * s);
}

template <typename T> inline const Vector3<T> operator/ (T s, const Vector3<T>& v)
{
	return Vector3<T>(s / v.x, s / v.y, s / v.z);
}

template <typename T> inline std::ostream& operator<< (std::ostream& out, const Vector3<T>& v)
{
	out << "v(" << v.x << ", " << v.y << ", " << v.z << ")";
	return out;
}

typedef Vector3<int> Vector3i;
typedef Vector3<float> Vector3f;
typedef Vector3<double> Vector3d;