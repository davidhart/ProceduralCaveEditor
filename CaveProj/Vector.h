#pragma once

template <typename T>
class Vector2
{
public:
	Vector2(T x, T y) : x(x), y(y) { }
	Vector2( ) : x(0), y(0) { }

	inline void Normalise()
	{ 
		float length = sqrt(x*x+y*y); 
		x /= length;
		y /= length; 
	}

	inline const Vector2<T> operator- (const Vector2<T>& r) const
	{
		return Vector2<T>(x-r.x, y-r.y);
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

	const Vector3<T>& operator+= (const Vector3<T>& r)
	{
		x += r.x;
		y += r.y;
		z += r.z;
		return *this;
	}

	T x;
	T y;
	T z;
};

inline const Vector3<float> operator* (const Vector3<float>& v, float s)
{
	return Vector3<float>(v.x * s, v.y * s, v.z * s);
}

inline const Vector3<float> operator* (float s, const Vector3<float>& v)
{
	return Vector3<float>(v.x * s, v.y * s, v.z * s);
}

typedef Vector3<int> Vector3i;
typedef Vector3<float> Vector3f;
typedef Vector3<double> Vector3d;