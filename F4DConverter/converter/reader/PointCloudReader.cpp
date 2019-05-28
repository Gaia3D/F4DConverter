#include "stdafx.h"

#if defined(POINTCLOUDFORMAT)


#include "PointCloudReader.h"

#include <fstream>  // std::ifstream

#include <proj_api.h>

#include <ogr_spatialref.h>
#include <cpl_conv.h>

#include <geo_normalize.h>

#include "liblas/liblas.hpp"

#include "../util/utility.h"


PointCloudReader::PointCloudReader()
{
}


PointCloudReader::~PointCloudReader()
{
}

bool PointCloudReader::readRawDataFile(std::string& filePath)
{
	std::string::size_type dotPosition = filePath.rfind(".");
	std::string::size_type fileExtLength = filePath.length() - dotPosition - 1;
	std::string fileExt = filePath.substr(dotPosition + 1, fileExtLength);
	std::transform(fileExt.begin(), fileExt.end(), fileExt.begin(), towlower);

	if (fileExt.compare(std::string("las")) == 0)
		return readLasFile(filePath);
	else if (fileExt.compare(std::string("tpc")) == 0)
		return readTemporaryPointCloudFile(filePath);
	else
		return false;
}

#define ALLOWED_MAX_POINT_COUNT 100000000

void splitOriginalDataIntoSubDivisionsAndDump(liblas::Reader& reader, std::string& proj4String, std::map<std::string, std::string>& fileContainer);

