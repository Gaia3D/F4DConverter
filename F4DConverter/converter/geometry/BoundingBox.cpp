
#include "stdafx.h"

#include "BoundingBox.h"

namespace gaia3d
{
	BoundingBox::BoundingBox()
	{
		minX = minY = minZ = 10E9;

		maxX = maxY = maxZ = -10E9;

		isInitialized = false;
	}

	BoundingBox::~BoundingBox()
	{
	}

	void BoundingBox::addBox(BoundingBox& bbox)
	{
		addPoint(bbox.maxX, bbox.maxY, bbox.maxZ);
		addPoint(bbox.minX, bbox.minY, bbox.minZ);
	}

	void BoundingBox::addPoint(double x, double y, double z)
	{
		if(!isInitialized)
		{
			init(x, y, z);
			return;
		}

		if(x < minX) minX = x;
		else if(x > maxX) maxX = x;

		if(y < minY) minY = y;
		else if(y > maxY) maxY= y;

		if(z < minZ) minZ= z;
		else if(z > maxZ) maxZ = z;
	}

	void BoundingBox::getCenterPoint(double& x, double& y, double& z)
	{
		x = (minX + maxX) / 2.0;
		y = (minY + maxY) / 2.0;
		z = (minZ + maxZ) / 2.0;
	}

	double BoundingBox::getMaxLength()
	{
		double xlen = getXLength();
		double ylen = getYLength();
		double zlen = getZLength();
		return ( xlen > ylen ? (xlen > zlen ? xlen : zlen) : (ylen > zlen ? ylen : zlen));
	}

	double BoundingBox::getXLength()
	{
		return maxX - minX;
	}

	double BoundingBox::getYLength()
	{
		return maxY - minY;
	}

	double BoundingBox::getZLength()
	{
		return maxZ - minZ;
	}

	void BoundingBox::init(double x, double y, double z)
	{
		minX = maxX = x;
		minY = maxY = y;
		minZ = maxZ = z;

		isInitialized = true;
	}

	void BoundingBox::divideBbox(int edgeDivisionCount, std::vector<BoundingBox>& result)
	{
		for (int i = 0; i < edgeDivisionCount; i++)
		{
			double minx = (minX * (edgeDivisionCount - i) + maxX * i) / edgeDivisionCount;
			double maxx = (minX * (edgeDivisionCount- i - 1) + maxX * (i + 1)) / edgeDivisionCount;
			for (int j = 0; j < edgeDivisionCount; j++)
			{
				double miny = (minY * (edgeDivisionCount - j) + maxY * j) / edgeDivisionCount;
				double maxy = (minY * (edgeDivisionCount - j - 1) + maxY * (j + 1)) / edgeDivisionCount;
				for (int k = 0; k < edgeDivisionCount; k++)
				{
					double minz = (minZ * (edgeDivisionCount - k) + maxZ * k) / edgeDivisionCount;
					double maxz = (minZ * (edgeDivisionCount - k - 1) + maxZ * (k + 1)) / edgeDivisionCount;

					BoundingBox thisBox;
					thisBox.addPoint(minx, miny, minz);
					thisBox.addPoint(maxx, maxy, maxz);

					result.push_back(thisBox);
				}
			}
		}
	}

	void BoundingBox::expand(double dist)
	{
		if (!isInitialized)
			return;

		minX -= dist;
		minY -= dist;
		minZ -= dist;

		maxX += dist;
		maxY += dist;
		maxZ += dist;
	}
}