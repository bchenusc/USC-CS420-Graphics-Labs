#pragma once
#include "SplineActor.h"
#include <vector>

using std::vector;

struct RailActor: public Actor
{
	RailActor(SplineActor& spline);

	virtual void Init();
	virtual void Draw(GLuint program);

	// VBO Handles
	GLuint vertextHandle;
	GLuint indexHandle;

	// VAO Handle
	GLuint vaoHandle;

private:
	void initVBOBuffers();
	SplineActor* refSpline;
	vector<Point> vertices;
	vector<unsigned> indexBuffer;

	const double size = 0.1;
};

RailActor::RailActor(SplineActor& spline)
{
	refSpline = &spline;
}

void RailActor::Init()
{
	vertices.reserve(refSpline->normals.size() * 4);
	indexBuffer.reserve(refSpline->normals.size() * 4);

	for (unsigned int i = 0; i < refSpline->normals.size(); ++i)
	{
		Point p = refSpline->points[i];
		Point n = refSpline->normals[i];
		Point b = pCross(refSpline->tangents[0], n);
		vertices.push_back(p + (b - n) * size); //v0
		vertices.push_back(p + (n + b) * size);	   //v1
		vertices.push_back(p + ((n - b) * size)); //v2
		vertices.push_back(p + ((n * -1.0) - b) * size); //v3
	}

	for (unsigned int i = 0; i < vertices.size() - 5; ++i)
	{
		indexBuffer.push_back(i);
		indexBuffer.push_back(i+1);
		indexBuffer.push_back(i+5);
		indexBuffer.push_back(i+4);
	}
	for (unsigned int i = 0; i < refSpline->normals.size() - 1; ++i)
	{
		indexBuffer.push_back(i);
		indexBuffer.push_back(i+3);
		indexBuffer.push_back(i+4);
		indexBuffer.push_back(i+7);
	}

	initVBOBuffers();
}

void RailActor::Draw(GLuint program)
{
	// Bind VAO
	glGenVertexArrays(1, &vaoHandle);
	glBindVertexArray(vaoHandle);

	// Attribute 1: Position
	GLuint attrib_pos = glGetAttribLocation(program, "position");
	glEnableVertexAttribArray(attrib_pos);
	glBindBuffer(GL_ARRAY_BUFFER, vertextHandle);
	glVertexAttribPointer(attrib_pos, 3, GL_DOUBLE, GL_FALSE, 0, (void*)0);

	// Draw Wireframe
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexHandle);
	glDrawElements(GL_QUADS, indexBuffer.size(), GL_UNSIGNED_INT, (void*)0);
	glBindVertexArray(0);

	// Attribute 2: Texture
	//GLuint attrib_tex = glGetAttribLocation(program, "texCoord");
	//glEnableVertexAttribArray(attrib_tex);
	// Bind the texture
	//glBindBuffer(GL_ARRAY_BUFFER, texCoordHandle);
	//glVertexAttribPointer(attrib_tex, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	//glEnable(GL_TEXTURE_2D);
	//glBindTexture(GL_TEXTURE_2D, texHandle);
}

void RailActor::initVBOBuffers()
{
	glGenBuffers(1, &vertextHandle);
	glBindBuffer(GL_ARRAY_BUFFER, vertextHandle);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Point), &vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &indexHandle);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexHandle);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBuffer.size() * sizeof(unsigned int), &indexBuffer[0], GL_STATIC_DRAW);
}
