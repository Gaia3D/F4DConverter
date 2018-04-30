#pragma once

#include <vector>

#include "../predefinition.h"

namespace gaia3d
{
	class Vertex;
	struct Vbo
	{
		std::vector<Vertex*> vertices;
		std::vector<unsigned short> indices;
		double triangleSizeThresholds[TriangleSizeLevels];
		unsigned int indexMarker[TriangleSizeLevels];
		Vbo()
		{
			double value[TriangleSizeLevels] = TriangleSizeThresholds;
			memcpy(triangleSizeThresholds, value, sizeof(double)*TriangleSizeLevels);
		}

	};
}