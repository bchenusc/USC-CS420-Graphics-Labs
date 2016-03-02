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
void initMapImage(const char* fileName);
void initMap3D();
void initTerrainVertices();
void initTerrainColors();
void initTerrainIndices();
void initBuffers();
void initPipelineProgram();
void bindProgram();
void bindProjectionMatrixToProgram();
void bindAndDrawVertexToProgram();

// hw2
void initTerrain();
void initSpline();
void initSplineBuffers();
void initTerrainBuffers();
void initTerrainTexture();
void generateBasisVector();
void generateSplinePoints(int controlPoints);
void drawSpline();
void drawGround();

void displayFunc();
void idleFunc();
void initScene();
void keyboardFunc(unsigned char key, int x, int y);
void mouseButtonFunc(int button, int state, int x, int y);
void mouseMotionFunc(int x, int y);
void mouseMotionDragFunc(int x, int y);
void mouseMotionDragFunc(int button, int state, int x, int y);
void reshapeFunc(int w, int h);
void saveScreenshot(const char* filename);

int mousePos[2]; // x,y coordinate of the mouse position

int leftMouseButton = 0; // 1 if pressed, 0 if not 
int middleMouseButton = 0; // 1 if pressed, 0 if not
int rightMouseButton = 0; // 1 if pressed, 0 if not

// state of the world
float camRotate[3] = { 0.0f, 0.0f, 0.0f };
float camTranslate[3] = { 0.0f, 0.0f, 0.0f };
float camZoom[3] = { 1.0f, 1.0f, 1.0f };

int windowWidth = 1280;
int windowHeight = 720;
char windowTitle[512] = "CSCI 420 homework II";

ImageIO * heightmapImage;

// ############### My Code ############### //
enum DRAW_STATE { DS_POINT = 0, DS_WIRE = 1};
DRAW_STATE drawState = DS_WIRE;

OpenGLMatrix* matrix;
BasicPipelineProgram* pipelineProgram;
GLuint program;

const float CAMERA_HEIGHT = 1.0f;
const float CAMERA_DEPTH = 1.0f;
const float MAX_COLOR = 255.0f;

GLuint splineVertexBuffer;
GLuint terrainVertexBuffer;
GLuint indexBuffer;
GLuint splineArray;
GLuint terrainArray;


int oldTime = 0; /* Calculates delta time. */
float idleRotationSpeed = 0.025f;
bool allowIdleScreenCapture = false;

float rotateSensitivity = 0.05f;
float translateSensitivity = 0.025f;
float zoomSensitivity = 0.05f; 
float scaleSensitivity = 0.01f;

int screenshotCounter = 0;
float screenshotDelayCounter = 0;
const float screenshotDelay = 0.0666f;
bool startScreenshotRecording = false;

// Hw2
Spline* splines /* The splines array */;
int numSplines /* Total number of splines. */;

std::vector<unsigned int> wireFrameIndex;
std::vector<Point> splinePoints;
Point4 catmullBasis[4];
const double sTensionParameter = 0.5;
const double uStepSize = 0.01;

float terrainVertices[6][3] =
{
	{ -1, 0, -1 },
	{ -1, 0, 1 },
	{ 1, 0, -1 },
	{ 1, 0, 1 },
	{ 2, 0, -1 },
	{ 2, 0, 1}
};

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		cout << "The arguments are incorrect." << endl;
		cout << "usage: ./hw1 <heightmap file>" << endl;
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
	initSpline();
	initSpline(argc, argv, &splines, numSplines);
	initSplineBuffers();

	initTerrain();
	initTerrainBuffers();

	// sink forever into the glut loop
	glutMainLoop();
}

