#pragma once
#include "Defines.h"
#include <math.h>
#include "Vector3.h"

struct Light;

struct Sphere
{
	double position[3];
	double color_diffuse[3];
	double color_specular[3];
	double shininess;
	double radius;
};