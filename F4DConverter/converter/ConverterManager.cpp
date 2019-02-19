// ./converter/ConverterManager.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "ConverterManager.h"

#include <io.h>
#include <sys/stat.h>

#include "proj_api.h"

#include "../argumentDefinition.h"
#include "./reader/ReaderFactory.h"
#include "./process/ConversionProcessor.h"
#include "./process/SceneControlVariables.h"
#include "./writer/F4DWriter.h"
#include "./util/utility.h"
#include "./util/json/json.h"

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

	bYAxisUp = false;

	bAlignPostionToCenter = false;

	meshType = 0;

	bUseReferenceLonLat = false;

	bUseEpsg = false;
}

CConverterManager::~CConverterManager()
{
	if(processor != NULL)
		delete processor;
}


// CConverterManager 멤버 함수

bool CConverterManager::initialize(std::map<std::string, std::string>& arguments)
{
	std::string programPath = arguments[ProgramPath];
	std::string programFolder;

	size_t lastSlashPos = programPath.rfind('/');
	programFolder = programPath.substr(0, lastSlashPos + 1);
	std::string projLibPath = programFolder + std::string("proj");
	const char* epsgPath = projLibPath.c_str();
	pj_set_searchpath(1, &epsgPath);

	if (!setProcessConfiguration(arguments))
	{
		pj_set_searchpath(0, NULL);
		return false;
	}

	if(processor == NULL)
		processor = new ConversionProcessor();

	return processor->initialize();
}

void CConverterManager::uninitialize()
{
	pj_set_searchpath(0, NULL);

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

	std::map<std::string, std::string> targetFiles;
	collectTargetFiles(inputFolder, targetFiles);
	if (targetFiles.empty())
	{
		LogWriter::getLogWriter()->addContents(std::string(ERROR_FLAG), false);
		LogWriter::getLogWriter()->addContents(std::string(NO_DATA_OR_INVALID_PATH), false);
		LogWriter::getLogWriter()->addContents(inputFolder, true);
		return false;
	}

	processDataFiles(targetFiles);

	return true;
}

/*
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
*/

