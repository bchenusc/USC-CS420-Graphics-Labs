/*
  CSCI 420 Computer Graphics, USC
  Assignment 1: Height Fields
  C++ starter code

  Student username: <type your USC username here>
  */

#include <iostream>
#include <cstring>
#include <vector>
#include <sstream>
#include <iomanip>

#include "openGLHeader.h"
#include "glutHeader.h"
#include "imageIO.h"
#include "openGLMatrix.h"
#include "basicPipelineProgram.h"

#include "SplineTool.h"
#include "PlaneActor.h"
#include "PlanePlacements.h"
#include "SplineActor.h"
#include "Textures.h"

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

// hw2
void initTextures();
void initActors();
void drawActors();

void displayFunc();
void idleFunc();
void initScene();
void keyboardFunc(unsigned char key, int x, int y);
void reshapeFunc(int w, int h);
void saveScreenshot(const char* filename);

// Camera
float camLookAtEye[3] = { 0.0f, 0.0f, 0.0f };
float camLookAtTarget[3] = { 0.0f, 0.0f, 0.0f };

int windowWidth = 1280;
int windowHeight = 720;
char windowTitle[512] = "CSCI 420 homework II";

ImageIO * heightmapImage;

// ############### My Code ############### //
OpenGLMatrix* matrix;
BasicPipelineProgram* pipelineProgram;
GLuint program;

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

int coasterStep = 0;
const float coasterMoveSpeed = 0.001f;
float coasterMoveCounter = 0;

SplineActor splineActor;

const int textureCouns = 2;
string textures[] =
{
	"./Hw2Textures/ground.jpg",
	"./Hw2Textures/sky.jpg"
};

const int textureHandles[]
{
	0, // ground.jpg
	1, // sky.jpg
};

PlaneActor groundActor(textureHandles[0], groundVertices);
PlaneActor skyTopActor(textureHandles[1], skyTopVertices);
PlaneActor skyH1Actor(textureHandles[1], skyHorizVerts1);
PlaneActor skyH2Actor(textureHandles[1], skyHorizVerts2);
PlaneActor skyH3Actor(textureHandles[1], skyHorizVerts3);
PlaneActor skyH4Actor(textureHandles[1], skyHorizVerts4);

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
	initSpline(argc, argv, &splineActor.splines, splineActor.splineNums);
	initTextures();
	initActors();

	// sink forever into the glut loop
	glutMainLoop();
}

void initTextures()
{
	for (int i = 0; i < 2; i++)
	{
		initTextureWrapper(textures[i], textureHandles[i]);
	}
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

void initActors()
{
	splineActor.Init();
	groundActor.Init();
	skyTopActor.Init();
	skyH1Actor.Init();
	skyH2Actor.Init();
	skyH3Actor.Init();
	skyH4Actor.Init();
}

void drawActors()
{
	splineActor.Draw(program);
	groundActor.Draw(program);
	skyTopActor.Draw(program);
	skyH1Actor.Draw(program);
	skyH2Actor.Draw(program);
	skyH3Actor.Draw(program);
	skyH4Actor.Draw(program);
}

void initPipelineProgram()
{
	if (pipelineProgram != nullptr) delete pipelineProgram;
	pipelineProgram = new BasicPipelineProgram();
	pipelineProgram->Init("../openGLHelper-starterCode");
}

void displayFunc()
{
	glClear(GL_COLOR_BUFFER_BIT |
		GL_DEPTH_BUFFER_BIT);
	matrix->LoadIdentity();

	// Camera Normals
	float normalX = (float)splineActor.normals[coasterStep].x;
	float normalY = (float)splineActor.normals[coasterStep].y;
	float normalZ = (float)splineActor.normals[coasterStep].z;

	// Camera Look targets.
	camLookAtTarget[0] = camLookAtEye[0] + normalX - (float)splineActor.tangents[coasterStep].x * 4.0f;
	camLookAtTarget[1] = camLookAtEye[1] + normalY - (float)splineActor.tangents[coasterStep].y * 4.0f;
	camLookAtTarget[2] = camLookAtEye[2] + normalZ - (float)splineActor.tangents[coasterStep].z * 4.0f;

	matrix->LookAt(
		camLookAtEye[0] + normalX, camLookAtEye[1] + normalY, camLookAtEye[2] + normalZ,
		camLookAtTarget[0], camLookAtTarget[1], camLookAtTarget[2],
		normalX, normalY, normalZ);

	// Actors
	bindProgram();
	drawActors();

	glutSwapBuffers();
}

void bindProgram()
{
	pipelineProgram->Bind();
	program = pipelineProgram->GetProgramHandle();
	bindProjectionMatrixToProgram();
	drawActors();
}

void bindProjectionMatrixToProgram()
{
	// Write modelview matrix to shader
	GLint h_modelViewMatrix = glGetUniformLocation(program, "projectionModelViewMatrix");
	float m[16]; // column major.
	matrix->GetProjectionModelViewMatrix(m);
	glUniformMatrix4fv(h_modelViewMatrix, 1, GL_FALSE, m);
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
		camLookAtEye[0] = (float)splineActor.points[coasterStep].x;
		camLookAtEye[1] = (float)splineActor.points[coasterStep].y;
		camLookAtEye[2] = (float)splineActor.points[coasterStep].z;

		if (coasterStep > 0)
		{
			--coasterStep;
		}
		else
		{
			coasterStep = splineActor.tangents.size() - 1;
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


