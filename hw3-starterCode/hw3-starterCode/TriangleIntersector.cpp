#pragma once

#include "TriangleIntersector.h"
#include "SphereIntersector.h"
#include "Light.h"

Triangle* TriangleIntersector::TestIntersectionArray(const Vector3& ray, const Vector3& origin, Triangle* triangleArray,
	unsigned count, Vector3& outIntersectionPoint)
{
	Vector3 closestIntersectionPoint(-MAX_RAYCAST_DIST, -MAX_RAYCAST_DIST, -MAX_RAYCAST_DIST);
	Triangle* triangleCache = NULL;

	for (unsigned i = 0; i < count; ++i)
	{
		Vector3 tempIntersectionPoint;
		Triangle* tempTriangle = &triangleArray[i];

		// If you successfully intersect the sphere and the intersection point is closer to the camera,
		// then cache that intersection point and sphere.
		if (TestIntersection(ray, origin, *tempTriangle, tempIntersectionPoint)
			&& tempIntersectionPoint.z > closestIntersectionPoint.z)
		{
			closestIntersectionPoint = tempIntersectionPoint;
			triangleCache = tempTriangle;
		}
	}
	outIntersectionPoint = closestIntersectionPoint;
	return triangleCache;
}

Vector3 TriangleIntersector::CalculateLighting(const Vector3& origin, const Triangle& triangle,
	const Vector3& P/*intersection*/, Light* lights, unsigned count, Sphere* spheres, int num_spheres,
	Triangle* triangles, int num_triangles)
{
	Vector3 A(triangle.v[0].position[0], triangle.v[0].position[1], triangle.v[0].position[2]);
	Vector3 B(triangle.v[1].position[0], triangle.v[1].position[1], triangle.v[1].position[2]);
	Vector3 C(triangle.v[2].position[0], triangle.v[2].position[1], triangle.v[2].position[2]);
	Vector3 AB = B.Subtract(A);
	Vector3 AC = C.Subtract(A);
	Vector3 N = AB.Cross(C.Subtract(A));
	Vector3::Normalize(N);

	// See: https://classes.soe.ucsc.edu/cmps160/Fall10/resources/barycentricInterpolation.pdf
	// Descart and Fermat area of a triangle
	Vector3 sphereColor(triangle.v[0].color_diffuse[0], triangle.v[0].color_diffuse[1], triangle.v[0].color_diffuse[2]);
	Vector3 sphereSpec(triangle.v[0].color_specular[0], triangle.v[0].color_specular[1], triangle.v[0].color_specular[2]);
	double shininess = triangle.v[0].shininess;

	// Lighting
	Vector3 phong(AMBIENT_COLOR / 255.0, AMBIENT_COLOR / 255.0, AMBIENT_COLOR / 255.0);
	for (unsigned i = 0; i < count; ++i)
	{
		// Conversion to Vector3s
		Vector3 lightPosition(lights[i].position[0], lights[i].position[1], lights[i].position[2]);
		Vector3 lightColor(lights[i].color[0], lights[i].color[1], lights[i].color[2]);

		// Lighting calculations
		Vector3 L = lightPosition.Subtract(P);
		Vector3::Normalize(L);
		Vector3 NegL = L;
		Vector3::Negate(NegL);
		Vector3 R = Vector3::Reflect(NegL, N);
		Vector3 V = origin.Subtract(P);
		Vector3::Normalize(V);

		//if (!isShadowedPixelT(lightPosition, P, spheres, num_spheres, triangles, num_triangles))
		{
			double NdotL = N.Dot(L);
			phong = phong.Add(sphereColor.Multiply(MathTools::Saturate(NdotL)));
			phong = phong.Add(sphereSpec.Multiply(pow(MathTools::Saturate(R.Dot(V)), shininess)));
			phong = lightColor.Multiply(phong);
		}
	}
	MathTools::Saturate(phong);
	return phong;
}

bool TriangleIntersector::TestIntersection(const Vector3& ray, const Vector3& origin, const Triangle& triangle,
	Vector3& outIntersectionPoint) const
{
	// See: https://www.cs.princeton.edu/courses/archive/fall00/cs426/lectures/raycast/sld017.htm
	Vector3 A(triangle.v[0].position[0], triangle.v[0].position[1], triangle.v[0].position[2]);
	Vector3 B(triangle.v[1].position[0], triangle.v[1].position[1], triangle.v[1].position[2]);
	Vector3 C(triangle.v[2].position[0], triangle.v[2].position[1], triangle.v[2].position[2]);
	Vector3 AB = B.Subtract(A);
	Vector3 N = AB.Cross(C.Subtract(A));
	Vector3::Normalize(N);

	// Use a point on the triangle "A" to find "d".
	double d = -1 * A.Dot(N);

	// Ray - Plane intersection
	// t = -(Po dot N + d) / (V dot N)
	// P = Po + tV
	double VDotN = ray.Dot(N);
	if (VDotN == 0) return false;
	double t = -1 * (origin.Dot(N) + d) / VDotN;
	Vector3 P = origin.Add(ray.Multiply(t));

	Vector3 ABxAP = AB.Cross(P.Subtract(A));
	Vector3 BCxBP = C.Subtract(B).Cross(P.Subtract(B));
	Vector3 CAxCP = A.Subtract(C).Cross(P.Subtract(C));

	if (ABxAP.Dot(BCxBP) > 0 && ABxAP.Dot(CAxCP) > 0 && BCxBP.Dot(CAxCP) > 0
		|| ABxAP.Dot(BCxBP) < 0 && ABxAP.Dot(CAxCP) < 0 && BCxBP.Dot(CAxCP) < 0)
	{
		outIntersectionPoint = P;
		return true;
	}
	return false;
}

bool TriangleIntersector::isShadowedPixelT(const Vector3& lightPosition, const Vector3& intersection, Sphere* spheres, unsigned num_spheres, Triangle* triangles, unsigned num_triangles)
{
	Vector3 ray = lightPosition.Subtract(intersection);
	Vector3::Normalize(ray);

	Vector3 outTriangleIntersectionPoint;
	Triangle* triIntersect = TestIntersectionArray(ray, intersection, triangles, num_triangles, outTriangleIntersectionPoint);

	Vector3 outSphereIntersectionPoint;
	SphereIntersector sphIntersector;
	Sphere* sphIntersect = sphIntersector.TestIntersectionArray(ray, intersection, spheres, num_spheres, outSphereIntersectionPoint);

	return triIntersect != NULL || sphIntersect != NULL;
}