#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <time.h>
#include <fstream>
#include <vector>

#include <GL\glew.h>
#include <GL\glut.h>
#include <GL\GL.h>

using namespace std;

// Defines
#define WINDOW_H	512
#define WINDOW_W	512
#define TABLE_W		243.8 / 2	//cm
#define TABLE_H		129.8 / 2	//cm
#define RADIUS		5.7		//cm
#define MASS		170.0	//g
#define FPS			30
#define PI			3.14159265

// Ball struct definition
typedef struct {
	double x, y;
	double vx, vy;
	double rx, ry;

	double r, g, b;
} Ball;

// "Global" variables
static bool start = false;
static Ball* balls[4];
static double fricRoll;
static double fricSlid;
static double restitution;

// Function declarations
void graphicsInit();
void billiardsInit(char filename[]);
void display();
void keyboard(unsigned char key, int x, int y);
void timestep(int step);
void collideBalls(Ball* a, Ball* b);




// main wheeee

int main (int argc, char **argv) {
	//TODO argument checking

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
	billiardsInit(argv[1]);
	glutMainLoop();
}

// Initilizes all the graphical stuff.
// Sets up the matricies and camera and other fun stuff
void graphicsInit(){
	// Projection transformation
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-TABLE_W + 5.0, TABLE_W + 5.0, -TABLE_W + 5.0, TABLE_W + 5.0, -100.0, 100.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(	0.0,	-15.0,	10.0,
				0.0,	0.0,	0.0,
				0.0,	1.0,	0.0);
	
	// Other stuff
	glEnable(GL_DEPTH_TEST);
}


// Initilizes simulation
// Sets up the billiard balls and initial conditions
void billiardsInit(char filename[]){
	ifstream file(filename, ios::in);
	char line[256];

	if (file.is_open()) {
		// Billiard balls
		for (int i = 0; i < 4; i++){
			file.getline(line, 256);
			balls[i] = (Ball*)malloc(sizeof(Ball));
			balls[i]->r = atof(strtok(line, " "));
			balls[i]->g = atof(strtok(NULL, " "));
			balls[i]->b = atof(strtok(NULL, " "));
			balls[i]->x = atof(strtok(NULL, " "));
			balls[i]->y = atof(strtok(NULL, " "));
			balls[i]->vx = 0.0;
			balls[i]->vy = 0.0;
			balls[i]->rx = 0.0;
			balls[i]->ry = 0.0;
		}

		// Cue Ball initial velocity
		file.getline(line, 256);
		balls[0]->vx = atof(strtok(line, " "));
		balls[0]->vy = atof(strtok(NULL, " "));
		strtok(NULL, " "); // Burning unused data
		balls[0]->rx = atof(strtok(NULL, " "));
		balls[0]->ry = atof(strtok(NULL, " "));

		// Simulation constants
		file.getline(line, 256);
		fricSlid = atof(strtok(line, " "));
		fricRoll = atof(strtok(NULL, " "));
		restitution = atof(strtok(NULL, " "));
	}
}



// What it says on the tin. Displays the scene.
void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);

	// Draw the table
	glColor3f(0.1, 0.5, 0.1);
	glBegin(GL_QUADS);
		glNormal3d(0.0, 0.0, 1.0);
		glVertex3f( TABLE_W,  TABLE_H, 0.0);
		glVertex3f(-TABLE_W,  TABLE_H, 0.0);
		glVertex3f(-TABLE_W, -TABLE_H, 0.0);
		glVertex3f( TABLE_W, -TABLE_H, 0.0);
	glEnd();

	// Draw the balls
	for (int i = 0; i < 4; i++){
		glPushMatrix();
		glTranslated(balls[i]->x, balls[i]->y, RADIUS);
		glColor3f(balls[i]->r, balls[i]->g, balls[i]->b);
		glutSolidSphere(RADIUS, 10, 10);
		glPopMatrix();
	}

	glutSwapBuffers();
}


