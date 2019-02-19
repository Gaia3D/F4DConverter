#include "stdafx.h"

#ifdef IFCFORMAT

#ifdef _WIN32
#include <Windows.h>
#endif

#include "IfcReader.h"

#include "IfcLoader.h"

#include "../geometry/TrianglePolyhedron.h"
#include "../geometry/ColorU4.h"
#include "../util/utility.h"

IfcReader::IfcReader()
{
	unitScaleFactor = 1.0;

	bHasGeoReferencingInfo = false;

	bCoordinateInfoInjected = false;
}


IfcReader::~IfcReader()
{
}

bool IfcReader::readRawDataFile(std::string& filePath)
{
	aIfcLoader* loader = createIfcLoader();
	loader->setVertexReductionMode(false);

	std::wstring wFilePath = gaia3d::StringUtility::convertUtf8ToWideString(filePath);

	if (!loader->loadIfcFile(wFilePath))
	{
		destroyIfcLoader(loader);
		return false;
	}

	size_t polyhedronCount = loader->getPolyhedronCount();
	double* vertexPositions;
	float* color4;
	size_t* indices;
	gaia3d::TrianglePolyhedron* polyhedron;
	gaia3d::Vertex* vertex;
	gaia3d::Surface* surface;
	gaia3d::Triangle* triangle;
	size_t vertexCount, surfaceCount, triangleCount;
	for (size_t i = 0; i < polyhedronCount; i++)
	{
		polyhedron = new gaia3d::TrianglePolyhedron;
		polyhedron->setColorMode(gaia3d::SingleColor);
		color4 = loader->getRepresentativeColor(i);
		if(color4[3] == 0.0f)
			polyhedron->setSingleColor(DefaultColor);
		else
			polyhedron->setSingleColor(MakeColorU4((unsigned char)(color4[0] * 255), (unsigned char)(color4[1] * 255), (unsigned char)(color4[2] * 255)));

		polyhedron->setHasNormals(false);

		vertexCount = loader->getVertexCount(i);
		vertexPositions = loader->getVertexPositions(i);
		for (size_t j = 0; j < vertexCount; j++)
		{
			vertex = new gaia3d::Vertex;
			vertex->position.set(vertexPositions[3*j] * unitScaleFactor, vertexPositions[3 * j + 1] * unitScaleFactor, vertexPositions[3 * j + 2] * unitScaleFactor);
			polyhedron->getVertices().push_back(vertex);
		}

		surfaceCount = loader->getSurfaceCount(i);
		for (size_t j = 0; j < surfaceCount; j++)
		{
			surface = new gaia3d::Surface;

			triangleCount = loader->getTrialgleCount(i, j);
			indices = loader->getTriangleIndices(i, j);
			for (size_t k = 0; k < triangleCount; k++)
			{
				triangle = new gaia3d::Triangle;
				triangle->getVertexIndices()[0] = indices[3 * k];
				triangle->getVertexIndices()[1] = indices[3 * k + 1];
				triangle->getVertexIndices()[2] = indices[3 * k + 2];
				triangle->getVertices()[0] = polyhedron->getVertices()[indices[3 * k]];
				triangle->getVertices()[1] = polyhedron->getVertices()[indices[3 * k + 1]];
				triangle->getVertices()[2] = polyhedron->getVertices()[indices[3 * k + 2]];

				surface->getTriangles().push_back(triangle);
			}

			polyhedron->getSurfaces().push_back(surface);
		}

		polyhedron->setId(container.size());

		std::wstring wObjectId = loader->getGuid(i);
		std::string objectId = gaia3d::StringUtility::convertWideStringToUtf8(wObjectId);

		polyhedron->addStringAttribute(std::string(ObjectGuid), objectId);

		container.push_back(polyhedron);
	}

	destroyIfcLoader(loader);
	
	return true;
}

void IfcReader::clear()
{
	container.clear();

	textureContainer.clear();
}

#endif