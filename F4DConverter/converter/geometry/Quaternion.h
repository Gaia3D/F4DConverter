#pragma once

#include "Point3D.h"

namespace gaia3d
{
	enum TSentitPerfil {DESCONEGUT, CW, CCW};

	class Quaternion
	{
	public:

		double w, x, y, z;

		Quaternion(void);
		~Quaternion(void);

		void operator = (const Quaternion &q){w=q.w; x=q.x; y=q.y; z=q.z;}

		Quaternion operator *(const Quaternion &q)const
		{
			Quaternion t;
			t.w=(w*q.w - x*q.x - y*q.y - z*q.z);
			t.x=(w*q.x + x*q.w + y*q.z - z*q.y);
			t.y=(w*q.y - x*q.z + y*q.w + z*q.x);
			t.z=(w*q.z + x*q.y - y*q.x + z*q.w);
			return t;
		}

		Quaternion operator / (double d) const
		{
			Quaternion t;
			t.w=w/d; t.x=x/d; t.y=y/d; t.z=z/d;
			return t;
		}

		Quaternion operator + (const Quaternion &q) const
		{
			Quaternion t;
			t.w=w+q.w; t.x=x+q.x; t.y=y+q.y; t.z=z+q.z;
			return t;
		}

		Quaternion operator - (const Quaternion &q) const
		{
			Quaternion t;
			t.w=w-q.w; t.x=x-q.x; t.y=y-q.y; t.z=z-q.z;
			return t;
		}

		Quaternion operator ^ (const Quaternion &q) const // Prod Vectorial.***
		{
			Quaternion t;
			t.w=0.0; t.x=y*q.z-q.y*z; t.y=q.x*z-x*q.z; t.z=x*q.y-q.x*y;
			return t;
		}

		double operator % (const Quaternion &q) const // Prod Escalar.***
		{
			double prodEscalar;
			prodEscalar=w*q.w + x*q.x + y*q.y + z*q.z;
			return prodEscalar;
		}

		void set(double sw, double sx, double sy, double sz);
		double module(); // module.***
		void unitary(); // unitary.***
		void rotation(double ang_radians);
		Quaternion conjugate();
		double angleFrom(Quaternion v); // angle_respectVector.***
		bool isParallelTo(Quaternion v); // isParalelTo
		bool signsAreOppositeToEachOther(double a, double b);// areThereOppositeSign?
		bool isOppositeTo(Quaternion v);// isOppositeTo?
		double angleFromXAxis();// angle2d_respectXAxis
		double projectionAngleFrom(Quaternion v); // angle2d_respectVector
		void rotation(double ang, double axis_x, double axis_y, double axis_z);
		void rotation(double ang, Point3D *axis);
		TSentitPerfil sense2DFrom(Quaternion v); // sense2d_respectVector.***
	};
}