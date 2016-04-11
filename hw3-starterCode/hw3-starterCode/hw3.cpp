/* **************************
 * CSCI 420
 * Assignment 3 Raytracer
 * Name: <Your name here>
 * *************************
*/

#ifdef WIN32
  #include <windows.h>
#endif

#if defined(WIN32) || defined(linux)
  #include <GL/gl.h>
  #include <GL/glut.h>
#elif defined(__APPLE__)
  #include <OpenGL/gl.h>
  #include <GLUT/glut.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
  #define strcasecmp _stricmp
#endif

#include <imageIO.h>
#include <math.h>

#define MAX_TRIANGLES 20000
#define MAX_SPHERES 100
#define MAX_LIGHTS 100

char * filename = NULL;

//different display modes
#define MODE_DISPLAY 1
#define MODE_JPEG 2

int mode = MODE_DISPLAY;

//you may want to make these smaller for debugging purposes
//#define WIDTH 640
//#define HEIGHT 480
#define WIDTH 640
#define HEIGHT 480

//the field of view of the camera
#define fov 60.0
#define camScale 8.0
#define MAX_RAYCAST_DIST 10000.0

#define MAX_COLOR 255
#define AMBIENT_COLOR 0

unsigned char buffer[HEIGHT][WIDTH][3];

struct Vertex
{
  double position[3];
  double color_diffuse[3];
  double color_specular[3];
  double normal[3];
  double shininess;
};

struct Triangle
{
  Vertex v[3];
};

struct Sphere
{
  double position[3];
  double color_diffuse[3];
  double color_specular[3];
  double shininess;
  double radius;
};

struct Light
{
  double position[3];
  double color[3];
};

struct Vector3
{
	double x = 0;
	double y = 0;
	double z = 0;

	Vector3(){}

	Vector3(double x, double y, double z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}

	static void Normalize(Vector3& v)
	{
		double length = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
		v.x = v.x / length;
		v.y = v.y / length;
		v.z = v.z / length;
	}
};

const Vector3 V_ZERO(0.0, 0.0, 0.0);

Vector3 Multiply(const Vector3& v, double scalar)
{
	return Vector3(v.x * scalar, v.y * scalar, v.z * scalar);
}

Vector3 Multiply(const Vector3& a, const Vector3& b)
{
	Vector3 retVal;
	retVal.x = a.x * b.x;
	retVal.y = a.y * b.y;
	retVal.z = a.z * b.z;
	return retVal;
}

Vector3 Negate(const Vector3& v)
{
	return Vector3(-v.x, -v.y, -v.z);
}

Vector3 Add(const Vector3& a, const Vector3& b)
{
	return Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
}

Vector3 Sub(const Vector3& a, const Vector3& b)
{
	return Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
}

