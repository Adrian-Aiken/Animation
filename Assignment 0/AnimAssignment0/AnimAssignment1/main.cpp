#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <time.h>
#include <fstream>
#include <vector>

#include <GL\glew.h>
#include <GL\glut.h>
#include <GL\GL.h>

#include "Quaternion.h"
#include "vecmath.h"

#define FPS 30

using namespace std;

//Program stuff
GLuint program;

// Function declearations - they're defined later, just declared for now
void display();
void init();
void keyboard( unsigned char key, int x, int y);
void animate(int i);
void finished(int i);

bool start = false;

// Interpolation!
double catmullRomInterp(int x0, int x1, int x2, int x3, double t);
double linearInterp(double x1, double x2, double t);
Quaternion slerp(const Quaternion &a, const Quaternion &b, double t);
double easeInOutQuad(double t, double start, double change, double duration);

// Globals
long elapsed = 0;

// Positions and Rotations;
double x, y = -50;
double z = 0;
double ax = 0, ay = 0, az = 0, angle = 0;

// Lighting
GLfloat diffuse[] = {0.0, 0.0, 1.0, 1.0};
GLfloat lposition[] = {1.0, 1.0, 1.0, 0.0};

// Keyframe struct
typedef struct {
	double t;			// Timecode
	Point3 p;			// Position
	Quaternion a;		// Rotation
} Keyframe;

// Keyframe tracking and such
vector<Keyframe*> keyframes;
Keyframe *keyPrev, *keyNext;
int keyframesElapsed = 0;

// Just draws a shape and animates it for 20 seconds.
int main (int argc, char **argv) {
	// Argument checking
	if (argc < 2) exit(1);

	// Set up openGL & window stuff
	glutInit( &argc, argv );
	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
	glutInitWindowSize( 512, 512 );
	glutCreateWindow( "Assignment 0" );

	// Set up program
	//program = setUpAShader( "vshader.glsl", "fshader.glsl" );
	//if (!program) {
	//	cerr << "Error setting up shaders" << endl;
	//	exit(1);
	//}


	// Define functions to be used
	glutDisplayFunc( display );
	glutKeyboardFunc( keyboard );

	// init keyframe stuff
	keyframes = vector<Keyframe*>();
	ifstream file(argv[1], ios::in);
	char line[256];
	if (file.is_open())
		while ( file.getline(line, 256) ){
			Keyframe* k = (Keyframe*)malloc(sizeof(Keyframe));
			k->t =		atof(strtok(line, " ")) * 1000.0;

			double x =		atof(strtok(NULL, " "));
			double y =		atof(strtok(NULL, " "));
			double z =		atof(strtok(NULL, " "));
			k->p = Point3(x, y, z);

			double ax = atof(strtok(NULL, " ")), ay = atof(strtok(NULL, " ")), az = atof(strtok(NULL, " "));
			double angle = atof(strtok(NULL, " "));

			k->a = Quaternion(ax, ay, az, angle);
			//k->a.normalize();

			keyframes.push_back(k);
		}

	keyPrev =	keyframes[  keyframesElapsed];
	keyNext =	keyframes[++keyframesElapsed];

	init();
	glutMainLoop();
}

// Initialize the scene and other type stuff
void init() {
	/* define the projection transformation */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-40.0, 40.0, -40.0, 40.0, -100.0, 100.0);

	// Set up the lighting
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, lposition);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);

	// Culling!
	glEnable(GL_DEPTH_TEST);


	//-------------------------
	// File IO stuff

}

// Displays the image as it currently should be
void display() {
	// display prep
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);

	// Draw the teapot
	glPushMatrix();
	glTranslated(x, y, z);
	glRotated(angle, ax, ay, az);
	glutSolidTeapot(8.0);
	glPopMatrix();

	// Buffers
	glutSwapBuffers();
}

