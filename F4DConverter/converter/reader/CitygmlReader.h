#pragma once

#if defined(CITYGMLFORMAT)

#ifdef _DEBUG
#pragma comment(lib, "../external/xercesc/lib/xerces-c_3D.lib")
#pragma comment(lib, "../external/libcitygml/lib/citygmld.lib")
#else
#pragma comment(lib, "../external/xercesc/lib/xerces-c_3.lib")
#pragma comment(lib, "../external/libcitygml/lib/citygml.lib")
#endif

#include "aReader.h"

class CitygmlReader : public aReader
{
public:
	CitygmlReader();
	virtual ~CitygmlReader();

public:
	virtual bool readRawDataFile(std::string& filePath);

	virtual void clear();
};

#endif

