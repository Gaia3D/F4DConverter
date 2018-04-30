
#include "stdafx.h"

#include "Vertex.h"

namespace gaia3d
{
	Vertex::Vertex()
	{
		textureCoordinate[0] = textureCoordinate[1] = 0.0;
		color = DefaultColor;
	}

	Vertex::~Vertex()
	{
	}
}