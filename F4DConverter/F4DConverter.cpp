// F4DConverter.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"

#include <vector>
#include <string>
#include <algorithm>

#ifdef _WIN32
#include <Windows.h>
#endif

#include "argumentDefinition.h"
#include "converter/ConverterManager.h"
#include "converter/LogWriter.h"
#include "converter/util/utility.h"

#ifdef _WIN32

bool extractArguments(int argc, wchar_t* argv[], std::map<std::string, std::string>& arguments);

int wmain(int argc, wchar_t* argv[])
{

	// set CPU priority as 'high' before start
#ifdef _WIN32
	SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
#elif
#endif

	// basic argument validation
	if (argc < 2)
	{
		// TODO(khj 20180424) : NYI must print log messages through logger system
		printf("[Error]No Argument.\n");
		return -1;
	}

	// extract arguments
	std::map<std::string, std::string> arguments;
	if (!extractArguments(argc - 1, argv + 1, arguments))
	{
		// TODO(khj 20180424) : NYI must print log messages through logger system
		printf("[Error]Invalid Arguments.\n");
		return -2;
	}

	// arguments log
	printf("[Info]Arguments are following.\n");
	std::map<std::string, std::string>::iterator iter = arguments.begin();
	for (; iter != arguments.end(); iter++)
	{
		printf("%s : %s\n", iter->first.c_str(), iter->second.c_str());
	}

	std::string programPath = gaia3d::StringUtility::convertWideStringToUtf8(std::wstring(argv[0]));
	std::replace(programPath.begin(), programPath.end(), '\\', '/');
	arguments[ProgramPath] = programPath;

	// TODO(khj 20180424) : NYI must make log file through logger system
	// start log writer if needed
	if (arguments.find(LogFilePath) != arguments.end())
	{
		LogWriter::getLogWriter()->start();
		LogWriter::getLogWriter()->setFullPath(arguments[LogFilePath]);
	}

	// process
	if (CConverterManager::getConverterManager()->initialize(arguments))
	{
		CConverterManager::getConverterManager()->process();
		CConverterManager::getConverterManager()->uninitialize();

		// TODO(khj 20180424) : NYI must make log file through logger system
		// finish and save log if log writing started
		if (LogWriter::getLogWriter()->isStarted())
		{
			LogWriter::getLogWriter()->finish();
			LogWriter::getLogWriter()->save();
		}

		return 0;
	}
	else
	{
		// TODO(khj 20180424) : NYI must make log file through logger system
		// finish and save log if log writing started
		if (LogWriter::getLogWriter()->isStarted())
		{
			LogWriter::getLogWriter()->finish();
			LogWriter::getLogWriter()->save();
		}

		return -3;
	}
}

