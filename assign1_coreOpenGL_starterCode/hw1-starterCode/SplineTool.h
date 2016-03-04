#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

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

// spline struct 
// contains how many control points the spline has, and an array of control points 
struct Spline
{
	int numControlPoints;
	Point * points;
};

// Give 4 control points and return the position of the calculated point.
Point CatmullRomAlgorithm(double u, const Point4& b1, const Point4& b2, const Point4& b3, const Point4& b4,
											const Point& c1, const Point& c2, const Point& c3, const Point& c4)
{
	// Crazy ass expansion of a (1x4) + (4x4) * (4x3)
	Point point = (c1 * b1.x + c2 * b1.y + c3 * b1.z + c4 * b1.w) * (u * u * u)
				+ (c1 * b2.x + c2 * b2.y + c3 * b2.z + c4 * b2.w) * (u * u)
				+ (c1 * b3.x + c2 * b3.y + c3 * b3.z + c4 * b3.w) * u
				+ (c1 * b4.x + c2 * b4.y + c3 * b4.z + c4 * b4.w);
	point = point;

	return point;
}

// Give 4 control points and return the position of the calculated point.
Point CatmullRomAlgorithmDerivative(double u, const Point4& b1, const Point4& b2, const Point4& b3, const Point4& b4,
	const Point& c1, const Point& c2, const Point& c3, const Point& c4)
{
	// Crazy ass expansion of a (1x4) + (4x4) * (4x3)
	Point point = (c1 * b1.x + c2 * b1.y + c3 * b1.z + c4 * b1.w) * (u * u) * 3
		+ (c1 * b2.x + c2 * b2.y + c3 * b2.z + c4 * b2.w) * (u)* 2
		+ (c1 * b3.x + c2 * b3.y + c3 * b3.z + c4 * b3.w);
	point = point;

	return point;
}

// Normalizes point p.
Point Normalize(const Point& outPoint)
{
	float length = sqrtf((float)(outPoint.x * outPoint.x + outPoint.y * outPoint.y + outPoint.z * outPoint.z));
	Point p(outPoint.x / length, outPoint.y / length, outPoint.z / length);
	return p;
}

int loadSplines(char * argv, Spline** splines, int& numSplines)
{
	char * cName = (char *)malloc(128 * sizeof(char));
	FILE * fileList;
	FILE * fileSpline;
	int iType, i = 0, j, iLength;

	// load the track file 
	fileList = fopen(argv, "r");
	if (fileList == NULL)
	{
		printf("can't open file\n");
		exit(1);
	}

	// stores the number of splines in a global variable 
	fscanf(fileList, "%d", &numSplines);

	*splines = (Spline*)malloc(numSplines * sizeof(Spline));

	// reads through the spline files 
	for (j = 0; j < numSplines; j++)
	{
		i = 0;
		fscanf(fileList, "%s", cName);
		fileSpline = fopen(cName, "r");

		if (fileSpline == NULL)
		{
			printf("can't open file\n");
			exit(1);
		}

		// gets length for spline file
		fscanf(fileSpline, "%d %d", &iLength, &iType);

		// allocate memory for all the points
		(*splines)[j].points = (Point *)malloc(iLength * sizeof(Point));
		(*splines)[j].numControlPoints = iLength;

		// saves the data to the struct
		while (fscanf(fileSpline, "%lf %lf %lf",
			&(*splines)[j].points[i].x,
			&(*splines)[j].points[i].y,
			&(*splines)[j].points[i].z) != EOF)
		{
			i++;
		}
	}

	free(cName);

	return 0;
}

// Note: You should combine this file
// with the solution of homework 1.

// Note for Windows/MS Visual Studio:
// You should set argv[1] to track.txt.
// To do this, on the "Solution Explorer",
// right click your project, choose "Properties",
// go to "Configuration Properties", click "Debug",
// then type your track file name for the "Command Arguments".
// You can also repeat this process for the "Release" configuration.

void initSpline(int argc, char ** argv, Spline** splines, int& numSplines)
{
	
	if (argc<2)
	{
		printf("usage: %s <trackfile>\n", argv[0]);
		exit(0);
	}

	// load the splines from the provided filename
	loadSplines(argv[1], splines, numSplines);

	printf("Loaded %d spline(s).\n", numSplines);
	for (int i = 0; i<numSplines; i++)
		printf("Num control points in spline %d: %d.\n", i, (*splines)[i].numControlPoints);
}