// Animates the teapot:
//  X = 5t
//  Y = 5t
//  Z = constant
//  Y rot = 18t (degrees)
//  X/Z rot = 0
void animate(int i) {
	elapsed += FPS;
	
	// Update frames
	if (elapsed >= keyNext->t) {
		if (keyframes.size()-1 == keyframesElapsed) {
			glutTimerFunc(5000, finished, 0);
			return;
		}
		keyPrev =	keyframes[  keyframesElapsed];
		keyNext =	keyframes[++keyframesElapsed];
	}

	// Current tween's elapsed time
	double tweent = (double)elapsed - keyPrev->t;
	double t = tweent / (keyNext->t - keyPrev->t);

	// Calculate current (x,y,z) coords using LERP
	//x = linearInterp(keyPrev->p.x, keyNext->p.x, t);
	//y = linearInterp(keyPrev->p.y, keyNext->p.y, t);
	//z = linearInterp(keyPrev->p.z, keyNext->p.z, t);


	// Calculate current (x,y,z) coords using Catmull-Rom splines
	Keyframe* p0 = keyframes[max(keyframesElapsed - 2, 0)];
	Keyframe* p1 = keyframes[keyframesElapsed - 1];
	Keyframe* p2 = keyframes[min(keyframesElapsed, (int)keyframes.size() - 1)];
	Keyframe* p3 = keyframes[min(keyframesElapsed + 1, (int)keyframes.size() - 1)];

	x = catmullRomInterp(p0->p.x, p1->p.x, p2->p.x, p3->p.x, t);
	y = catmullRomInterp(p0->p.y, p1->p.y, p2->p.y, p3->p.y, t);
	z = catmullRomInterp(p0->p.z, p1->p.z, p2->p.z, p3->p.z, t);

	// Calculate rotation coords
	Quaternion qrot = slerp(keyPrev->a, keyNext->a, t);
	//qrot.normalize();
	//ax = qrot.x;
	//ay = qrot.y;
	//az = qrot.z;
	//angle = qrot.theta;

	//printf("Animating: (%f,%f,%f) - %f degrees - %d elapsed\n", x, y, z, angle, elapsed);

	glutTimerFunc(1000 / FPS, animate, i + 1);
	glutPostRedisplay();
}

// Interpolates between two points using Catmull-Rom interpolation
// x1 = index of P1
double catmullRomInterp(int x0, int x1, int x2, int x3, double t){
	return 0.5 * ((2.0 * x1) +
		(-x0 + x2) * t +
		(2.0 * x0 - 5 * x1 + 4 * x2 - x3) * (t*t) +
		(-x0 + 3.0 * x1 - 3.0 * x2 + x3) * (t*t*t));
}

// Interpolates between the two values linearly
double linearInterp(double x1, double x2, double t){
	return (x1 * (1.0 - t) ) + (x2 * t);
}

// Interpolates between two quaternions
Quaternion lerpQuaternion(const Quaternion &a, const Quaternion &b, float t){
	Quaternion result = a*(1.0-t) + b*t;
	result.normalize();
	return result;
}

Quaternion slerp(const Quaternion &a, const Quaternion &b, double t){
	if (t <= 0.0) return a;
	if (t >= 1.0) return b;

	double dot = (a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.theta * b.theta);
	Quaternion c;

	if (dot < 0.0) {
		dot = -dot;
		c = -b;
	} else {
		c = b;
	}

	if (dot < 0.95) {
		double angle = acosf(dot);
		return (a*sinf(angle*(1-t)) + b*sinf(angle*t))/sinf(angle);
	} else {
		return lerpQuaternion(a, c, t);
	}
}

// "Finishes" the animation by exiting after a pause
void finished(int i) {
	exit(0);
}

// Keyboard stuff
void keyboard (unsigned char key, int x, int y){
	switch(key) {
	case 033: //escape
		exit(0);
		break;
	default:
		if (!start) glutTimerFunc(50, animate, 0);
		start = true;
		break;
	}
}