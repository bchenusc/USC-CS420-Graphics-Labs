/*
  CSCI 420 Computer Graphics, USC
  Assignment 1: Height Fields
  C++ starter code

  Student username: <type your USC username here>
*/

#include <iostream>
#include <cstring>
#include "openGLHeader.h"
#include "glutHeader.h"

#include "imageIO.h"
#include "openGLMatrix.h"
#include "basicPipelineProgram.h"
#include <vector>
#include <sstream>
#include <iomanip>
#include "SplineTool.h"
#include "Textures.h"
#include "Spline.h"

#ifdef WIN32
  #ifdef _DEBUG
    #pragma comment(lib, "glew32d.lib")
  #else
    #pragma comment(lib, "glew32.lib")
  #endif
#endif

#ifdef WIN32
  char shaderBasePath[1024] = SHADER_BASE_PATH;
#else
  char shaderBasePath[1024] = "../openGLHelper";
#endif

using namespace std;

void initHandlers();
void initPipelineProgram();
void bindProgram();
void bindProjectionMatrixToProgram();
void drawScene();

// hw2
void initGround();
void initGroundBuffers();
void initGroundTextures();
void drawGround();
void setTextureUnit(GLint unit);

void displayFunc();
void idleFunc();
void initScene();
void keyboardFunc(unsigned char key, int x, int y);
void reshapeFunc(int w, int h);
void saveScreenshot(const char* filename);

// Camera
float camLookat[3] = { 0.0f, 0.0f, 0.0f };

int windowWidth = 1280;
int windowHeight = 720;
char windowTitle[512] = "CSCI 420 homework II";

ImageIO * heightmapImage;

// ############### My Code ############### //
OpenGLMatrix* matrix;
BasicPipelineProgram* pipelineProgram;
GLuint program;

// VBO Handles

GLuint groundVertexHandle;
GLuint groundTexCoordHandle;
GLuint groundTexHandle;

// VAO Handles

GLuint groundVAOHandle;

int oldTime = 0; /* Calculates delta time. */
float idleRotationSpeed = 0.025f;
bool allowIdleScreenCapture = false;

float rotateSensitivity = 0.05f;
float zoomSensitivity = 0.025f; 
float scaleSensitivity = 0.01f;

int screenshotCounter = 0;
float screenshotDelayCounter = 0;
const float screenshotDelay = 0.0666f;
bool startScreenshotRecording = false;

// Hw2


int coasterTangentCounter = 0;
const float coasterMoveSpeed = 0.001f;
float coasterMoveCounter = 0;



//Sky* sky;

float terrainVertices[4][3] =
{
	{ -256, -10, -256 },
	{ -256, -10, 256 },
	{ 256, -10, 256 },
	{ 256, -10, -256 },
};

float terrainTexCoord[4][2] =
{
	{0,0},
	{0,1},
	{1,1},
	{1,0}
};

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		cout << "The arguments are incorrect." << endl;
		cout << "usage: ./hw1 <track file>" << endl;
		exit(EXIT_FAILURE);
	}

	cout << "Initializing GLUT..." << endl;
	glutInit(&argc, argv);

	cout << "Initializing OpenGL..." << endl;
	#ifdef __APPLE__
	glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
	#else
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
	#endif

	glutInitWindowSize(windowWidth, windowHeight);
	glutInitWindowPosition(0, 0);
	glutCreateWindow(windowTitle);

	cout << "OpenGL Version: " << glGetString(GL_VERSION) << endl;
	cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << endl;
	cout << "Shading Language Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

	initHandlers();

	// init glew
	#ifdef __APPLE__
	// nothing is needed on Apple
	#else
	// Windows, Linux
	GLint result = glewInit();
	if (result != GLEW_OK)
	{
		cout << "error: " << glewGetErrorString(result) << endl;
		exit(EXIT_FAILURE);
	}
	#endif

	// do initialization
	initScene();
	
	// hw2
	initSpline(argc, argv, &splines, splineNums);
	splineInitBuffers();

	initGround();
	//sky = new Sky(program);
	//sky->initSky();

	// sink forever into the glut loop
	glutMainLoop();
}

void initHandlers()
{
	// tells glut to use a particular display function to redraw 
	glutDisplayFunc(displayFunc);
	// perform animation inside idleFunc
	glutIdleFunc(idleFunc);
	// callback for resizing the window
	glutReshapeFunc(reshapeFunc);
	// callback for pressing the keys on the keyboard
	glutKeyboardFunc(keyboardFunc);
}

void initScene()
{
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	if (matrix != nullptr) delete matrix;
	matrix = new OpenGLMatrix();
	initPipelineProgram();
}

void initPipelineProgram()
{
	if (pipelineProgram != nullptr) delete pipelineProgram;
	pipelineProgram = new BasicPipelineProgram();
	pipelineProgram->Init("../openGLHelper-starterCode");
}






void initGround()
{
	initGroundTextures();
	initGroundBuffers();
}

void initGroundTextures()
{
	int code = initTexture("./Hw2Textures/ground.jpg", groundTexHandle);
	if (code != 0)
	{
		printf("Error loading the texture image. \n");
		exit(EXIT_FAILURE);
	}
}

void initGroundBuffers()
{
	glGenBuffers(1, &groundVertexHandle);
	glBindBuffer(GL_ARRAY_BUFFER, groundVertexHandle);
	glBufferData(GL_ARRAY_BUFFER, sizeof(terrainVertices), terrainVertices, GL_STATIC_DRAW);

	glGenBuffers(1, &groundTexCoordHandle);
	glBindBuffer(GL_ARRAY_BUFFER, groundTexCoordHandle);
	glBufferData(GL_ARRAY_BUFFER, sizeof(terrainTexCoord), terrainTexCoord, GL_STATIC_DRAW);

}

