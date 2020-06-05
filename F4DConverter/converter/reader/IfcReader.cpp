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

#include "../LogWriter.h"

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
			// new log
			LogWriter::getLogWriter()->changeCurrentConversionJobStatus(LogWriter::failure);
			LogWriter::getLogWriter()->addDescriptionToCurrentConversionJobLog(std::string(" IfcReader::readRawDataFile : failed to initialize proj"));
			return false;
		}
	}

	aIfcLoader* loader = createIfcLoader();
	loader->setVertexReductionMode(false);

	std::wstring wFilePath = gaia3d::StringUtility::convertUtf8ToWideString(filePath);

	if (!loader->loadIfcFile(wFilePath))
	{
		destroyIfcLoader(loader);

		// new log
		LogWriter::getLogWriter()->changeCurrentConversionJobStatus(LogWriter::failure);
		LogWriter::getLogWriter()->addDescriptionToCurrentConversionJobLog(std::string(" IfcReader::readRawDataFile : loading failure"));
		return false;
	}

	if (splitFilter.empty())
	{
		size_t polyhedronCount = loader->getPolyhedronCount();
		double* vertexPositions;
		float* color4;
		size_t* indices;
		gaia3d::TrianglePolyhedron* polyhedron;
		gaia3d::Vertex* vertex;
		gaia3d::Surface* surface;
		gaia3d::Triangle* triangle;
		size_t vertexCount, surfaceCount, triangleCount;
		wchar_t guidString[1024];
		for (size_t i = 0; i < polyhedronCount; i++)
		{
			polyhedron = new gaia3d::TrianglePolyhedron;
			polyhedron->setColorMode(gaia3d::SingleColor);
			color4 = loader->getRepresentativeColor(i);
			if (color4[3] == 0.0f)
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

			memset(guidString, 0x00, sizeof(wchar_t) * 1024);
			loader->getGuid(i, guidString);
			std::wstring wObjectId(guidString);
			std::string objectId = gaia3d::StringUtility::convertWideStringToUtf8(wObjectId);

			polyhedron->addStringAttribute(std::string(ObjectGuid), objectId);
#ifdef ONLYFORHYNIX
			if (filePath.find(std::string("CJ_M15M01ALL_PP_XX_Chemical")) != std::string::npos)
			{
				polyhedron->setColorMode(gaia3d::ColorMode::SingleColor);
				polyhedron->setSingleColor(MakeColorU4(255, 0, 211));
			}
			if (filePath.find(std::string("CJ_M15M01ALL_PP_XX_Gas_1")) != std::string::npos)
			{
				polyhedron->setColorMode(gaia3d::ColorMode::SingleColor);
				polyhedron->setSingleColor(MakeColorU4(255, 0, 0));
			}
			if (filePath.find(std::string("CJ_M15M01ALL_PP_XX_Gas_2")) != std::string::npos)
			{
				polyhedron->setColorMode(gaia3d::ColorMode::SingleColor);
				polyhedron->setSingleColor(MakeColorU4(255, 0, 0));
			}
			if (filePath.find(std::string("CJ_M15M01ALL_PP_XX_PGS")) != std::string::npos)
			{
				polyhedron->setColorMode(gaia3d::ColorMode::SingleColor);
				polyhedron->setSingleColor(MakeColorU4(171, 242, 0));
			}
			if (filePath.find(std::string("CJ_M15M01ALL_PP_XX_Water_1")) != std::string::npos)
			{
				polyhedron->setColorMode(gaia3d::ColorMode::SingleColor);
				polyhedron->setSingleColor(MakeColorU4(0, 216, 255));
			}
			if (filePath.find(std::string("CJ_M15M01ALL_PP_XX_Water_2")) != std::string::npos)
			{
				polyhedron->setColorMode(gaia3d::ColorMode::SingleColor);
				polyhedron->setSingleColor(MakeColorU4(0, 216, 255));
			}
			if (filePath.find(std::string("CJ_M15M01ALL_PP_XX_WF")) != std::string::npos)
			{
				polyhedron->setColorMode(gaia3d::ColorMode::SingleColor);
				polyhedron->setSingleColor(MakeColorU4(0, 84, 255));
			}
			if (filePath.find(std::string("CJ_M15M01ALL_PP_XX_WW")) != std::string::npos)
			{
				polyhedron->setColorMode(gaia3d::ColorMode::SingleColor);
				polyhedron->setSingleColor(MakeColorU4(140, 140, 140));
			}
#endif

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

				// new log
				LogWriter::getLogWriter()->changeCurrentConversionJobStatus(LogWriter::failure);
				LogWriter::getLogWriter()->addDescriptionToCurrentConversionJobLog(std::string(" IfcReader::readRawDataFile : boungding box center coordinate transform failure"));
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
	}
	else
	{
		std::string fileName;
		size_t finalSlashIndex;
		if (filePath.rfind('\\') != std::string::npos)
		{
			finalSlashIndex = filePath.rfind('\\');
			if (filePath.rfind('/') != std::string::npos && filePath.rfind('/') > finalSlashIndex)
				finalSlashIndex = filePath.rfind('/');
		}
		else
			finalSlashIndex = filePath.rfind('/');

		fileName = filePath.substr(finalSlashIndex + 1, filePath.rfind('.') - finalSlashIndex - 1);

		size_t storyCount = loader->getStoryCount();
		printf("[INFO]total story count : %zd\n", storyCount);
		std::string dataKey;
		for (size_t i = 0; i < storyCount; i++)
		{
			size_t divisionCount = loader->getStoryDivisionCount(i);
			printf("-[INFO]total story division count : %zd\n", divisionCount);
			for (size_t j = 0; j < divisionCount; j++)
			{
				dataKey = fileName + std::string("_") + std::to_string(i) + std::string("_") + std::to_string(j);
				containers[dataKey] = std::vector<gaia3d::TrianglePolyhedron*>();

				size_t polyhedronCount = loader->getPolyhedronCount(i, j);
				printf("--[INFO]total polyhedron count : %zd\n", polyhedronCount);
				double* vertexPositions;
				float* color4;
				size_t* indices;
				gaia3d::TrianglePolyhedron* polyhedron;
				gaia3d::Vertex* vertex;
				gaia3d::Surface* surface;
				gaia3d::Triangle* triangle;
				size_t vertexCount, surfaceCount, triangleCount;
				wchar_t guidString[1024];
				for (size_t k = 0; k < polyhedronCount; k++)
				{
					polyhedron = new gaia3d::TrianglePolyhedron;
					polyhedron->setColorMode(gaia3d::SingleColor);
					color4 = loader->getRepresentativeColor(i, j, k);
					if (color4[3] == 0.0f)
						polyhedron->setSingleColor(DefaultColor);
					else
						polyhedron->setSingleColor(MakeColorU4((unsigned char)(color4[0] * 255), (unsigned char)(color4[1] * 255), (unsigned char)(color4[2] * 255)));

					polyhedron->setHasNormals(false);

					vertexCount = loader->getVertexCount(i, j, k);
					vertexPositions = loader->getVertexPositions(i, j, k);
					if (bYAxisUp)
					{
						for (size_t m = 0; m < vertexCount; m++)
						{
							vertex = new gaia3d::Vertex;
							vertex->position.set(vertexPositions[3 * m] * unitScaleFactor + offsetX, (-vertexPositions[3 * m + 2]) * unitScaleFactor + offsetY, vertexPositions[3 * m + 1] * unitScaleFactor + offsetZ);
							polyhedron->getVertices().push_back(vertex);
						}
					}
					else
					{
						for (size_t m = 0; m < vertexCount; m++)
						{
							vertex = new gaia3d::Vertex;
							vertex->position.set(vertexPositions[3 * m] * unitScaleFactor + offsetX, vertexPositions[3 * m + 1] * unitScaleFactor + offsetY, vertexPositions[3 * m + 2] * unitScaleFactor + offsetZ);
							polyhedron->getVertices().push_back(vertex);
						}
					}

					surfaceCount = loader->getSurfaceCount(i, j, k);
					for (size_t m = 0; m < surfaceCount; m++)
					{
						surface = new gaia3d::Surface;

						triangleCount = loader->getTrialgleCount(i, j, k, m);
						indices = loader->getTriangleIndices(i, j, k, m);
						for (size_t n = 0; n < triangleCount; n++)
						{
							triangle = new gaia3d::Triangle;
							triangle->getVertexIndices()[0] = indices[3 * n];
							triangle->getVertexIndices()[1] = indices[3 * n + 1];
							triangle->getVertexIndices()[2] = indices[3 * n + 2];
							triangle->getVertices()[0] = polyhedron->getVertices()[indices[3 * n]];
							triangle->getVertices()[1] = polyhedron->getVertices()[indices[3 * n + 1]];
							triangle->getVertices()[2] = polyhedron->getVertices()[indices[3 * n + 2]];

							surface->getTriangles().push_back(triangle);
						}

						polyhedron->getSurfaces().push_back(surface);
					}

					polyhedron->setId(containers[dataKey].size());

					memset(guidString, 0x00, sizeof(wchar_t) * 1024);
					loader->getGuid(i, j, k, guidString);
					std::wstring wObjectId(guidString);
					std::string objectId = gaia3d::StringUtility::convertWideStringToUtf8(wObjectId);

					polyhedron->addStringAttribute(std::string(ObjectGuid), objectId);

					containers[dataKey].push_back(polyhedron);
				}


				
			}
		}

		destroyIfcLoader(loader);

		// transform coordinates if information for georeferencing is injected.
		if (bCoordinateInfoInjected)
		{
			gaia3d::BoundingBox bbox;
			std::map<std::string, std::vector<gaia3d::TrianglePolyhedron*>>::iterator iterSubgroup = containers.begin();
			for (; iterSubgroup != containers.end(); iterSubgroup++)
			{
				for (size_t i = 0; i < iterSubgroup->second.size(); i++)
				{
					std::vector<gaia3d::Vertex*>& vertices = iterSubgroup->second[i]->getVertices();
					size_t vertexCount = vertices.size();
					for (size_t j = 0; j < vertexCount; j++)
						bbox.addPoint(vertices[j]->position.x, vertices[j]->position.y, vertices[j]->position.z);
				}
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
				iterSubgroup = containers.begin();
				for (; iterSubgroup != containers.end(); iterSubgroup++)
				{
					for (size_t i = 0; i < iterSubgroup->second.size(); i++)
						delete iterSubgroup->second[i];

					iterSubgroup->second.clear();
				}
				containers.clear();

				// new log
				LogWriter::getLogWriter()->changeCurrentConversionJobStatus(LogWriter::failure);
				LogWriter::getLogWriter()->addDescriptionToCurrentConversionJobLog(std::string(" IfcReader::readRawDataFile : boungding box center coordinate transform failure"));
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
			iterSubgroup = containers.begin();
			for (; iterSubgroup != containers.end(); iterSubgroup++)
			{
				for (size_t i = 0; i < iterSubgroup->second.size(); i++)
				{
					std::vector<gaia3d::Vertex*>& vertices = iterSubgroup->second[i]->getVertices();
					size_t vertexCount = vertices.size();
					gaia3d::Vertex* vertex;
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