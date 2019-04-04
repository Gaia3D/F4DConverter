#include "stdafx.h"

#if defined(POINTCLOUDFORMAT)


#include "PointCloudReader.h"

#include "liblas/liblas.hpp"
#include <fstream>  // std::ifstream
#include <proj_api.h>
//#include <ogr_spatialref.h>

#include "../util/utility.h"


PointCloudReader::PointCloudReader()
{
}


PointCloudReader::~PointCloudReader()
{
}

bool PointCloudReader::readRawDataFile(std::string& filePath)
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
//	const std::vector<liblas::VariableRecord> vlrs = header.GetVLRs();
//	std::string const lasProjId("LASF_Projection");
//	std::string const liblasId("liblas");
//
//	size_t vlrCount = vlrs.size();
//	for (size_t i = 0; i < vlrCount; i++)
//	{
//		liblas::VariableRecord vlr = vlrs[i];
//
//		unsigned short recordId = vlr.GetRecordId();
//		if (recordId == 2112) //OGR Wkt
//		{
//			const liblas::IndexVLRData data = vlr.GetData();
//			size_t wktLength = data.size();
//			unsigned char* poWkt = new unsigned char[wktLength];
//			memset(poWkt, 0x00, sizeof(unsigned char)*wktLength);
//			for (size_t j = 0; j < wktLength; j++)
//			{
//				poWkt[j] = data[j];
//			}
//
//				
//			std::string stringWkt((char*)poWkt);
//			delete[] poWkt;
//			const char* finalWkt = stringWkt.c_str();
//
//			OGRSpatialReference srs(NULL);
//#if GDAL_VERSION_MAJOR > 2 || (GDAL_VERSION_MAJOR == 2 && GDAL_VERSION_MINOR >= 3)
//			if (OGRERR_NONE != srs.importFromWkt(stringWkt.c_str()))
//#else
//			if (OGRERR_NONE != srs.importFromWkt(const_cast<char **> (&poWKT)))
//#endif
//			{
//				break;
//			}
//
//			char* proj4;
//			srs.exportToProj4(&proj4);
//			originalSrsProjString = std::string(proj4);
//			CPLFree(proj4);
//		}
//		else // geotiff
//		{
//		}
//	}

	liblas::SpatialReference spatialReference = header.GetSRS();
	originalSrsProjString = spatialReference.GetProj4();

	if (originalSrsProjString.empty())
		return false;

	projPJ pjSrc = pj_init_plus(originalSrsProjString.c_str());
	if (!pjSrc)
	{
		ifs.close();
		return false;
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

void PointCloudReader::clear()
{
	container.clear();

	textureContainer.clear();
}

#endif
