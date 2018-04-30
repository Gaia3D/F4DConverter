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

	std::string inputFolderPath, outputFolderPath;

	std::string idPrefix, idSuffix;

public:
	static CConverterManager* getConverterManager() { return &m_ConverterManager; }


public:
	bool initialize();

	bool processSingleFile(std::string& filePath);

	void setProcessConfiguration(std::map<std::string, std::string>& arguments);

	void process();

	void uninitialize();

private:
	bool processDataFolder();

	bool writeIndexFile();

	void processDataFolder(std::string& inputFolder);

	bool processDataFile(std::string& filePath, aReader* reader);
};



