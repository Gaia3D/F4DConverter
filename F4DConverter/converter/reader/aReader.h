#pragma once

#include <string>
#include <vector>
#include <map>

#include "../geometry/TrianglePolyhedron.h"

class aReader abstract
{
public:
	aReader() {}

	virtual ~aReader() {}

public:
	virtual bool readRawDataFile(std::string& filePath) = 0;

	virtual void clear() = 0;

	virtual std::vector<gaia3d::TrianglePolyhedron*>& getDataContainer() {return container;}

	virtual std::map<std::string, std::string>& getTextureInfoContainer() { return textureContainer; }

	virtual void setUnitScaleFactor(double factor) { unitScaleFactor = factor; }

	virtual bool doesHasGeoReferencingInfo() { return bHasGeoReferencingInfo; }

	virtual void getGeoReferencingInfo(double& lon, double& lat) { lon = refLon; lat = refLat; }

protected:
	std::vector<gaia3d::TrianglePolyhedron*> container;

	std::map<std::string, std::string> textureContainer;

	double unitScaleFactor;

	bool bHasGeoReferencingInfo;

	double refLon, refLat;
};