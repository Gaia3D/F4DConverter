
#include "stdafx.h"

#include "OctreeBox.h"

#include "BoundingBox.h"

#include "../util/utility.h"

namespace gaia3d
{
	OctreeBox::OctreeBox(OctreeBox* owner)
	:parent(owner)
	{
		level = (parent == NULL) ? 0 : parent->level + 1;

		minX = minY = minZ = maxX = maxY = maxZ = 0.0;
	}

	OctreeBox::~OctreeBox()
	{
		size_t childCount = children.size();
		for(size_t i = 0; i < childCount; i++)
			delete children[i];
	}

	void OctreeBox::clear()
	{
		size_t childCount = children.size();
		for(size_t i = 0; i < childCount; i++)
		{
			children[i]->clear();
			delete children[i];
		}
		children.clear();

		meshes.clear();
	}

	void OctreeBox::getAllLeafBoxes(std::vector<OctreeBox*>& container, bool bExceptEmptyBox)
	{
		size_t childCount = children.size();
		if(childCount >0 )
		{
			for(size_t i=0; i< childCount; i++)
			{
				children[i]->getAllLeafBoxes(container, bExceptEmptyBox);
			}
		}
		else
		{
			if(bExceptEmptyBox)
			{
				if(meshes.size() > 0)
					container.push_back(this);
			}
			else
				container.push_back(this);

		}
	}

	void OctreeBox::copyDimensionsFromOtherOctreeBox(OctreeBox& input)
	{
		size_t childCount = children.size();
		for(size_t i = 0; i < childCount; i++)
			delete children[i];

		children.clear();

		setSize(input.minX, input.minY, input.minZ, input.maxX, input.maxY, input.maxZ);

		unsigned char originalDepth = input.getDepth();
		if(originalDepth > 0)
			makeTree(originalDepth);
	}

	void OctreeBox::makeTree(unsigned char depth)
	{
		if(level < depth)
		{
			// 1) Create 8 children octrees
			for(size_t i = 0; i < 8; i++)
			{
				OctreeBox* child = makeChild();
				children.push_back(child);
			}

			// 2) set size of each child octree
			//this->Set_SizesSubBoxes();
			double halfX, halfY, halfZ;
			halfX= (maxX+minX)/2.0;
			halfY= (maxY+minY)/2.0;
			halfZ= (maxZ+minZ)/2.0;
			children[0]->setSize(minX, minY, minZ, halfX, halfY, halfZ);
			children[1]->setSize(halfX, minY, minZ, maxX, halfY, halfZ);
			children[2]->setSize(halfX, halfY, minZ, maxX, maxY, halfZ);
			children[3]->setSize(minX, halfY, minZ, halfX, maxY, halfZ);
			children[4]->setSize(minX, minY, halfZ, halfX, halfY, maxZ);
			children[5]->setSize(halfX, minY, halfZ, maxX, halfY, maxZ);
			children[6]->setSize(halfX, halfY, halfZ, maxX, maxY, maxZ);
			children[7]->setSize(minX, halfY, halfZ, halfX, maxY, maxZ);

			// 3) Make tree for subBoxes.***
			size_t childCount = children.size();
			for(size_t i = 0; i < childCount; i++)
				children[i]->makeTree(depth);
		}
	}

	unsigned char OctreeBox::getDepth()
	{
		if(children.size() > 0)
			return children[0]->getDepth();
		else
			return level;
	}

	VisionOctreeBox::VisionOctreeBox(OctreeBox* owner)
	:OctreeBox(owner)
	{
	}

	VisionOctreeBox::~VisionOctreeBox()
	{
	}

	void VisionOctreeBox::getInternalDivisionPoints(std::vector<Point3D>& container, double scanStepX, double scanStepY, double scanStepZ)
	{
		int div_x = (int)ceil((maxX-minX)/scanStepX);
		int div_y = (int)ceil((maxY-minY)/scanStepY);
		int div_z = (int)ceil((maxZ-minZ)/scanStepZ);

		double x_value, y_value, z_value;

		for(int i=0; i<div_x+1; i++)
		{
			for(int j=0; j<div_y+1; j++)
			{
				for(int k=0; k<div_z+1; k++)
				{
					x_value = minX + i*scanStepX;
					y_value = minY + j*scanStepY;
					z_value = minZ + k*scanStepZ;
					if(x_value > maxX)x_value = maxX;
					if(y_value > maxY)y_value = maxY;
					if(z_value > maxZ)z_value = maxZ;

					Point3D point; 
					point.set(x_value, y_value, z_value);

					container.push_back(point);
				}
			}
		}
	}
	
	SpatialOctreeBox::SpatialOctreeBox(OctreeBox* owner)
	:OctreeBox(owner)
	{
		interiorOcclusionInfo = new VisionOctreeBox(NULL);
		exteriorOcclusionInfo = new VisionOctreeBox(NULL);

		netSurfaceMesh = NULL;
	}

	SpatialOctreeBox::~SpatialOctreeBox()
	{
		if(interiorOcclusionInfo != NULL)
			delete interiorOcclusionInfo;

		if(exteriorOcclusionInfo != NULL)
			delete exteriorOcclusionInfo;

		if(netSurfaceMesh != NULL)
			delete netSurfaceMesh;
	}

