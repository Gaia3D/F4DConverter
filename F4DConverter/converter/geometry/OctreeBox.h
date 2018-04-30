#pragma once

#include <vector>
#include <map>

#include "TrianglePolyhedron.h"

namespace gaia3d
{
	class OctreeBox
	{
	public:
		OctreeBox(OctreeBox* owner);

		virtual ~OctreeBox();

	public:
		OctreeBox* parent;
		std::vector<OctreeBox*> children;

		std::vector<TrianglePolyhedron*> meshes;

		unsigned char level;

		double minX, minY, minZ, maxX, maxY, maxZ;

		void setSize(double xMin, double yMin, double zMin, double xMax, double yMax, double zMax)
		{minX = xMin; minY = yMin; minZ = zMin; maxX = xMax; maxY = yMax; maxZ = zMax;}

		void clear();

		void getAllLeafBoxes(std::vector<OctreeBox*>& container, bool bExceptEmptyBox = false);

		void copyDimensionsFromOtherOctreeBox(OctreeBox& input);

		void makeTree(unsigned char depth);

		unsigned char getDepth();

		virtual OctreeBox* makeChild() = 0;
	};

	class VisionOctreeBox : public OctreeBox
	{
	public:
		VisionOctreeBox(OctreeBox* owner);

		virtual ~VisionOctreeBox();

	public:
		void getInternalDivisionPoints(std::vector<Point3D>& container, double scanStepX, double scanStepY, double scanStepZ);

		virtual OctreeBox* makeChild() {return new VisionOctreeBox(this);}
	};

	class SpatialOctreeBox : public OctreeBox
	{
	public:
		SpatialOctreeBox(OctreeBox* owner);

		virtual ~SpatialOctreeBox();

	public:
		size_t octreeId;

		gaia3d::VisionOctreeBox* interiorOcclusionInfo;
		gaia3d::VisionOctreeBox* exteriorOcclusionInfo;

		gaia3d::TrianglePolyhedron* netSurfaceMesh;

		virtual OctreeBox* makeChild() {return new SpatialOctreeBox(this);}

		virtual void makeTreeOfUnfixedDepth(double minSize, bool isObjectInOnlyOneLeaf);

		void setOctreeId(size_t parentId = 0, size_t orderOfChild = 0);

		void distributeMeshesIntoEachChildren(bool isObjectInOnlyOneLeaf, bool propagateToDescendents = true);
	};
}