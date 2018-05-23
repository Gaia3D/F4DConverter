// ./converter/ConverterManager.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "ConverterManager.h"

#include <io.h>
#include <sys/stat.h>

#include "../argumentDefinition.h"
#include "./reader/ReaderFactory.h"
#include "./process/ConversionProcessor.h"
#include "./process/SceneControlVariables.h"
#include "./writer/F4DWriter.h"

#include "LogWriter.h"


// CConverterManager

CConverterManager CConverterManager::m_ConverterManager;

CConverterManager::CConverterManager()
{
	processor = NULL;

	bCreateIndices =  bConversion = false;

	bOcclusionCulling = false;

	unitScaleFactor = 1.0;

	skinLevel = 3;
}

CConverterManager::~CConverterManager()
{
	if(processor != NULL)
		delete processor;
}


// CConverterManager 멤버 함수

bool CConverterManager::initialize()
{
	if(processor == NULL)
		processor = new ConversionProcessor();

	return processor->initialize();
}

void CConverterManager::uninitialize()
{
	if (processor != NULL)
		processor->uninitialize();
}

bool CConverterManager::processDataFolder()
{
	std::string inputFolder = inputFolderPath;
	std::string outputFolder = outputFolderPath;
	// test if output folder exist
	bool outputFolderExist = false;
	if (_access(outputFolder.c_str(), 0) == 0)
	{
		struct stat status;
		stat(outputFolder.c_str(), &status);
		if (status.st_mode & S_IFDIR)
			outputFolderExist = true;
	}

	if (!outputFolderExist)
	{
		LogWriter::getLogWriter()->addContents(std::string(ERROR_FLAG), false);
		LogWriter::getLogWriter()->addContents(std::string(NO_DATA_OR_INVALID_PATH), false);
		LogWriter::getLogWriter()->addContents(outputFolder, true);
		return false;
	}

	bool inputFolderExist = false;
	if (_access(inputFolder.c_str(), 0) == 0)
	{
		struct stat status;
		stat(inputFolder.c_str(), &status);
		if (status.st_mode & S_IFDIR)
			inputFolderExist = true;
	}

	if (!inputFolderExist)
	{
		LogWriter::getLogWriter()->addContents(std::string(ERROR_FLAG), false);
		LogWriter::getLogWriter()->addContents(std::string(NO_DATA_OR_INVALID_PATH), false);
		LogWriter::getLogWriter()->addContents(inputFolder, true);
		return false;
	}

	processDataFolder(inputFolder);


	return true;
}

bool CConverterManager::processSingleFile(std::string& filePath)
{
	std::string outputFolder = outputFolderPath;
	bool outputFolderExist = false;
	if (_access(outputFolder.c_str(), 0) == 0)
	{
		struct stat status;
		stat(outputFolder.c_str(), &status);
		if (status.st_mode & S_IFDIR)
			outputFolderExist = true;
	}

	if (!outputFolderExist)
	{
		LogWriter::getLogWriter()->addContents(std::string(ERROR_FLAG), false);
		LogWriter::getLogWriter()->addContents(std::string(NO_DATA_OR_INVALID_PATH), true);
		return false;
	}

	bool bRawDataFileExists = false;
	if (_access(filePath.c_str(), 0) == 0)
	{
		struct stat status;
		stat(filePath.c_str(), &status);
		if ((status.st_mode & S_IFDIR) != S_IFDIR)
			bRawDataFileExists = true;
	}

	if (!bRawDataFileExists)
	{
		LogWriter::getLogWriter()->addContents(std::string(ERROR_FLAG), false);
		LogWriter::getLogWriter()->addContents(std::string(NO_DATA_OR_INVALID_PATH), true);
		return false;
	}

	aReader* reader = ReaderFactory::makeReader(filePath);
	if (reader == NULL)
	{
		LogWriter::getLogWriter()->addContents(std::string(ERROR_FLAG), false);
		LogWriter::getLogWriter()->addContents(std::string(UNSUPPORTED_FORMAT), true);
		return false;
	}

	std::string fileName;
	std::string::size_type slashPosition = filePath.find_last_of("\\/");
	if (slashPosition == std::string::npos)
		fileName = filePath;
	else
		fileName = filePath.substr(slashPosition + 1, filePath.length() - slashPosition - 1);

	reader->setUnitScaleFactor(unitScaleFactor);

	if (!processDataFile(filePath, reader))
	{
		LogWriter::getLogWriter()->addContents(filePath, true);
		delete reader;
		processor->clear();
		return false;
	}

	delete reader;

	std::string::size_type dotPosition = fileName.rfind(".");
	std::string fullId = fileName.substr(0, dotPosition);
	if (!idPrefix.empty())
		fullId = idPrefix + fullId;

	if (!idSuffix.empty())
		fullId += idSuffix;

	processor->addAttribute(std::string(F4DID), fullId);

	F4DWriter writer(processor);
	writer.setWriteFolder(outputFolder);
	if (!writer.write())
	{
		LogWriter::getLogWriter()->addContents(filePath, true);
		processor->clear();
		return false;
	}

	processor->clear();

	return true;
}

