#pragma once

#if defined(INDOORGMLFORMAT)

#ifdef _DEBUG
#pragma comment(lib, "../external/xercesc/lib/xerces-c_3D.lib")
#pragma comment(lib, "../external/libcitygml/lib/citygmld.lib")
#else
#pragma comment(lib, "../external/xercesc/lib/xerces-c_3.lib")
#pragma comment(lib, "../external/libcitygml/lib/citygml.lib")
#endif
#include "xercesc/parsers/XercesDOMParser.hpp"
#include "xercesc/dom/DOM.hpp"
#include "xercesc/sax/HandlerBase.hpp"
#include "xercesc/util/XMLString.hpp"
#include "xercesc/util/PlatformUtils.hpp"

#include "aReader.h"


using namespace xercesc;
class GeometryManager;

namespace gaia3d
{
	class TrianglePolyhedron;
}

class IndoorGMLReader : public aReader
{
public:
	IndoorGMLReader();
	virtual ~IndoorGMLReader();
	

public:
	virtual bool readRawDataFile(std::string& filePath);
	
	virtual void clear();

private:
	GeometryManager parseIndoorGeometry(DOMDocument* dom);
	bool readIndoorSpace(DOMDocument* dom, std::vector<gaia3d::TrianglePolyhedron*>& container, double& lon, double& lat);

	
};

#endif