// Timestep. Goes to the next timestep in the simulation.
// Major assumption - that the thing runs smoothly at 30fps.
// Actual display/simulation will be done seperately, this only
// Manages it all
void timestep(int step) {
	for (int i = 0; i < 4; i++){
		Ball* ball = balls[i];
		// Step 1 - Calculation of forces
		double fRollx = ball->vx * -fricRoll;
		double fRolly = ball->vy * -fricRoll;

		// Step 2 - Integrate position & momentum
		// Location
		ball->x += ball->vx;
		ball->y += ball->vy;

		// Rotation
		// Adjusted translation
		// TODO LATER? BONUS POINTS
		
		// Momentum
		ball->vx += fRollx;
		ball->vy += fRolly;

		// Step 3 - Collision detection & response
		// Ball-cushion collision
		if (ball->x >= TABLE_W - RADIUS
			|| ball->x <= -(TABLE_W - RADIUS) )
		{
			// Remove energy & reverse direction
			ball->vx *= -restitution;

			// Find overshoot and reposition ball
			double overshoot = abs(ball->x) - (TABLE_W - RADIUS);
			ball->x += overshoot * restitution * (ball->x < 0 ? 1 : -1);
		}
		if (ball->y >= TABLE_H - RADIUS
			|| ball->y <= -(TABLE_H - RADIUS) )
		{
			// Remove energy & reverse direction
			ball->vy *= -restitution;

			// Find overshoot and reposition ball
			double overshoot = abs(ball->y) - (TABLE_H - RADIUS);
			ball->y += overshoot * restitution * (ball->y < 0 ? 1 : -1);
		}
	}
	
	for (int i = 0; i < 4; i++){
		Ball* ball = balls[i];
		// Ball-ball collision
		for (int j = i+1; j < 4; j++){
			// Test collision - simple radius test
			double dx = abs(ball->x - balls[j]->x);
			double dy = abs(ball->y - balls[j]->y);
			double ballDist = sqrt( (dx * dx) + (dy * dy) );
			if (ballDist >= (2 * RADIUS)) continue;
			
			// They are colliding - make them go away!
			collideBalls(ball, balls[j]);
		}

		// Step 4 - Calculate velocities

	}

	glutTimerFunc(1000 / FPS, timestep, step + 1);
	glutPostRedisplay();
}


// collide 2 balls

void collideBalls( Ball* a, Ball* b){
	double dx = a->x - b->x;
	double dy = a->y - b->y;

	// Angle of collision
	double angle = atan2(dy, dx);

	// Calculate speed & direction of balls
	double speeda = sqrt( (a->vx*a->vx) + (a->vy*a->vy) );
	double speedb = sqrt( (b->vx*b->vx) + (b->vy*b->vy) );
	double dira   = atan2(a->vy, a->vx);
	double dirb   = atan2(b->vy, b->vx);

	// Rotation of vectors
	double vxa = speeda * cos(dira - angle);
	double vya = speeda * sin(dira - angle);
	double vxb = speedb * cos(dirb - angle);
	double vyb = speedb * sin(dira - angle);

	// Update of velocities of the balls
	double finalxa = vxb; //((MASS - MASS) * vxa) + ((MASS + MASS) * vxb) / (MASS + MASS);
	double finalxb = vxa; //((MASS + MASS) * vxa) + ((MASS - MASS) * vxb) / (MASS + MASS);
	double finalya = vya;
	double finalyb = vyb;

	// Rotate back to world xy coordinates
	a->vx = (cos(angle) * finalxa) + (cos(angle + (PI/2)) * finalya);
	a->vy = (sin(angle) * finalxa) + (sin(angle + (PI/2)) * finalya);
	b->vx = (cos(angle) * finalxb) + (cos(angle + (PI/2)) * finalyb);
	b->vy = (sin(angle) * finalxb) + (sin(angle + (PI/2)) * finalyb);



	// Move balls to position
	a->x += a->vx;		a->y += a->vy;
	b->x += b->vx;		b->y += b->vy;
}

// Keyboard stuff
void keyboard (unsigned char key, int x, int y){
	switch(key) {
	case 033: //escape
		exit(0);
		break;
	default:
		if (!start) glutTimerFunc(50, timestep, 0);
		start = true;
		break;
	}
}