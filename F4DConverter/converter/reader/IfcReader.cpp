#include "stdafx.h"

#ifdef IFCFORMAT

#ifdef _WIN32
#include <Windows.h>
#endif

#include "IfcReader.h"

#include "IfcLoader.h"

#include <proj_api.h>

#include "../geometry/TrianglePolyhedron.h"
#include "../geometry/ColorU4.h"
#include "../util/utility.h"

IfcReader::IfcReader()
{
}


IfcReader::~IfcReader()
{
}

bool IfcReader::readRawDataFile(std::string& filePath)
{
	// before processing more, check if the georeferencing information is valid
	projPJ pjSrc = NULL, pjWgs84 = NULL;
	if (bCoordinateInfoInjected)
	{
		std::string originalSrsProjString = makeProj4String();
		std::string wgs84ProjString("+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs");

		pjSrc = pj_init_plus(originalSrsProjString.c_str());
		pjWgs84 = pj_init_plus(wgs84ProjString.c_str());
		if (pjSrc == NULL || pjWgs84 == NULL)
		{
			printf("[ERROR][proj4]CANNOT initialize SRS\n");
			return false;
		}
	}

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
		if (bYAxisUp)
		{
			for (size_t j = 0; j < vertexCount; j++)
			{
				vertex = new gaia3d::Vertex;
				vertex->position.set(vertexPositions[3 * j] * unitScaleFactor + offsetX, (-vertexPositions[3 * j + 2]) * unitScaleFactor + offsetY, vertexPositions[3 * j + 1] * unitScaleFactor + offsetZ);
				polyhedron->getVertices().push_back(vertex);
			}
		}
		else
		{
			for (size_t j = 0; j < vertexCount; j++)
			{
				vertex = new gaia3d::Vertex;
				vertex->position.set(vertexPositions[3 * j] * unitScaleFactor + offsetX, vertexPositions[3 * j + 1] * unitScaleFactor + offsetY, vertexPositions[3 * j + 2] * unitScaleFactor + offsetZ);
				polyhedron->getVertices().push_back(vertex);
			}
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

	// transform coordinates if information for georeferencing is injected.
	if (bCoordinateInfoInjected)
	{
		gaia3d::BoundingBox bbox;
		size_t meshCount = container.size();
		for (size_t i = 0; i < meshCount; i++)
		{
			std::vector<gaia3d::Vertex*>& vertices = container[i]->getVertices();
			size_t vertexCount = vertices.size();
			for (size_t j = 0; j < vertexCount; j++)
				bbox.addPoint(vertices[j]->position.x, vertices[j]->position.y, vertices[j]->position.z);
		}

		double cx, cy, cz;
		bbox.getCenterPoint(cx, cy, cz);

		refLon = cx; refLat = cy;
		double alt = cz;
		int errorCode = pj_transform(pjSrc, pjWgs84, 1, 1, &refLon, &refLat, &alt);
		char* errorMessage = pj_strerrno(errorCode);
		if (errorMessage != NULL)
		{
			printf("[ERROR][proj4]%s\n", errorMessage);
			size_t meshCount = container.size();
			for (size_t i = 0; i < meshCount; i++)
				delete container[i];
			container.clear();
			return false;
		}

		refLon *= RAD_TO_DEG;
		refLat *= RAD_TO_DEG;

		bHasGeoReferencingInfo = true;

		double absPosOfCenterXY[3];
		alt = 0.0;
		gaia3d::GeometryUtility::wgs84ToAbsolutePosition(refLon, refLat, alt, absPosOfCenterXY);
		double m[16];
		gaia3d::GeometryUtility::transformMatrixAtAbsolutePosition(absPosOfCenterXY[0], absPosOfCenterXY[1], absPosOfCenterXY[2], m);
		gaia3d::Matrix4 globalTransformMatrix;
		globalTransformMatrix.set(m[0], m[4], m[8], m[12],
			m[1], m[5], m[9], m[13],
			m[2], m[6], m[10], m[14],
			m[3], m[7], m[11], m[15]);
		gaia3d::Matrix4 inverseGlobalTransMatrix = globalTransformMatrix.inverse();

		double px, py, pz;
		for (size_t i = 0; i < meshCount; i++)
		{
			std::vector<gaia3d::Vertex*>& vertices = container[i]->getVertices();
			size_t vertexCount = vertices.size();
			for (size_t j = 0; j < vertexCount; j++)
			{
				vertex = vertices[j];
				px = vertex->position.x;
				py = vertex->position.y;
				pz = vertex->position.z;

				pj_transform(pjSrc, pjWgs84, 1, 1, &px, &py, &pz);
				px *= RAD_TO_DEG;
				py *= RAD_TO_DEG;

				double absPosOfTargetPointArray[3];
				gaia3d::GeometryUtility::wgs84ToAbsolutePosition(px, py, pz, absPosOfTargetPointArray);
				gaia3d::Point3D absPosOfTargetPoint;
				absPosOfTargetPoint.set(absPosOfTargetPointArray[0], absPosOfTargetPointArray[1], absPosOfTargetPointArray[2]);
				vertex->position = inverseGlobalTransMatrix * absPosOfTargetPoint;
			}
		}
	}
	
	return true;
}

void IfcReader::clear()
{
	container.clear();

	textureContainer.clear();
}

#endif