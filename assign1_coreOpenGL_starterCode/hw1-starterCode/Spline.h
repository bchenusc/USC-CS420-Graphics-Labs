#pragma once
#include "SplineTool.h"
#include "glutHeader.h"
#include "openGLHeader.h"
#include <vector>

using std::vector;

void splineInitBuffers();
void splineGeneratePoints(int controlPoints);
void splineDraw();
void splineGenerateBasisVector();

// World Spline
Spline* splines;
int splineNums;

// VBO Handles
GLuint splineVertexHandle;
GLuint splineIndexHandle;

// VAO Handle
GLuint splineVAOHandle;

// VBO Buffers
std::vector<Point> splinePoints;
std::vector<Point> splineTangents;
std::vector<Point> splineNormals;
vector<unsigned> splineWireDrawIndexBuffer;

// Spline Calculation Values.
Point4 splineCatmullBasis[4];
const double splineTensionParam = 0.5;
const double splineUStepSize = 0.01;

void splineInitBuffers()
{
	splineGeneratePoints(splines[0].numControlPoints);

	/*
	Index Array
	Looks like: 0, 1, 1, 2, 2, 3, 3, 4, 4 ... n - 2, n - 2, n - 1
	*/

	splineWireDrawIndexBuffer.reserve(1000);
	splineWireDrawIndexBuffer.push_back(0);
	for (unsigned i = 1; i < splinePoints.size(); ++i)
	{
		splineWireDrawIndexBuffer.push_back(i);
		splineWireDrawIndexBuffer.push_back(i);
	}
	splineWireDrawIndexBuffer.push_back(splinePoints.size() - 1);

	/*
	GLuint vertexBuffer (vec3);
	*/
	glGenBuffers(1, &splineVertexHandle);
	glBindBuffer(GL_ARRAY_BUFFER, splineVertexHandle);
	glBufferData(GL_ARRAY_BUFFER, splinePoints.size() * sizeof(Point), &splinePoints[0], GL_STATIC_DRAW);

	glGenBuffers(1, &splineIndexHandle);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, splineIndexHandle);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, splineWireDrawIndexBuffer.size() * sizeof(unsigned int), &splineWireDrawIndexBuffer[0], GL_STATIC_DRAW);
}

void drawSpline(GLuint program)
{
	// Bind VAO Handles
	glGenVertexArrays(1, &splineVAOHandle);
	glBindVertexArray(splineVAOHandle);

	// Attribute 1: Position
	GLuint attrib_pos = glGetAttribLocation(program, "position");
	glEnableVertexAttribArray(attrib_pos);
	glBindBuffer(GL_ARRAY_BUFFER, splineVertexHandle);
	glVertexAttribPointer(attrib_pos, 3, GL_DOUBLE, GL_FALSE, 0, (void*)0);

	// Draw Wireframe
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, splineIndexHandle);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDrawElements(GL_LINES, splineWireDrawIndexBuffer.size(), GL_UNSIGNED_INT, (void*)0);
	glBindVertexArray(0);
}

void splineGeneratePoints(int controlPoints)
{
	splineGenerateBasisVector();
	splinePoints.reserve(controlPoints * (int)(1.0 / splineUStepSize));
	splineTangents.reserve(controlPoints * (int)(1.0 / splineUStepSize));
	splineNormals.reserve(controlPoints * (int)(1.0 / splineUStepSize));

	// Generate the points on the spline.
	for (int i = 0; i < controlPoints - 3; ++i)
	{
		for (double u = 0.0; u <= 1.0; u += splineUStepSize)
		{
			// Calculate spline points
			splinePoints.push_back(
				CatmullRomAlgorithm(
				u, splineCatmullBasis[0], splineCatmullBasis[1], splineCatmullBasis[2], splineCatmullBasis[3],
				splines[0].points[i], splines[0].points[i + 1], splines[0].points[i + 2], splines[0].points[i + 3]));

			// Calculate spline tangents.
			splineTangents.push_back(Normalize(
				CatmullRomAlgorithmDerivative(
				u, splineCatmullBasis[0], splineCatmullBasis[1], splineCatmullBasis[2], splineCatmullBasis[3],
				splines[0].points[i], splines[0].points[i + 1], splines[0].points[i + 2], splines[0].points[i + 3])));
		}
	}

	// Generate normals
	Point n(0, 1, 0);
	Point b = Normalize(pCross(splineTangents[0], n));
	// Generate  the normals.
	for (int i = 0; i < controlPoints - 3; ++i)
	{
		for (double u = 0.0; u <= 1.0; u += splineUStepSize)
		{
			// Calculate normal for camera.

			splineNormals.push_back(n);
			if (splineNormals.size() < splineTangents.size())
			{
				n = Normalize(pCross(b, splineTangents[splineNormals.size()]));
				b = Normalize(pCross(splineTangents[splineNormals.size()], n));
			}
			else
			{
				break;
			}
		}
	}
}

void splineGenerateBasisVector()
{
	// Basis vector is used to generate the spline.
	const double s = splineTensionParam;
	splineCatmullBasis[0] = Point4(-s, 2 - s, s - 2, s);
	splineCatmullBasis[1] = Point4(2 * s, s - 3, 3 - 2 * s, -s);
	splineCatmullBasis[2] = Point4(-s, 0, s, 0);
	splineCatmullBasis[3] = Point4(0, 1, 0, 0);
}

