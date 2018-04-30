
#include "stdafx.h"

#include "Quaternion.h"

#include <math.h>

namespace gaia3d
{
	Quaternion::Quaternion(void)
	{
	}


	Quaternion::~Quaternion(void)
	{
	}

	void Quaternion::set(double sw, double sx, double sy, double sz)
	{
		w=sw; x=sx; y=sy; z=sz;
	}

	double Quaternion::module()
	{
		return (sqrt(w*w+x*x+y*y+z*z));
	}

	void Quaternion::unitary()
	{
		double modul=this->module();
		w/=modul; 
		x/=modul;
		y/=modul;
		z/=modul;
	}

	void Quaternion::rotation(double ang_radians)
	{
		this->unitary();

		w=cos(ang_radians/2.0);
		x*=sin(ang_radians/2.0);
		y*=sin(ang_radians/2.0);
		z*=sin(ang_radians/2.0);
	}

	Quaternion Quaternion::conjugate()
	{
		Quaternion q;
		q.w=w; q.x=-x; q.y=-y; q.z=-z;
		return q;
	}

	double Quaternion::angleFrom(Quaternion v)
	{
		// a*b = |a|*|b|*cos(alfa)
		// Nota: a*b = ab
		// Nota: |a|*|b| = ab2
		double ang;
		double ab, ab2;
		double M_PI = 3.14159265358979323846;

		ab= x*v.x+ y*v.y+ z*v.z;
		ab2= module()*v.module();

		if(ab/ab2<=-1.0)ang=M_PI;
		else
		{
			if(ab/ab2>=1.0)ang=0.0;
			else ang=cos(ab/ab2);
		}
		return ang;// Sempre dona ang petit. L'ang entre(1,1,1) i (-1,-1,-1) dona zero...comprobar.***
	}

	bool Quaternion::isParallelTo(Quaternion v)
	{
		// Perque 2 vectors siguin paralels => prod vectorial = zero.***
		bool es_paralel;
		Quaternion vProd;
		double error=10E-11;

		vProd=this->operator^(v);
		
		if(vProd.x >= error || vProd.x <= -error ||
			vProd.y >= error || vProd.y <= -error ||
			vProd.z >= error || vProd.z <= -error)
			es_paralel=false;
		else
			es_paralel=true;

		return es_paralel;
	}

	bool Quaternion::signsAreOppositeToEachOther(double a, double b)
	{
		// Aquesta funcio serveix per la seguent funcio: bool Es_Oposat_a(Quaternion v).***
		bool son_oposats;

		if(a>0.0 && b<0.0)son_oposats=true;
		else
		{
			if(a<0.0 && b>0.0)son_oposats=true;
			else son_oposats=false;
		}
		return son_oposats;
	}

	bool Quaternion::isOppositeTo(Quaternion v)
	{
		bool son_oposats;
		bool oposat_x, oposat_y, oposat_z;
		double tolerance=10E-13;

		// x.***
		if(x >= tolerance || x <= -tolerance)
		{
			if(this->signsAreOppositeToEachOther(x, v.x))oposat_x=true;
			else oposat_x=false;
		}
		else
		{
			if(v.x >= tolerance || v.x <= - tolerance) oposat_x=false;
			else oposat_x=true;
		}

		// y.***
		if(y >= tolerance || y <= -tolerance)
		{
			if(this->signsAreOppositeToEachOther(y, v.y))oposat_y=true;
			else oposat_y=false;
		}
		else 
		{
			if(v.y >= tolerance || v.y <= -tolerance) oposat_y=false;
			else oposat_y=true;
		}

		// z.***
		if(z >= tolerance || z <= -tolerance)
		{
			if(this->signsAreOppositeToEachOther(z, v.z))oposat_z=true;
			else oposat_z=false;
		}
		else
		{
			if(v.z >= tolerance || v.z <= -tolerance) oposat_z=false;
			else oposat_z=true;
		}

		if(oposat_x && oposat_y && oposat_z)son_oposats=true;
		else son_oposats=false;
		return son_oposats;
	}

	double Quaternion::angleFromXAxis()
	{
		// Funcio 2D.***
		double ang;
		double tolerance=10E-11;
		double M_PI = 3.14159265358979323846;

		double xa, ya;

		xa=x*100000.0;// Per salvar vectors molt petits.***
		ya=y*100000.0;// Per salvar vectors molt petits.***

		// Hem de salvar divisions per zero.***
		
		if(xa >= tolerance || xa <= -tolerance)
		{
			ang=atan(ya/xa);
			// Hem de fer un post-tractament.***
			if(xa>=0.0)
			{
				if(ya<0.0)ang+=2.0*M_PI;
			}
			else
			{
				if(ya>0.0)ang=M_PI+ang;
				else ang+=M_PI;// Aquesta sentencia es igual a l'anterior.***
			}
		}
		else
		{
			// El vector sembla vertical.***
			if(ya>0.0)ang=M_PI*0.5;
			else ang=M_PI*1.5;
		}
		return ang;
	}

	double Quaternion::projectionAngleFrom(Quaternion v)
	{
		// Funcio 2D.***
		// L'ang que busquem es per anar desde el Meu_Vector cap a v.***
		// L'ang pot ser positiu o negatiu.***
		double ang_v, ang_meu_v;
		double ang;
		double M_PI = 3.14159265358979323846;

		ang_v=v.angleFromXAxis();
		ang_meu_v=this->angleFromXAxis();
		// Hem de fer un tractament.***
		if(ang_meu_v>ang_v)ang_v+=2.0*M_PI;
		ang=ang_v-ang_meu_v;
		if(ang>M_PI)ang-=M_PI*2.0;

		return ang;
	}

	void Quaternion::rotation(double ang, double axis_x, double axis_y, double axis_z)
	{
		// Code copied From Carve.***
		double tolerance = 10E-13;
		double s = sqrt(axis_x*axis_x + axis_y*axis_y + axis_z*axis_z);
		if (s >= tolerance || s<= -tolerance) {
			double c = 1.0 / s;
			double omega = -0.5 * ang;
			s = sin(omega);
			x = axis_x * c * s;
			y = axis_y * c * s;
			z = axis_z * c * s;
			w = cos(omega);
			unitary();
		} else {
			x = y = z = 0.0;
			w = 1.0;
		}
	}

	void Quaternion::rotation(double ang, Point3D *axis)
	{
		rotation(ang, axis->x, axis->y, axis->z);
	}

	TSentitPerfil Quaternion::sense2DFrom(Quaternion v)
	{
		TSentitPerfil sentit;
		if(this->projectionAngleFrom(v)>=0.0)sentit=CCW;
		else sentit=CW;
		return sentit;
	}
}