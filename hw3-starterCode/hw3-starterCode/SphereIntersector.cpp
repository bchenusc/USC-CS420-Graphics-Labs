#pragma once

#include "SphereIntersector.h"
#include "TriangleIntersector.h"
#include "Light.h"

Sphere* SphereIntersector::TestIntersectionArray(const Vector3& ray, const Vector3& origin, Sphere* sphereArray, unsigned count, Vector3& outIntersectionPoint)
{
	Vector3 closestIntersectionPoint(-MAX_RAYCAST_DIST, -MAX_RAYCAST_DIST, -MAX_RAYCAST_DIST);
	Sphere* sphereCache = NULL;

	for (unsigned i = 0; i < count; ++i)
	{
		Vector3 tempIntersectionPoint;
		Sphere* tempSphere = &sphereArray[i];

		// If you successfully intersect the sphere and the intersection point is closer to the camera,
		// then cache that intersection point and sphere.
		if (TestIntersection(ray, origin, *tempSphere, tempIntersectionPoint)
			&& tempIntersectionPoint.z > closestIntersectionPoint.z)
		{
			closestIntersectionPoint = tempIntersectionPoint;
			sphereCache = tempSphere;
		}
	}
	outIntersectionPoint = closestIntersectionPoint;
	return sphereCache;
}

bool SphereIntersector::TestIntersection(const Vector3& ray, const Vector3& origin, Sphere& sphere, Vector3& outIntersectionPoint) const
{
	// See: https://www.cs.princeton.edu/courses/archive/fall00/cs426/lectures/raycast/sld013.htm
	Vector3 spherePos(sphere.position[0], sphere.position[1], sphere.position[2]);
	Vector3 hypotenuse = spherePos.Subtract(origin);
	double opposite = hypotenuse.Dot(ray);

	// Sphere is in opposite direction aka behind camera.
	if (opposite < 0) return false;

	// Ray's distance is greater than the radius of the sphere.
	// Thus ray does not intersect sphere.
	double adjacentSqr = hypotenuse.Dot(hypotenuse) - opposite * opposite;
	if (adjacentSqr > sphere.radius * sphere.radius) return false;

	// thc in the slides.
	double offset = sqrt(sphere.radius * sphere.radius - adjacentSqr);
	double intersectionLength = opposite - offset;

	outIntersectionPoint = ray.Multiply(intersectionLength);
	return true;
}

Vector3 SphereIntersector::CalculateLighting(const Vector3& origin, const Sphere& sphere,
	const Vector3& P/*intersection*/, Light* lights, unsigned count, Sphere* spheres, int num_spheres,
	Triangle* triangles, int num_triangles, const Vector3& ambient)
{
	Vector3 spherePos(sphere.position[0], sphere.position[1], sphere.position[2]);

	// Lighting
	Vector3 phong(ambient);
	for (unsigned i = 0; i < count; ++i)
	{
		// Conversion to Vector3s
		Vector3 lightPosition(lights[i].position[0], lights[i].position[1], lights[i].position[2]);
		Vector3 lightColor(lights[i].color[0], lights[i].color[1], lights[i].color[2]);
		Vector3 sphereColor(sphere.color_diffuse[0], sphere.color_diffuse[1], sphere.color_diffuse[2]);
		Vector3 sphereSpec(sphere.color_specular[0], sphere.color_specular[1], sphere.color_specular[2]);

		// Lighting calculations
		Vector3 L = lightPosition.Subtract(P);
		Vector3 N = P.Subtract(spherePos);
		Vector3::Normalize(L);
		Vector3::Normalize(N);
		Vector3 NegL = L;
		Vector3::Negate(NegL);
		Vector3 R = Vector3::Reflect(NegL, N);
		Vector3 V = origin.Subtract(P);
		Vector3::Normalize(V);

		if (!isShadowedPixelS(lightPosition, P, spheres, num_spheres, triangles, num_triangles))
		{
			double NdotL = N.Dot(L);
			phong = phong.Add(sphereColor.Multiply(MathTools::Saturate(NdotL)));
			phong = phong.Add(sphereSpec.Multiply(pow(MathTools::Saturate(R.Dot(V)), sphere.shininess)));
			phong = lightColor.Multiply(phong);
		}
	}
	MathTools::Saturate(phong);
	return phong;
}

bool SphereIntersector::isShadowedPixelS(const Vector3& lightPosition, const Vector3& intersection, Sphere* spheres, unsigned num_spheres, Triangle* triangles, unsigned num_triangles)
{
	Vector3 ray = lightPosition.Subtract(intersection);
	Vector3::Normalize(ray);

	TriangleIntersector triIntersector;
	Vector3 outTriangleIntersectionPoint;
	Triangle* triIntersect = triIntersector.TestIntersectionArray(ray, intersection, triangles, num_triangles, outTriangleIntersectionPoint);

	Vector3 outSphereIntersectionPoint;
	Sphere* sphIntersect = TestIntersectionArray(ray, intersection, spheres, num_spheres, outSphereIntersectionPoint);

	return triIntersect != NULL || sphIntersect != NULL;
}