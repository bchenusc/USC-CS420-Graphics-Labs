#pragma once
#include <vector>

#include "SplineTool.h"
#include "glutHeader.h"
#include "openGLHeader.h"
#include "Actor.h"
#include "Textures.h"
#include "CrossActor.h"
#include "SplineActor.h"

using std::vector;

struct CrossActorManager : public Actor
{
	CrossActorManager();
	CrossActorManager(GLuint textureHandle, double crossWidth, SplineActor* refSpline);
	virtual void Init();
	virtual void Draw(GLuint program);

	// VAO Handle
	GLuint vaoHandle;

private:
	GLuint textureHandle;
	SplineActor* spline;
	double crossWidth;

	vector<CrossActor> crosses;
};

CrossActorManager::CrossActorManager()
{

}

CrossActorManager::CrossActorManager(GLuint textureHandle, double crossWidth, SplineActor* refSpline)
{
	this->textureHandle = textureHandle;
	this->spline = refSpline;
	this->crossWidth = crossWidth;
}

void CrossActorManager::Init()
{
	unsigned size = spline->tangents.size();
	for (unsigned i = 0; i < size; ++i)
	{
		if (i % 100 == 0)
		{
			// Make a cross section every 4 vertices.
			crosses.push_back(CrossActor(textureHandle, spline));
		}
	}

	for (unsigned i = 0; i < crosses.size(); i++)
	{
		crosses[i].Init();
	}
}

void CrossActorManager::Draw(GLuint program)
{
	glGenVertexArrays(1, &vaoHandle);
	glBindVertexArray(vaoHandle);

	for (unsigned i = 0; i < crosses.size(); i++)
	{
		crosses[i].Draw(program);
	}

	glBindVertexArray(0);
}