void CConverterManager::processDataFolder(std::string& inputFolder)
{
	_finddata_t fd;
	long long handle;
	int result = 1;
	std::string fileFilter = inputFolder + std::string("/*.*");
	handle = _findfirst(fileFilter.c_str(), &fd);

	if (handle == -1)
		return;

	std::vector<std::string> dataFiles;
	std::vector<std::string> subFolders;
	while (result != -1)
	{
		if ((fd.attrib & _A_SUBDIR) == _A_SUBDIR)
		{
			if (std::string(fd.name) != std::string(".") && std::string(fd.name) != std::string(".."))
			{
				std::string subFolderFullPath = inputFolder + "/" + std::string(fd.name);
				subFolders.push_back(subFolderFullPath);
			}
		}
		else
		{
			dataFiles.push_back(std::string(fd.name));
		}

		result = _findnext(handle, &fd);
	}

	_findclose(handle);

	size_t subFolderCount = subFolders.size();
	for (size_t i = 0; i < subFolderCount; i++)
	{
		processDataFolder(subFolders[i]);
	}

	size_t dataFileCount = dataFiles.size();
	if (dataFileCount == 0)
		return;

	// TODO(khj 20180417) : NYI setup conversion configuration here
	// now, only set wheter do occlusion culling or not
	processor->setVisibilityIndexing(bOcclusionCulling);
	processor->setSkinLevel(skinLevel);
	// TODO(khj 20180417) end

	std::string outputFolder = outputFolderPath;

	std::string fullId;
	for (size_t i = 0; i < dataFileCount; i++)
	{
		// 1. raw data file을 하나씩 변환
		std::string dataFileFullPath = inputFolder + std::string("/") + dataFiles[i];

		aReader* reader = ReaderFactory::makeReader(dataFileFullPath);
		if (reader == NULL)
			continue;

		LogWriter::getLogWriter()->numberOfFilesToBeConverted += 1;
		reader->setUnitScaleFactor(unitScaleFactor);

		if (!processDataFile(dataFileFullPath, reader))
		{
			LogWriter::getLogWriter()->addContents(dataFiles[i], true);
			delete reader;
			processor->clear();
			continue;
		}

		delete reader;

		std::string::size_type dotPosition = dataFiles[i].rfind(".");
		fullId = dataFiles[i].substr(0, dotPosition);
		if (!idPrefix.empty())
			fullId = idPrefix + fullId;

		if (!idSuffix.empty())
			fullId += idSuffix;

		processor->addAttribute(std::string(F4DID), fullId);


		// 2. 변환 결과에서 bbox centerpoint를 로컬 원점으로 이동시키는 변환행렬 추출

		// 3. 변환 결과를 저장
		F4DWriter writer(processor);
		writer.setWriteFolder(outputFolder);
		if (!writer.write())
		{
			LogWriter::getLogWriter()->addContents(dataFiles[i], true);
			processor->clear();
			continue;
		}

		// 4. processor clear
		processor->clear();
		LogWriter::getLogWriter()->numberOfFilesConverted += 1;
	}
}

bool CConverterManager::writeIndexFile()
{
	F4DWriter writer(NULL);
	writer.setWriteFolder(outputFolderPath);
	writer.writeIndexFile();

	return true;
}

bool CConverterManager::processDataFile(std::string& filePath, aReader* reader)
{
	if (!reader->readRawDataFile(filePath))
	{
		LogWriter::getLogWriter()->addContents(std::string(ERROR_FLAG), false);
		LogWriter::getLogWriter()->addContents(std::string(CANNOT_LOAD_FILE), false);
		return false;
	}

	if (reader->getDataContainer().size() == 0)
	{
		LogWriter::getLogWriter()->addContents(std::string(ERROR_FLAG), false);
		LogWriter::getLogWriter()->addContents(std::string(NO_DATA_IN_RAW_DATA), false);
		return false;
	}

	if(!processor->proceedConversion(reader->getDataContainer(), reader->getTextureInfoContainer()))
	{
		LogWriter::getLogWriter()->addContents(std::string(ERROR_FLAG), false);
		LogWriter::getLogWriter()->addContents(std::string(CONVERSION_FAILURE), false);
		return false;
	}

	return true;
}

void CConverterManager::setProcessConfiguration(std::map<std::string, std::string>& arguments)
{
	if (arguments.find(InputFolder) != arguments.end())
	{
		bConversion = true;
		inputFolderPath = arguments[InputFolder];
		outputFolderPath = arguments[OutputFolder];

		if (arguments.find(PerformOC) != arguments.end())
		{
			if (arguments[PerformOC] == std::string("Y") || arguments[PerformOC] == std::string("y"))
				bOcclusionCulling = true;
			else
				bOcclusionCulling = false;
		}
		else
			bOcclusionCulling = false;

		if (arguments.find(UnitScaleFactor) != arguments.end())
			unitScaleFactor = std::stod(arguments[UnitScaleFactor]);

		if (arguments.find(SkinLevelNsm) != arguments.end())
			skinLevel = (unsigned char)(unsigned int)std::stoi(arguments[SkinLevelNsm]);
	}
	else
		bConversion = false;

	if (arguments.find(CreateIndex) != arguments.end())
	{
		if (arguments[CreateIndex] == std::string("Y") ||
			arguments[CreateIndex] == std::string("y"))
			bCreateIndices = true;
		else
			bCreateIndices = false;
	}
	else
		bCreateIndices = false;

	if (arguments.find(IdPrefix) != arguments.end())
		idPrefix = arguments[IdPrefix];

	if (arguments.find(IdSuffix) != arguments.end())
		idSuffix = arguments[IdSuffix];
}

void CConverterManager::process()
{
	if (bConversion)
	{
		if (processor->getSceneControlVariables()->m_width == 0 || processor->getSceneControlVariables()->m_height == 0 ||
			processor->getSceneControlVariables()->m_myhDC == 0 || processor->getSceneControlVariables()->m_hRC == 0)
			return;

		processDataFolder();
	}

	if (bCreateIndices)
		writeIndexFile();
}