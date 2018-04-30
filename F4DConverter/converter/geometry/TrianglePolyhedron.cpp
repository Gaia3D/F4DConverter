

#include "stdafx.h"

#include "TrianglePolyhedron.h"
#include "../predefinition.h"

namespace gaia3d
{

	TrianglePolyhedron::TrianglePolyhedron()
	{
		refInfo.model = NULL;
		refInfo.mat.identity();
		refInfo.modelIndex = MaxUnsignedLong;

		hasNormals = hasTextureCoordinates = false;

		colorMode = NoColor;
		singleColor = 0UL;

		id = MaxUnsignedLong;
	}


	TrianglePolyhedron::~TrianglePolyhedron()
	{
		size_t vertexCount = vertices.size();
		for(size_t i = 0; i < vertexCount; i++)
			delete vertices[i];
		vertices.clear();

		size_t surfaceCount = surfaces.size();
		for(size_t i = 0; i < surfaceCount; i++)
			delete surfaces[i];
		surfaces.clear();

		size_t vboCount = vbos.size();
		for(size_t i = 0; i < vboCount; i++)
			delete vbos[i];
		vbos.clear();
	}

	void TrianglePolyhedron::addStringAttribute(std::string keyString, std::string valueString)
	{
		stringAttributes.insert(std::map<std::string, std::string>::value_type(keyString, valueString));
	}

	bool TrianglePolyhedron::doesStringAttributeExist(std::string keyString)
	{
		if (stringAttributes.find(keyString) == stringAttributes.end())
			return false;
		
		return true;
	}

	std::string TrianglePolyhedron::getStringAttribute(std::string keyString)
	{
		if (!doesStringAttributeExist(keyString))
			return std::string();

		return stringAttributes[keyString];
	}

	bool TrianglePolyhedron::doesHaveAnyExteriorSurface()
	{
		size_t surfaceCount = surfaces.size();
		for(size_t i = 0; i < surfaceCount; i++)
		{
			if(surfaces[i]->isExterior())
				return true;
		}

		return false;
	}

}