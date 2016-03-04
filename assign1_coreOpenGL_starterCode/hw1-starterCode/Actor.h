#pragma once
#include "openGLHeader.h"

struct Actor
{
	virtual void Init() = 0;
	virtual void Draw(GLuint program) = 0;
};