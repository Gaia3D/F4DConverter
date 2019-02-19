#pragma once

#if defined(POINTCLOUDFORMAT)

#include "aReader.h"

#ifdef _DEBUG
#pragma comment(lib, "../external/liblas/lib/liblasD.lib")
#else
#pragma comment(lib, "../external/liblas/lib/liblas.lib")
#endif


class PointCloudReader : public aReader
{
public:
	PointCloudReader();
	~PointCloudReader();

public:
	virtual bool readRawDataFile(std::string& filePath);

	virtual void clear();
};

#endif

