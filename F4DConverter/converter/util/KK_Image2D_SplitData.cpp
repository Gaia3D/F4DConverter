
#include "stdafx.h"

#include "KK_Image2D_SplitData.h"
#include "KK_Rectangle.h"
#include "../geometry/Triangle.h"
#include "../geometry/Vertex.h"

CKK_Image2D_SplitData::CKK_Image2D_SplitData()
{
	m_rectangleImage_original = 0;
	m_rectangleImage_splitted = 0;
}


CKK_Image2D_SplitData::~CKK_Image2D_SplitData()
{
	if(m_rectangleImage_original)
		delete m_rectangleImage_original;

	if(m_rectangleImage_splitted)
		delete m_rectangleImage_splitted;
}

void CKK_Image2D_SplitData::Get_Vertices(std::vector<gaia3d::Vertex*> &vec_vertices)
{
	// this function returns no repeated vertices of the triangles.***
	std::map<gaia3d::Vertex*, int> map_vertices;
	size_t trianglesCount = this->m_vec_triangles.size();
	for (size_t i = 0; i < trianglesCount; i++)
	{
		gaia3d::Triangle *tri = this->m_vec_triangles[i];
		map_vertices[tri->getVertices()[0]] = 1;
		map_vertices[tri->getVertices()[1]] = 1;
		map_vertices[tri->getVertices()[2]] = 1;
	}

	std::map<gaia3d::Vertex*, int>::iterator it;
	for (it = map_vertices.begin(); it != map_vertices.end(); it++)
	{
		vec_vertices.push_back(it->first);
	}
}

bool CKK_Image2D_SplitData::TEST__AreTexCoordsInside_rectangleOriginal()
{
	bool allTrianglesTexCoordsAreInsideOfOriginalRectangle = true;

	double error = 10E-12;
	size_t trianglesCount = this->m_vec_triangles.size();
	for (size_t i = 0; i < trianglesCount; i++)
	{
		gaia3d::Triangle *tri = this->m_vec_triangles[i];
		double *texCoord_0 = tri->getVertices()[0]->textureCoordinate;
		double *texCoord_1 = tri->getVertices()[1]->textureCoordinate;
		double *texCoord_2 = tri->getVertices()[2]->textureCoordinate;

		if (!this->m_rectangleImage_original->Intersection_withPoint(texCoord_0[0], texCoord_0[1], error))
			return false;

		if (!this->m_rectangleImage_original->Intersection_withPoint(texCoord_1[0], texCoord_1[1], error))
			return false;

		if (!this->m_rectangleImage_original->Intersection_withPoint(texCoord_2[0], texCoord_2[1], error))
			return false;
	}

	return allTrianglesTexCoordsAreInsideOfOriginalRectangle;
}