bool extractArguments(int argc, wchar_t* argv[], std::map<std::string, std::string>& arguments)
{
	std::vector<std::wstring> tokens;
	for (int i = 0; i < argc; i++)
		tokens.push_back(std::wstring(argv[i]));

	size_t tokenCount = tokens.size();
	if (tokenCount % 2 != 0)
	{
		printf("[ERROR][Invalid Arguments] Unmatched Key-Value pair\n");
		return false;
	}

	for (size_t i = 0; i < tokenCount; i++)
	{
		if (tokens[i].substr(0, 1) == std::wstring(L"#"))
		{
			if (i == tokenCount - 1 || tokens[i + 1].substr(0, 1) == std::wstring(L"#"))
				return false;

			if (tokens[i] == std::wstring(InputFolderW))
			{
				arguments[InputFolder] = gaia3d::StringUtility::convertWideStringToUtf8(tokens[i+1]);
				i++;
				continue;
			}

			if (tokens[i] == std::wstring(OutputFolderW))
			{
				arguments[OutputFolder] = gaia3d::StringUtility::convertWideStringToUtf8(tokens[i + 1]);
				i++;
				continue;
			}

			if (tokens[i] == std::wstring(LogFilePathW))
			{
				arguments[LogFilePath] = gaia3d::StringUtility::convertWideStringToUtf8(tokens[i + 1]);
				i++;
				continue;
			}

			if (tokens[i] == std::wstring(CreateIndexW))
			{
				arguments[CreateIndex] = gaia3d::StringUtility::convertWideStringToUtf8(tokens[i + 1]);
				i++;
				continue;
			}

			if (tokens[i] == std::wstring(IdPrefixW))
			{
				arguments[IdPrefix] = gaia3d::StringUtility::convertWideStringToUtf8(tokens[i + 1]);
				i++;
				continue;
			}

			if (tokens[i] == std::wstring(IdSuffixW))
			{
				arguments[IdSuffix] = gaia3d::StringUtility::convertWideStringToUtf8(tokens[i + 1]);
				i++;
				continue;
			}

			if (tokens[i] == std::wstring(PerformOCW))
			{
				arguments[PerformOC] = gaia3d::StringUtility::convertWideStringToUtf8(tokens[i + 1]);
				i++;
				continue;
			}

			if (tokens[i] == std::wstring(UnitScaleFactorW))
			{
				arguments[UnitScaleFactor] = gaia3d::StringUtility::convertWideStringToUtf8(tokens[i + 1]);
				i++;
				continue;
			}

			if (tokens[i] == std::wstring(SkinLevelNsmW))
			{
				arguments[SkinLevelNsm] = gaia3d::StringUtility::convertWideStringToUtf8(tokens[i + 1]);
				i++;
				continue;
			}

			if (tokens[i] == std::wstring(IsYAxisUpW))
			{
				arguments[IsYAxisUp] = gaia3d::StringUtility::convertWideStringToUtf8(tokens[i + 1]);
				i++;
				continue;
			}

			if (tokens[i] == std::wstring(ReferenceLonLatW))
			{
				arguments[ReferenceLonLat] = gaia3d::StringUtility::convertWideStringToUtf8(tokens[i + 1]);
				i++;
				continue;
			}

			if (tokens[i] == std::wstring(MeshTypeW))
			{
				arguments[MeshType] = gaia3d::StringUtility::convertWideStringToUtf8(tokens[i + 1]);
				i++;
				continue;
			}

			if (tokens[i] == std::wstring(AlignToW))
			{
				arguments[AlignTo] = gaia3d::StringUtility::convertWideStringToUtf8(tokens[i + 1]);
				i++;
				continue;
			}

			if (tokens[i] == std::wstring(EpsgW))
			{
				arguments[Epsg] = gaia3d::StringUtility::convertWideStringToUtf8(tokens[i + 1]);
				i++;
				continue;
			}

			if (tokens[i] == std::wstring(OffsetXW))
			{
				arguments[OffsetX] = gaia3d::StringUtility::convertWideStringToUtf8(tokens[i + 1]);
				i++;
				continue;
			}

			if (tokens[i] == std::wstring(OffsetYW))
			{
				arguments[OffsetY] = gaia3d::StringUtility::convertWideStringToUtf8(tokens[i + 1]);
				i++;
				continue;
			}

			if (tokens[i] == std::wstring(OffsetZW))
			{
				arguments[OffsetZ] = gaia3d::StringUtility::convertWideStringToUtf8(tokens[i + 1]);
				i++;
				continue;
			}

			if (tokens[i] == std::wstring(ProjectNameW))
			{
				arguments[ProjectName] = gaia3d::StringUtility::convertWideStringToUtf8(tokens[i + 1]);
				i++;
				continue;
			}

			if (tokens[i] == std::wstring(SplitFilterW))
			{
				arguments[SplitFilter] = gaia3d::StringUtility::convertWideStringToUtf8(tokens[i + 1]);
				i++;
				continue;
			}
		}
		else
		{
			printf("[ERROR][Invalid Arguments] Argument sentence error.\n");
			return false;
		}
	}

	if (arguments.find(OutputFolder) == arguments.end())
	{
		printf("[ERROR][Invalid Arguments] #outputFolder MUST be entered.\n");
		return false;
	}

	if (arguments.find(InputFolder) == arguments.end() && arguments.find(CreateIndex) == arguments.end())
	{
		printf("[ERROR][Invalid Arguments] One of #inputFolder and #indexing MUST be entered.\n");
		return false;
	}

	if (arguments.find(InputFolder) != arguments.end())
	{
		if (arguments.find(LogFilePath) == arguments.end() || arguments.find(MeshType) == arguments.end())
		{
			printf("[ERROR][Invalid Arguments] #log and #meshType are MANDATORY when #inputFolder is used.\n");
			return false;
		}
	}

	if (arguments.find(CreateIndex) != arguments.end())
	{
		if (arguments[CreateIndex] != std::string("Y") &&
			arguments[CreateIndex] != std::string("y") &&
			arguments[CreateIndex] != std::string("N") &&
			arguments[CreateIndex] != std::string("n"))
		{
			printf("[ERROR][Invalid Arguments] Value of #indexing MUST be one of [Y, y, N, n].\n");
			return false;
		}
	}

	if (arguments.find(PerformOC) != arguments.end())
	{
		if (arguments[PerformOC] != std::string("Y") &&
			arguments[PerformOC] != std::string("y") &&
			arguments[PerformOC] != std::string("N") &&
			arguments[PerformOC] != std::string("n"))
		{
			printf("[ERROR][Invalid Arguments] Value of #oc MUST be one of [Y, y, N, n].\n");
			return false;
		}
			
	}

	if (arguments.find(UnitScaleFactor) != arguments.end())
	{
		try
		{
			double scaleFactor = std::stod(arguments[UnitScaleFactor]);

			if (scaleFactor < 0.001)
			{
				printf("[ERROR][Invalid Arguments] Value of #usf MUST be over or equal to 0.001.\n");
				return false;
			}
		}
		catch (const std::invalid_argument& error)
		{
			std::string errorMessage = error.what();
			printf("[ERROR][Invalid Arguments] Value of #usf : %s.\n", errorMessage.c_str());
			return false;
		}
		catch (const std::out_of_range& error)
		{
			std::string errorMessage = error.what();
			printf("[ERROR][Invalid Arguments] Value of #usf : %s.\n", errorMessage.c_str());
			return false;
		}
	}

	if (arguments.find(SkinLevelNsm) != arguments.end())
	{
		try
		{
			std::string skinLevel = arguments[SkinLevelNsm];
			int nSkinLevel = std::stoi(skinLevel);

			if (nSkinLevel > 6 || nSkinLevel < 1)
			{
				printf("[ERROR][Invalid Arguments] Value of #skinLevel MUST be one of [1, 2, 3, 4, 5, 6].\n");
				return false;
			}
		}
		catch (const std::invalid_argument& error)
		{
			std::string errorMessage = error.what();
			printf("[ERROR][Invalid Arguments] Value of #skinLevel : %s.\n", errorMessage.c_str());
			return false;
		}
		catch (const std::out_of_range& error)
		{
			std::string errorMessage = error.what();
			printf("[ERROR][Invalid Arguments] Value of #skinLevel : %s.\n", errorMessage.c_str());
			return false;
		}
	}

	if (arguments.find(IsYAxisUp) != arguments.end())
	{
		if (arguments[IsYAxisUp] != std::string("Y") &&
			arguments[IsYAxisUp] != std::string("y") &&
			arguments[IsYAxisUp] != std::string("N") &&
			arguments[IsYAxisUp] != std::string("n"))
		{
			printf("[ERROR][Invalid Arguments] Value of #isYAxisUp MUST be one of [Y, y, N, n].\n");
			return false;
		}
	}

	if (arguments.find(ReferenceLonLat) != arguments.end() &&
		arguments.find(Epsg) != arguments.end())
	{
		printf("[ERROR][Invalid Arguments] #epsg and #referenceLonLat CANNOT be used at the same time.\n");
		return false;
	}

	if (arguments.find(ReferenceLonLat) != arguments.end() &&
		(arguments.find(OffsetX) != arguments.end() || arguments.find(OffsetY) != arguments.end()))
	{
		printf("[ERROR][Invalid Arguments] #referenceLonLan CANNOT be used with #offsetX or #offsetY.\n");
		return false;
	}

	if (arguments.find(AlignTo) != arguments.end() &&
		(arguments.find(Epsg) != arguments.end() || arguments.find(ReferenceLonLat) != arguments.end()))
	{
		printf("[ERROR][Invalid Arguments] #alignTo CANNOT be used with #epsg or #referenceLonLat.\n");
		return false;
	}

	if (arguments.find(ReferenceLonLat) != arguments.end())
	{
		size_t lonLatLength = arguments[ReferenceLonLat].length();
		char* original = new char[lonLatLength + 1];
		memset(original, 0x00, sizeof(char)*(lonLatLength + 1));
		memcpy(original, arguments[ReferenceLonLat].c_str(), lonLatLength);
		char* lon = std::strtok(original, ",");
		if (lon == NULL)
		{
			delete[] original;
			printf("[ERROR][Invalid Arguments] Value of #referenceLonLat MUST be of [numericValue,numericValue] format.\n");
			return false;
		}
		char* lat = std::strtok(NULL, ",");
		if (lat == NULL)
		{
			delete[] original;
			printf("[ERROR][Invalid Arguments] Value of #referenceLonLat MUST be of [numericValue,numericValue] format.\n");
			return false;
		}

		try
		{
			double refLon = std::stod(lon);
			double refLat = std::stod(lat);
		}
		catch (const std::invalid_argument& error)
		{
			std::string errorMessage = error.what();
			delete[] original;
			printf("[ERROR][Invalid Arguments] Value of #referenceLonLat : %s\n", errorMessage.c_str());
			return false;
		}
		catch (const std::out_of_range& error)
		{
			std::string errorMessage = error.what();
			delete[] original;
			printf("[ERROR][Invalid Arguments] Value of #referenceLonLat : %s\n", errorMessage.c_str());
			return false;
		}

		delete[] original;
	}

	if (arguments.find(MeshType) != arguments.end())
	{
		try
		{
			int meshType = std::stoi(arguments[MeshType]);

			//if(meshType != 1 && meshType != 2) // AIT version
			//if (meshType != 0) // release version
			//if (meshType != 2 && meshType != 0) // for romania
			if (meshType > 3 || meshType < 0) // for full type.
			{
				printf("[ERROR][Invalid Arguments] Value of #meshType MUST be one of [0, 1, 2, 3].\n");
				return false;
			}
		}
		catch (const std::invalid_argument& error)
		{
			std::string errorMessage = error.what();
			printf("[ERROR][Invalid Arguments] Value of #meshType : %s.\n", errorMessage.c_str());
			return false;
		}
		catch (const std::out_of_range& error)
		{
			std::string errorMessage = error.what();
			printf("[ERROR][Invalid Arguments] Value of #meshType : %s.\n", errorMessage.c_str());
			return false;
		}
	}
	
	if (arguments.find(AlignTo) != arguments.end())
	{
		try
		{
			int meshType = std::stoi(arguments[MeshType]);

			//if(meshType != 1 && meshType != 2) // AIT version
			//if (meshType != 0) // release version
			//if (meshType != 2 && meshType != 0) // for romania
			if (meshType > 1 || meshType < 0) // for full type.
			{
				printf("[ERROR][Invalid Arguments] Value of #alignTo MUST be one of [0, 1].\n");
				return false;
			}
		}
		catch (const std::invalid_argument& error)
		{
			std::string errorMessage = error.what();
			printf("[ERROR][Invalid Arguments] Value of #alignTo : %s.\n", errorMessage.c_str());
			return false;
		}
		catch (const std::out_of_range& error)
		{
			std::string errorMessage = error.what();
			printf("[ERROR][Invalid Arguments] Value of #alignTo : %s.\n", errorMessage.c_str());
			return false;
		}
	}

	if (arguments.find(Epsg) != arguments.end())
	{
		try
		{
			int nEpsgCode = std::stoi(arguments[Epsg]);
		}
		catch (const std::invalid_argument& error)
		{
			std::string errorMessage = error.what();
			printf("[ERROR][Invalid Arguments] Value of #epsg : %s.\n", errorMessage.c_str());
			return false;
		}
		catch (const std::out_of_range& error)
		{
			std::string errorMessage = error.what();
			printf("[ERROR][Invalid Arguments] Value of #epsg : %s.\n", errorMessage.c_str());
			return false;
		}
	}

	if (arguments.find(OffsetX) != arguments.end())
	{
		try
		{
			double offsetX = std::stod(arguments[OffsetX]);
		}
		catch (const std::invalid_argument& error)
		{
			std::string errorMessage = error.what();
			printf("[ERROR][Invalid Arguments] Value of #offsetX : %s.\n", errorMessage.c_str());
			return false;
		}
		catch (const std::out_of_range& error)
		{
			std::string errorMessage = error.what();
			printf("[ERROR][Invalid Arguments] Value of #offsetX : %s.\n", errorMessage.c_str());
			return false;
		}
	}

	if (arguments.find(OffsetY) != arguments.end())
	{
		try
		{
			double offsetY = std::stod(arguments[OffsetY]);
		}
		catch (const std::invalid_argument& error)
		{
			std::string errorMessage = error.what();
			printf("[ERROR][Invalid Arguments] Value of #offsetY : %s.\n", errorMessage.c_str());
			return false;
		}
		catch (const std::out_of_range& error)
		{
			std::string errorMessage = error.what();
			printf("[ERROR][Invalid Arguments] Value of #offsetY : %s.\n", errorMessage.c_str());
			return false;
		}
	}

	if (arguments.find(OffsetZ) != arguments.end())
	{
		try
		{
			double offsetZ = std::stod(arguments[OffsetZ]);
		}
		catch (const std::invalid_argument& error)
		{
			std::string errorMessage = error.what();
			printf("[ERROR][Invalid Arguments] Value of #offsetZ : %s.\n", errorMessage.c_str());
			return false;
		}
		catch (const std::out_of_range& error)
		{
			std::string errorMessage = error.what();
			printf("[ERROR][Invalid Arguments] Value of #offsetZ : %s.\n", errorMessage.c_str());
			return false;
		}
	}

	if (arguments.find(ProjectName) != arguments.end())
	{
		// , \ / : * ? " < > |  can't be used in the string of project name
		if (arguments[ProjectName].find(std::string(",")) != std::string::npos ||
			arguments[ProjectName].find(std::string("\\")) != std::string::npos ||
			arguments[ProjectName].find(std::string("/")) != std::string::npos || 
			arguments[ProjectName].find(std::string(":")) != std::string::npos || 
			arguments[ProjectName].find(std::string("*")) != std::string::npos || 
			arguments[ProjectName].find(std::string("?")) != std::string::npos || 
			arguments[ProjectName].find(std::string("\"")) != std::string::npos || 
			arguments[ProjectName].find(std::string("<")) != std::string::npos || 
			arguments[ProjectName].find(std::string(">")) != std::string::npos || 
			arguments[ProjectName].find(std::string("|")) != std::string::npos )
		{
			printf("[ERROR][Invalid Arguments] One of characters [ , \\ / : * ? \" < > | ] can't be used in project name : %s.\n", arguments[ProjectName].c_str());
			return false;
		}
	}

	return true;
}
#else

