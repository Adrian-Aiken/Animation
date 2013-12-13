#include <fstream>
#include <string.h>
#include <math.h>
#include <GL/glut.h>

#include "BVH.h"

#define BUFFER_LEN 1024*4

// Default constructor/destructor
BVH::BVH(void){motion = NULL; Clear();}
BVH::~BVH(void){Clear();}


// Clearer - clears everything pretty much
void BVH::Clear(){
	int i;
	for (i = 0; i < channels.size(); i++)	delete channels[i];
	for (i = 0; i < joints.size(); i++)		delete joints[i];
	//if (motion != NULL)						delete motion;

	is_load_success = false;

	fileName = "";
	
	num_channel = 0;
	channels.clear();
	joints.clear();
	joint_index.clear();

	numFrames = 0;
	interval = 0.0;
	motion = NULL;
}


// Actual constructor
BVH::BVH(const char* file_name){
	Load(file_name);
}


// Loader
void BVH::Load(const char* file_name){
		ifstream	file;
	char		line[ BUFFER_LEN ];
	char*		token;
	char		seperator[] = " :,\t";
	vector< Joint* >	joint_stack;
	Joint*		joint = NULL;
	Joint*		new_joint = NULL;
	bool		is_site = false;
	double		x, y, z;
	int			i, j;

	Clear();

	// Opens the file
	fileName = file_name;
	file.open( fileName, ios::in );
	if (!file.is_open()) return;


	// Goes through the file
	while (!file.eof()){

		// Gathers the line and figures out what it is
		file.getline( line, BUFFER_LEN );
		token = strtok( line, seperator );

		// Tons of cases for lines below

		// Empty line - shouldn't happen but whatever
		if ( token == NULL )	continue;


		// New joint starting on the stack
		if ( strcmp( token, "{" ) == 0 ){
			joint_stack.push_back( joint );
			joint = new_joint;
			continue;
		}

		// Joint done, going back up the tree
		if ( strcmp( token, "}" ) == 0 ){
			joint = joint_stack.back();
			joint_stack.pop_back();
			is_site = false;
			continue;
		}

		// Got new joint data to handle
		if ( (strcmp( token, "ROOT" ) == 0) ||
			 (strcmp( token, "JOINT") == 0 ) ) {

			// Set up joint
			new_joint = new Joint();
			new_joint->index = joints.size();
			new_joint->parent = joint;
			new_joint->has_site = false;
			new_joint->offset[0] = 0.0;  new_joint->offset[1] = 0.0;  new_joint->offset[2] = 0.0;
			new_joint->site[0] = 0.0;  new_joint->site[1] = 0.0;  new_joint->site[2] = 0.0;
			joints.push_back( new_joint );
			if ( joint )
				joint->children.push_back( new_joint );

			// Name the joint
			token = strtok( NULL, "");
			while (*token == ' ') token++;
			new_joint->name = token;

			// Store the joint
			joint_index[ new_joint->name ] = new_joint;
			continue;
		}

		// End Site
		if ( (strcmp( token, "End" ) == 0 ) ){
			new_joint = joint;
			is_site = true;
			continue;
		}

		// Offset data
		if ( strcmp (token, "OFFSET") == 0) {
			// Offset coordinates
			x = atof(strtok(NULL, seperator));
			y = atof(strtok(NULL, seperator));
			z = atof(strtok(NULL, seperator));

			// End site case or not
			if (is_site) {
				joint->has_site = true;
				joint->site[0] = x;
				joint->site[1] = y;
				joint->site[2] = z;
			} else {
				joint->offset[0] = x;
				joint->offset[1] = y;
				joint->offset[2] = z;
			}
			continue;
		}

		if ( strcmp( token, "CHANNELS" ) == 0 )
		{
			joint->channels.resize( atoi( strtok(NULL, seperator) ) );

			// For every channel
			for ( i=0; i < joint->channels.size(); i++ )
			{
				// Channel setup
				Channel *  channel = new Channel();
				channel->joint = joint;
				channel->index = channels.size();
				channels.push_back( channel );
				joint->channels[ i ] = channel;

				// Figure out channel
				token = strtok( NULL, seperator );
				if ( strcmp( token, "Xrotation" ) == 0 )
					channel->type = X_ROTATION;
				else if ( strcmp( token, "Yrotation" ) == 0 )
					channel->type = Y_ROTATION;
				else if ( strcmp( token, "Zrotation" ) == 0 )
					channel->type = Z_ROTATION;
				else if ( strcmp( token, "Xposition" ) == 0 )
					channel->type = X_POSITION;
				else if ( strcmp( token, "Yposition" ) == 0 )
					channel->type = Y_POSITION;
				else if ( strcmp( token, "Zposition" ) == 0 )
					channel->type = Z_POSITION;
			}
		}

		// Motion section begin = this one ends
		if ( strcmp( token, "MOTION" ) == 0 )
			break;
	}


	// Now to figure out / store motion data
	file.getline( line, BUFFER_LEN );
	token = strtok( line, seperator );
	token = strtok( NULL, seperator );
	numFrames = atoi( token );

	file.getline( line, BUFFER_LEN );
	token = strtok( line, ":" );
	token = strtok( NULL, seperator );
	interval = atof( token );

	num_channel = channels.size();
	motion = new double[ numFrames * num_channel ];

	// actually retrieve motion data
	for (i = 0; i < numFrames; i++){
		file.getline( line, BUFFER_LEN );
		token = strtok( line, seperator );
		for(j = 0; j < num_channel; j++ ){
			if (token == NULL) return;
			motion[ i*num_channel + j] = atof(token);
			token = strtok( NULL, seperator);
		}
	}

	// aaaaaaand we're done
	file.close();
	is_load_success = true;
	return;
}