double Dot(const Vector3& a, const Vector3& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

void Cross(const Vector3& a, const Vector3& b, Vector3& out)
{
	out.x = a.y * b.z - a.z * b.y;
	out.y = a.z * b.x - a.x * b.z;
	out.z = a.x * b.y - a.y * b.x;
}

double Saturate(double i)
{
	if (i <= 0) return 0;
	if (i >= 1) return 1;
	return i;
}

void Saturate(Vector3& v)
{
	v.x = Saturate(v.x);
	v.y = Saturate(v.y);
	v.z = Saturate(v.z);
}

Vector3 Reflect(const Vector3& v, const Vector3& n)
{
	return Sub(v, Multiply(n, 2 * Dot(v, n)));
}

Triangle triangles[MAX_TRIANGLES];
Sphere spheres[MAX_SPHERES];
Light lights[MAX_LIGHTS];
double ambient_light[3];

int num_triangles = 0;
int num_spheres = 0;
int num_lights = 0;

void plot_pixel_display(int x,int y,unsigned char r,unsigned char g,unsigned char b);
void plot_pixel_jpeg(int x,int y,unsigned char r,unsigned char g,unsigned char b);
void plot_pixel(int x,int y,unsigned char r,unsigned char g,unsigned char b);

bool handle_object_intersection(int x, int y, Vector3& ray, Vector3& outIntersection);
bool isShadowedPixel(int x, int y, const Vector3& lightPosition, const Vector3& intersection);

//MODIFY THIS FUNCTION
void draw_scene()
{
  //a simple test output
  for(unsigned int x=0; x<WIDTH; x++)
  {
    glPointSize(2.0);  
    glBegin(GL_POINTS);
    for(unsigned int y=0; y<HEIGHT; y++)
    {
	  // Send rays out from (0,0,0) to -z
		Vector3 ray((double)x - WIDTH / 2.0, (double)y - HEIGHT / 2.0, HEIGHT / camScale * tan(fov / 2));
		Vector3::Normalize(ray);

		Vector3 intersection(-MAX_RAYCAST_DIST, -MAX_RAYCAST_DIST, -MAX_RAYCAST_DIST);
		// Loop through objets
		if (!handle_object_intersection(x,y, ray, intersection))
		{
			plot_pixel(x, y, MAX_COLOR, MAX_COLOR, MAX_COLOR);
		}
    }
    glEnd();
    glFlush();
  }
  printf("Done!\n"); fflush(stdout);
}

bool ray_intersect_triangle(int x, int y, Vector3& ray, Triangle& triangle, Vector3& out)
{
	Vector3 A(triangle.v[0].position[0], triangle.v[0].position[1], triangle.v[0].position[2]);
	Vector3 B(triangle.v[1].position[0], triangle.v[1].position[1], triangle.v[1].position[2]);
	Vector3 C(triangle.v[2].position[0], triangle.v[2].position[1], triangle.v[2].position[2]);

	Vector3 AB = Sub(B, A);
	Vector3 AC = Sub(C, A);

	Vector3 planeNormal;
	Cross(AB, AC, planeNormal);
	Vector3::Normalize(planeNormal);

	// Find d using P dot N + d = 0.
	// Sub P in for a triangle point.
	double d = -1 * Dot(A, planeNormal);

	// END - Finish solving the plane equation.

	// Ray - Plane intersection
	// t = -(Po dot N + d) / (V dot N)
	// P = Po + tV
	double VDotN = Dot(ray, planeNormal);
	if (VDotN == 0) return false;
	double t = -1 * (Dot(V_ZERO, planeNormal) + d) / VDotN;
	Vector3 P = Multiply(ray, t);

	if (P.z >= out.z)
	{
		Vector3 ABxAP;
		Vector3 BCxBP;
		Vector3 CAxCP;
		Cross(AB, Sub(P, A), ABxAP);
		Cross(Sub(C, B), Sub(P, B), BCxBP);
		Cross(Sub(A, C), Sub(P, C), CAxCP);

		bool ret = false;

		if (Dot(ABxAP, BCxBP) > 0 && Dot(ABxAP, CAxCP) > 0 && Dot(BCxBP, CAxCP) > 0 
			|| Dot(ABxAP, BCxBP) < 0 && Dot(ABxAP, CAxCP) < 0 && Dot(BCxBP, CAxCP) < 0)
		{
			out = P;
			return true;
		}
	}
	return false;
}

bool ray_intersect_triangle_wrapper(int x, int y, Vector3& ray, Triangle& triangle, Vector3& out)
{
	if (ray_intersect_triangle(x, y, ray, triangle, out))
	{
		plot_pixel(x, y,
			0,
			0,
			0);
		return true;
	}
	return false;
}



bool ray_intersect_sphere(const Vector3& camera, int x, int y, Vector3& v,  Sphere& sphere, Vector3& out, Vector3& outPhong, bool doLighting)
{
	Vector3 sphere_position(sphere.position[0], sphere.position[1], sphere.position[2]);
	sphere_position = Sub(sphere_position, camera);
	double opposite = Dot(sphere_position, v);

	// Sphere is in opposite direction aka behind camera.
	if (opposite < 0) return false;

	double adjacentSqr = Dot(sphere_position, sphere_position) - opposite * opposite;

	// Ray's distance is greater than the radius of the sphere.
	// Thus ray does not intersect sphere.
	if (adjacentSqr > sphere.radius * sphere.radius) return false;

	// Intersection position closest to the camera is:
	double offset = sqrt(sphere.radius * sphere.radius - adjacentSqr);
	double intersection1 = opposite - offset;

	Vector3 P = Multiply(v, intersection1);
	if (P.z >= out.z)
	{
		out = P;

		if (doLighting)
		{
			// Lighting
			Vector3 phong(AMBIENT_COLOR, AMBIENT_COLOR, AMBIENT_COLOR);
			for (int i = 0; i < num_lights; ++i)
			{
				// Conversion to Vector3s
				Vector3 lightPosition(lights[i].position[0], lights[i].position[1], lights[i].position[2]);
				Vector3 lightColor(lights[i].color[0], lights[i].color[1], lights[i].color[2]);
				Vector3 color(sphere.color_diffuse[0], sphere.color_diffuse[1], sphere.color_diffuse[2]);
				Vector3 specular(sphere.color_specular[0], sphere.color_specular[1], sphere.color_specular[2]);

				// Lighting calculations
				Vector3 L = Sub(lightPosition, P); Vector3::Normalize(L);
				Vector3 N = Sub(P, sphere_position); Vector3::Normalize(N);
				Vector3 R = Reflect(Negate(L), N);
				Vector3 V = Sub(camera, P); Vector3::Normalize(V);

				if (!isShadowedPixel(x, y, lightPosition, P))
				{
					phong = Add(phong, Multiply(color, Saturate(Dot(N, L))));
					phong = Add(phong, Multiply(specular, pow(Saturate(Dot(R, V)), sphere.shininess)));
					phong = Multiply(lightColor, phong);
				}
			}
		outPhong = phong;
		Saturate(outPhong);
		}
		return true;
	}
	return false;
}

bool ray_intersect_sphere_wrapper(int x, int y, Vector3& v, Sphere& sphere, Vector3& out)
{
	Vector3 outPhong;
	Vector3 camera(0, 0, 0);
	if (ray_intersect_sphere(camera, x, y, v, sphere, out, outPhong, true))
	{
		plot_pixel(x, y,
			(char)(outPhong.x * MAX_COLOR),
			(char)(outPhong.y * MAX_COLOR),
			(char) (outPhong.z * MAX_COLOR));
		return true;
	}
	return false;
}

bool isShadowedPixel(int x, int y, const Vector3& lightPosition, const Vector3& intersection)
{
	Vector3 ray = Sub(lightPosition, intersection);
	Vector3 outIntersection;
	Vector3 outPhong;
	for (int i = 0; i < num_triangles; ++i)
	{
		if (ray_intersect_triangle(x, y, ray, triangles[i], outIntersection))
		{
			return true;
		}
	}
	for (int i = 0; i < num_spheres; ++i)
	{
		if (ray_intersect_sphere(intersection, x, y, ray, spheres[i], outIntersection, outPhong, false))
		{
			return true;
		}
	}
	return false;
}

bool handle_object_intersection(int x, int y, Vector3& ray, Vector3& outIntersection)
{
	bool intersects = false;

	for (int i = 0; i < num_triangles; ++i)
	{
		intersects |= ray_intersect_triangle_wrapper(x, y, ray, triangles[i], outIntersection);
	}
	for (int i = 0; i < num_spheres; ++i)
	{
		intersects |= ray_intersect_sphere_wrapper(x, y, ray, spheres[i], outIntersection);
	}

	return intersects;
}

void plot_pixel_display(int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
  glColor3f(((float)r) / 255.0f, ((float)g) / 255.0f, ((float)b) / 255.0f);
  glVertex2i(x,y);
}

void plot_pixel_jpeg(int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
  buffer[y][x][0] = r;
  buffer[y][x][1] = g;
  buffer[y][x][2] = b;
}

void plot_pixel(int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
  plot_pixel_display(x,y,r,g,b);
  if(mode == MODE_JPEG)
    plot_pixel_jpeg(x,y,r,g,b);
}

void save_jpg()
{
  printf("Saving JPEG file: %s\n", filename);

  ImageIO img(WIDTH, HEIGHT, 3, &buffer[0][0][0]);
  if (img.save(filename, ImageIO::FORMAT_JPEG) != ImageIO::OK)
    printf("Error in Saving\n");
  else 
    printf("File saved Successfully\n");
}

void parse_check(const char *expected, char *found)
{
  if(strcasecmp(expected,found))
  {
    printf("Expected '%s ' found '%s '\n", expected, found);
    printf("Parse error, abnormal abortion\n");
    exit(0);
  }
}

void parse_doubles(FILE* file, const char *check, double p[3])
{
  char str[100];
  fscanf(file,"%s",str);
  parse_check(check,str);
  fscanf(file,"%lf %lf %lf",&p[0],&p[1],&p[2]);
  printf("%s %lf %lf %lf\n",check,p[0],p[1],p[2]);
}

void parse_rad(FILE *file, double *r)
{
  char str[100];
  fscanf(file,"%s",str);
  parse_check("rad:",str);
  fscanf(file,"%lf",r);
  printf("rad: %f\n",*r);
}

void parse_shi(FILE *file, double *shi)
{
  char s[100];
  fscanf(file,"%s",s);
  parse_check("shi:",s);
  fscanf(file,"%lf",shi);
  printf("shi: %f\n",*shi);
}

int loadScene(char *argv)
{
  FILE * file = fopen(argv,"r");
  int number_of_objects;
  char type[50];
  Triangle t;
  Sphere s;
  Light l;
  fscanf(file,"%i", &number_of_objects);

  printf("number of objects: %i\n",number_of_objects);

  parse_doubles(file,"amb:",ambient_light);

  for(int i=0; i<number_of_objects; i++)
  {
    fscanf(file,"%s\n",type);
    printf("%s\n",type);
    if(strcasecmp(type,"triangle")==0)
    {
      printf("found triangle\n");
      for(int j=0;j < 3;j++)
      {
        parse_doubles(file,"pos:",t.v[j].position);
        parse_doubles(file,"nor:",t.v[j].normal);
        parse_doubles(file,"dif:",t.v[j].color_diffuse);
        parse_doubles(file,"spe:",t.v[j].color_specular);
        parse_shi(file,&t.v[j].shininess);
      }

      if(num_triangles == MAX_TRIANGLES)
      {
        printf("too many triangles, you should increase MAX_TRIANGLES!\n");
        exit(0);
      }
      triangles[num_triangles++] = t;
    }
    else if(strcasecmp(type,"sphere")==0)
    {
      printf("found sphere\n");

      parse_doubles(file,"pos:",s.position);
      parse_rad(file,&s.radius);
      parse_doubles(file,"dif:",s.color_diffuse);
      parse_doubles(file,"spe:",s.color_specular);
      parse_shi(file,&s.shininess);

      if(num_spheres == MAX_SPHERES)
      {
        printf("too many spheres, you should increase MAX_SPHERES!\n");
        exit(0);
      }
      spheres[num_spheres++] = s;
    }
    else if(strcasecmp(type,"light")==0)
    {
      printf("found light\n");
      parse_doubles(file,"pos:",l.position);
      parse_doubles(file,"col:",l.color);

      if(num_lights == MAX_LIGHTS)
      {
        printf("too many lights, you should increase MAX_LIGHTS!\n");
        exit(0);
      }
      lights[num_lights++] = l;
    }
    else
    {
      printf("unknown type in scene description:\n%s\n",type);
      exit(0);
    }
  }
  return 0;
}

void display()
{
}

void init()
{
  glMatrixMode(GL_PROJECTION);
  glOrtho(0,WIDTH,0,HEIGHT,1,-1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glClearColor(0,0,0,0);
  glClear(GL_COLOR_BUFFER_BIT);
}

void idle()
{
  //hack to make it only draw once
  static int once=0;
  if(!once)
  {
    draw_scene();
    if(mode == MODE_JPEG)
      save_jpg();
  }
  once=1;
}

int main(int argc, char ** argv)
{
  if ((argc < 2) || (argc > 3))
  {  
    printf ("Usage: %s <input scenefile> [output jpegname]\n", argv[0]);
    exit(0);
  }
  if(argc == 3)
  {
    mode = MODE_JPEG;
    filename = argv[2];
  }
  else if(argc == 2)
    mode = MODE_DISPLAY;

  glutInit(&argc,argv);
  loadScene(argv[1]);

  glutInitDisplayMode(GLUT_RGBA | GLUT_SINGLE);
  glutInitWindowPosition(0,0);
  glutInitWindowSize(WIDTH,HEIGHT);
  int window = glutCreateWindow("Ray Tracer");
  glutDisplayFunc(display);
  glutIdleFunc(idle);
  init();
  glutMainLoop();
}