void CConverterManager::processDataFiles(std::map<std::string, std::string>& targetFiles)
{

	// TODO(khj 20180417) : NYI setup conversion configuration here
	// now, only set wheter do occlusion culling or not
	processor->setVisibilityIndexing(bOcclusionCulling);
	processor->setSkinLevel(skinLevel);
	processor->setYAxisUp(bYAxisUp);
	processor->setAlignPostionToCenter(bAlignPostionToCenter);
	processor->setMeshType(meshType);
	switch (meshType)
	{
	case 1: // single random-shaped 3d mesh
		processor->setSkinLevel(50);
		processor->setLeafSpatialOctreeSize(40.0f);
		break;
	case 2: // splitted random-shaped 3d mesh
		processor->setSkinLevel(51);
		processor->setLeafSpatialOctreeSize(40.0f);
		break;
	case 3: // point cloud
		processor->setLeafSpatialOctreeSize(40.0f);
		processor->setAlignPostionToCenter(true);
		break;
	}
	// TODO(khj 20180417) end

	//// hard-cord for japan(AIST) realistic mesh and romania data
	//processor->setVisibilityIndexing(false);
	/*processor->setYAxisUp(false);
	processor->setAlignPostionToCenter(bAlignPostionToCenter);
	processor->setMeshType(meshType);
	switch(meshType)
	{
	case 1:
		processor->setSkinLevel(50);
		break;
	case 2:
		processor->setSkinLevel(51);
		break;
	}
	processor->setLeafSpatialOctreeSize(40.0f);*/

	//// hard-cord for new york citygml
	//processor->setVisibilityIndexing(false);
	//processor->setUseNsm(false);
	//processor->setYAxisUp(true);
	//processor->setAlignPostionToCenter(true);
	//processor->setLeafSpatialOctreeSize(422.0f);

	std::string outputFolder = outputFolderPath;

	std::string fullId;
	std::map<std::string, double> centerXs, centerYs;
	std::map<std::string, std::string>::iterator iter = targetFiles.begin();
	for (; iter != targetFiles.end(); iter++)
	{
		// 1. convert raw data files repectively
		std::string dataFile = iter->first;
		std::string dataFileFullPath = iter->second;

		aReader* reader = ReaderFactory::makeReader(dataFileFullPath);
		if (reader == NULL)
			continue;

		printf("===== Start processing this file : %s\n", dataFile.c_str());

		LogWriter::getLogWriter()->numberOfFilesToBeConverted += 1;
		reader->setUnitScaleFactor(unitScaleFactor);

		// 1-1. inject coordinate information into reader before reading
		if (bUseEpsg)
			reader->injectSrsInfo(epsgCode);

		if (bUseReferenceLonLat)
			reader->injectOringinInfo(referenceLon, referenceLat);

		if (!processDataFile(dataFileFullPath, reader))
		{
			LogWriter::getLogWriter()->addContents(dataFileFullPath, true);
			delete reader;
			processor->clear();
			continue;
		}

		// 1.1 get embedded representative lon/lat of original dataset
		if (reader->doesHasGeoReferencingInfo())
		{
			double lon, lat;
			reader->getGeoReferencingInfo(lon, lat);
			centerXs[dataFile] = lon;
			centerYs[dataFile] = lat;
		}

		delete reader;

		std::string::size_type dotPosition = dataFile.rfind(".");
		fullId = dataFile.substr(0, dotPosition);
		if (!idPrefix.empty())
			fullId = idPrefix + fullId;

		if (!idSuffix.empty())
			fullId += idSuffix;

		processor->addAttribute(std::string(F4DID), fullId);

		// 2. save the result
		F4DWriter writer(processor);
		writer.setWriteFolder(outputFolder);
		if (!writer.write())
		{
			LogWriter::getLogWriter()->addContents(dataFileFullPath, true);
			processor->clear();
			continue;
		}

		// 3. processor clear
		processor->clear();
		LogWriter::getLogWriter()->numberOfFilesConverted += 1;
	}

	// save representative lon / lat of F4D if a reference file exists
	if (!centerXs.empty() && !centerYs.empty())
	{
		writeRepresentativeLonLatOfEachData(centerXs, centerYs);
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

bool CConverterManager::setProcessConfiguration(std::map<std::string, std::string>& arguments)
{
	if (arguments.find(InputFolder) != arguments.end())
	{
		bConversion = true;
		inputFolderPath = arguments[InputFolder];

		if (arguments.find(MeshType) != arguments.end())
		{
			meshType = std::stoi(arguments[MeshType]);
		}

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

	if (arguments.find(OutputFolder) != arguments.end())
	{
		outputFolderPath = arguments[OutputFolder];
	}

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

	if (arguments.find(IsYAxisUp) != arguments.end())
	{
		if (arguments[IsYAxisUp] == std::string("Y") ||
			arguments[IsYAxisUp] == std::string("y"))
			bYAxisUp = true;
		else
			bYAxisUp = false;
	}

	if (arguments.find(ReferenceLonLat) != arguments.end())
	{
		size_t lonLatLength = arguments[ReferenceLonLat].length();
		char* original = new char[lonLatLength + 1];
		memset(original, 0x00, sizeof(char)*(lonLatLength + 1));
		memcpy(original, arguments[ReferenceLonLat].c_str(), lonLatLength);
		char* lon = std::strtok(original, ",");
		char* lat = std::strtok(NULL, ",");
		referenceLon = std::stod(lon);
		referenceLat = std::stod(lat);
		delete[] original;
		bUseReferenceLonLat = true;
	}

	if (arguments.find(AlignToCenter) != arguments.end())
	{
		if (arguments[AlignToCenter] == std::string("Y") ||
			arguments[AlignToCenter] == std::string("y"))
			bAlignPostionToCenter = true;
		else
			bAlignPostionToCenter = false;
	}

	if (arguments.find(Epsg) != arguments.end())
	{
		epsgCode = arguments[Epsg];

		std::string proj4String = std::string("+init=epsg:") + epsgCode;

		projPJ pjEpsg;
		pjEpsg = pj_init_plus(proj4String.c_str());
		if (pjEpsg == NULL)
		{
			char* errorMsg = pj_strerrno(pj_errno);
			LogWriter::getLogWriter()->addContents(std::string(UNSUPPERTED_EPSG_CODE), false);
			LogWriter::getLogWriter()->addContents(epsgCode, true);

			return false;
		}

		bUseEpsg = true;
	}

	return true;
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

void CConverterManager::collectTargetFiles(std::string& inputFolder, std::map<std::string, std::string>& targetFiles)
{
	_finddata_t fd;
	long long handle;
	int result = 1;
	std::string fileFilter = inputFolder + std::string("/*.*");
	handle = _findfirst(fileFilter.c_str(), &fd);

	if (handle == -1)
		return;

	std::vector<std::string> subFolders;
	while (result != -1)
	{
		if ((fd.attrib & _A_SUBDIR) == _A_SUBDIR)
		{
			if (std::string(fd.name) != std::string(".") && std::string(fd.name) != std::string(".."))
			{
				std::string subFolder = std::string(fd.name);
#ifdef _WIN32
				subFolder = gaia3d::StringUtility::convertMultibyteToUtf8(subFolder);
#endif
				std::string subFolderFullPath = inputFolder + "/" + subFolder;
				subFolders.push_back(subFolderFullPath);
			}
		}
		else
		{
			std::string dataFile = std::string(fd.name);
#ifdef _WIN32
			dataFile = gaia3d::StringUtility::convertMultibyteToUtf8(dataFile);
#endif
			std::string dataFileFullPath = inputFolder + std::string("/") + dataFile;
			targetFiles[dataFile] = dataFileFullPath;
		}

		result = _findnext(handle, &fd);
	}

	_findclose(handle);

	size_t subFolderCount = subFolders.size();
	for (size_t i = 0; i < subFolderCount; i++)
	{
		collectTargetFiles(subFolders[i], targetFiles);
	}
}

void CConverterManager::writeRepresentativeLonLatOfEachData(std::map<std::string, double>& posXs, std::map<std::string, double>& posYs)
{
	Json::Value arrayNode(Json::arrayValue);

	std::map<std::string, double>::iterator iter = posXs.begin();
	for (; iter != posXs.end(); iter++)
	{
		Json::Value f4d(Json::objectValue);

		// data_key
		std::string fileName = iter->first;
		std::string::size_type dotPosition = fileName.rfind(".");
		std::string dataKey = fileName.substr(0, dotPosition);
		f4d["data_key"] = dataKey;

		// longitude and latitude
		f4d["longitude"] = iter->second;
		f4d["latitude"] = posYs[iter->first];

		arrayNode.append(f4d);
	}

	Json::StyledWriter writer;
	std::string documentContent = writer.write(arrayNode);
	std::string lonLatFileFullPath = outputFolderPath + std::string("/lonsLats.json");
	FILE* file = NULL;
	file = fopen(lonLatFileFullPath.c_str(), "wt");
	fprintf(file, "%s", documentContent.c_str());
	fclose(file);
}