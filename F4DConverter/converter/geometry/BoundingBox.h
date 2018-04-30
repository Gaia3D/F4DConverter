#pragma once

#include <vector>

namespace gaia3d
{
	class BoundingBox
	{
	public:
		BoundingBox();

		virtual ~BoundingBox();

	public:
		double minX, minY, minZ, maxX, maxY, maxZ;

		bool isInitialized;

		void addBox(BoundingBox& bbox);

		void addPoint(double x, double y, double z);

		void getCenterPoint(double& x, double& y, double& z);

		double getMaxLength();

		double getXLength();

		double getYLength();

		double getZLength();

		void init(double x, double y, double z);

		void divideBbox(int edgeDivisionCount, std::vector<BoundingBox>& result);

		void expand(double dist);
	};
}