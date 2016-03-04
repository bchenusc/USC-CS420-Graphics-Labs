#pragma once
#include <vector>
#include "SplineTool.h"
#include "glutHeader.h"
#include "openGLHeader.h"
#include "Actor.h"

using std::vector;

struct SplineActor : public Actor
{
	// Inherited
	virtual void Init();
	virtual void Draw(GLuint program);

	// VBO Buffers
	std::vector<Point> points;
	std::vector<Point> tangents;
	std::vector<Point> normals;
	vector<unsigned> wireIndexBuffer;

	// World Spline
	Spline* splines;
	int splineNums;

private:
	void splineInitVBOBuffers();
	void splineGeneratePoints(int controlPoints);
	void splineGenerateTangents(int controlPoints);
	void splineGenerateNormals(int controlPoints);
	void splineDraw(GLuint program);
	void splineGenerateBasisVector();

	// Spline Calculation Values.
	Point4 catmullBasis[4];
	const double splineTensionParam = 0.5;
	const double splineUStepSize = 0.01;

	// VBO Handles
	GLuint vertexHandle;
	GLuint indexHandle;

	// VAO Handle
	GLuint splineVAOHandle;
};

void SplineActor::Init()
{
	splineGenerateBasisVector();
	splineGeneratePoints(splines[0].numControlPoints);
	splineGenerateTangents(splines[0].numControlPoints);
	splineGenerateNormals(splines[0].numControlPoints);
	splineInitVBOBuffers();
}

void SplineActor::splineGeneratePoints(int controlPoints)
{
	points.reserve(controlPoints * (int)(1.0 / splineUStepSize));

	// Generate the points on the spline.
	for (int i = 0; i < controlPoints - 3; ++i)
	{
		for (double u = 0.0; u <= 1.0; u += splineUStepSize)
		{
			// Calculate spline points
			points.push_back(
				CatmullRomAlgorithm(
				u, catmullBasis[0], catmullBasis[1], catmullBasis[2], catmullBasis[3],
				splines[0].points[i], splines[0].points[i + 1], splines[0].points[i + 2], splines[0].points[i + 3]));
		}
	}
}

void SplineActor::splineGenerateTangents(int controlPoints)
{
	tangents.reserve(controlPoints * (int)(1.0 / splineUStepSize));

	// Generate the tangents to the spline.
	for (int i = 0; i < controlPoints - 3; ++i)
	{
		for (double u = 0.0; u <= 1.0; u += splineUStepSize)
		{
			// Calculate spline tangents.
			tangents.push_back(Normalize(
				CatmullRomAlgorithmDerivative(
				u, catmullBasis[0], catmullBasis[1], catmullBasis[2], catmullBasis[3],
				splines[0].points[i], splines[0].points[i + 1], splines[0].points[i + 2], splines[0].points[i + 3])));
		}
	}
}

void SplineActor::splineGenerateNormals(int controlPoints)
{
	normals.reserve(controlPoints * (int)(1.0 / splineUStepSize));

	// Predefine up as the initial normal vector.
	Point n(0, 1, 0);
	Point b = Normalize(pCross(tangents[0], n));
	// Generate the normals.
	for (int i = 0; i < controlPoints - 3; ++i)
	{
		for (double u = 0.0; u <= 1.0; u += splineUStepSize)
		{
			// Calculate normal for camera to use as the up vector..
			normals.push_back(n);
			if (normals.size() < tangents.size())
			{
				n = Normalize(pCross(b, tangents[normals.size()]));
				b = Normalize(pCross(tangents[normals.size()], n));
			}
			else
			{
				break;
			}
		}
	}
}

void SplineActor::splineGenerateBasisVector()
{
	// Basis vector is used to generate the spline.
	const double s = splineTensionParam;
	catmullBasis[0] = Point4(-s, 2 - s, s - 2, s);
	catmullBasis[1] = Point4(2 * s, s - 3, 3 - 2 * s, -s);
	catmullBasis[2] = Point4(-s, 0, s, 0);
	catmullBasis[3] = Point4(0, 1, 0, 0);
}

void SplineActor::splineInitVBOBuffers()
{
	/*
	Index Array
	Looks like: 0, 1, 1, 2, 2, 3, 3, 4, 4 ... n - 2, n - 2, n - 1
	*/

	wireIndexBuffer.reserve(1000);
	wireIndexBuffer.push_back(0);
	for (unsigned i = 1; i < points.size(); ++i)
	{
		wireIndexBuffer.push_back(i);
		wireIndexBuffer.push_back(i);
	}
	wireIndexBuffer.push_back(points.size() - 1);

	/*
	GLuint vertexBuffer (vec3);
	*/
	glGenBuffers(1, &vertexHandle);
	glBindBuffer(GL_ARRAY_BUFFER, vertexHandle);
	glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(Point), &points[0], GL_STATIC_DRAW);

	glGenBuffers(1, &indexHandle);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexHandle);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, wireIndexBuffer.size() * sizeof(unsigned int), &wireIndexBuffer[0], GL_STATIC_DRAW);
}

void SplineActor::Draw(GLuint program)
{
	// Bind VAO Handles
	glGenVertexArrays(1, &splineVAOHandle);
	glBindVertexArray(splineVAOHandle);

	// Attribute 1: Position
	GLuint attrib_pos = glGetAttribLocation(program, "position");
	glEnableVertexAttribArray(attrib_pos);
	glBindBuffer(GL_ARRAY_BUFFER, vertexHandle);
	glVertexAttribPointer(attrib_pos, 3, GL_DOUBLE, GL_FALSE, 0, (void*)0);

	// Draw Wireframe
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexHandle);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDrawElements(GL_LINES, wireIndexBuffer.size(), GL_UNSIGNED_INT, (void*)0);
	glBindVertexArray(0);
}

