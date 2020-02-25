#pragma once

#include <string>
#include <vector>
#include <map>

#include "../geometry/TrianglePolyhedron.h"

#pragma comment(lib, "../external/proj/lib/proj_5_2.lib")

class aReader abstract
{
public:
	aReader() {}

	virtual ~aReader()
	{
		unitScaleFactor = 1.0;

		bHasGeoReferencingInfo = false;

		bCoordinateInfoInjected = false;

		bYAxisUp = false;

		offsetX = offsetY = offsetZ = 0.0;
	}

public:
	virtual bool readRawDataFile(std::string& filePath) = 0;

	virtual void clear() = 0;

	virtual std::vector<gaia3d::TrianglePolyhedron*>& getDataContainer() {return container;}

	virtual std::map<std::string, std::string>& getTextureInfoContainer() { return textureContainer; }

	virtual void setUnitScaleFactor(double factor) { unitScaleFactor = factor; }

	virtual void setOffset(double x, double y, double z) { offsetX = x; offsetY = y; offsetZ = z; }

	virtual void setYAxisUp(bool bUp) { bYAxisUp = bUp; }

	virtual bool doesHasGeoReferencingInfo() { return bHasGeoReferencingInfo; }

	virtual void getGeoReferencingInfo(double& lon, double& lat) { lon = refLon; lat = refLat; }

	virtual void injectOringinInfo(double& lon, double& lat) { lonOrigin = lon; latOrigin = lat; bCoordinateInfoInjected = true; }

	virtual void injectSrsInfo(std::string& epsg) { this->epsg = epsg; bCoordinateInfoInjected = true; }

	virtual std::map<std::string, std::string>& getTemporaryFiles() { return temporaryFiles; }

	virtual void setOutputFolderPath(std::string opf) { outputFolderPath = opf; }

protected:
	std::vector<gaia3d::TrianglePolyhedron*> container;

	std::map<std::string, std::string> textureContainer;

	bool bYAxisUp;

	double unitScaleFactor;

	double offsetX, offsetY, offsetZ;

	bool bHasGeoReferencingInfo;

	double refLon, refLat;

	std::string epsg;

	double lonOrigin, latOrigin;

	bool bCoordinateInfoInjected;

	std::string outputFolderPath;

	std::string makeProj4String()
	{
		std::string proj4String;

		if (epsg.empty())
		{
			proj4String = std::string("+proj=tmerc +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs");
			proj4String += std::string(" +lon_0=") + std::to_string(lonOrigin);
			proj4String += std::string(" +lat_0=") + std::to_string(latOrigin);
		}
		else
		{
			proj4String = std::string("+init=epsg:") + epsg;
		}

		return proj4String;
	}

	std::map<std::string, std::string> temporaryFiles;
};