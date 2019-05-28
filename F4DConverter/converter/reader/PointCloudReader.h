#pragma once

#if defined(POINTCLOUDFORMAT)

#include "aReader.h"

#ifdef _DEBUG
#pragma comment(lib, "../external/liblas/lib/liblasd.lib")
#else
#pragma comment(lib, "../external/liblas/lib/liblas.lib")
#endif

#pragma comment(lib, "../external/gdal/lib/gdal_i.lib")


class PointCloudReader : public aReader
{
public:
	PointCloudReader();
	~PointCloudReader();

public:
	virtual bool readRawDataFile(std::string& filePath);

	virtual void clear();

private:
	bool readLasFile(std::string& filePath);
	bool readTemporaryPointCloudFile(std::string& filePath);
};

#endif