void displayFunc()
{
	glClear(GL_COLOR_BUFFER_BIT |
		GL_DEPTH_BUFFER_BIT);
	matrix->LoadIdentity();
	// Camera controls:

	matrix->LookAt(
		camLookat[0],
		camLookat[1],
		camLookat[2], camLookat[0] + -splineTangents[coasterTangentCounter].x * 4.0f, 
		camLookat[1] - splineTangents[coasterTangentCounter].y * 4.0f,
		camLookat[2] + -splineTangents[coasterTangentCounter].z * 4.0f,
		splineNormals[coasterTangentCounter].x, 
		splineNormals[coasterTangentCounter].y, splineNormals[coasterTangentCounter].z);

	setTextureUnit(GL_TEXTURE0);

	bindProgram();
	glutSwapBuffers();
}

void setTextureUnit(GLint unit)
{
	glActiveTexture(unit); // select the active texture unit.
	//get a handle to the "textureImage" shader variable
	GLint h_textureImage = glGetUniformLocation(program, "TextureImage");
	glUniform1i(h_textureImage, unit - GL_TEXTURE);
}

void bindProgram()
{
	pipelineProgram->Bind();
	program = pipelineProgram->GetProgramHandle();
	bindProjectionMatrixToProgram();
	drawScene();
}

void bindProjectionMatrixToProgram()
{
	// Write modelview matrix to shader
	GLint h_modelViewMatrix = glGetUniformLocation(program, "projectionModelViewMatrix");
	float m[16]; // column major.
	matrix->GetProjectionModelViewMatrix(m);
	glUniformMatrix4fv(h_modelViewMatrix, 1, GL_FALSE, m);
}

void drawScene()
{
	drawSpline(program);
	drawGround();
	//sky->draw();
}


void drawGround()
{
	/*
	GLuint vertexArray;
	*/
	glGenVertexArrays(1, &groundVAOHandle);
	glBindVertexArray(groundVAOHandle);
	
	// Attribute 1: Position
	GLuint attrib_pos = glGetAttribLocation(program, "position");
	glEnableVertexAttribArray(attrib_pos);
	glBindBuffer(GL_ARRAY_BUFFER, groundVertexHandle);
	glVertexAttribPointer(attrib_pos, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// Attribute 2: Texture
	GLuint attrib_tex = glGetAttribLocation(program, "texCoord");
	glEnableVertexAttribArray(attrib_tex);
	// bind the texture
	glBindBuffer(GL_ARRAY_BUFFER, groundTexCoordHandle);
	glVertexAttribPointer(attrib_tex, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindTexture(GL_TEXTURE_2D, groundTexHandle);
	glDrawArrays(GL_QUADS, 0, 4);
	glBindVertexArray(0);
}

void idleFunc()
{
	// Calculate deltaTime
	int newTime = glutGet(GLUT_ELAPSED_TIME);
	int deltaTime = newTime - oldTime;
	oldTime = newTime;

	// Move the camera
	if (coasterMoveCounter <= 0)
	{
		coasterMoveCounter = coasterMoveSpeed + coasterMoveCounter;
		camLookat[0] = splinePoints[coasterTangentCounter].x;
		camLookat[1] = splinePoints[coasterTangentCounter].y;
		camLookat[2] = splinePoints[coasterTangentCounter].z;
		
		if (coasterTangentCounter > 0)
		{
			--coasterTangentCounter;
		}
		else
		{
			coasterTangentCounter = splineTangents.size() - 1;
		}

	}
	if (coasterMoveCounter <= coasterMoveSpeed)
	{
		coasterMoveCounter -= deltaTime;
	}

	glutPostRedisplay();

	// Screen recording:
	if (startScreenshotRecording)
	{
		// Create screenshots.
		if (screenshotDelayCounter <= 0)
		{
			screenshotDelayCounter = screenshotDelay;
			std::stringstream ss;
			ss << "Screenshots/anim-" << std::setfill('0') << std::setw(4) << screenshotCounter << ".jpg"; // Prints 000x - xxxx
			saveScreenshot(ss.str().c_str());
			++screenshotCounter;
		}
		if (screenshotDelayCounter <= screenshotDelay)
		{
			screenshotDelayCounter -= deltaTime;
		}
	}
}

void reshapeFunc(int w, int h)
{
  glViewport(0, 0, w, h);

  // setup perspective matrix...
  GLfloat aspect = (GLfloat)w / (GLfloat)h;
  glViewport(0, 0, w, h);
  matrix->SetMatrixMode(OpenGLMatrix::Projection);
  matrix->LoadIdentity();
  matrix->Perspective(60, aspect, 0.01f, 1000);
  matrix->SetMatrixMode(OpenGLMatrix::ModelView);
}

void keyboardFunc(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27: // ESC key
		exit(0); // exit the program
		break;

	case 'p':
		startScreenshotRecording = !startScreenshotRecording;
		break;
	
	case 'x':
		saveScreenshot("screenshot.jpg");
		break;
	}
}

// write a screenshot to the specified filename
void saveScreenshot(const char * filename)
{
	unsigned char * screenshotData = new unsigned char[windowWidth * windowHeight * 3];
	glReadPixels(0, 0, windowWidth, windowHeight, GL_RGB, GL_UNSIGNED_BYTE, screenshotData);

	ImageIO screenshotImg(windowWidth, windowHeight, 3, screenshotData);

	if (screenshotImg.save(filename, ImageIO::FORMAT_JPEG) == ImageIO::OK)
		cout << "File " << filename << " saved successfully." << endl;
	else cout << "Failed to save file " << filename << '.' << endl;

	delete[] screenshotData;
}


