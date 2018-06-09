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

	bool bOcclusionCulling;
	double unitScaleFactor;
	unsigned char skinLevel;
	bool bYAxisUp;
	bool bAlignPostionToCenter;
	bool bRealisticMesh;
	std::string referenceFileName;
	double referenceLon, referenceLat;
	double referencePosX, referencePosY;

	std::string inputFolderPath, outputFolderPath;

	std::string idPrefix, idSuffix;

public:
	static CConverterManager* getConverterManager() { return &m_ConverterManager; }


public:
	bool initialize();

	//bool processSingleFile(std::string& filePath);

	void setProcessConfiguration(std::map<std::string, std::string>& arguments);

	void process();

	void uninitialize();

private:
	bool processDataFolder();

	void collectTargetFiles(std::string& inputFolder, std::map<std::string, std::string>& targetFiles);

	bool writeIndexFile();

	void processDataFiles(std::map<std::string, std::string>& targetFiles);

	bool processDataFile(std::string& filePath, aReader* reader);

	void writeRepresentativeLonLatOfEachData(std::map<std::string, double>& posXs, std::map<std::string, double>& posYs);
};



