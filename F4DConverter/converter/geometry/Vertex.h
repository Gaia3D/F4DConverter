#pragma once

#include "Point3D.h"

#include "ColorU4.h"

namespace gaia3d
{
	class Vertex
	{
	public:
		Vertex();

		virtual ~Vertex();

	public:
		Point3D position;
		Point3D normal;
		double textureCoordinate[2];
		ColorU4 color;
	};
}