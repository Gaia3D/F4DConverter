#include "stdafx.h"

#if defined(CITYGMLFORMAT)

#include "CitygmlReader.h"

#include <algorithm>

#include <citygml/citygml.h>
#include <citygml/citymodel.h>
#include <citygml/envelope.h>
#include <citygml/geometry.h>
#include <citygml/implictgeometry.h>
#include <citygml/polygon.h>
#include <citygml/texture.h>

#include <proj_api.h>

#include "../geometry/TrianglePolyhedron.h"
#include "../util/utility.h"

bool readCity(std::shared_ptr<const citygml::CityModel>& city,
			std::string& folderPath,
			double& lon, double& lat,
			std::vector<gaia3d::TrianglePolyhedron*>& container,
			std::map<std::string, std::string>& textureContainer);
void createCityObject(const citygml::CityObject& object,
					std::string& folderPath,
					std::vector<gaia3d::TrianglePolyhedron*>& container,
					std::map<std::string, std::string>& textureContainer,
					projPJ pjSrc, projPJ pjWgs84, gaia3d::Matrix4& inverseGlobalTransMat);
unsigned int getMostDetailedLod(const citygml::CityObject& object);
void convertGeometryToTrianglePolyhedrons(const citygml::Geometry& geometry,
										std::string& folderPath,
										std::vector<gaia3d::TrianglePolyhedron*>& container,
										std::map<std::string, std::string>& textureContainer,
										gaia3d::Matrix4* mat,
										projPJ pjSrc, projPJ pjWgs84, gaia3d::Matrix4& inverseGlobalTransMat);

CitygmlReader::CitygmlReader()
{
}

CitygmlReader::~CitygmlReader()
{
}

bool CitygmlReader::readRawDataFile(std::string& filePath)
{
	citygml::ParserParams params;
	std::shared_ptr<const citygml::CityModel> city;

	try
	{
		city = citygml::load(filePath, params);
	}
	catch (const std::runtime_error& e)
	{
		printf("[ERROR]%s\n", e.what());
		return false;
	}

	if (city == NULL)
		return false;

	std::string folderPath;
	size_t lastSlashIndex = filePath.find_last_of("\\/");
	if (lastSlashIndex != std::string::npos)
		folderPath = filePath.substr(0, lastSlashIndex + 1);

	if (!readCity(city, folderPath, refLon, refLat, container, textureContainer))
		return false;

	bHasGeoReferencingInfo = true;

	return true;
}

void CitygmlReader::clear()
{
	container.clear();

	textureContainer.clear();
}

