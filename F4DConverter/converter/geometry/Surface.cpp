
#include "stdafx.h"

#include "Surface.h"

namespace gaia3d
{
	Surface::Surface()
	{
		bExterior = false;
	}

	Surface::~Surface()
	{
		size_t triangleCount = triangles.size();
		for(size_t i = 0; i < triangleCount; i++)
			delete triangles[i];
		triangles.clear();
	}
}