bool PointCloudReader::readLasFile(std::string& filePath)
{
	// open a .las file with input file stream
	std::ifstream ifs;
	ifs.open(filePath, std::ios::in | std::ios::binary);

	// create a reader for the .las file with the input file stream
	liblas::ReaderFactory f;
	liblas::Reader reader = f.CreateWithStream(ifs);

	// access to the header block
	liblas::Header const& header = reader.GetHeader();

	// find CRS info of this .las for georeferencing
	std::string originalSrsProjString;

	liblas::SpatialReference spatialReference = header.GetSRS();
	originalSrsProjString = spatialReference.GetProj4();

	if (originalSrsProjString.empty())
	{
		// in this case, must get srs info through wkt or geotiff api

		std::string wkt = spatialReference.GetWKT();
		if (!wkt.empty())
		{
			// in this case, must change wkt info into proj4
			const char* poWKT = wkt.c_str();
			OGRSpatialReference srs(NULL);
			if (OGRERR_NONE != srs.importFromWkt(const_cast<char **> (&poWKT)))
				return false;

			char* pszProj4 = NULL;
			srs.exportToProj4(&pszProj4);
			if (OGRERR_NONE != srs.exportToProj4(&pszProj4))
				return false;

			originalSrsProjString = std::string(pszProj4);
			CPLFree(pszProj4);

			if (originalSrsProjString.empty())
				return false;
		}
		else
		{
			// in this case, must change geotiff info into proj4
			const GTIF* originalGtif = spatialReference.GetGTIF();
			if (originalGtif == NULL)
				return false;

			GTIF* temp = NULL;
			memcpy(&temp, &originalGtif, sizeof(GTIF*));
			GTIFDefn defn;
			if (GTIFGetDefn(temp, &defn) == 0)
				return false;

			char* pszWKT = GTIFGetOGISDefn(temp, &defn);
			if (pszWKT == NULL)
				return false;

			OGRSpatialReference srs(NULL);
			char* pOriginalWkt = pszWKT;
			if (OGRERR_NONE != srs.importFromWkt(&pOriginalWkt))
			{
				CPLFree(pszWKT);
				return false;
			}

			char* pszProj4 = NULL;
			if (OGRERR_NONE != srs.exportToProj4(&pszProj4))
			{
				CPLFree(pszWKT);
				return false;
			}

			originalSrsProjString = std::string(pszProj4);
			
			CPLFree(pszProj4);
			CPLFree(pszWKT);

			if (originalSrsProjString.empty())
				return false;
		}
	}

	projPJ pjSrc = pj_init_plus(originalSrsProjString.c_str());
	if (!pjSrc)
	{
		ifs.close();
		return false;
	}

	unsigned int pointCount = header.GetPointRecordsCount();
	if (pointCount > ALLOWED_MAX_POINT_COUNT)
	{
		splitOriginalDataIntoSubDivisionsAndDump(reader, originalSrsProjString, temporaryFiles);
		ifs.close();

		return true;
	}
	
	std::string wgs84ProjString("+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs");
	projPJ pjDst = pj_init_plus(wgs84ProjString.c_str());
	if(!pjDst)
	{
		ifs.close();
		return false;
	}

	double cx = (header.GetMaxX() + header.GetMinX())/2.0;
	double cy = (header.GetMaxY() + header.GetMinY())/2.0;
	double cz = (header.GetMaxZ() + header.GetMinZ())/2.0;
	refLon = cx, refLat = cy;
	double alt = cz;
	int errorCode = pj_transform(pjSrc, pjDst, 1, 1, &refLon, &refLat, &alt);
	char* errorMessage = pj_strerrno(errorCode);
	if (errorMessage != NULL)
	{
		printf("[ERROR]%s\n", errorMessage);
		ifs.close();
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

	// retrieve point geometries
	gaia3d::TrianglePolyhedron* polyhedron = new gaia3d::TrianglePolyhedron;
	double px, py, pz;
	unsigned short maxColorChannelValue = 0;
	std::vector<unsigned short> reds, greens, blues;
	while (reader.ReadNextPoint())
	{
		gaia3d::Vertex* vertex = new gaia3d::Vertex;

		// position
		liblas::Point const& p = reader.GetPoint();
		px = p.GetX();
		py = p.GetY();
		pz = p.GetZ();

		pj_transform(pjSrc, pjDst, 1, 1, &px, &py, &pz);
		px *= RAD_TO_DEG;
		py *= RAD_TO_DEG;

		double absPosOfTargetPointArray[3];
		gaia3d::GeometryUtility::wgs84ToAbsolutePosition(px, py, pz, absPosOfTargetPointArray);
		gaia3d::Point3D absPosOfTargetPoint;
		absPosOfTargetPoint.set(absPosOfTargetPointArray[0], absPosOfTargetPointArray[1], absPosOfTargetPointArray[2]);
		vertex->position = inverseGlobalTransMatrix * absPosOfTargetPoint;

		// color
		liblas::Color const &color = p.GetColor();
		unsigned short or , og, ob;
		if (color.GetRed() != 0 || color.GetGreen() != 0 || color.GetBlue() != 0)
		{
			or = color.GetRed(); og = color.GetGreen(); ob = color.GetBlue();
			if (maxColorChannelValue < or )
				maxColorChannelValue = or ;
			if (maxColorChannelValue < og )
				maxColorChannelValue = og ;
			if (maxColorChannelValue < ob )
				maxColorChannelValue = ob ;

			reds.push_back(or);
			greens.push_back(og);
			blues.push_back(ob);
		}
		else
		{
			or = p.GetIntensity();
			if (maxColorChannelValue < or )
				maxColorChannelValue = or ;

			reds.push_back(or );
			greens.push_back(or );
			blues.push_back(or );
		}

		polyhedron->getVertices().push_back(vertex);
	}

	if (maxColorChannelValue > 255)
	{
		size_t vertexCount = polyhedron->getVertices().size();
		unsigned char r, g, b;
		for (size_t i = 0; i < vertexCount; i++)
		{
			r = (unsigned char)(reds[i] >> 8);
			g = (unsigned char)(greens[i] >> 8);
			b = (unsigned char)(blues[i] >> 8);
			polyhedron->getVertices()[i]->color = MakeColorU4(r, g, b);
		}
	}
	else
	{
		size_t vertexCount = polyhedron->getVertices().size();
		unsigned char r, g, b;
		for (size_t i = 0; i < vertexCount; i++)
		{
			r = (unsigned char)(reds[i]);
			g = (unsigned char)(greens[i]);
			b = (unsigned char)(blues[i]);
			polyhedron->getVertices()[i]->color = MakeColorU4(r, g, b);
		}
	}

	printf("[Info]input point count : %zd\n", polyhedron->getVertices().size());

	polyhedron->setId(container.size());
	container.push_back(polyhedron);

	ifs.close();

	return true;
}

bool PointCloudReader::readTemporaryPointCloudFile(std::string& filePath)
{
	return false;
}

void PointCloudReader::clear()
{
	container.clear();

	textureContainer.clear();
}

void splitOriginalDataIntoSubDivisionsAndDump(liblas::Reader& reader, std::string& proj4String, std::map<std::string, std::string>& fileContainer)
{
}

#endif
