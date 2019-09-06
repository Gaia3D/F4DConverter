#pragma once

#if defined(CLASSICFORMAT)

#pragma comment(lib, "../external/assimp-4.1/lib/x64/assimp-vc140-mt.lib")

#include "aReader.h"

class ClassicFormatReader : public aReader
{
public:
	ClassicFormatReader();
	virtual ~ClassicFormatReader();

public:
	virtual bool readRawDataFile(std::string& filePath);

	virtual void clear();

};

#endif
