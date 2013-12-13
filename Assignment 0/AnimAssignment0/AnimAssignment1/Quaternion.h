/*#define M_PI 3.14159265
#define toRad(x) ((x)*(M_PI/180.0))

class Point3D
{
public:
	double x,y,z;
};

class Quaternion{
public:
	double w;
	Point3D u;

	Quaternion(){}
	Quaternion(double x, double y, double z, double angle) {
		u.x = x;
		u.y = y;
		u.z = z;
		w = angle;
	}

	inline void Multiply(const Quaternion q)
		{
			Quaternion tmp;
			tmp.u.x = ((w * q.u.x) + (u.x * q.w) + (u.y * q.u.z) - (u.z * q.u.y));
			tmp.u.y = ((w * q.u.y) - (u.x * q.u.z) + (u.y * q.w) + (u.z * q.u.x));
			tmp.u.z = ((w * q.u.z) + (u.x * q.u.y) - (u.y * q.u.x) + (u.z * q.w));
			tmp.w = ((w * q.w) - (u.x * q.u.x) - (u.y * q.u.y) - (u.z * q.u.z));
			*this = tmp;
		}

	inline double Norm()
	{return sqrt(u.x*u.x+u.y*u.y+u.z*u.z+w*w);}


	inline void Normalize()
	{
	double norm=Norm();
	u.x/=norm;u.y/=norm;u.z/=norm;
	}

	inline void Conjugate()
	{
	u.x=-u.x;
	u.y=-u.y;
	u.z=-u.z;
	}

	inline void Inverse()
	{
	double norm=Norm();
	Conjugate();
	u.x/=norm;
	u.y/=norm;
	u.z/=norm;
	w/=norm;
	}

	void ExportToMatrix(float matrix[16]) 
{
	float wx, wy, wz, xx, yy, yz, xy, xz, zz;
	// adapted from Shoemake
	xx = u.x * u.x;
	xy = u.x * u.y;
	xz = u.x * u.z;
	yy = u.y * u.y;
	zz = u.z * u.z;
	yz = u.y * u.z;

	wx = w * u.x;
	wy = w * u.y;
	wz = w * u.z;

	matrix[0] = 1.0f - 2.0f*(yy + zz);
	matrix[4] = 2.0f*(xy - wz);
	matrix[8] = 2.0f*(xz + wy);
	matrix[12] = 0.0;
 
	matrix[1] = 2.0f*(xy + wz);
	matrix[5] = 1.0f - 2.0f*(xx + zz);
	matrix[9] = 2.0f*(yz - wx);
	matrix[13] = 0.0;

	matrix[2] = 2.0f*(xz - wy);
	matrix[6] = 2.0f*(yz + wx);
	matrix[10] = 1.0f - 2.0f*(xx + yy);
	matrix[14] = 0.0;

	matrix[3] = 0;
	matrix[7] = 0;
	matrix[11] = 0;
	matrix[15] = 1;
}
	
};*/














#include <cmath>

class Quaternion
{
public:
	Quaternion() : x(0), y(0), z(0), theta(0) {}
	Quaternion(double _x, double _y, double _z, double angle) :
		x(_x), y(_y), z(_z), theta(angle) {}
	~Quaternion(void){}


	Quaternion operator*(const Quaternion& a) {
		return Quaternion(	(theta*a.x) + (x*a.theta) + (y*a.z) - (z*a.y),
							(theta*a.y) - (x*a.z) + (y*a.theta) + (z*a.x),
							(theta*a.z) + (x*a.y) - (y*a.x) + (z*a.theta),
							(theta*a.theta) - (x*a.x) - (y*a.y) - (z*a.z)	);
	}

	void normalize() {
		double mag = magnitude();
		x /= mag;
		y /= mag;
		z /= mag;
		//theta /= mag;
	}

	double magnitude() {
		return sqrt( (theta*theta) + (x*x) + (y*y) + (z*z) );
	}

	double x, y, z, theta;
};

// Dot product - despite the operator
inline double operator^(const Quaternion& a, const Quaternion& b) {
	return (a.x*b.x) + (a.y*b.y) + (a.z*b.z) + (a.theta*b.theta);
}

inline Quaternion operator-(const Quaternion& a){
	return Quaternion(-a.x, -a.y, -a.z, -a.theta);
}

inline Quaternion operator*(const Quaternion& a, double b){
	return Quaternion( a.x * b, a.y * b, a.z * b, a.theta * b);
}

inline Quaternion operator/(const Quaternion& a, double b){
	return Quaternion( a.x / b, a.y / b, a.z / b, a.theta / b);
}

Quaternion operator+(const Quaternion& a, const Quaternion& b) {
		return Quaternion( b.x + a.x, b.y + a.y, b.z + a.z, b.theta + a.theta );
}