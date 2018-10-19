#pragma once

#include <string>
#include <map>

// CConverterManager 명령 대상입니다.

class aReader;
class ConversionProcessor;

class CConverterManager
{
private:
	CConverterManager();

public:
	virtual ~CConverterManager();

private:
	static CConverterManager m_ConverterManager;

	ConversionProcessor* processor;

	bool bCreateIndices, bConversion;

	std::string programPath;
	bool bOcclusionCulling;
	double unitScaleFactor;
	unsigned char skinLevel;
	bool bYAxisUp;
	bool bAlignPostionToCenter;
	bool bUseReferenceLonLat;
	double referenceLon, referenceLat;
	int meshType;
	bool bUseEpsg;
	std::string epsgCode;

	std::string inputFolderPath, outputFolderPath;

	std::string idPrefix, idSuffix;

public:
	static CConverterManager* getConverterManager() { return &m_ConverterManager; }


public:
	bool initialize(std::map<std::string, std::string>& arguments);

	//bool processSingleFile(std::string& filePath);

	void process();

	void uninitialize();

private:
	bool setProcessConfiguration(std::map<std::string, std::string>& arguments);

	bool processDataFolder();

	void collectTargetFiles(std::string& inputFolder, std::map<std::string, std::string>& targetFiles);

	bool writeIndexFile();

	void processDataFiles(std::map<std::string, std::string>& targetFiles);

	bool processDataFile(std::string& filePath, aReader* reader);

	std::string makeProj4String();

	void writeRepresentativeLonLatOfEachData(std::map<std::string, double>& posXs, std::map<std::string, double>& posYs, std::string proj4String);
};