bool readCity(std::shared_ptr<const citygml::CityModel>& city,
			std::string& folderPath,
			double& lon, double& lat,
			std::vector<gaia3d::TrianglePolyhedron*>& container,
			std::map<std::string, std::string>& textureContainer)
{
	const citygml::ConstCityObjects& roots = city->getRootCityObjects();
	if (roots.size() == 0)
		return false;

	const citygml::Envelope envelope = city->getEnvelope();

	// split srsName into words
	std::string srsName = envelope.srsName();
	std::vector<std::string> splittedWords;
	char* srsChars = new char[srsName.length() + 1];
	memset(srsChars, 0x00, sizeof(char) * (srsName.length() + 1));
	memcpy(srsChars, srsName.c_str(), sizeof(char)*(srsName.length()));
	char* token = std::strtok(srsChars, " ,:");
	while (token != NULL)
	{
		splittedWords.push_back(std::string(token));

		token = std::strtok(NULL, " ,:");
	}
	delete srsChars;

	// extract EPSG codes
	std::vector<std::string> epsgCodes;
	for (size_t i = 0; i < splittedWords.size(); i++)
	{
		std::string word = splittedWords[i];
		std::transform(word.begin(), word.end(), word.begin(), towlower);

		if (word.compare(std::string("epsg")) != 0)
			continue;

		if (i != splittedWords.size() - 1)
			epsgCodes.push_back(splittedWords[i+1]);
	}

	if (epsgCodes.empty())
		return false;

	// select an available EPSG code
	std::string originalSrsProjString;
	for (size_t i = 0; i < epsgCodes.size(); i++)
	{
		std::string testProjString = std::string("+init=epsg:") + epsgCodes[i];
		projPJ pj_test = pj_init_plus(testProjString.c_str());

		if (pj_test)
		{
			originalSrsProjString = testProjString;
			break;
		}
	}
	if (originalSrsProjString.empty())
		return false;

	// get bounding box
	const TVec3d bboxLowerPoint = envelope.getLowerBound();
	const TVec3d bboxUpperPoint = envelope.getUpperBound();
	gaia3d::Point3D bboxCenter;
	bboxCenter.set((bboxLowerPoint.x + bboxUpperPoint.x) / 2.0, (bboxLowerPoint.y + bboxUpperPoint.y) / 2.0, (bboxLowerPoint.z + bboxUpperPoint.z) / 2.0);

	// find lon&lat of bounding box center
	std::string wgs84ProjString("+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs");
	lon = bboxCenter.x;
	lat = bboxCenter.y;
	double alt = bboxCenter.z;
	projPJ pjSrc, pjDst;

	if (!(pjDst = pj_init_plus(wgs84ProjString.c_str())) || !(pjSrc = pj_init_plus(originalSrsProjString.c_str())))
		return false;

	int errorCode = pj_transform(pjSrc, pjDst, 1, 1, &lon, &lat, &alt);
	char* errorMessage = pj_strerrno(errorCode);
	if (errorMessage != NULL)
	{
		printf("[ERROR]%s\n", errorMessage);
		return false;
	}

	lon *= RAD_TO_DEG;
	lat *= RAD_TO_DEG;

	double absPosOfCenterXY[3];
	alt = 0.0;
	gaia3d::GeometryUtility::wgs84ToAbsolutePosition(lon, lat, alt, absPosOfCenterXY);
	double m[16];
	gaia3d::GeometryUtility::transformMatrixAtAbsolutePosition(absPosOfCenterXY[0], absPosOfCenterXY[1], absPosOfCenterXY[2], m);
	gaia3d::Matrix4 globalTransformMatrix;
	globalTransformMatrix.set(m[0], m[4], m[8], m[12],
		m[1], m[5], m[9], m[13],
		m[2], m[6], m[10], m[14],
		m[3], m[7], m[11], m[15]);
	gaia3d::Matrix4 inverseGlobalTransMatrix = globalTransformMatrix.inverse();

	// transform objects in citygml into triangle polyhedrons
	for (size_t i = 0; i < roots.size(); i++)
	{
		const citygml::CityObject &cityObject = *roots[i];

		createCityObject(cityObject, folderPath, container, textureContainer, pjSrc, pjDst, inverseGlobalTransMatrix);
	}

	return true;
}

void createCityObject(const citygml::CityObject& object,
					std::string& folderPath,
					std::vector<gaia3d::TrianglePolyhedron*>& container,
					std::map<std::string, std::string>& textureContainer,
					projPJ pjSrc, projPJ pjWgs84, gaia3d::Matrix4& inverseGlobalTransMat)
{
	unsigned int mostDetailedLod = getMostDetailedLod(object);

	// create triangle polyhedron of geometries in this object
	for (unsigned int i = 0; i < object.getGeometriesCount(); i++)
	{
		const citygml::Geometry& geometry = object.getGeometry(i);
		if (geometry.getLOD() < mostDetailedLod)
			continue;

		convertGeometryToTrianglePolyhedrons(geometry, folderPath, container, textureContainer, NULL, pjSrc, pjWgs84, inverseGlobalTransMat);
	}

	// do same thing if implicit geometries exist
	for (unsigned int i = 0; i < object.getImplicitGeometryCount(); i++)
	{
		const citygml::ImplicitGeometry& implicitGeometry = object.getImplicitGeometry(i);
		for (unsigned int j = 0; j < implicitGeometry.getGeometriesCount(); j++)
		{
			const citygml::Geometry& geometry = implicitGeometry.getGeometry(j);
			if (geometry.getLOD() < mostDetailedLod)
				continue;

			const citygml::TransformationMatrix tMat = implicitGeometry.getTransformMatrix();
			const double* tMatArray = tMat.getMatrix(); // rowMajor matrix.***
			gaia3d::Matrix4 transformMatrix;
			transformMatrix.set(tMatArray[0], tMatArray[4], tMatArray[8], tMatArray[12],
								tMatArray[1], tMatArray[5], tMatArray[9], tMatArray[13],
								tMatArray[2], tMatArray[6], tMatArray[10], tMatArray[14],
								tMatArray[3], tMatArray[7], tMatArray[11], tMatArray[15]);

			// transform matrix in citygml has only rotation, so add reference position to the matrix
			TVec3d refPoint = implicitGeometry.getReferencePoint();
			transformMatrix.m[3][0] += refPoint.x;
			transformMatrix.m[3][1] += refPoint.y;
			transformMatrix.m[3][2] += refPoint.z;
			convertGeometryToTrianglePolyhedrons(geometry, folderPath, container, textureContainer, &transformMatrix, pjSrc, pjWgs84, inverseGlobalTransMat);
		}
	}

	// do same thing if childre exist
	for (unsigned int i = 0; i < object.getChildCityObjectsCount(); ++i)
	{
		createCityObject(object.getChildCityObject(i), folderPath, container, textureContainer, pjSrc, pjWgs84, inverseGlobalTransMat);
	}
}

