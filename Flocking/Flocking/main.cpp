#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <time.h>
#include <fstream>
#include <vector>

#include <GL\glew.h>
#include <GL\glut.h>
#include <GL\GL.h>

#include "vecmath.h"

// Defines
#define WINDOW_H	1024
#define WINDOW_W	1024
#define SIM_DIST	40.0
#define FPS			30
#define CHANGE		0.25

// Function declearations - they're defined later, just declared for now
void display();
void graphicsInit();
void simulationInit(char* inputFile);
void keyboard( unsigned char key, int x, int y);
void animate(int i);
Vector3 colAvoid(int boid);
Vector3 velMatch(int boid);
Vector3 center  (int boid);
Vector3 fLeader (int boid);

// Various varied variables
bool start = false;
long elapsed;
int nboids;				// # of boids
double collision, velocitym, centering, follow; // Weights for behaviors
double range; // Range for 'flock'

// Lighting
GLfloat diffuse[] = {0.75, 0.75, 0.75, 1.0};
GLfloat specular[] = {0.9, 0.9, 0.9, 1.0};
GLfloat ambient[] = {0.1, 0.1, 0.1, 1.0};
GLfloat lposition[] = {1.0, 1.0, 1.0, 0.0};

GLfloat bAmbient[] = {0.0, 0.0, 0.15, 1.0};
GLfloat bDiffuse[] = {0.0, 0.0, 0.8, 1.0};

GLfloat lAmbient[] = {0.15, 0.0, 0.0, 1.0};
GLfloat lDiffuse[] = {0.8, 0.0, 0.0, 1.0};


// Keyframe struct
typedef struct {
	double t;			// Timecode
	Point3 p;			// Position
} Keyframe;

// Keyframe tracking and such
vector<Keyframe*> keyframes;
Keyframe *keyPrev, *keyNext;
int keyframesElapsed = 0;


// Boids!
typedef struct {
	Point3 p;		// Position
	Vector3 v;		// Current vector
	Vector3 a;		// Acceleration vector
} Boid;
vector<Boid*> getNeighbors(int boid);
vector<Boid*> boids;
double lx, ly, lz;  // Leader position

int main (int argc, char **argv) {
	// Set up openGL & window stuff
	glutInit( &argc, argv );
	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
	glutInitWindowSize( WINDOW_W, WINDOW_H );
	glutCreateWindow( "Flocking - Animation Assignment 4" );

	// Define glut functions to be used
	glutDisplayFunc( display );
	glutKeyboardFunc( keyboard);

	// Let's do this!
	graphicsInit();
	simulationInit(argv[1]);
	glutMainLoop();
}


void graphicsInit(){
	/* define the projection transformation */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-40.0, 40.0, -40.0, 40.0, -100.0, 100.0);
	
	// Set up the lighting
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_POSITION, lposition);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);

	// Culling!
	glEnable(GL_DEPTH_TEST);
}



void simulationInit(char* inputFile){
	// Read in the file
	ifstream file(inputFile, ios::in);
	char line[256];
	if (!file.is_open()) exit(1);

	file.getline(line, 256);
	nboids = atoi(strtok(line, " "));
	collision = atof(strtok(NULL, " "));
	velocitym = atof(strtok(NULL, " "));
	centering = atof(strtok(NULL, " "));
	follow = atof(strtok(NULL, " "));
	range =  pow(atof(strtok(NULL, " ")), 2);
	
	keyframes = vector<Keyframe*>();
	while (file.getline(line, 256)) {
			Keyframe* k = (Keyframe*)malloc(sizeof(Keyframe));
			k->t =		atof(strtok(line, " ")) * 1000.0;

			double x =		atof(strtok(NULL, " "));
			double y =		atof(strtok(NULL, " "));
			double z =		atof(strtok(NULL, " "));
			k->p = Point3(x, y, z);

			keyframes.push_back(k);
	}
	keyPrev =	keyframes[  keyframesElapsed];
	keyNext =	keyframes[++keyframesElapsed];
	file.close();

	srand(time(NULL));
	for (int i = 0; i < nboids; i++){
		Boid* b = (Boid*)malloc(sizeof(Boid));
		b->p = Point3(	(float)(rand() % (2*(int)SIM_DIST)) - SIM_DIST,
						(float)(rand() % (2*(int)SIM_DIST)) - SIM_DIST,
						(float)(rand() % (2*(int)SIM_DIST)) - SIM_DIST);
		b->v = Vector3(	(float)rand(),
						(float)rand(),
						(float)rand());
		b->v.normalize();
		boids.push_back(b);
	}
}

void display(){
	// display prep
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);

	glPushMatrix();
	glLoadIdentity();

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, bAmbient );
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, bDiffuse );
	// Draw the boids
	for (int i = 0; i < boids.size(); i++){
		Boid* b = boids[i];
		glLoadIdentity();
		glTranslated(b->p.x, b->p.y, b->p.z);
		glutSolidSphere(0.75, 10, 10);
	}

	// Draw the leader
	glLoadIdentity();
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, lAmbient );
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, lDiffuse );
	glTranslated(lx, ly, lz);
	glutSolidSphere(1.5, 10, 10);
	
	glPopMatrix();

	// Buffers
	glutSwapBuffers();
}

