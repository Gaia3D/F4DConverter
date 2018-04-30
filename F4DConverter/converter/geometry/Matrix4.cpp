
#include "stdafx.h"

#include "Matrix4.h"

#include <math.h>
#include <stdexcept>

namespace gaia3d
{
	Matrix4::Matrix4()
	{
		identity();
	}

	Matrix4::~Matrix4()
	{}

	void Matrix4::rotation(Quaternion *q)
	{
		const double w = q->w;
		const double x = q->x;
		const double y = q->y;
		const double z = q->z;
		m[0][0] = 1 - 2 * y*y - 2 * z*z; m[1][0] = 2 * x*y - 2 * z*w;     m[2][0] = 2 * x*z + 2 * y*w;     m[3][0] = 0.0;
		m[0][1] = 2 * x*y + 2 * z*w;     m[1][1] = 1 - 2 * x*x - 2 * z*z; m[2][1] = 2 * y*z - 2 * x*w;     m[3][1] = 0.0;
		m[0][2] = 2 * x*z - 2 * y*w;     m[1][2] = 2 * y*z + 2 * x*w;     m[2][2] = 1 - 2 * x*x - 2 * y*y; m[3][2] = 0.0;
		m[0][3] = 0.0;               m[1][3] = 0.0;               m[2][3] = 0.0;               m[3][3] = 1.0;
	}

	void Matrix4::rotation(double ang_rad, Point3D *axis)
	{
		Quaternion q;
		q.rotation(ang_rad, axis);
		this->rotation(&q);
	}

	void Matrix4::rotation(double ang_rad, double axis_x, double axis_y, double axis_z)
	{
		Quaternion q;

		q.rotation(ang_rad, axis_x, axis_y, axis_z);
		this->rotation(&q);
	}

	void Matrix4::rotationInDegree(double ang_degree, double axis_x, double axis_y, double axis_z)
	{
		double M_PI = 3.14159265358979323846;
		double ang_rad = ang_degree * M_PI / 180.0;
		this->rotation(ang_rad, axis_x, axis_y, axis_z);
	}

	void Matrix4::perspective(double fov, double n, double f)
	{
		double s;
		double MPI_Div_180 = 3.14159265358979323846 / 180.0;

		s = 1.0 / (tan(fov*0.5*MPI_Div_180));

		m[0][0] = s;   m[1][0] = 0.0; m[2][0] = 0.0;      m[3][0] = 0.0;
		m[0][1] = 0.0; m[1][1] = s;   m[2][1] = 0.0;      m[3][1] = 0.0;
		m[0][2] = 0.0; m[1][2] = 0.0; m[2][2] = -f / (f - n); m[3][2] = -f*n / (f - n);
		m[0][3] = 0.0; m[1][3] = 0.0; m[2][3] = -1.0;     m[3][3] = 0.0;
	}

	void Matrix4::perspective(double fov, double aspect, double n, double f)
	{
		double s, dp;
		double MPI_Div_180 = 3.14159265358979323846 / 180.0;

		s = 1.0 / (tan(fov*0.5*MPI_Div_180));
		dp = n - f;

		m[0][0] = s / aspect;   m[1][0] = 0.0; m[2][0] = 0.0;      m[3][0] = 0.0;
		m[0][1] = 0.0;        m[1][1] = s;   m[2][1] = 0.0;      m[3][1] = 0.0;
		m[0][2] = 0.0;		m[1][2] = 0.0; m[2][2] = (f + n) / dp; m[3][2] = (2.0*f*n) / dp;
		m[0][3] = 0.0;		m[1][3] = 0.0; m[2][3] = -1.0;     m[3][3] = 0.0;
	}

	void Matrix4::perspectiveInverse(double fov, double aspect, double n, double f)
	{
		double s, dp;
		double MPI_Div_180 = 3.14159265358979323846 / 180.0;

		s = 1.0 / (tan(fov*0.5*MPI_Div_180));
		dp = n - f;

		m[0][0] = aspect / s;   m[1][0] = 0.0;	 m[2][0] = 0.0;           m[3][0] = 0.0;
		m[0][1] = 0.0;        m[1][1] = 1.0 / s;   m[2][1] = 0.0;           m[3][1] = 0.0;
		m[0][2] = 0.0;		m[1][2] = 0.0;     m[2][2] = 0.0;           m[3][2] = -1.0;
		m[0][3] = 0.0;		m[1][3] = 0.0;     m[2][3] = dp / (2.0*f*n);  m[3][3] = (f + n) / (2.0*f*n);
	}

	Point3D Matrix4::operator * (const Point3D &q) const
	{
		Point3D t;

		t.x = q.x*m[0][0] + q.y*m[1][0] + q.z*m[2][0] + m[3][0];
		t.y = q.x*m[0][1] + q.y*m[1][1] + q.z*m[2][1] + m[3][1];
		t.z = q.x*m[0][2] + q.y*m[1][2] + q.z*m[2][2] + m[3][2];

		return t;
	}

