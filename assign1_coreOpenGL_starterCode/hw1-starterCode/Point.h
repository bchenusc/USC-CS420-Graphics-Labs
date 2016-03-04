#pragma once

// represents one control point along the spline 
struct Point
{
	double x;
	double y;
	double z;

	Point(double nx, double ny, double nz)
	{
		x = nx;
		y = ny;
		z = nz;
	}

	Point operator +(const Point& p)
	{
		return Point(p.x + x, p.y + y, p.z + z);
	}
	Point operator -(const Point& p)
	{
		return Point(p.x - x, p.y - y, p.z - z);
	}
};

Point operator* (const Point& lhs, double scalar)
{
	return Point(lhs.x*scalar, lhs.y*scalar, lhs.z*scalar);
}

Point pCross(const Point& u, const Point& v)
{
	Point p(u.y * v.z - u.z * v.y, u.z*v.x - u.x*v.z, u.x*v.y - u.y*v.x);
	return p;
}

struct Point4
{
	double x;
	double y;
	double z;
	double w;

	Point4(){ }

	Point4(double nx, double ny, double nz, double nw)
	{
		x = nx;
		y = ny;
		z = nz;
		w = nw;
	}
};