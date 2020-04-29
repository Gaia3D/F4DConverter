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
		///< vertex의 위치
		Point3D position;
		///< vertex의 normal
		Point3D normal;
		///< vertex의 tetureCoordinate. 2차원 좌표. 각각 0부터 1사이의 값을 가진다.
		double textureCoordinate[2];
		///< vertex의 색
		ColorU4 color;
	};
}