#pragma once

#ifdef IFCLOADER_DLL_EXPORTS
#define IFCLOADER_DLL_API  __declspec(dllexport)   // export DLL information
#else
#define IFCLOADER_DLL_API  __declspec(dllimport)   // import DLL information
#endif

#include <SDKDDKVer.h>
#include <string>

class IFCLOADER_DLL_API aIfcLoader abstract
{
public:
	virtual bool loadIfcFile(std::wstring& filePath) = 0;

	virtual void setVertexReductionMode(bool bOn) = 0;

	virtual size_t getPolyhedronCount() = 0;
	virtual float* getRepresentativeColor(size_t polyhedronIndex) = 0;
	virtual std::wstring getGuid(size_t polyhedronIndex) = 0;
	virtual size_t getVertexCount(size_t polyhedronIndex) = 0;
	virtual double* getVertexPositions(size_t polyhedronIndex) = 0;
	virtual size_t getSurfaceCount(size_t polyhedronIndex) = 0;
	virtual size_t getTrialgleCount(size_t polyhedronIndex, size_t surfaceIndex) = 0;
	virtual size_t* getTriangleIndices(size_t polyhedronIndex, size_t surfaceIndex) = 0;

	virtual bool loadOnlyPropertiesFromIfc(std::wstring& filePath) = 0;
	virtual void setAttributesExtraction(bool bOn) = 0;
	virtual std::string getObjectAttributes() = 0;
	virtual std::string getProjectAttributes() = 0; // not available now

#ifdef TMPTEST
	virtual size_t getAttributeTypeCount() = 0;
	virtual std::string getAttributeType(size_t i) = 0;
#endif
};

IFCLOADER_DLL_API aIfcLoader* createIfcLoader();
IFCLOADER_DLL_API void destroyIfcLoader(aIfcLoader* aLoader);