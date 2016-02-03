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
void initArrays();
void initBuffers();
void initPipelineProgram();
void bindProgram();
void bindProjectionMatrixToProgram();
void bindAndDrawVertexToProgram();

void displayFunc();
void idleFunc();
void initScene(int argc, char* argv[]);
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

typedef enum { ROTATE, TRANSLATE, SCALE } CONTROL_STATE;
CONTROL_STATE controlState = ROTATE;

// state of the world
float landRotate[3] = { 0.0f, 0.0f, 0.0f };
float landTranslate[3] = { 0.0f, 0.0f, 0.0f };
float landScale[3] = { 1.0f, 1.0f, 1.0f };

int windowWidth = 1280;
int windowHeight = 720;
char windowTitle[512] = "CSCI 420 homework I";

ImageIO * heightmapImage;

// ############### My Code ############### //
enum DRAW_STATE {DS_POINT, DS_WIRE, DS_SOLID} ;
DRAW_STATE drawState = DS_SOLID;

OpenGLMatrix* matrix;
BasicPipelineProgram* pipelineProgram;
GLuint program;

const float WORLDMAP_SIZE = 255.0f /* Represents one length side. */;
const float WORLDMAP_MAX_HEIGHT = 5.0f /* Scales map larger or smaller. */;
const float CAMERA_HEIGHT = 100.0f;
const float CAMERA_DEPTH = 100.0f;

GLuint vertexBuffer;
GLuint colorBuffer;
GLuint indexBuffer;
GLuint vertexArray;

std::vector<glm::vec3> heightMapVertices;
std::vector<glm::vec4> heightMapVertexColors;
std::vector<unsigned int> heightMapIndices;

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
	initScene(argc, argv);

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

// 1. Loads the image.
// 2. Initializes the vertex count.
// 3. Creates the vertex buffer.
// 4. Initalizes the pipeline.
void initScene(int argc, char *argv[])
{
	// load the image from a jpeg disk file to main memory
	heightmapImage = new ImageIO();
	if (heightmapImage->loadJPEG(argv[1]) != ImageIO::OK)
	{
		cout << "Error reading image " << argv[1] << "." << endl;
		exit(EXIT_FAILURE);
	}

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	matrix = new OpenGLMatrix();

	// Cleanup
	heightMapVertices.clear();
	heightMapVertexColors.clear();
	heightMapIndices.clear();

	// Initialize vertices data:
	unsigned int height = heightmapImage->getHeight();
	unsigned int width = heightmapImage->getWidth();
	unsigned int vertices = width * height;
	heightMapVertices.resize(vertices);
	heightMapVertexColors.resize(vertices);

	// Populate the vertices
	const float offset = WORLDMAP_SIZE / (width - 1); /* Assume width > 1 */
	for (unsigned i = 0; i < vertices; ++i)
	{
		heightMapVertices[i].x = offset * (i % width) - (WORLDMAP_SIZE / 2.0f); /* Assume width > 0 */
		heightMapVertices[i].z = i > (width - 1) ? 
									(i / width) * (-offset) + (WORLDMAP_SIZE / 2.0f) /* Draw map from close (+) to far (-) while camera facing -z. */:
									WORLDMAP_SIZE / 2.0f /* Draw first row at the default of 1.0f. */;
		heightMapVertices[i].y = heightmapImage->getPixel(i % width, i / width, 0) / WORLDMAP_MAX_HEIGHT; /* TODO: populate with correct height. */
	}

	// Populate the color of the vertices.
	for (unsigned i = 0; i < vertices; ++i)
	{
		unsigned char color = heightmapImage->getPixel(i % width, i / width, 0);
		heightMapVertexColors[i] = glm::vec4(color/300.0f, color/256.0f, color/255.0f, 1);
	}

	// Populate the index buffer
	const unsigned numStrips = height - 1;
	const unsigned numDegens = 2 * (numStrips - 1);
	const unsigned verticesPerStrip = 2 * width;
	heightMapIndices.resize((verticesPerStrip * numStrips) + numDegens);

	unsigned int indexOffset = 0;
	// Loop over the number of strips from top to bottom.
	for (unsigned r = 0; r < numStrips; ++r)
	{
		// Check if we are writing a degenerate.
		if (r > 0)
		{
			heightMapIndices[indexOffset++] = r * height;
		}

		// Writing the strip
		for (unsigned c = 0; c < width; ++c)
		{
			heightMapIndices[indexOffset++] = r * height + c;
			heightMapIndices[indexOffset++] = (r + 1) * height + c;
		}

		// Last degenerate must repeat last vertex.
		if (r < height - 2)
		{
			heightMapIndices[indexOffset++] = (r + 1) * height + width - 1;
		}
	}

	// Final setup
	initArrays();
	initBuffers();
	initPipelineProgram();
}

void initArrays()
{
	/*
		GLuint vertexArray;
	*/
	glGenVertexArrays(1, &vertexArray);
	glBindVertexArray(vertexArray);
}

