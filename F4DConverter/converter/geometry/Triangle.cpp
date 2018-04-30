
#include "stdafx.h"

#include "Triangle.h"

namespace gaia3d
{
	
	Triangle::Triangle()
	{
	}

	Triangle::~Triangle()
	{
	}

	void Triangle::alignVertexNormalsToPlaneNormal()
	{
		vertex[0]->normal.set(this->normal.x, this->normal.y, this->normal.z);
		vertex[1]->normal.set(this->normal.x, this->normal.y, this->normal.z);
		vertex[2]->normal.set(this->normal.x, this->normal.y, this->normal.z);
	}
}