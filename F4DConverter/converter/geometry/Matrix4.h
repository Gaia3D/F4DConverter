#pragma once

#include "Point3D.h"
#include "Quaternion.h"

namespace gaia3d
{
	class Matrix4
	{
	public:
		Matrix4();

		virtual ~Matrix4();

	public:
		double m[4][4]; // m[col][row]

		void identity()
		{
			m[0][0]=1.0; m[1][0]=0.0; m[2][0]=0.0; m[3][0]=0.0;
			m[0][1]=0.0; m[1][1]=1.0; m[2][1]=0.0; m[3][1]=0.0;
			m[0][2]=0.0; m[1][2]=0.0; m[2][2]=1.0; m[3][2]=0.0;
			m[0][3]=0.0; m[1][3]=0.0; m[2][3]=0.0; m[3][3]=1.0;
		}

		void set(Matrix4 *mat)
		{
			this->set(mat->m[0][0], mat->m[0][1], mat->m[0][2], mat->m[0][3],
					  mat->m[1][0], mat->m[1][1], mat->m[1][2], mat->m[1][3],
					  mat->m[2][0], mat->m[2][1], mat->m[2][2], mat->m[2][3],
					  mat->m[3][0], mat->m[3][1], mat->m[3][2], mat->m[3][3]);
		}
		void set(Matrix4& mat)
		{
			this->set(mat.m[0][0], mat.m[0][1], mat.m[0][2], mat.m[0][3],
					  mat.m[1][0], mat.m[1][1], mat.m[1][2], mat.m[1][3],
					  mat.m[2][0], mat.m[2][1], mat.m[2][2], mat.m[2][3],
					  mat.m[3][0], mat.m[3][1], mat.m[3][2], mat.m[3][3]);
		}

		void set(double value_11, double value_12, double value_13, double value_14,
				double value_21, double value_22, double value_23, double value_24,
				double value_31, double value_32, double value_33, double value_34,
				double value_41, double value_42, double value_43, double value_44)
		{
			m[0][0]=value_11; m[1][0]=value_21; m[2][0]=value_31; m[3][0]=value_41;
			m[0][1]=value_12; m[1][1]=value_22; m[2][1]=value_32; m[3][1]=value_42;
			m[0][2]=value_13; m[1][2]=value_23; m[2][2]=value_33; m[3][2]=value_43;
			m[0][3]=value_14; m[1][3]=value_24; m[2][3]=value_34; m[3][3]=value_44;
		}

		void rotation(Quaternion *q);

		void rotation(double ang_rad, Point3D *axis);

		void rotation(double ang_rad, double axis_x, double axis_y, double axis_z);

		void rotationInDegree(double ang_degree, double axis_x, double axis_y, double axis_z);

		void translation(Point3D *pos)
		{
			translation(pos->x, pos->y, pos->z);
		}

		void translation(double x, double y, double z)
		{
			m[0][0]=1.0; m[1][0]=0.0; m[2][0]=0.0; m[3][0]=x;
			m[0][1]=0.0; m[1][1]=1.0; m[2][1]=0.0; m[3][1]=y;
			m[0][2]=0.0; m[1][2]=0.0; m[2][2]=1.0; m[3][2]=z;
			m[0][3]=0.0; m[1][3]=0.0; m[2][3]=0.0; m[3][3]=1.0;
		}

		void perspective(double fov, double n, double f);

		void perspective(double fov, double aspect, double n, double f);

		void perspectiveInverse(double fov, double aspect, double n, double f);

		void frustum(double l, double r, double t, double b, double n, double f)
		{
			m[0][0]=2.0*n/(r-l); m[1][0]=0.0;         m[2][0]=(t+l)/(r-l);   m[3][0]=0.0;
			m[0][1]=0.0;         m[1][1]=2.0*n/(t-b); m[2][1]=(t+b)/(t-b);   m[3][1]=0.0;
			m[0][2]=0.0;		 m[1][2]=0.0;         m[2][2]=-(f+n)/(f-n);  m[3][2]=-2.0*f*n/(f-n);
			m[0][3]=0.0;         m[1][3]=0.0;         m[2][3]=-1.0;          m[3][3]=0.0;

		}

		Point3D operator * (const Point3D &q) const;

		void applyOnlyRotationOnPoint(Point3D& q);

		Matrix4 operator * (const Matrix4 &A);

		void operator = (const Matrix4& A)
		{
			for (unsigned char i = 0; i < 4; i++)
				for (unsigned char j = 0; j < 4; j++)
					m[i][j] = A.m[i][j];
		}

		void getFloatArray(float* result)
		{
			for (unsigned char i = 0; i < 4; i++)
			{
				for (unsigned char j = 0; j < 4; j++)
				{
					result[i*4 + j] = (float)m[j][i];
				}
			}
		}

		void getDoubleArray(double* result)
		{
			for (unsigned char i = 0; i < 4; i++)
			{
				for (unsigned char j = 0; j < 4; j++)
				{
					result[j + i * 4] = m[j][i];
				}
			}
		}

		Matrix4 inverse();

		Matrix4 transpose();

		void getOnlyRotationFloatArray(float* rotation);

		void getOnlyRotationDoubleArray(double* rotation);

		unsigned char getMatrixType(double error);
	};
}