bool extractArguments(int argc, char* argv[], std::map<std::string, std::string>& arguments);

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		// TODO(khj 20180424) : NYI must print log messages through logger system
		printf("[Error]No Argument.\n");
		return -1;
	}

	// extract arguments
	std::map<std::string, std::string> arguments;
	if (!extractArguments(argc - 1, argv + 1, arguments))
	{
		// TODO(khj 20180424) : NYI must print log messages through logger system
		printf("[Error]Invalid Arguments.\n");
		return -1;
	}

	std::string programPath = std::string(argv[0]);
	std::replace(programPath.begin(), programPath.end(), '\\', '/');
	arguments[ProgramPath] = programPath;

	// TODO(khj 20180424) : NYI must make log file through logger system
	// start log writer if needed
	if (arguments.find(LogFilePath) != arguments.end())
	{
		LogWriter::getLogWriter()->start();
		LogWriter::getLogWriter()->setFullPath(arguments[LogFilePath]);
	}

	// process
	if (CConverterManager::getConverterManager()->initialize(arguments))
	{
		CConverterManager::getConverterManager()->process();
		CConverterManager::getConverterManager()->uninitialize();
	}

	// TODO(khj 20180424) : NYI must make log file through logger system
	// finish and save log if log writing started
	if (LogWriter::getLogWriter()->isStarted())
	{
		LogWriter::getLogWriter()->finish();
		LogWriter::getLogWriter()->save();
	}

	return 0;
}

