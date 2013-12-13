#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <time.h>
#include <fstream>
#include <vector>

#include <GL\glew.h>
#include <GL\glut.h>
#include <GL\GL.h>

#include "BVH.h"

using namespace std;

// Defines
#define WINDOW_H	512
#define WINDOW_W	512
#define FLOOR_INT	25.0


// Funciton delecarations
void graphicsInit();
void display();
void keyboard(unsigned char key, int x, int y);
void timestep(int step);
void wireframeInit(const char* filename);


// Variables
static BVH* skeleton;
static bool start = false;
static int delayTimeMS;
static int frame = 0;
static double floorHeight;


int main (int argc, char **argv) {
	// Set up openGL & window stuff
	glutInit( &argc, argv );
	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
	glutInitWindowSize( WINDOW_W, WINDOW_H );
	glutCreateWindow( "Billiards - Animation Assignment 2" );

	// Define glut functions to be used
	glutDisplayFunc( display );
	glutKeyboardFunc( keyboard);

	//Set 'er up and let 'er go!
	graphicsInit();
	wireframeInit(argv[1]);
	glutMainLoop();
}


void wireframeInit(const char* filename){
	skeleton = new BVH(filename);
	delayTimeMS = skeleton->interval * 1000;
	floorHeight = skeleton->getFloorDepth();
	int a = 5;
}


void graphicsInit(){
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-100.0, 100.0, -100.0, 100.0, -1000.0, 1000.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(	100.0, 100.0, 100.0,
				0.0, 0.0, 0.0,
				0.0, 1.0, 0.0);
}

void display(){
	// prep stuff
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);


	// Look at where the skeleton will be
	skeleton->RenderFigure(frame);
	BVH::Coord lookpt = skeleton->getCenterPoint(frame);
	glLoadIdentity();
	gluLookAt(	0.0, 0.0, 0.0,
				lookpt.x, lookpt.y, lookpt.z,
				0.0, 1.0, 0.0);
	// draw the skeleton once

	glutSwapBuffers();
}


void timestep(int step){
	frame++;

	if (frame < skeleton->numFrames)
		glutTimerFunc(delayTimeMS, timestep, 0);
	else
		frame = skeleton->numFrames - 1;
	
	glutPostRedisplay();
}


void keyboard (unsigned char key, int x, int y){
	switch(key) {
	case 033: //escape
		exit(0);
		break;
	default:
		if (!start) glutTimerFunc(delayTimeMS, timestep, 0);
		start = true;
		break;
	}
}


