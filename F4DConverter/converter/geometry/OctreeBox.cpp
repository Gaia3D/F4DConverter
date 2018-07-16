
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

	void SpatialOctreeBox::makeTreeOfUnfixedDepth(double minSize, bool isObjectInOnlyOneLeaf, bool bSplitMesh)
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
			if (bSplitMesh)
				splitMeshIntoEachChildren();
			else
				distributeMeshesIntoEachChildren(isObjectInOnlyOneLeaf, false);

			// 4) Make tree for subBoxes.***
			for(size_t i = 0; i < 8; i++)
			{
				if(children[i]->meshes.size() > 0)
					((SpatialOctreeBox*)children[i])->makeTreeOfUnfixedDepth(minSize, isObjectInOnlyOneLeaf, bSplitMesh);
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

	void SpatialOctreeBox::splitMeshIntoEachChildren()
	{
		size_t childCount = children.size();
		if (childCount == 0)
			return;

		double tolerance = 10E-6;
		bool anyTriangleIntersectsWithOctree;
		double v[3][3];
		size_t surfaceCount, triangleCount;
		size_t meshCount = meshes.size();
		for (size_t i = 0; i < meshCount; i++)
		{
			gaia3d::TrianglePolyhedron* mesh = meshes[i];

			for (size_t j = 0; j < childCount; j++)
			{
				BoundingBox& bbox = mesh->getBoundingBox();

				// 1st test : bbox intersection test
				if (children[j]->maxX + tolerance < bbox.minX || children[j]->minX > bbox.maxX + tolerance ||
					children[j]->maxY + tolerance < bbox.minY || children[j]->minY > bbox.maxY + tolerance ||
					children[j]->maxZ + tolerance < bbox.minZ || children[j]->minZ > bbox.maxZ + tolerance)
				{
					continue;
				}

				// TESTCODE-start
				if (j != 0)
				{
					delete mesh;
					break;
				}
				// TESTCODE-end

				// 2nd test : intersection test for all triangles in a mesh
				anyTriangleIntersectsWithOctree = false;
				surfaceCount = mesh->getSurfaces().size();
				for (size_t k = 0; k < surfaceCount; k++)
				{
					triangleCount = mesh->getSurfaces()[k]->getTriangles().size();
					for (size_t l = 0; l < triangleCount; l++)
					{
						// extract positions of 3 vertices of a triangle
						for (size_t m = 0; m < 3; m++)
						{
							v[m][0] = mesh->getSurfaces()[k]->getTriangles()[l]->getVertices()[m]->position.x;
							v[m][1] = mesh->getSurfaces()[k]->getTriangles()[l]->getVertices()[m]->position.y;
							v[m][2] = mesh->getSurfaces()[k]->getTriangles()[l]->getVertices()[m]->position.z;
						}

						// intersection test between this triangle and this child octree box
						if (GeometryUtility::doesTriangleIntersectWithBox(v[0][0], v[0][1], v[0][2],
							v[1][0], v[1][1], v[1][2],
							v[2][0], v[2][1], v[2][2],
							children[j]->minX, children[j]->minY, children[j]->minZ,
							children[j]->maxX, children[j]->maxY, children[j]->maxZ))
						{
							anyTriangleIntersectsWithOctree = true;
							break;
						}
					}
					if (anyTriangleIntersectsWithOctree)
						break;
				}

				if (!anyTriangleIntersectsWithOctree)
					continue;

				// clip intersected part and assign it to this children
				gaia3d::TrianglePolyhedron* intersectedPart = NULL;
				gaia3d::TrianglePolyhedron* nonIntersectedPart = NULL;
				clipIntersectedPartWithBox(mesh,
					children[j]->minX, children[j]->minY, children[j]->minZ,
					children[j]->maxX, children[j]->maxY, children[j]->maxZ,
					&intersectedPart, &nonIntersectedPart);

				if(intersectedPart != NULL)
				{
					calculateBoundingBox(intersectedPart);
					children[j]->meshes.push_back(intersectedPart);
				}

				delete mesh;
				if (nonIntersectedPart == NULL)
				{
					break;
				}

				if (j != childCount - 1)
				{
					calculateBoundingBox(nonIntersectedPart);

					mesh = nonIntersectedPart;
				}
				else
				{
					delete nonIntersectedPart;
				}
			}
		}

		meshes.clear();
	}

	void SpatialOctreeBox::clipIntersectedPartWithBox(gaia3d::TrianglePolyhedron* mesh,
													double minx, double miny, double minz, double maxx, double maxy, double maxz,
													gaia3d::TrianglePolyhedron** intersected,
													gaia3d::TrianglePolyhedron** nonIntersected)
	{
		std::vector<gaia3d::Triangle*> intersectedTriangles;
		std::vector<gaia3d::Triangle*> nonIntersectedTriangles;

		size_t surfaceCount, triangleCount;
		surfaceCount = mesh->getSurfaces().size();
		double v[3][3];
		for (size_t i = 0; i < surfaceCount; i++)
		{
			gaia3d::Surface* surface = mesh->getSurfaces()[i];
			triangleCount = surface->getTriangles().size();
			for (size_t j = 0; j < triangleCount; j++)
			{
				gaia3d::Triangle* triangle = surface->getTriangles()[j];

				for (size_t m = 0; m < 3; m++)
				{
					v[m][0] = triangle->getVertices()[m]->position.x;
					v[m][1] = triangle->getVertices()[m]->position.y;
					v[m][2] = triangle->getVertices()[m]->position.z;
				}

				// intersection test between this triangle and this child octree box
				if (GeometryUtility::doesTriangleIntersectWithBox(v[0][0], v[0][1], v[0][2],
					v[1][0], v[1][1], v[1][2],
					v[2][0], v[2][1], v[2][2],
					minx, miny, minz,
					maxx, maxy, maxz))
				{
					intersectedTriangles.push_back(triangle);
				}
				else
				{
					nonIntersectedTriangles.push_back(triangle);
				}
			}
		}

		if (!intersectedTriangles.empty())
		{
			*intersected = new gaia3d::TrianglePolyhedron;
			gaia3d::Surface* surface = new gaia3d::Surface;
			(*intersected)->getSurfaces().push_back(surface);
			triangleCount = intersectedTriangles.size();
			for (size_t i = 0; i < triangleCount; i++)
			{
				gaia3d::Triangle* triangle = intersectedTriangles[i];

				gaia3d::Triangle* newTriangle = new gaia3d::Triangle;
				surface->getTriangles().push_back(newTriangle);

				gaia3d::Vertex* vertex0 = triangle->getVertices()[0];
				gaia3d::Vertex* newVertex0 = new gaia3d::Vertex;
				newVertex0->color = vertex0->color;
				newVertex0->normal = vertex0->normal;
				newVertex0->position = vertex0->position;
				newVertex0->textureCoordinate[0] = vertex0->textureCoordinate[0];
				newVertex0->textureCoordinate[1] = vertex0->textureCoordinate[1];

				gaia3d::Vertex* vertex1 = triangle->getVertices()[1];
				gaia3d::Vertex* newVertex1 = new gaia3d::Vertex;
				newVertex1->color = vertex1->color;
				newVertex1->normal = vertex1->normal;
				newVertex1->position = vertex1->position;
				newVertex1->textureCoordinate[0] = vertex1->textureCoordinate[0];
				newVertex1->textureCoordinate[1] = vertex1->textureCoordinate[1];

				gaia3d::Vertex* vertex2 = triangle->getVertices()[2];
				gaia3d::Vertex* newVertex2 = new gaia3d::Vertex;
				newVertex2->color = vertex2->color;
				newVertex2->normal = vertex2->normal;
				newVertex2->position = vertex2->position;
				newVertex2->textureCoordinate[0] = vertex2->textureCoordinate[0];
				newVertex2->textureCoordinate[1] = vertex2->textureCoordinate[1];

				newTriangle->setVertices(newVertex0, newVertex1, newVertex2);
				size_t index = (*intersected)->getVertices().size();
				(*intersected)->getVertices().push_back(newVertex0);
				(*intersected)->getVertices().push_back(newVertex1);
				(*intersected)->getVertices().push_back(newVertex2);

				newTriangle->setVertexIndices(index, index+1, index+2);
			}

			(*intersected)->setHasNormals(mesh->doesThisHaveNormals());
			(*intersected)->setHasTextureCoordinates(mesh->doesThisHaveTextureCoordinates());
			(*intersected)->setColorMode(mesh->getColorMode());
			(*intersected)->setSingleColor(mesh->getSingleColor());
			(*intersected)->getStringAttributes().insert(mesh->getStringAttributes().begin(), mesh->getStringAttributes().end());
		}

		if (!nonIntersectedTriangles.empty())
		{
			*nonIntersected = new gaia3d::TrianglePolyhedron;
			gaia3d::Surface* surface = new gaia3d::Surface;
			(*nonIntersected)->getSurfaces().push_back(surface);
			triangleCount = nonIntersectedTriangles.size();
			for (size_t i = 0; i < triangleCount; i++)
			{
				gaia3d::Triangle* triangle = nonIntersectedTriangles[i];

				gaia3d::Triangle* newTriangle = new gaia3d::Triangle;
				surface->getTriangles().push_back(newTriangle);

				gaia3d::Vertex* vertex0 = triangle->getVertices()[0];
				gaia3d::Vertex* newVertex0 = new gaia3d::Vertex;
				newVertex0->color = vertex0->color;
				newVertex0->normal = vertex0->normal;
				newVertex0->position = vertex0->position;
				newVertex0->textureCoordinate[0] = vertex0->textureCoordinate[0];
				newVertex0->textureCoordinate[1] = vertex0->textureCoordinate[1];

				gaia3d::Vertex* vertex1 = triangle->getVertices()[1];
				gaia3d::Vertex* newVertex1 = new gaia3d::Vertex;
				newVertex1->color = vertex1->color;
				newVertex1->normal = vertex1->normal;
				newVertex1->position = vertex1->position;
				newVertex1->textureCoordinate[0] = vertex1->textureCoordinate[0];
				newVertex1->textureCoordinate[1] = vertex1->textureCoordinate[1];

				gaia3d::Vertex* vertex2 = triangle->getVertices()[2];
				gaia3d::Vertex* newVertex2 = new gaia3d::Vertex;
				newVertex2->color = vertex2->color;
				newVertex2->normal = vertex2->normal;
				newVertex2->position = vertex2->position;
				newVertex2->textureCoordinate[0] = vertex2->textureCoordinate[0];
				newVertex2->textureCoordinate[1] = vertex2->textureCoordinate[1];

				newTriangle->setVertices(newVertex0, newVertex1, newVertex2);
				size_t index = (*nonIntersected)->getVertices().size();
				(*nonIntersected)->getVertices().push_back(newVertex0);
				(*nonIntersected)->getVertices().push_back(newVertex1);
				(*nonIntersected)->getVertices().push_back(newVertex2);

				newTriangle->setVertexIndices(index, index+1, index+2);
			}

			(*nonIntersected)->setHasNormals(mesh->doesThisHaveNormals());
			(*nonIntersected)->setHasTextureCoordinates(mesh->doesThisHaveTextureCoordinates());
			(*nonIntersected)->setColorMode(mesh->getColorMode());
			(*nonIntersected)->setSingleColor(mesh->getSingleColor());
			(*nonIntersected)->getStringAttributes().insert(mesh->getStringAttributes().begin(), mesh->getStringAttributes().end());
		}
	}

	void SpatialOctreeBox::calculateBoundingBox(gaia3d::TrianglePolyhedron* mesh)
	{
		if (mesh->getBoundingBox().isInitialized)
			return;

		std::vector<gaia3d::Vertex*>& vertices = mesh->getVertices();
		size_t vertexCount = vertices.size();
		for (size_t j = 0; j < vertexCount; j++)
		{

			mesh->getBoundingBox().addPoint(vertices[j]->position.x, vertices[j]->position.y, vertices[j]->position.z);
		}
	}

	PointDistributionOctree::PointDistributionOctree(PointDistributionOctree* owner)
		:parent(owner)
	{
		level = (parent == NULL) ? 0 : parent->level + 1;

		minX = minY = minZ = maxX = maxY = maxZ = 0.0;
	}

	PointDistributionOctree::~PointDistributionOctree()
	{
		clear();
	}

	void PointDistributionOctree::makeTreeOfUnfixedDepth(double minSize, bool isObjectInOnlyOneLeaf)
	{
		double xLength = maxX - minX, yLength = maxY - minY, zLength = maxZ - minZ;
		double maxEdgeLength = (xLength > yLength) ? ((xLength > zLength) ? xLength : zLength) : ((yLength > zLength) ? yLength : zLength);
		double tolerance = minSize * 0.4;
		if (maxEdgeLength > minSize + tolerance)
		{
			for (size_t i = 0; i < 8; i++)
			{
				PointDistributionOctree* child = new PointDistributionOctree(this);
				children.push_back(child);
			}

			// 2) set size of each child octree
			//this->Set_SizesSubBoxes();
			double halfX, halfY, halfZ;
			halfX = (maxX + minX) / 2.0;
			halfY = (maxY + minY) / 2.0;
			halfZ = (maxZ + minZ) / 2.0;
			children[0]->setSize(minX, minY, minZ, halfX, halfY, halfZ);
			children[1]->setSize(halfX, minY, minZ, maxX, halfY, halfZ);
			children[2]->setSize(halfX, halfY, minZ, maxX, maxY, halfZ);
			children[3]->setSize(minX, halfY, minZ, halfX, maxY, halfZ);
			children[4]->setSize(minX, minY, halfZ, halfX, halfY, maxZ);
			children[5]->setSize(halfX, minY, halfZ, maxX, halfY, maxZ);
			children[6]->setSize(halfX, halfY, halfZ, maxX, maxY, maxZ);
			children[7]->setSize(minX, halfY, halfZ, halfX, maxY, maxZ);

			// 3) distribute meshes into each children
			distributeVerticesIntoEachChildren(isObjectInOnlyOneLeaf);

			// 4) Make tree for subBoxes.***
			for (size_t i = 0; i < 8; i++)
			{
				if (children[i]->vertices.size() > 0)
					children[i]->makeTreeOfUnfixedDepth(minSize, isObjectInOnlyOneLeaf);
			}
		}
	}

	void PointDistributionOctree::distributeVerticesIntoEachChildren(bool isObjectInOnlyOneLeaf)
	{
		size_t childCount = children.size();
		if (childCount == 0)
			return;

		size_t vertexCount = vertices.size();
		for (size_t i = 0; i < vertexCount; i++)
		{
			gaia3d::Vertex* vertex = vertices[i];

			for (size_t j = 0; j < childCount; j++)
			{
				if (children[j]->maxX < vertex->position.x || children[j]->minX > vertex->position.x ||
					children[j]->maxY < vertex->position.y || children[j]->minY > vertex->position.y ||
					children[j]->maxZ < vertex->position.z || children[j]->minZ > vertex->position.z)
					continue;

				children[j]->vertices.push_back(vertex);

				break;
			}
		}

		if (isObjectInOnlyOneLeaf)
			vertices.clear();
	}

	void PointDistributionOctree::setOctreeId(size_t parentId, size_t orderOfChild)
	{
		if (level == 0)
			octreeId = 0;
		else
			octreeId = parentId * 10 + (orderOfChild + 1);

		size_t childCount = children.size();
		for (size_t i = 0; i < childCount; i++)
		{
			children[i]->setOctreeId(octreeId, i);
		}
	}

	void PointDistributionOctree::getAllLeafBoxes(std::vector<PointDistributionOctree*>& container)
	{
		size_t childCount = children.size();
		if (childCount >0)
		{
			for (size_t i = 0; i< childCount; i++)
			{
				children[i]->getAllLeafBoxes(container);
			}
		}
		else
		{
			if (vertices.size() > 0)
				container.push_back(this);
		}
	}

	PointDistributionOctree* PointDistributionOctree::getIntersectedLeafOctree(Vertex* vertex)
	{
		if (maxX < vertex->position.x || minX > vertex->position.x ||
			maxY < vertex->position.y || minY > vertex->position.y ||
			maxZ < vertex->position.z || minZ > vertex->position.z)
			return NULL;

		if (children.empty())
			return this;

		size_t childCount = children.size();
		for (size_t i = 0; i < childCount; i++)
		{
			PointDistributionOctree* child = children[i]->getIntersectedLeafOctree(vertex);
			if (child != NULL)
				return child;
		}

		return NULL;
	}

	void PointDistributionOctree::clear()
	{
		size_t childCount = children.size();
		for (size_t i = 0; i < childCount; i++)
		{
			children[i]->clear();
			delete children[i];
		}
		children.clear();

		vertices.clear();
	}
}