// Get the center point of the figure - used to focus on it
BVH::Coord BVH::getCenterPoint( int frame ){
	Coord center;
	double* data = motion + frame * num_channel;
	center.x = data[0];
	center.y = data[1];
	center.z = data[2];

	return center;
}

// Get the 'floor' - the total negative height from the root
double BVH::getFloorDepth(){
	Joint* node = joint_index.at("LeftAnkle");

	double depth = 0.0;
	depth += node->site[1];
	while (node){
		depth += node->offset[1];
		node = node->parent;
	}

	return depth;
}

// Render the figure
void BVH::RenderFigure(int frame) {
	RenderFigure( joints[0], motion + frame * num_channel);
}
void BVH::RenderFigure(const Joint* joint, const double* data){

	glPushMatrix();

	// Translate to the joint's location
	if (joint->parent == NULL)	glTranslatef( data[0], data[1], data[2] );
	else						glTranslatef(	joint->offset[0],
												joint->offset[1],
												joint->offset[2]);

	// Draw the joint
	glColor3f(1.0, 0.0, 0.0);
	glutSolidSphere(1.5, 8.0, 8.0);
	glColor3f(0.0, 0.0, 0.0);

	// Rotate each joint to the proper position
	for (int i = 0; i < joint->channels.size(); i++){
		Channel* channel = joint->channels[i];
		switch(channel->type) {
		case X_ROTATION:
			glRotatef( data[channel->index], 1.0f, 0.0f, 0.0f ); break;
		case Y_ROTATION:
			glRotatef( data[channel->index], 0.0f, 1.0f, 0.0f ); break;
		case Z_ROTATION:
			glRotatef( data[channel->index], 0.0f, 0.0f, 1.0f ); break;
		}
	}

	// Handle case where this is a 'leaf' node and there will be children
	if (joint->children.size() == 0 )
		RenderBone( 0.0f, 0.0f, 0.0f, joint->site[0], joint->site[1], joint->site[2] );

	// LOL IDUNNO
	//if (joint->children.size() == 1)
	if (joint->children.size() >= 1) {
		float center[3] = {0.0, 0.0, 0.0};
		for (int i = 0; i < joint->children.size(); i++){
			Joint* child = joint->children[i];
			center[0] += child->offset[0];
			center[1] += child->offset[1];
			center[2] += child->offset[2];
		}
		center[0] /= joint->children.size() + 1;
		center[1] /= joint->children.size() + 1;
		center[2] /= joint->children.size() + 1;

		RenderBone( 0.0, 0.0, 0.0, center[0], center[1], center[2]);

		for (int i = 0; i < joint->children.size(); i++){
			Joint* child = joint->children[i];
			RenderBone( center[0], center[1], center[2],
				child->offset[0], child->offset[1], child->offset[2]);
		}
	}

	// Recursively call for all further children
	for (int i = 0; i < joint->children.size(); i++)
		RenderFigure( joint->children[i], data );

	glPopMatrix();

}



// Renders an individual bone
void BVH::RenderBone( float x0, float y0, float z0, float x1, float y1, float z1 ){
	
	GLdouble	vx = x1 - x0;
	GLdouble	vy = y1 - y0;
	GLdouble	vz = z1 - z0;
	if (vz == 0) vz = 0.0001;

	GLdouble bone_length = sqrt(vx*vx + vy*vy + vz*vz);	

	// Because cylinders needs quadratic objects
	GLUquadricObj* quad_obj = gluNewQuadric();
	gluQuadricDrawStyle( quad_obj, GLU_FILL );
	gluQuadricNormals( quad_obj, GLU_SMOOTH );

	glPushMatrix();
	

	// Now we draw
	glTranslated( x0, y0, z0 );

	// Normalization
	GLdouble length = sqrt(vx*vx + vy*vy + vz*vz);
	if (length < 0.0001) {
		vx = 0.0; vy = 0.0; vz = 1.0; length = 1.0;
	}
	vx /= length; vy /= length; vz /= length;

	// Up stuff
	GLdouble upx = 0.0, upy = 1.0, upz = 0.0;

	double sidex = upy * vz - upz * vy;
	double sidey = upz * vx - upx * vz;
	double sidez = upx * vy - upy * vx;

	// Side normalization
	length = sqrt(sidex*sidex + sidey*sidey + sidez*sidez);
	if (length < 0.0001) {
		sidex = 1.0; sidey = 0.0; sidez = 0.0; length = 1.0;
	}
	sidex /= length; sidey /= length; sidez /= length;

	upx = vy*sidez - vz*sidey;
	upy = vz*sidex - vx*sidez;
	upz = vx*sidey - vy*sidex;

	GLdouble m[16] = {	sidex,	sidey,	sidez,	0.0,
						upx,	upy,	upz,	0.0,
						vx,		vy,		vz,		0.0,
						0.0,	0.0,	0.0,	1.0	};
	glMultMatrixd(m);

	// Cylinder info
	GLdouble radius = 0.5;
	GLdouble slices = 8.0;
	GLdouble stack = 3.0;

	glColor3f(1.0, 0.0, 0.0);
	gluCylinder( quad_obj, radius, radius, bone_length, slices, stack );
	//glutSolidSphere(10.0, 10, 10);
	glColor3f(0.0, 0.0, 0.0);

	glPopMatrix();

}