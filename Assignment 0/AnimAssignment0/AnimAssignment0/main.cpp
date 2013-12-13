#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <GL\glew.h>
#include <GL\glut.h>
#include <GL\GL.h>

// Function declearations - they're defined later, just declared for now
void display();
void init();
void keyboard( unsigned char key, int x, int y);
void animate(int i);

// Globals
long elapsed = 0;

// Positions and Rotations;
double x, y = -50;
double z = 0;
double angle = 0;

// Lighting
GLfloat diffuse[] = {0.0, 0.0, 1.0, 1.0};
GLfloat lposition[] = {1.0, 1.0, 1.0, 0.0};

// Just draws a shape and animates it for 20 seconds.
int main (int argc, char **argv) {
	// Set up openGL & window stuff
	glutInit( &argc, argv );
	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
	glutInitWindowSize( 512, 512 );
	glutCreateWindow( "Assignment 0" );

	// Define functions to be used
	glutDisplayFunc( display );
	glutKeyboardFunc( keyboard );

	init();
	glutMainLoop();
}

// Initialize the scene and other type stuff
void init() {
	/* define the projection transformation */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-60.0, 60.0, -60.0, 60.0, -100.0, 100.0);

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
	glRotated(angle, 0.0, 1.0, 0.0);
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
	elapsed += 50;
	double t = (double)elapsed / 1000.0;
	x = y = (5 * t) - 50;
	angle = 18 * t;

	printf("Animating: (%f,%f,%f) - %f degrees - %d elapsed\n", x, y, z, angle, elapsed);

	glutTimerFunc(50, animate, i + 1);
	glutPostRedisplay();
}

// Keyboard stuff
void keyboard (unsigned char key, int x, int y){
	switch(key) {
	case 033: //escape
		exit(0);
	default:
		glutTimerFunc(50, animate, 0);
	}
}