void initHandlers()
{
	// tells glut to use a particular display function to redraw 
	glutDisplayFunc(displayFunc);
	// perform animation inside idleFunc
	glutIdleFunc(idleFunc);
	// callback for mouse drags
	glutMotionFunc(mouseMotionDragFunc);
	// callback for idle mouse movement
	glutPassiveMotionFunc(mouseMotionFunc);
	// callback for mouse button changes
	glutMouseFunc(mouseButtonFunc);
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

void initSpline()
{

}

void initTerrain()
{

}

void initSplineBuffers()
{
	generateSplinePoints(splines[0].numControlPoints);

	/*
	Index Array
	Looks like: 0, 1, 1, 2, 2, 3, 3, 4, 4 ... n - 2, n - 2, n - 1
	*/

	wireFrameIndex.reserve(1000);
	wireFrameIndex.push_back(0);
	for (unsigned i = 1; i < splinePoints.size(); ++i)
	{
		wireFrameIndex.push_back(i);
		wireFrameIndex.push_back(i);
	}
	wireFrameIndex.push_back(splinePoints.size() - 1);

	/*
		GLuint vertexBuffer (vec3);
	*/
	glGenBuffers(1, &splineVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, splineVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, splinePoints.size() * sizeof(Point), &splinePoints[0], GL_STATIC_DRAW);	

	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, wireFrameIndex.size() * sizeof(unsigned int), &wireFrameIndex[0], GL_STATIC_DRAW);
}

void initTerrainBuffers()
{
	glGenBuffers(1, &terrainVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, terrainVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(terrainVertices), terrainVertices, GL_STATIC_DRAW);
}

void initTerrainTextures()
{

}

void generateSplinePoints(int controlPoints)
{
	generateBasisVector();
	splinePoints.reserve(controlPoints * (int)(1.0 / uStepSize));

	// Generate the points on the spline.
	for (int i = 0; i < controlPoints - 3; ++i)
	{
		for (double u = 0.0; u <= 1.0; u += uStepSize)
		{
			splinePoints.push_back(
				CatmullRomAlgorithm(
				u, catmullBasis[0], catmullBasis[1], catmullBasis[2], catmullBasis[3],
				splines[0].points[i], splines[0].points[i + 1], splines[0].points[i + 2], splines[0].points[i + 3]));
		}
	}
}

void generateBasisVector()
{
	// Basis vector is used to generate the spline.
	const double s = sTensionParameter;
	catmullBasis[0] = Point4(-s, 2 - s, s - 2, s);
	catmullBasis[1] = Point4(2*s, s-3, 3-2*s, -s);
	catmullBasis[2] = Point4(-s, 0, s, 0);
	catmullBasis[3] = Point4(0, 1, 0, 0);
}

void displayFunc()
{
	glClear(GL_COLOR_BUFFER_BIT |
		GL_DEPTH_BUFFER_BIT);
	matrix->LoadIdentity();
	// Camera controls:
	matrix->LookAt(0, CAMERA_HEIGHT + camZoom[2], CAMERA_DEPTH + camZoom[2], 0, 0, 0, 0, 1, 0);
	matrix->Translate(camTranslate[0], camTranslate[1], 0);
	matrix->Rotate(camRotate[0], 1, 0, 0);
	matrix->Rotate(camRotate[1], 0, 1, 0);
	matrix->Rotate(camRotate[2], 0, 0, 1);
	
	bindProgram();
	glutSwapBuffers();
}

void bindProgram()
{
	pipelineProgram->Bind();
	program = pipelineProgram->GetProgramHandle();
	bindProjectionMatrixToProgram();
	bindAndDrawVertexToProgram();
}

void bindProjectionMatrixToProgram()
{
	// Write modelview matrix to shader
	GLint h_modelViewMatrix = glGetUniformLocation(program, "projectionModelViewMatrix");
	float m[16]; // column major.
	matrix->GetProjectionModelViewMatrix(m);
	glUniformMatrix4fv(h_modelViewMatrix, 1, GL_FALSE, m);
}

void bindAndDrawVertexToProgram()
{
	drawSpline();
	drawGround();
}

