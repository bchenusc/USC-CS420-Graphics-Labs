#pragma once

#include "Sphere.h"
#include "Triangle.h"

struct TriangleIntersector
{
	// Returns the triangle pointer closest to the camera or null
	Triangle* TestIntersectionArray(const Vector3& ray, const Vector3& origin, Triangle* triangleArray,
		unsigned count, Vector3& outIntersectionPoint);

	// Returns the saturated phong values.
	Vector3 CalculateLighting(const Vector3& origin, const Triangle& triangle,
		const Vector3& P/*intersection*/, Light* lights, unsigned count, Sphere* spheres, int num_spheres,
		Triangle* triangles, int num_triangles);

private:
	// Returns true if successfully intersects.
	bool TestIntersection(const Vector3& ray, const Vector3& origin, const Triangle& triangle,
		Vector3& outIntersectionPoint) const;

	bool isShadowedPixelT(const Vector3& lightPosition, const Vector3& intersection, 
		Sphere* spheres, unsigned num_spheres, Triangle* triangles, unsigned num_triangles);

	void CalculateColor(const Vector3& P, const Triangle& triangle, Vector3& outDiffuse, 
		Vector3& outSpecular, Vector3& outNormal, double& outShininess);
};