void initBuffers()
{
	/*
		GLuint vertexBuffer (vec3);
		GLuint colorBuffer (vec4);
		GLuint indexBuffer (uint);
	*/
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, heightMapVertices.size() * sizeof(glm::vec3), &heightMapVertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &colorBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
	glBufferData(GL_ARRAY_BUFFER, heightMapVertexColors.size() * sizeof(glm::vec4), &heightMapVertexColors[0], GL_STATIC_DRAW);

	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, heightMapIndices.size() * sizeof(unsigned int), &heightMapIndices[0], GL_STATIC_DRAW);
}

void initPipelineProgram()
{
	pipelineProgram = new BasicPipelineProgram();
	pipelineProgram->Init("../openGLHelper-starterCode");
}

void displayFunc()
{
	// render some stuff...
	glClear(GL_COLOR_BUFFER_BIT |
		GL_DEPTH_BUFFER_BIT);
	matrix->LoadIdentity();
	matrix->LookAt(0, CAMERA_HEIGHT + landTranslate[1], CAMERA_DEPTH + landScale[2], 0, 0, 0, 0, 1, 0);
	matrix->Rotate(landRotate[0], 1, 0, 0);
	matrix->Rotate(landRotate[1], 0, 1, 0);
	matrix->Rotate(landRotate[2], 0, 0, 1);
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
	// Attribute 1: Position
	GLuint attrib_pos = glGetAttribLocation(program, "position");
	glEnableVertexAttribArray(attrib_pos);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glVertexAttribPointer(attrib_pos, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// Attribute 2: Color
	GLuint attrib_color = glGetAttribLocation(program, "color");
	glEnableVertexAttribArray(attrib_color);
	glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
	glVertexAttribPointer(attrib_color, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// Attribute 3: Index
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

	// Draw
	glDrawElements(GL_TRIANGLE_STRIP, heightMapIndices.size(), GL_UNSIGNED_INT, (void*)0);
	glBindVertexArray(0);
}

void idleFunc()
{
  // do some stuff... 

  // for example, here, you can save the screenshots to disk (to make the animation)

  // make the screen update 
  glutPostRedisplay();
}

void reshapeFunc(int w, int h)
{
  glViewport(0, 0, w, h);

  // setup perspective matrix...
  GLfloat aspect = (GLfloat)w / (GLfloat)h;
  glViewport(0, 0, w, h);
  matrix->SetMatrixMode(OpenGLMatrix::Projection);
  matrix->LoadIdentity();
  matrix->Perspective(90, aspect, 0.01f, 1000);
  matrix->SetMatrixMode(OpenGLMatrix::ModelView);
}

void mouseMotionDragFunc(int x, int y)
{
  // mouse has moved and one of the mouse buttons is pressed (dragging)

  // the change in mouse position since the last invocation of this function
  int mousePosDelta[2] = { x - mousePos[0], y - mousePos[1] };

  switch (controlState)
  {
    // translate the landscape
    case TRANSLATE:
      if (leftMouseButton)
      {
        // control x,y translation via the left mouse button
        landTranslate[0] += mousePosDelta[0] * 0.01f;
        landTranslate[1] -= mousePosDelta[1] * 0.01f;
      }
      if (middleMouseButton)
      {
        // control z translation via the middle mouse button
        landTranslate[2] += mousePosDelta[1] * 0.01f;
      }
      break;

    // rotate the landscape
    case ROTATE:
      if (leftMouseButton)
      {
        // control x,y rotation via the left mouse button
        landRotate[0] += mousePosDelta[1];
        landRotate[1] += mousePosDelta[0];
      }
      if (middleMouseButton)
      {
        // control z rotation via the middle mouse button
        landRotate[2] += mousePosDelta[1];
      }
      break;

    // scale the landscape
    case SCALE:
      if (leftMouseButton)
      {
        // control x,y scaling via the left mouse button
        landScale[0] *= 1.0f + mousePosDelta[0] * 0.01f;
        landScale[1] *= 1.0f - mousePosDelta[1] * 0.01f;
      }
      if (middleMouseButton)
      {
        // control z scaling via the middle mouse button
        landScale[2] *= 1.0f - mousePosDelta[1] * 0.01f;
      }
      break;
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
    break;

    case GLUT_MIDDLE_BUTTON:
      middleMouseButton = (state == GLUT_DOWN);
    break;

    case GLUT_RIGHT_BUTTON:
      rightMouseButton = (state == GLUT_DOWN);
    break;
  }

  // keep track of whether CTRL and SHIFT keys are pressed
  switch (glutGetModifiers())
  {
    case GLUT_ACTIVE_CTRL:
      controlState = TRANSLATE;
    break;

    case GLUT_ACTIVE_SHIFT:
      controlState = SCALE;
    break;

    // if CTRL and SHIFT are not pressed, we are in rotate mode
    default:
      controlState = ROTATE;
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

		break;

	case '2':
		// Draw wires.

		break;

	case '3':
		// Draw Mesh

		break;

	case '-':
		// Zoom in 
		
		break;

	case ' ':
		cout << "You pressed the spacebar." << endl;
		break;

	case 'x':
		// take a screenshot
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


