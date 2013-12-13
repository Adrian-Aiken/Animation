#pragma once


#include <vector>
#include <map>
#include <string>

using namespace std;


class BVH
{
public:

	enum ChannelEnum {
		X_ROTATION, Y_ROTATION, Z_ROTATION,
		X_POSITION, Y_POSITION, Z_POSITION
	};
	struct	Joint;

	struct Channel {
		Joint*		joint;
		ChannelEnum type;
		int			index;
	};

	struct Coord {
		double x, y, z;
	};

	struct	Joint{
		string		name;
		int			index;
		Joint*		parent;
		vector< Joint* > children;
		double		offset[3];
		bool		has_site;
		double		site[3];
		vector< Channel* > channels;
	};

public:
	bool	is_load_success;

	string	fileName;
	//string motion_name;

	int		num_channel;
	vector< Channel* >	channels;
	vector< Joint* >	joints;
	map< string, Joint* > joint_index;

	int		numFrames;
	double	interval;
	double*	motion;

public:
	// Setup stuff
	BVH(void);
	BVH( const char* file_name);
	~BVH(void);

	void	Clear();

	void	Load( const char* file_name);


public:
	// Operations on it
	bool isLoadSuccess() const {return is_load_success;};
	Coord getCenterPoint( int frame );
	double getFloorDepth();
	void RenderFigure( int frame );
	static void RenderFigure( const Joint* root, const double* data);
	static void RenderBone( float x0, float y0, float z0, float x1, float y1, float z1 );
};