bool extractArguments(int argc, char* argv[], std::map<std::string, std::string>& arguments)
{
	std::vector<std::string> tokens;
	for (int i = 0; i < argc; i++)
		tokens.push_back(std::string(argv[i]));

	size_t tokenCount = tokens.size();
	for (size_t i = 0; i < tokenCount; i++)
	{
		if (tokens[i].substr(0, 1) == std::string("#"))
		{
			if (i == tokenCount - 1 || tokens[i + 1].substr(0, 1) == std::string("#"))
			{
				return false;
			}

			int neededLength;
			char* param;
			if (tokens[i] == std::string(InputFolder))
			{
				arguments[InputFolder] = tokens[i + 1];
				i++;
				continue;
			}

			if (tokens[i] == std::string(OutputFolder))
			{
				arguments[OutputFolder] = tokens[i + 1];
				i++;
				continue;
			}

			if (tokens[i] == std::string(LogFilePath))
			{
				arguments[LogFilePath] = tokens[i + 1];
				i++;
				continue;
			}

			if (tokens[i] == std::string(CreateIndex))
			{
				arguments[CreateIndex] = tokens[i + 1];
				i++;
				continue;
			}

			if (tokens[i] == std::string(IdPrefix))
			{
				arguments[IdPrefix] = tokens[i + 1];
				i++;
				continue;
			}

			if (tokens[i] == std::string(IdSuffix))
			{
				arguments[IdSuffix] = tokens[i + 1];
				i++;
				continue;
			}

			if (tokens[i] == std::string(PerformOC))
			{
				arguments[PerformOC] = tokens[i + 1];
				i++;
				continue;
			}

			if (tokens[i] == std::string(UnitScaleFactor))
			{
				arguments[UnitScaleFactor] = tokens[i + 1];
				i++;
				continue;
			}

			if (tokens[i] == std::string(SkinLevelNsm))
			{
				arguments[SkinLevelNsm] = tokens[i + 1];
				i++;
				continue;
			}

			if (tokens[i] == std::string(IsYAxisUp))
			{
				arguments[IsYAxisUp] = tokens[i + 1];
				i++;
				continue;
			}

			if (tokens[i] == std::string(ReferenceLonLat))
			{
				arguments[ReferenceLonLat] = tokens[i + 1];
				i++;
				continue;
			}

			if (tokens[i] == std::string(MeshType))
			{
				arguments[MeshType] = tokens[i + 1];
				i++;
				continue;
			}

			if (tokens[i] == std::string(AlignTo))
			{
				arguments[AlignTo] = tokens[i + 1];
				i++;
				continue;
			}

			if (tokens[i] == std::string(Epsg))
			{
				arguments[Epsg] = tokens[i + 1];
				i++;
				continue;
			}

			if (tokens[i] == std::string(OffsetX))
			{
				arguments[OffsetX] = tokens[i + 1];
				i++;
				continue;
			}

			if (tokens[i] == std::string(OffsetY))
			{
				arguments[OffsetY] = tokens[i + 1];
				i++;
				continue;
			}

			if (tokens[i] == std::string(OffsetZ))
			{
				arguments[OffsetZ] = tokens[i + 1];
				i++;
				continue;
			}

			if (tokens[i] == std::string(ProjectName))
			{
				arguments[ProjectName] = tokens[i + 1];
				i++;
				continue;
			}

			if (tokens[i] == std::wstring(SplitFilter))
			{
				arguments[SplitFilter] = tokens[i + 1];
				i++;
				continue;
			}
		}
		else
			return false;
	}

	if (arguments.find(OutputFolder) == arguments.end())
		return false;

	if (arguments.find(InputFolder) == arguments.end() && arguments.find(CreateIndex) == arguments.end())
		return false;

	if (arguments.find(InputFolder) != arguments.end())
	{
		if (arguments.find(LogFilePath) == arguments.end())
			return false;
	}

	if (arguments.find(CreateIndex) != arguments.end())
	{
		if (arguments[CreateIndex] != std::string("Y") &&
			arguments[CreateIndex] != std::string("y") &&
			arguments[CreateIndex] != std::string("N") &&
			arguments[CreateIndex] != std::string("n"))
			return false;
	}

	if (arguments.find(PerformOC) != arguments.end())
	{
		if (arguments[PerformOC] != std::string("Y") &&
			arguments[PerformOC] != std::string("y") &&
			arguments[PerformOC] != std::string("N") &&
			arguments[PerformOC] != std::string("n"))
			return false;
	}

	if (arguments.find(UnitScaleFactor) != arguments.end())
	{
		try
		{
			double scaleFactor = std::stod(arguments[UnitScaleFactor]);

			if (scaleFactor < 0.001)
				return false;
		}
		catch (const std::invalid_argument& error)
		{
			std::string errorMessage = error.what();
			return false;
		}
		catch (const std::out_of_range& error)
		{
			std::string errorMessage = error.what();
			return false;
		}
	}

	if (arguments.find(SkinLevelNsm) != arguments.end())
	{
		try
		{
			std::string wskinLevel = arguments[SkinLevelNsm];
			int skinLevel = std::stoi(wskinLevel);

			if (skinLevel > 3 || skinLevel < 1)
				return false;
		}
		catch (const std::invalid_argument& error)
		{
			std::string errorMessage = error.what();
			return false;
		}
		catch (const std::out_of_range& error)
		{
			std::string errorMessage = error.what();
			return false;
		}
	}

	if (arguments.find(IsYAxisUp) != arguments.end())
	{
		if (arguments[IsYAxisUp] != std::string("Y") &&
			arguments[IsYAxisUp] != std::string("y") &&
			arguments[IsYAxisUp] != std::string("N") &&
			arguments[IsYAxisUp] != std::string("n"))
			return false;
	}

	if (arguments.find(ReferenceLonLat) != arguments.end())
	{
		size_t lonLatLength = arguments[ReferenceLonLat].length();
		char* original = new char[lonLatLength + 1];
		memset(original, 0x00, sizeof(char)*(lonLatLength + 1));
		memcpy(original, arguments[ReferenceLonLat].c_str(), lonLatLength);
		char* lon = std::strtok(original, ",");
		if (lon == NULL)
		{
			delete[] original;
			return false;
		}
		char* lat = std::strtok(NULL, ",");
		if (lat == NULL)
		{
			delete[] original;
			return false;
		}

		delete[] original;

		try
		{
			double refLon = std::stod(lon);
			double refLat = std::stod(lat);
		}
		catch (const std::invalid_argument& error)
		{
			std::string errorMessage = error.what();
			return false;
		}
		catch (const std::out_of_range& error)
		{
			std::string errorMessage = error.what();
			return false;
		}
	}

	if (arguments.find(Epsg) != arguments.end())
	{
		try
		{
			int nEpsgCode = std::stoi(arguments[Epsg]);
		}
		catch (const std::invalid_argument& error)
		{
			std::string errorMessage = error.what();
			return false;
		}
		catch (const std::out_of_range& error)
		{
			std::string errorMessage = error.what();
			return false;
		}
	}

	if (arguments.find(OffsetX) != arguments.end())
	{
		try
		{
			double offsetX = std::stod(arguments[OffsetX]);
		}
		catch (const std::invalid_argument& error)
		{
			std::string errorMessage = error.what();
			return false;
		}
		catch (const std::out_of_range& error)
		{
			std::string errorMessage = error.what();
			return false;
		}
	}

	if (arguments.find(OffsetY) != arguments.end())
	{
		try
		{
			double offsetY = std::stod(arguments[OffsetY]);
		}
		catch (const std::invalid_argument& error)
		{
			std::string errorMessage = error.what();
			return false;
		}
		catch (const std::out_of_range& error)
		{
			std::string errorMessage = error.what();
			return false;
		}
	}

	if (arguments.find(OffsetZ) != arguments.end())
	{
		try
		{
			double offsetY = std::stod(arguments[OffsetZ]);
		}
		catch (const std::invalid_argument& error)
		{
			std::string errorMessage = error.what();
			return false;
		}
		catch (const std::out_of_range& error)
		{
			std::string errorMessage = error.what();
			return false;
		}
	}

	if (arguments.find(ProjectName) != arguments.end())
	{
		// , \ / : * ? " < > |  can't be used in the string of project name
		if (arguments[ProjectName].find(std::string(",")) != std::string::npos ||
			arguments[ProjectName].find(std::string("\\")) != std::string::npos ||
			arguments[ProjectName].find(std::string("/")) != std::string::npos ||
			arguments[ProjectName].find(std::string(":")) != std::string::npos ||
			arguments[ProjectName].find(std::string("*")) != std::string::npos ||
			arguments[ProjectName].find(std::string("?")) != std::string::npos ||
			arguments[ProjectName].find(std::string("\"")) != std::string::npos ||
			arguments[ProjectName].find(std::string("<")) != std::string::npos ||
			arguments[ProjectName].find(std::string(">")) != std::string::npos ||
			arguments[ProjectName].find(std::string("|")) != std::string::npos)
		{
			printf("[ERROR][Invalid Arguments] One of characters [ , \\ / : * ? \" < > | ] can't be used in project name : %s.\n", arguments[ProjectName].c_str());
			return false;
		}
	}

	return true;
}

#endif


