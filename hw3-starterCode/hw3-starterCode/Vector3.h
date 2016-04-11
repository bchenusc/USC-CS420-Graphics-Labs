#pragma once
#include <math.h>

struct Vector3
{
	double x = 0;
	double y = 0;
	double z = 0;

	Vector3(){}

	Vector3(double x, double y, double z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}

	static void Normalize(Vector3& v)
	{
		double length = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
		v.x = v.x / length;
		v.y = v.y / length;
		v.z = v.z / length;
	}

	static void Negate(Vector3& v)
	{
		v.x = -v.x;
		v.y = -v.y;
		v.z = -v.z;
	}

	static Vector3 Reflect(const Vector3& v, const Vector3& n)
	{
		return v.Subtract(n.Multiply(2 * v.Dot(n)));
	}

	Vector3 Subtract(const Vector3& rhs) const
	{
		return Vector3(x - rhs.x, y - rhs.y, z - rhs.z);
	}

	double Dot(const Vector3& rhs) const
	{
		return x * rhs.x + y * rhs.y + z * rhs.z;
	}

	Vector3 Multiply(double scalar) const
	{
		return Vector3(x * scalar, y * scalar, z * scalar);
	}

	Vector3 Multiply(const Vector3& rhs) const
	{
		return Vector3(x * rhs.x, y * rhs.y, z * rhs.z);
	}

	Vector3 Add(const Vector3& rhs) const
	{
		return Vector3(x + rhs.x, y + rhs.y, z + rhs.z);
	}

	Vector3 Cross(const Vector3& rhs) const
	{
		return Vector3(y * rhs.z - z * rhs.y, z * rhs.x - x * rhs.z, x * rhs.y - y * rhs.x);
	}
};

class MathTools
{
public:
	static double Saturate(double i)
	{
		if (i <= 0) return 0;
		if (i >= 1) return 1;
		return i;
	}

	static void Saturate(Vector3& v)
	{
		v.x = Saturate(v.x);
		v.y = Saturate(v.y);
		v.z = Saturate(v.z);
	}
};
