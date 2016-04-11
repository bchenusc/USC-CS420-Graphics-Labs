#pragma once
#include <math.h>

#include "Defines.h"
#include "Vector3.h"
#include "Sphere.h"
#include "Triangle.h"

struct SphereIntersector
{
	// Returns the sphere pointer closest to the camera or null
	Sphere* TestIntersectionArray(const Vector3& ray, const Vector3& origin, Sphere* sphereArray, unsigned count, Vector3& outIntersectionPoint);

	// Returns the saturated phong values.
	Vector3 CalculateLighting(const Vector3& origin, const Sphere& sphere,
		const Vector3& P/*intersection*/, Light* lights, unsigned count, Sphere* spheres, int num_spheres,
		Triangle* triangles, int num_triangles);

private:
	// Returns true if successfully intersects.
	bool TestIntersection(const Vector3& ray, const Vector3& origin, Sphere& sphere, Vector3& outIntersectionPoint) const;
	bool SphereIntersector::isShadowedPixelS(const Vector3& lightPosition, const Vector3& intersection, Sphere* spheres, unsigned num_spheres, Triangle* triangles, unsigned num_triangles);
};