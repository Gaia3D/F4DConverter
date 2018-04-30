// F4DConverter.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"

#include <vector>
#include <string>

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
		return -1;
	}

	// TODO(khj 20180424) : NYI must make log file through logger system
	// start log writer if needed
	if (arguments.find(LogFilePath) != arguments.end())
	{
		LogWriter::getLogWriter()->start();
		LogWriter::getLogWriter()->setFullPath(arguments[LogFilePath]);
	}

	// process
	CConverterManager::getConverterManager()->initialize();
	CConverterManager::getConverterManager()->setProcessConfiguration(arguments);
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

bool extractArguments(int argc, wchar_t* argv[], std::map<std::string, std::string>& arguments)
{
	std::vector<std::wstring> tokens;
	for (int i = 0; i < argc; i++)
		tokens.push_back(std::wstring(argv[i]));

	size_t tokenCount = tokens.size();
	for (size_t i = 0; i < tokenCount; i++)
	{
		if (tokens[i].substr(0, 1) == std::wstring(L"-"))
		{
			if (i == tokenCount - 1 || tokens[i + 1].substr(0, 1) == std::wstring(L"-"))
			{
				return false;
			}

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
			std::string skinLevel = arguments[SkinLevelNsm];
			int nSkinLevel = std::stoi(skinLevel);

			if (nSkinLevel > 3 || nSkinLevel < 1)
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

	// TODO(khj 20180424) : NYI must make log file through logger system
	// start log writer if needed
	if (arguments.find(LogFilePath) != arguments.end())
	{
		LogWriter::getLogWriter()->start();
		LogWriter::getLogWriter()->setFullPath(arguments[LogFilePath]);
	}

	// process
	CConverterManager::getConverterManager()->initialize();
	CConverterManager::getConverterManager()->setProcessConfiguration(arguments);
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

bool extractArguments(int argc, char* argv[], std::map<std::string, std::string>& arguments)
{
	std::vector<std::string> tokens;
	for (int i = 0; i < argc; i++)
		tokens.push_back(std::string(argv[i]));

	size_t tokenCount = tokens.size();
	for (size_t i = 0; i < tokenCount; i++)
	{
		if (tokens[i].substr(0, 1) == std::string("-"))
		{
			if (i == tokenCount - 1 || tokens[i + 1].substr(0, 1) == std::string("-"))
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

	return true;
}

#endif