void convertGeometryToTrianglePolyhedrons(const citygml::Geometry& geometry,
										std::string& folderPath,
										std::vector<gaia3d::TrianglePolyhedron*>& container,
										std::map<std::string, std::string>& textureContainer,
										gaia3d::Matrix4* mat,
										projPJ pjSrc, projPJ pjWgs84, gaia3d::Matrix4& inverseGlobalTransMat)
{
	// A geometry should be converted into multiple triangle polyhedrons(mesh == triangle polyhedron) when mutiple textures are used in the geometry.
	// (by the rule of 1 polyhedron on 1 texture)
	// So, a geometry is divided along the texture names and each meshes of result is assgined concerned texture name as its name

	std::map<std::string, gaia3d::TrianglePolyhedron*> polyhedronMapper;
	std::string noTexturePolyhedronKey("NoTexture");

	unsigned int polygonCount = geometry.getPolygonsCount();
	for (unsigned int i = 0; i < polygonCount; i++)
	{
		const citygml::Polygon& p = *(geometry.getPolygon(i));
		const std::vector<TVec3d>& vertices = p.getVertices();
		const std::vector<unsigned int>& indices = p.getIndices();
		std::vector<TVec2f> texCoords;

		if (vertices.size() < 3 || indices.size() < 3)
			continue;

		// 1. select or create master polyhedron
		std::string polyhedronKey = noTexturePolyhedronKey;
		bool bFront = true;
		std::vector<std::string> allTextureThemes = p.getAllTextureThemes(bFront);
		if (!allTextureThemes.empty())
		{
			std::shared_ptr<const citygml::Texture> textureGML = p.getTextureFor(allTextureThemes[0]);
			texCoords = p.getTexCoordsForTheme(allTextureThemes[0], bFront);
			if(vertices.size() == texCoords.size())
				polyhedronKey = textureGML->getUrl();
		}

		gaia3d::TrianglePolyhedron* masterPolyhedron = NULL;
		if (polyhedronMapper.find(polyhedronKey) == polyhedronMapper.end())
		{
			polyhedronMapper[polyhedronKey] = new gaia3d::TrianglePolyhedron;
			polyhedronMapper[polyhedronKey]->addStringAttribute(std::string(ObjectGuid), geometry.getId());
			polyhedronMapper[polyhedronKey]->setId(container.size());
			container.push_back(polyhedronMapper[polyhedronKey]);
			if(polyhedronKey.compare(noTexturePolyhedronKey) != 0)
				container.back()->setHasTextureCoordinates(true);
		}
		masterPolyhedron = polyhedronMapper[polyhedronKey];
		size_t prevVertexCount = masterPolyhedron->getVertices().size();

		// match 1 polygon to 1 surface
		gaia3d::Surface* surface = new gaia3d::Surface;
		masterPolyhedron->getSurfaces().push_back(surface);

		// vertices
		std::vector<gaia3d::Vertex*> verticesInPolygon;
		double px, py, pz;
		for (size_t j = 0; j < vertices.size(); j++)
		{
			TVec3d v = vertices[j];
			gaia3d::Vertex* vertex = new gaia3d::Vertex;
			
			if (mat != NULL)
			{
				gaia3d::Point3D srsPos;
				srsPos.set(v.x, v.y, v.z);
				srsPos = (*mat) * srsPos;
				px = srsPos.x;
				py = srsPos.y;
				pz = srsPos.z;
			}
			else
			{
				px = v.x;
				py = v.y;
				pz = v.z;
			}

			pj_transform(pjSrc, pjWgs84, 1, 1, &px, &py, &pz);
			px *= RAD_TO_DEG;
			py *= RAD_TO_DEG;

			double absPosOfTargetPointArray[3];
			gaia3d::GeometryUtility::wgs84ToAbsolutePosition(px, py, pz, absPosOfTargetPointArray);
			gaia3d::Point3D absPosOfTargetPoint;
			absPosOfTargetPoint.set(absPosOfTargetPointArray[0], absPosOfTargetPointArray[1], absPosOfTargetPointArray[2]);
			vertex->position = inverseGlobalTransMat * absPosOfTargetPoint;

			verticesInPolygon.push_back(vertex);
			masterPolyhedron->getVertices().push_back(vertex);
		}

		// triangles
		size_t triangleCount = indices.size() / 3;
		double error = 1E-5;
		for (size_t j = 0; j < triangleCount; j++)
		{
			unsigned int index0 = indices[j * 3];
			unsigned int index1 = indices[j * 3 + 1];
			unsigned int index2 = indices[j * 3 + 2];

			gaia3d::Vertex* vertex0 = verticesInPolygon[index0];
			gaia3d::Vertex* vertex1 = verticesInPolygon[index1];
			gaia3d::Vertex* vertex2 = verticesInPolygon[index2];

			gaia3d::Point3D planeNormal;
			gaia3d::GeometryUtility::calculatePlaneNormal(
					vertex0->position.x, vertex0->position.y, vertex0->position.z,
					vertex1->position.x, vertex1->position.y, vertex1->position.z,
					vertex2->position.x, vertex2->position.y, vertex2->position.z,
					planeNormal.x, planeNormal.y, planeNormal.z,
					false);

			if (!planeNormal.normalize(error))
				continue;

			gaia3d::Triangle* triangle = new gaia3d::Triangle;
			triangle->setVertices(vertex0, vertex1, vertex2);
			triangle->setVertexIndices(prevVertexCount + index0, prevVertexCount + index1, prevVertexCount + index2);
			surface->getTriangles().push_back(triangle);
		}

		if (polyhedronKey.compare(noTexturePolyhedronKey) == 0)
			continue;

		// texture coordinate
		for (size_t j = 0; j < texCoords.size(); j++)
		{
			TVec2f texCoord = texCoords[j];
			gaia3d::Vertex* vertex = verticesInPolygon[j];

			vertex->textureCoordinate[0] = texCoord.x;
			vertex->textureCoordinate[1] = texCoord.y;
		}

		// texture material
		if (masterPolyhedron->doesStringAttributeExist(std::string(TextureName)))
			continue;

		std::string textureUrl = polyhedronKey;
		std::string textureName;
		size_t lastSlashIndex = textureUrl.find_last_of("\\/");
		if (lastSlashIndex == std::string::npos)
			textureName = textureUrl;
		else
		{
			size_t fileNameLength = textureUrl.length() - lastSlashIndex - 1;
			textureName = textureUrl.substr(lastSlashIndex + 1, fileNameLength);
		}

		masterPolyhedron->addStringAttribute(std::string(TextureName), textureName);

		if (textureContainer.find(textureName) != textureContainer.end())
			continue;

		std::string textureFullPath = folderPath + textureUrl;
		textureContainer[textureName] = textureFullPath;
	}

	// do same things if children exist
	for (unsigned int i = 0; i < geometry.getGeometriesCount(); i++)
	{
		convertGeometryToTrianglePolyhedrons(geometry.getGeometry(i), folderPath, container, textureContainer, mat, pjSrc, pjWgs84, inverseGlobalTransMat);
	}
}

