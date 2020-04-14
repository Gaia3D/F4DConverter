#pragma once

#include <vector>

#include "../predefinition.h"

namespace gaia3d
{
	class Vertex;
	///< 해당 구조체는 그래픽카드가 이해하는 데이터로 변환하기 쉬운 형식으로 이루어져 있다.
	///< Vbo로 변환할 때, 삼각형의 가장 큰 변을 삼각형의 크기로 간주하고 이를 기준으로 정렬해서 크기순으로 정렬한다.
	///< Vbo로 변환되는 단위는 삼각형 단위이면서, 삼각형들을 이루는 vertex의 총 수가 65532개를 넘지 않도록 변환한다.
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