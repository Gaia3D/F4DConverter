#pragma once

#ifdef IFCFORMAT

#include "aReader.h"

#ifdef _DEBUG
#pragma comment(lib, "../bin/lib/IfcLoaderd.lib")
#else
#pragma comment(lib, "../bin/lib/IfcLoader.lib")
#endif

class IfcReader :
	public aReader
{
public:
	IfcReader();
	virtual ~IfcReader();

public:
	virtual bool readRawDataFile(std::string& filePath);

	virtual void clear();
};

#endif

