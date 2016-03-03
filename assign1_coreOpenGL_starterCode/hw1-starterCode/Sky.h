#pragma once
#include <vector>
#include "glutHeader.h"
#include "openGLHeader.h"
#include "Terrain.h"
#include "imageIO.h"


using std::vector;

struct Sky
{

	Sky(const GLuint& program);
	~Sky();
	void initSky();
	void draw();
	void initTextures();
	void initBuffers();

	GLuint program;
	GLuint vertexBuffer;
	GLuint textureBuffer;
	GLuint textureHandle;
	GLuint vao;

	float vertices[4][3];
	float textureCoordinates[4][2];
};

Sky::Sky(const GLuint& program)
{
	this->program = program;
	initSky();
	initTextures();
	initBuffers();
}

Sky::~Sky()
{
	glDeleteTextures(1, &textureHandle);
}

void Sky::initSky()
{
	vertices[0][0] = -256; vertices[0][1] = -5; vertices[0][2] = -256;
	vertices[1][0] = -256; vertices[1][1] = -5; vertices[1][2] = 256;
	vertices[2][0] = 256; vertices[2][1] = -5; vertices[2][2] = 256;
	vertices[3][0] = 256; vertices[3][1] = -5; vertices[3][2] = -256;

	textureCoordinates[0][0] = 0; textureCoordinates[0][1] = 0;
	textureCoordinates[1][0] = 0; textureCoordinates[1][1] = 1;
	textureCoordinates[2][0] = 1; textureCoordinates[2][1] = 1;
	textureCoordinates[3][0] = 1; textureCoordinates[3][1] = 0;
}

void Sky::initTextures()
{
	int code = initTexture("./Hw2Textures/sky.jpg", textureHandle);
	if (code != 0)
	{
		printf("Error loading the texture image. \n");
		exit(EXIT_FAILURE);
	}
}

void Sky::initBuffers()
{
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &textureBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, textureBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(textureCoordinates), textureCoordinates, GL_STATIC_DRAW);
}

void Sky::draw()
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Attribute 1: Position
	GLuint attrib_pos = glGetAttribLocation(program, "position");
	glEnableVertexAttribArray(attrib_pos);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glVertexAttribPointer(attrib_pos, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// Attribute 2: Texture
	GLuint attrib_tex = glGetAttribLocation(program, "texCoord");
	glEnableVertexAttribArray(attrib_tex);
	// bind the texture
	glBindBuffer(GL_ARRAY_BUFFER, textureBuffer);
	glVertexAttribPointer(attrib_tex, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	//glBindTexture(GL_TEXTURE_2D, textureHandle);
	glDrawArrays(GL_QUADS, 0, 4);
	glBindVertexArray(0);
}