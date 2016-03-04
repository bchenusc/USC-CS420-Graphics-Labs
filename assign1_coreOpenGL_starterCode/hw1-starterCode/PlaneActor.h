#pragma once
#include <vector>
#include "SplineTool.h"
#include "glutHeader.h"
#include "openGLHeader.h"
#include "Actor.h"
#include "Textures.h"
#include <string>

using std::string;

struct PlaneActor : public Actor
{
	PlaneActor(string texture, const float verts[4][3]);
	~PlaneActor();
	virtual void Init();
	virtual void Draw(GLuint program);

	// VBO Handles
	GLuint vertextHandle;
	GLuint texCoordHandle;
	GLuint texHandle;

	// VAO Handle
	GLuint vaoHandle;

private :
	void initVBOBuffers();
	void initPlaneTexture();

	float texCoords[4][2];
	float vertices[4][3];
	string texture;
};

PlaneActor::PlaneActor(string textureFile, const float verts[4][3])
{
	// Copy over the correct vertices.
	for (int row = 0; row < 4; ++row)
	{
		for (int col = 0; col < 3; ++col)
		{
			vertices[row][col] = verts[row][col];
		}
	}

	texCoords[0][0] = 0; texCoords[0][1] = 0;
	texCoords[1][0] = 0; texCoords[1][1] = 1;
	texCoords[2][0] = 1; texCoords[2][1] = 1;
	texCoords[3][0] = 1; texCoords[3][1] = 0;

	texture = textureFile;
	texHandle = makeTextureHandle();
}

PlaneActor::~PlaneActor()
{
	glDeleteTextures(1, &texHandle);
}

void PlaneActor::Init()
{
	initPlaneTexture();
	initVBOBuffers();
}

void PlaneActor::Draw(GLuint program)
{
	// Bind VAO
	glGenVertexArrays(1, &vaoHandle);
	glBindVertexArray(vaoHandle);

	// Attribute 1: Position
	GLuint attrib_pos = glGetAttribLocation(program, "position");
	glEnableVertexAttribArray(attrib_pos);
	glBindBuffer(GL_ARRAY_BUFFER, vertextHandle);
	glVertexAttribPointer(attrib_pos, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// Attribute 2: Texture
	GLuint attrib_tex = glGetAttribLocation(program, "texCoord");
	glEnableVertexAttribArray(attrib_tex);
	// Bind the texture
	glBindBuffer(GL_ARRAY_BUFFER, texCoordHandle);
	glVertexAttribPointer(attrib_tex, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	setTextureUnit(GL_TEXTURE0, program);
	glBindTexture(GL_TEXTURE_2D, texHandle);

	glDrawArrays(GL_QUADS, 0, 4);
	glBindVertexArray(0);
	glDeleteTextures(1, &texHandle);
}

void PlaneActor::initPlaneTexture()
{
	int code = initTexture(texture.c_str(), texHandle);
	if (code != 0)
	{
		printf("Error loading the texture image. \n");
		exit(EXIT_FAILURE);
	}
}

void PlaneActor::initVBOBuffers()
{
	glGenBuffers(1, &vertextHandle);
	glBindBuffer(GL_ARRAY_BUFFER, vertextHandle);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &texCoordHandle);
	glBindBuffer(GL_ARRAY_BUFFER, texCoordHandle);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords), texCoords, GL_STATIC_DRAW);
}