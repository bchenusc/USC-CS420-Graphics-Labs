#pragma once
#include <vector>

#include "SplineActor.h"
#include "Point.h"

using std::vector;

struct CrossActor : public Actor
{
	CrossActor(GLuint texHandle, SplineActor& spline);

	virtual void Init();
	virtual void Draw(GLuint program);

	// VBO Handles
	GLuint vertextHandle;
	GLuint indexHandle;
	GLuint texCoordHandle;
	GLuint texHandle;

	// VAO Handle
	GLuint vaoHandle;

private:
	void initVBOBuffers();
	vector<Point> vertices;
	SplineActor* refSpline;

	vector<unsigned> indexBuffer;
	vector<Point2> texCoords;
};

CrossActor::CrossActor(GLuint texHandle, SplineActor& spline)
{
	this->texHandle = texHandle;
	refSpline = &spline;
}

void CrossActor::Init()
{
	vertices.reserve(refSpline->normals.size() * 4);
	indexBuffer.reserve(refSpline->normals.size() * 4);

	static const double  newMultiplier = 0.1;
	static const int modulusFilter = 5;
	static const int binormalMultiplier = 50;
	int multiplier = 1;
	int biMultiplier = 1;
	bool nextMultiplier = false;

	for (unsigned int i = 0; i < refSpline->normals.size(); ++i)
	{

		if (i % modulusFilter == 0)
		{
			multiplier = newMultiplier;
			biMultiplier = binormalMultiplier;
			nextMultiplier = true;
		}
		else
		{
			if (i % modulusFilter != 0 && nextMultiplier)
			{
				nextMultiplier = false;
				multiplier = newMultiplier;
				biMultiplier = binormalMultiplier;
			}
			else
			{
				multiplier = 1;
				biMultiplier = 9;
			}
		}

		Point p = refSpline->points[i];
		Point n = refSpline->normals[i];
		Point b = pCross(refSpline->tangents[i], n);
		vertices.push_back(p + ((b * biMultiplier) - n) * RAIL_SIZE * multiplier); //v0
		vertices.push_back(p + (n + (b * biMultiplier)) * RAIL_SIZE * multiplier); //v1
		vertices.push_back(p + ((n - (b * biMultiplier)) * RAIL_SIZE * multiplier)); //v2
		Point neg = n * -1.0;
		vertices.push_back(p + (neg - (b * biMultiplier)) * RAIL_SIZE * multiplier); //v3
	}

	for (unsigned int i = 0; i < vertices.size() - 5; ++i)
	{
		indexBuffer.push_back(i);
		indexBuffer.push_back(i + 1);
		indexBuffer.push_back(i + 4);
		indexBuffer.push_back(i + 5);
	}
	for (unsigned int i = 0; i < refSpline->normals.size() - 1; ++i)
	{
		indexBuffer.push_back(i);
		indexBuffer.push_back(i + 3);
		indexBuffer.push_back(i + 4);
		indexBuffer.push_back(i + 7);
	}

	for (unsigned int i = 0; i < indexBuffer.size(); i++)
	{
		switch (i % 4)
		{
		case 0:
			texCoords.push_back(Point2(0, 0));
			break;
		case 1:
			texCoords.push_back(Point2(1, 1));
			break;
		case 2:
			texCoords.push_back(Point2(0, 0));
			break;
		case 3:
			texCoords.push_back(Point2(1, 1));
		}
	}

	initVBOBuffers();
}

void CrossActor::Draw(GLuint program)
{
	// Bind VAO
	glGenVertexArrays(1, &vaoHandle);
	glBindVertexArray(vaoHandle);

	// Attribute 1: Position
	GLuint attrib_pos = glGetAttribLocation(program, "position");
	glEnableVertexAttribArray(attrib_pos);
	glBindBuffer(GL_ARRAY_BUFFER, vertextHandle);
	glVertexAttribPointer(attrib_pos, 3, GL_DOUBLE, GL_FALSE, 0, (void*)0);

	// Attribute 2: Texture
	GLuint attrib_tex = glGetAttribLocation(program, "texCoord");
	glEnableVertexAttribArray(attrib_tex);
	// Bind the texture
	glBindBuffer(GL_ARRAY_BUFFER, texCoordHandle);
	glVertexAttribPointer(attrib_tex, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texHandle);

	// Draw Wireframe
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexHandle);
	glDrawElements(GL_TRIANGLE_STRIP, indexBuffer.size(), GL_UNSIGNED_INT, (void*)0);
	glBindVertexArray(0);

}

void CrossActor::initVBOBuffers()
{
	glGenBuffers(1, &vertextHandle);
	glBindBuffer(GL_ARRAY_BUFFER, vertextHandle);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Point), &vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &indexHandle);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexHandle);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBuffer.size() * sizeof(unsigned int), &indexBuffer[0], GL_STATIC_DRAW);

	glGenBuffers(1, &texCoordHandle);
	glBindBuffer(GL_ARRAY_BUFFER, texCoordHandle);
	glBufferData(GL_ARRAY_BUFFER, texCoords.size() * sizeof(Point2), &texCoords[0], GL_STATIC_DRAW);
}