double catmullRomInterp(int x0, int x1, int x2, int x3, double t){
	return 0.5 * ((2.0 * x1) +
		(-x0 + x2) * t +
		(2.0 * x0 - 5 * x1 + 4 * x2 - x3) * (t*t) +
		(-x0 + 3.0 * x1 - 3.0 * x2 + x3) * (t*t*t));
}

void animate(int i){
	elapsed += FPS;
	
	// Update frames
	if (elapsed >= keyNext->t) {
		if (keyframes.size()-1 == keyframesElapsed) {
			keyPrev = keyframes[keyframesElapsed];
			keyNext = keyframes[0];
			keyframesElapsed = 0;
			elapsed = 0;
		} else {
			keyPrev =	keyframes[  keyframesElapsed];
			keyNext =	keyframes[++keyframesElapsed];
		}
	}

	// Current tween's elapsed time
	double tweent = (double)elapsed - keyPrev->t;
	double t = tweent / (keyNext->t - keyPrev->t);


	// Calculate current (x,y,z) coords using Catmull-Rom splines
	Keyframe* p0 = keyframes[max(keyframesElapsed - 2, 0)];
	Keyframe* p1 = keyframes[(keyframesElapsed - 1 )% keyframes.size()];
	Keyframe* p2 = keyframes[min(keyframesElapsed, (int)keyframes.size() - 1)];
	Keyframe* p3 = keyframes[min(keyframesElapsed + 1, (int)keyframes.size() - 1)];

	lx = catmullRomInterp(p0->p.x, p1->p.x, p2->p.x, p3->p.x, t);
	ly = catmullRomInterp(p0->p.y, p1->p.y, p2->p.y, p3->p.y, t);
	lz = catmullRomInterp(p0->p.z, p1->p.z, p2->p.z, p3->p.z, t);


	////////////
	// Boids! //
	////////////

	// Update boid's positions and upcoming velocities
	for (int i = 0; i < boids.size(); i++){
		Boid* b = boids[i];

		b->p.x += b->v.x;
		b->p.y += b->v.y;
		b->p.z += b->v.z;

		
		// Make sure boids stay in the area
		if (b->p.x < -SIM_DIST) b->p.x = -SIM_DIST;
		else if (b->p.x > SIM_DIST) b->p.x = SIM_DIST;
		if (b->p.y < -SIM_DIST) b->p.y = -SIM_DIST;
		else if (b->p.y > SIM_DIST) b->p.y = SIM_DIST;
		if (b->p.z < -SIM_DIST) b->p.z = -SIM_DIST;
		else if (b->p.z > SIM_DIST) b->p.z = SIM_DIST;
		
		Vector3 v = Vector3();
		v = v + (colAvoid(i) * collision);
		v = v + (velMatch(i) * velocitym);
		v = v + (center(i)   * centering);
		v = v + (fLeader(i)  * follow);

		b->a = v;
	}

	// Update the velocities
	for (int i = 0; i < boids.size(); i++) {
		boids[i]->v = boids[i]->v + (boids[i]->a * CHANGE);
		boids[i]->v.normalize();
	}



	glutTimerFunc(1000 / FPS, animate, i + 1);
	glutPostRedisplay();
}

Vector3 colAvoid(int boid){ 
	vector<Boid*> neighbors = getNeighbors(boid);

	Vector3 final = Vector3();
	Boid* b = boids[boid];
	for (int i = 0; i < neighbors.size(); i++)
		final = final + (b->p - neighbors[i]->p);

	if (b->p.distanceToSquared(Point3(lx, ly, lz)) < range)
		final = final + (b->p - Point3(lx, ly, lz));

	final.normalize();
	return final;
}

Vector3 velMatch(int boid){
	vector<Boid*> neighbors = getNeighbors(boid);

	Vector3 final = Vector3();
	Boid* b = boids[boid];
	for (int i = 0; i < neighbors.size(); i++)
		final = final + neighbors[i]->v;

	final.normalize();
	return final;
}

Vector3 center  (int boid){	
	vector<Boid*> neighbors = getNeighbors(boid);

	Vector3 final = Vector3();
	Boid* b = boids[boid];
	for (int i = 0; i < neighbors.size(); i++)
		final = final + (neighbors[i]->p - b->p);

	if (b->p.distanceToSquared(Point3(lx, ly, lz)) < range)
		final = final + (Point3(lx, ly, lz) - b->p);

	final.normalize();
	return final; }

Vector3 fLeader (int boid){ 
	Boid* b = boids[boid];

	Vector3 v = Vector3(lx - b->p.x, ly - b->p.y, lz - b->p.z);
	v.normalize();

	return v;
}

vector<Boid*> getNeighbors(int boid){
	vector<Boid*> neighbors = vector<Boid*>();
	Boid* b = boids[boid];

	for (int i = 0; i < nboids; i++){
		if (boid == i) continue;
		Boid* other = boids[i];

		if ( other->p.distanceToSquared(b->p) <= range )
			neighbors.push_back(other);
	}

	return neighbors;
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