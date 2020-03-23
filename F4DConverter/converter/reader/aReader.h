#pragma once

#include <string>
#include <vector>
#include <map>

//#include "../geometry/TrianglePolyhedron.h"

#pragma comment(lib, "../external/proj/lib/proj_5_2.lib")

namespace gaia3d
{
	class TrianglePolyhedron;
}

class aReader abstract
{
public:
	aReader()
	{
		unitScaleFactor = 1.0;

		bHasGeoReferencingInfo = false;

		bCoordinateInfoInjected = false;

		bYAxisUp = false;

		bBuildHiararchy = false;

		bAlignToBottomCenter = bAlignToCenter = false;
	}

	virtual ~aReader()
	{
	}

public:
	virtual bool readRawDataFile(std::string& filePath) = 0;

	virtual void clear() = 0;

	virtual std::vector<gaia3d::TrianglePolyhedron*>& getDataContainer() {return container;}

	virtual std::map<std::string, std::vector<gaia3d::TrianglePolyhedron*>>& getMultipleDataContainers() { return containers; }

	virtual std::map<std::string, std::string>& getTextureInfoContainer() { return textureContainer; }

	virtual void setUnitScaleFactor(double factor) { unitScaleFactor = factor; }

	virtual void setOffset(double x, double y, double z) { offsetX = x; offsetY = y; offsetZ = z; }

	virtual void setYAxisUp(bool bUp) { bYAxisUp = bUp; }

	virtual void setBuildHiararchy(bool bBuild) { bBuildHiararchy = bBuild; }

	virtual bool doesHasGeoReferencingInfo() { return bHasGeoReferencingInfo; }

	virtual bool doesHasAdditionalInfo() { return bHasAdditionalInfo; }

	virtual std::map<std::string, std::string>& getAdditionalInfo() { return additionalInfo; }

	virtual void getGeoReferencingInfo(double& lon, double& lat) { lon = refLon; lat = refLat; }

	virtual void injectOringinInfo(double& lon, double& lat) { lonOrigin = lon; latOrigin = lat; bCoordinateInfoInjected = true; }

	virtual void injectSrsInfo(std::string& epsg) { this->epsg = epsg; bCoordinateInfoInjected = true; }

	virtual void alignToBottomCenter(bool bAlign) { bAlignToBottomCenter = bAlign; }

	virtual void alignToCenter(bool bAlign) { bAlignToCenter = bAlign; }

	virtual std::map<std::string, std::string>& getTemporaryFiles() { return temporaryFiles; }

	virtual bool shouldGeometryBeDesroyedOutside() { return (!container.empty() && !containers.empty()); }

	virtual bool shouldRawDataBeConvertedToMuitiFiles() { return !containers.empty(); }

	virtual std::map<std::string, std::vector<std::string>>& getAncestorsOfEachSubGroup() { return ancestorsOfEachSubGroup; }

	virtual std::map<std::string, bool>& getSplitFilter() { return splitFilter; }


protected:
	std::vector<gaia3d::TrianglePolyhedron*> container;

	std::map<std::string, std::vector<gaia3d::TrianglePolyhedron*>> containers;

	std::map<std::string, std::string> textureContainer;

	bool bYAxisUp;

	double unitScaleFactor;

	double offsetX, offsetY, offsetZ;

	bool bHasGeoReferencingInfo;

	bool bHasAdditionalInfo;

	double refLon, refLat;

	std::string epsg;

	std::map<std::string, std::string> additionalInfo;

	double lonOrigin, latOrigin;

	bool bCoordinateInfoInjected;

	bool bAlignToBottomCenter;

	bool bAlignToCenter;

	bool bBuildHiararchy;

	std::map<std::string, std::vector<std::string>> ancestorsOfEachSubGroup;

	std::map<std::string, bool> splitFilter;

protected:


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