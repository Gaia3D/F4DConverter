
#include "stdafx.h"

#include <math.h>

#include "Point3D.h"

namespace gaia3d
{
	Point3D::Point3D()
	{
	}

	Point3D::~Point3D()
	{
	}

	void Point3D::set(double x, double y, double z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}

	double Point3D::squaredDistanceTo(Point3D& target)
	{
		return ((x - target.x)*(x - target.x) + (y - target.y)*(y - target.y) + (z - target.z)*(z - target.z));
	}

	double Point3D::magnitude()
	{
		return sqrt(x*x + y*y + z*z);
	}

	bool Point3D::normalize()
	{
		double tolerance = 1E-7;
		double mag = this->magnitude();
		if( mag >= tolerance || mag <= -tolerance)
		{
			x /= mag; y /= mag; z /= mag;
			return true;
		}
		else
		{
			x = 0.0; y = 0.0; z = 0.0;
			return false;
		}
	}
}