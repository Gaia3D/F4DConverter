#pragma once

#if defined(CLASSICFORMAT)

//#define ASSIMP_VERSION_3_1_1
#define ASSIMP_VERSION_3_2

#if defined(ASSIMP_VERSION_3_2)
#ifdef _DEBUG
#pragma comment(lib, "../external/assimp-3.2/lib/assimp-vc100-mtd.lib")
#else
#pragma comment(lib, "../external/assimp-3.2/lib/assimp-vc100-mt.lib")
#endif
#elif defined(ASSIMP_VERSION_3_1_1)
#pragma comment(lib, "../external/assimp-3.1.1-win-binaries/lib64/assimp.lib")
#endif


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