unsigned int getMostDetailedLod(const citygml::CityObject& object)
{
	unsigned int mostDetailedLod = 0;

	// find out most detailed LOD on geometries in this object
	unsigned int geometryCount = object.getGeometriesCount();
	for (unsigned int i = 0; i < geometryCount; i++)
	{
		const citygml::Geometry &geometry = object.getGeometry(i);

		if (geometry.getLOD() <= mostDetailedLod)
			continue;

		mostDetailedLod = geometry.getLOD();
	}

	// do same thing if implicit geometries exist
	unsigned int implicitGeometryCount = object.getImplicitGeometryCount();
	for (unsigned int i = 0; i < implicitGeometryCount; i++)
	{
		const citygml::ImplicitGeometry& implicitGeometry = object.getImplicitGeometry(i);

		unsigned int geometryCountInThis = implicitGeometry.getGeometriesCount();
		for (unsigned int j = 0; j < geometryCountInThis; j++)
		{
			const citygml::Geometry& geometry = implicitGeometry.getGeometry(j);

			if (geometry.getLOD() <= mostDetailedLod)
				continue;

			mostDetailedLod = geometry.getLOD();
		}
	}

	// do same thing if children exist
	unsigned int childCount = object.getChildCityObjectsCount();
	for (unsigned int i = 0; i < childCount; ++i)
	{
		unsigned int mostDetailedLodOfChild = getMostDetailedLod(object.getChildCityObject(i));

		if (mostDetailedLodOfChild <= mostDetailedLod)
			continue;
		
		mostDetailedLod = mostDetailedLodOfChild;
	}

	return mostDetailedLod;
}

#endif