	void SpatialOctreeBox::makeTreeOfUnfixedDepth(double minSize, bool isObjectInOnlyOneLeaf)
	{
		double xLength = maxX - minX, yLength = maxY - minY, zLength = maxZ - minZ;
		double maxEdgeLength = (xLength > yLength) ? ( (xLength > zLength) ? xLength : zLength ) : ((yLength > zLength) ? yLength : zLength);
		double tolerance = minSize * 0.4;
		if(maxEdgeLength > minSize + tolerance)
		{
			for(size_t i = 0; i < 8; i++)
			{
				OctreeBox* child = makeChild();
				children.push_back(child);
			}

			// 2) set size of each child octree
			//this->Set_SizesSubBoxes();
			double halfX, halfY, halfZ;
			halfX= (maxX+minX)/2.0;
			halfY= (maxY+minY)/2.0;
			halfZ= (maxZ+minZ)/2.0;
			children[0]->setSize(minX, minY, minZ, halfX, halfY, halfZ);
			children[1]->setSize(halfX, minY, minZ, maxX, halfY, halfZ);
			children[2]->setSize(halfX, halfY, minZ, maxX, maxY, halfZ);
			children[3]->setSize(minX, halfY, minZ, halfX, maxY, halfZ);
			children[4]->setSize(minX, minY, halfZ, halfX, halfY, maxZ);
			children[5]->setSize(halfX, minY, halfZ, maxX, halfY, maxZ);
			children[6]->setSize(halfX, halfY, halfZ, maxX, maxY, maxZ);
			children[7]->setSize(minX, halfY, halfZ, halfX, maxY, maxZ);

			// 3) distribute meshes into each children
			distributeMeshesIntoEachChildren(isObjectInOnlyOneLeaf, false);

			// 4) Make tree for subBoxes.***
			for(size_t i = 0; i < 8; i++)
			{
				if(children[i]->meshes.size() > 0)
					((SpatialOctreeBox*)children[i])->makeTreeOfUnfixedDepth(minSize, isObjectInOnlyOneLeaf);
			}
		}
	}

	void SpatialOctreeBox::setOctreeId(size_t parentId, size_t orderOfChild)
	{
		if(level == 0)
			octreeId = 0;
		else
			octreeId = parentId*10 + (orderOfChild+1);

		size_t childCount = children.size();
		for(size_t i = 0; i < childCount; i++)
		{
			((SpatialOctreeBox*)children[i])->setOctreeId(octreeId, i);
		}
	}

	void SpatialOctreeBox::distributeMeshesIntoEachChildren(bool isObjectInOnlyOneLeaf, bool propagateToDescendents )
	{
		size_t childCount = children.size();
		if(childCount == 0)
			return;

		size_t meshCount = meshes.size();
		size_t vertexCount, surfaceCount, triangleCount;
		double tolerance = 10E-6;
		bool anyTriangleIntersectsWithOctree;
		double v[3][3];
		std::vector<size_t> temp;
		for(size_t i = 0; i < meshCount; i++)
		{
			BoundingBox bbox;
			vertexCount = meshes[i]->getVertices().size();
			for(size_t j = 0; j < vertexCount; j++)
				bbox.addPoint(meshes[i]->getVertices()[j]->position.x, meshes[i]->getVertices()[j]->position.y, meshes[i]->getVertices()[j]->position.z);

			for(size_t j = 0; j < childCount; j++)
			{
				// 1차 테스트 : bbox 교차 테스트
				if(children[j]->maxX + tolerance < bbox.minX || children[j]->minX > bbox.maxX + tolerance ||
					children[j]->maxY + tolerance < bbox.minY || children[j]->minY > bbox.maxY + tolerance ||
					children[j]->maxZ + tolerance < bbox.minZ || children[j]->minZ > bbox.maxZ + tolerance )
					continue;

				// 2차 테스트 : mesh를 이루는 각각의 삼각형으로 교차 테스트
				anyTriangleIntersectsWithOctree = false;
				surfaceCount = meshes[i]->getSurfaces().size();
				for(size_t k = 0; k < surfaceCount; k++)
				{
					triangleCount = meshes[i]->getSurfaces()[k]->getTriangles().size();
					for(size_t l = 0; l < triangleCount; l++)
					{
						// 삼각형에서 꼭지점 3개 위치 정보 추출
						for(size_t m = 0; m < 3; m++)
						{
							v[m][0] = meshes[i]->getSurfaces()[k]->getTriangles()[l]->getVertices()[m]->position.x;
							v[m][1] = meshes[i]->getSurfaces()[k]->getTriangles()[l]->getVertices()[m]->position.y;
							v[m][2] = meshes[i]->getSurfaces()[k]->getTriangles()[l]->getVertices()[m]->position.z;
						}

						// 삼각형과 자식 octree가 교차하는지 테스트
						if( GeometryUtility::doesTriangleIntersectWithBox(v[0][0], v[0][1], v[0][2],
																		v[1][0], v[1][1], v[1][2],
																		v[2][0], v[2][1], v[2][2],
																		children[j]->minX, children[j]->minY, children[j]->minZ,
																		children[j]->maxX, children[j]->maxY, children[j]->maxZ) )
						{
							anyTriangleIntersectsWithOctree = true;
							break;
						}
					}
					if(anyTriangleIntersectsWithOctree)
						break;
				}

				if(!anyTriangleIntersectsWithOctree)
					continue;

				children[j]->meshes.push_back(meshes[i]);

				if(isObjectInOnlyOneLeaf)
				{
					meshes.erase(meshes.begin() + i);
					meshCount--;
					i--;
				}
			}
		}

		if(!isObjectInOnlyOneLeaf)
			meshes.clear();

		if(propagateToDescendents)
		{
			for(size_t i = 0; i < childCount; i++)
			{
				if(children[i]->meshes.size() == 0)
					continue;

				((SpatialOctreeBox*)children[i])->distributeMeshesIntoEachChildren(isObjectInOnlyOneLeaf);
			}
		}
	}
}