	void Matrix4::applyOnlyRotationOnPoint(Point3D& q)
	{
		double x = q.x*m[0][0] + q.y*m[1][0] + q.z*m[2][0];
		double y = q.x*m[0][1] + q.y*m[1][1] + q.z*m[2][1];
		double z = q.x*m[0][2] + q.y*m[1][2] + q.z*m[2][2];

		q.set(x, y, z);
	}

	Matrix4 Matrix4::operator * (const Matrix4 &A)
	{
		Matrix4 c;
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				c.m[i][j] = 0.0;
				for (int k = 0; k < 4; k++) {
					c.m[i][j] += A.m[k][j] * m[i][k];
				}
			}
		}

		return c;
	}

	Matrix4 Matrix4::inverse()
	{
		Matrix4 inv;

		double src0 = m[0][0];
		double src1 = m[0][1];
		double src2 = m[0][2];
		double src3 = m[0][3];
		double src4 = m[1][0];
		double src5 = m[1][1];
		double src6 = m[1][2];
		double src7 = m[1][3];
		double src8 = m[2][0];
		double src9 = m[2][1];
		double src10 = m[2][2];
		double src11 = m[2][3];
		double src12 = m[3][0];
		double src13 = m[3][1];
		double src14 = m[3][2];
		double src15 = m[3][3];

		// calculate pairs for first 8 elements (cofactors)
		double tmp0 = src10 * src15;
		double tmp1 = src11 * src14;
		double tmp2 = src9 * src15;
		double tmp3 = src11 * src13;
		double tmp4 = src9 * src14;
		double tmp5 = src10 * src13;
		double tmp6 = src8 * src15;
		double tmp7 = src11 * src12;
		double tmp8 = src8 * src14;
		double tmp9 = src10 * src12;
		double tmp10 = src8 * src13;
		double tmp11 = src9 * src12;

		// calculate first 8 elements (cofactors)
		double dst0 = (tmp0 * src5 + tmp3 * src6 + tmp4 * src7) - (tmp1 * src5 + tmp2 * src6 + tmp5 * src7);
		double dst1 = (tmp1 * src4 + tmp6 * src6 + tmp9 * src7) - (tmp0 * src4 + tmp7 * src6 + tmp8 * src7);
		double dst2 = (tmp2 * src4 + tmp7 * src5 + tmp10 * src7) - (tmp3 * src4 + tmp6 * src5 + tmp11 * src7);
		double dst3 = (tmp5 * src4 + tmp8 * src5 + tmp11 * src6) - (tmp4 * src4 + tmp9 * src5 + tmp10 * src6);
		double dst4 = (tmp1 * src1 + tmp2 * src2 + tmp5 * src3) - (tmp0 * src1 + tmp3 * src2 + tmp4 * src3);
		double dst5 = (tmp0 * src0 + tmp7 * src2 + tmp8 * src3) - (tmp1 * src0 + tmp6 * src2 + tmp9 * src3);
		double dst6 = (tmp3 * src0 + tmp6 * src1 + tmp11 * src3) - (tmp2 * src0 + tmp7 * src1 + tmp10 * src3);
		double dst7 = (tmp4 * src0 + tmp9 * src1 + tmp10 * src2) - (tmp5 * src0 + tmp8 * src1 + tmp11 * src2);

		// calculate pairs for second 8 elements (cofactors)
		tmp0 = src2 * src7;
		tmp1 = src3 * src6;
		tmp2 = src1 * src7;
		tmp3 = src3 * src5;
		tmp4 = src1 * src6;
		tmp5 = src2 * src5;
		tmp6 = src0 * src7;
		tmp7 = src3 * src4;
		tmp8 = src0 * src6;
		tmp9 = src2 * src4;
		tmp10 = src0 * src5;
		tmp11 = src1 * src4;

		// calculate second 8 elements (cofactors)
		double dst8 = (tmp0 * src13 + tmp3 * src14 + tmp4 * src15) - (tmp1 * src13 + tmp2 * src14 + tmp5 * src15);
		double dst9 = (tmp1 * src12 + tmp6 * src14 + tmp9 * src15) - (tmp0 * src12 + tmp7 * src14 + tmp8 * src15);
		double dst10 = (tmp2 * src12 + tmp7 * src13 + tmp10 * src15) - (tmp3 * src12 + tmp6 * src13 + tmp11 * src15);
		double dst11 = (tmp5 * src12 + tmp8 * src13 + tmp11 * src14) - (tmp4 * src12 + tmp9 * src13 + tmp10 * src14);
		double dst12 = (tmp2 * src10 + tmp5 * src11 + tmp1 * src9) - (tmp4 * src11 + tmp0 * src9 + tmp3 * src10);
		double dst13 = (tmp8 * src11 + tmp0 * src8 + tmp7 * src10) - (tmp6 * src10 + tmp9 * src11 + tmp1 * src8);
		double dst14 = (tmp6 * src9 + tmp11 * src11 + tmp3 * src8) - (tmp10 * src11 + tmp2 * src8 + tmp7 * src9);
		double dst15 = (tmp10 * src10 + tmp4 * src8 + tmp9 * src9) - (tmp8 * src9 + tmp11 * src10 + tmp5 * src8);

		// calculate determinant
		double epsilon20 = 0.00000000000000000001;
		double det = src0 * dst0 + src1 * dst1 + src2 * dst2 + src3 * dst3;

		if (fabs(det) < epsilon20) {
			throw std::runtime_error("matrix is not invertible because its determinate is zero.");
		}

		// calculate matrix inverse
		det = 1.0 / det;

		inv.m[0][0] = dst0 * det;
		inv.m[1][0] = dst1 * det;
		inv.m[2][0] = dst2 * det;
		inv.m[3][0] = dst3 * det;
		inv.m[0][1] = dst4 * det;
		inv.m[1][1] = dst5 * det;
		inv.m[2][1] = dst6 * det;
		inv.m[3][1] = dst7 * det;
		inv.m[0][2] = dst8 * det;
		inv.m[1][2] = dst9 * det;
		inv.m[2][2] = dst10 * det;
		inv.m[3][2] = dst11 * det;
		inv.m[0][3] = dst12 * det;
		inv.m[1][3] = dst13 * det;
		inv.m[2][3] = dst14 * det;
		inv.m[3][3] = dst15 * det;

		return inv;
	}

	Matrix4 Matrix4::transpose()
	{
		Matrix4 trans;

		double matrix10 = m[1][0];
		double matrix20 = m[2][0];
		double matrix30 = m[3][0];
		double matrix21 = m[2][1];
		double matrix31 = m[3][1];
		double matrix32 = m[3][2];

		trans.m[0][0] = m[0][0];
		trans.m[1][0] = m[0][1];
		trans.m[2][0] = m[0][2];
		trans.m[3][0] = m[0][3];
		trans.m[0][1] = matrix10;
		trans.m[1][1] = m[1][1];
		trans.m[2][1] = m[1][2];
		trans.m[3][1] = m[1][3];
		trans.m[0][2] = matrix20;
		trans.m[1][2] = matrix21;
		trans.m[2][2] = m[2][2];
		trans.m[3][2] = m[2][3];
		trans.m[0][3] = matrix30;
		trans.m[1][3] = matrix31;
		trans.m[2][3] = matrix32;
		trans.m[3][3] = m[3][3];

		return trans;
	}

	void Matrix4::getOnlyRotationFloatArray(float* rotation)
	{
		rotation[0] = float(m[0][0]); // col 0, row 0
		rotation[1] = float(m[1][0]); // col 1, row 0
		rotation[2] = float(m[2][0]); // col 2, row 0
		rotation[3] = float(m[0][1]); // col 0, row 1
		rotation[4] = float(m[1][1]); // col 1, row 1
		rotation[5] = float(m[2][1]); // col 2, row 1
		rotation[6] = float(m[0][2]); // col 0, row 2
		rotation[7] = float(m[1][2]); // col 1, row 2
		rotation[8] = float(m[2][2]); // col 2, row 2
	}

	void Matrix4::getOnlyRotationDoubleArray(double* rotation)
	{
		rotation[0] = (m[0][0]); // col 0, row 0
		rotation[1] = (m[1][0]); // col 1, row 0
		rotation[2] = (m[2][0]); // col 2, row 0
		rotation[3] = (m[0][1]); // col 0, row 1
		rotation[4] = (m[1][1]); // col 1, row 1
		rotation[5] = (m[2][1]); // col 2, row 1
		rotation[6] = (m[0][2]); // col 0, row 2
		rotation[7] = (m[1][2]); // col 1, row 2
		rotation[8] = (m[2][2]); // col 2, row 2
	}

	unsigned char Matrix4::getMatrixType(double error)
	{
		// matrixType = 0 -> identity matrix.
		// matrixType = 1 -> translate matrix.
		// matrixType = 2 -> transform matrix.
		bool is3by3PartIdentity = false;
		if (m[0][0] < 1.0+error && m[0][0] > 1.0-error)
			if (m[0][1] < error && m[0][1] > -error)
				if (m[0][2] < error && m[0][2] > -error)
					if (m[1][0] < error && m[1][0] > -error)
						if (m[1][1] < 1.0+error && m[1][1] > 1.0-error)
							if (m[1][2] < error && m[1][2] > -error)
								if (m[2][0] < error && m[2][0] > -error)
									if (m[2][1] < error && m[2][1] > -error)
										if (m[2][2] < 1.0+error && m[2][2] > 1.0-error)
											is3by3PartIdentity = true;

		unsigned char type = 2;
		if (is3by3PartIdentity)
		{
			type = 1;
			// check if there are translation.
			if (m[0][3] < error && m[0][3] > -error)
				if (m[1][3] < error && m[1][3] > -error)
					if (m[2][3] < error && m[2][3] > -error)
						if (m[3][0] < error && m[3][0] > -error)
							if (m[3][1] < error && m[3][1] > -error)
								if (m[3][2] < error && m[3][2] > -error)
									if (m[3][3] < 1.0+error && m[3][3] > 1.0-error)
										type = 0;
		}

		return type;
	}
}