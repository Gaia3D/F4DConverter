#pragma once

namespace gaia3d
{
	class Point3D
	{
	public:
		Point3D();

		virtual ~Point3D();

	public:
		double x, y, z;

		void set(double x, double y, double z);

		double squaredDistanceTo(Point3D& target);

		double magnitude();

		bool normalize(double tolerance = 1E-7);

		// operator overriding
		void operator = (const Point3D &q){x=q.x; y=q.y; z=q.z;}

		Point3D operator - (const Point3D &q) const
		{
			Point3D t;
			t.x=x-q.x; t.y=y-q.y; t.z=z-q.z;
			return t;
		}

		void operator -= (const Point3D& q)
		{
			x -= q.x; y -= q.y; z -= q.z;
		}

		Point3D operator + (const Point3D &q) const
		{
			Point3D t;
			t.x = x + q.x; t.y = y + q.y; t.z = z + q.z;
			return t;
		}

		void operator += (const Point3D& q)
		{
			x += q.x; y += q.y; z += q.z;
		}

		Point3D operator ^ (const Point3D &v) const
		{
			Point3D t;
			t.x=y*v.z-v.y*z; t.y=v.x*z-x*v.z; t.z=x*v.y-v.x*y;
			return t;
		}

		Point3D operator * (double valor)
		{
			Point3D t;
			t.x = x * valor; t.y = y * valor; t.z = z * valor;
			return t;
		}

		void operator *= (double valor)
		{
			x*=valor; y*=valor; z*=valor;
		}

		Point3D operator / (double valor)
		{
			Point3D t;
			t.x = x / valor; t.y = y / valor; t.z = z / valor;
			return t;
		}

		void operator /= (double valor)
		{
			x /= valor; y /= valor; z /= valor;
		}
	};
}