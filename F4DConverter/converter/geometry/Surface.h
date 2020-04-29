#pragma once

#include <vector>

#include "Triangle.h"

namespace gaia3d
{
	///< Surface는 triangle의 집합으로 이루어져 있다.
	class Surface
	{
	public:
		Surface();

		virtual ~Surface();

	protected:
		std::vector<Triangle*> triangles;

		///< 자신이 가지고 있는 triangle중 하나라도 외부에 노출되어 있다면, 이 surface는 외부에 노출된 surface이다.
		bool bExterior;

	public:
		std::vector<Triangle*>& getTriangles() {return triangles;}

		bool isExterior() {return bExterior;}

		void setIsExterior(bool bExterior) {this->bExterior = bExterior;}
	};
}