void drawSpline()
{
	/*
	GLuint vertexArray;
	*/
	glGenVertexArrays(1, &splineArray);
	glBindVertexArray(splineArray);
	// Attribute 1: Position
	GLuint attrib_pos = glGetAttribLocation(program, "position");
	glEnableVertexAttribArray(attrib_pos);
	glBindBuffer(GL_ARRAY_BUFFER, splineVertexBuffer);
	glVertexAttribPointer(attrib_pos, 3, GL_DOUBLE, GL_FALSE, 0, (void*)0);

	// Draw Indexed
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glDrawElements(GL_LINES, wireFrameIndex.size(), GL_UNSIGNED_INT, (void*)0);
	glBindVertexArray(0);
}

void drawGround()
{
	/*
	GLuint vertexArray;
	*/
	glGenVertexArrays(1, &terrainArray);
	glBindVertexArray(terrainArray);
	// Attribute 1: Position
	GLuint attrib_pos = glGetAttribLocation(program, "position");
	glEnableVertexAttribArray(attrib_pos);
	glBindBuffer(GL_ARRAY_BUFFER, terrainVertexBuffer);
	glVertexAttribPointer(attrib_pos, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);
	glBindVertexArray(0);
}

void idleFunc()
{
	// Calculate deltaTime
	int newTime = glutGet(GLUT_ELAPSED_TIME);
	int deltaTime = newTime - oldTime;
	oldTime = newTime;

	if ((leftMouseButton | rightMouseButton | middleMouseButton) == 1)
	{
		// Redisplay the screen as normal if mouse inputs.
		glutPostRedisplay();
	}
	else
	{
		// Rotate map by delta time if no input.
		//camRotate[1] += deltaTime * idleRotationSpeed;
		glutPostRedisplay();
	}

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

void mouseMotionDragFunc(int x, int y)
{
	// mouse has moved and one of the mouse buttons is pressed (dragging)
	// the change in mouse position since the last invocation of this function
	int mousePosDelta[2] = { x - mousePos[0], y - mousePos[1] };

	if (rightMouseButton)
	{
	// translate the camera via the right mouse button.
	camTranslate[0] += mousePosDelta[0] * translateSensitivity;
	camTranslate[1] -= mousePosDelta[1] * translateSensitivity;
	}

	if (leftMouseButton)
	{
	// translate the camera via the left mouse button.
	camRotate[0] += mousePosDelta[1] * rotateSensitivity;
	camRotate[1] += mousePosDelta[0] * rotateSensitivity;
	}

	// store the new mouse position
	mousePos[0] = x;
	mousePos[1] = y;
}

void mouseMotionFunc(int x, int y)
{
  // mouse has moved
  // store the new mouse position
  mousePos[0] = x;
  mousePos[1] = y;
}

void mouseButtonFunc(int button, int state, int x, int y)
{
  // a mouse button has has been pressed or depressed

  // keep track of the mouse button state, in leftMouseButton, middleMouseButton, rightMouseButton variables
  switch (button)
  {
    case GLUT_LEFT_BUTTON:
      leftMouseButton = (state == GLUT_DOWN);
	  oldTime = glutGet(GLUT_ELAPSED_TIME);
    break;

    case GLUT_MIDDLE_BUTTON:
      middleMouseButton = (state == GLUT_DOWN);
	  oldTime = glutGet(GLUT_ELAPSED_TIME);
    break;

    case GLUT_RIGHT_BUTTON:
      rightMouseButton = (state == GLUT_DOWN);
	  oldTime = glutGet(GLUT_ELAPSED_TIME);
    break;

	case 3 /* Scroll up */:
		if (state == GLUT_DOWN) { camZoom[2] -= 1.0f; }
	break;

	case 4 /* Scroll down */:
		if (state == GLUT_DOWN) { camZoom[2] += 1.0f; }
	break;
  }

  // store the new mouse position
  mousePos[0] = x;
  mousePos[1] = y;
}

void keyboardFunc(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27: // ESC key
		exit(0); // exit the program
		break;

	case '1':
		// Draw Points
		drawState = DRAW_STATE::DS_POINT;

		glutPostRedisplay();
		break;

	case '2':
		// Draw wires.
		drawState = DRAW_STATE::DS_WIRE;
		glutPostRedisplay();
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


