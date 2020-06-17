
#include "stdafx.h"

#include "LogWriter.h"

#include <fstream>
#include <chrono>
#include <ctime>

#include "util/json/json.h"

LogWriter LogWriter::logWriter; 
Json::Value logObject(Json::objectValue);
Json::Value currentConversionJob(Json::objectValue);
bool isConversionJobGoing = false;

LogWriter::LogWriter()
{
	numberOfFilesToBeConverted = 0;

	numberOfFilesConverted = 0;

	isSuccess = true;
}

LogWriter::~LogWriter()
{
}

///< Set full path of the program
void LogWriter::setFullPath(std::string& path)
{
	fullPath = path;
}

///< Add log contents
void LogWriter::addContents(std::string& contents, bool newLine)
{
	logContents += contents;
	if(newLine)
		logContents += std::string("\n");
}

///< Clear log contents
void LogWriter::clearContents()
{
	logContents.clear();
}

void LogWriter::save()
{
	Json::StyledWriter writer;
	std::string documentContent = writer.write(logObject);
	FILE* file = fopen(fullPath.c_str(), "wt");
	fprintf(file, "%s", documentContent.c_str());
	fclose(file);

	logObject.clear();
	currentConversionJob.clear();

	//std::ofstream outFile;
	//outFile.open(fullPath);

	//// 1. result
	//char stringLine[1024];
	//memset(stringLine, 0x00, sizeof(char)* 1024);
	//sprintf(stringLine, "%u of %u files have been converted.\n", numberOfFilesConverted, numberOfFilesToBeConverted);
	//outFile << stringLine;

	//outFile << "-----------------------------------------------------\n";

	//// 2. conversion time
	//outFile << "start time : " << startTime << "\n";
	//outFile << "end time   : " << endTime << "\n";

	//outFile << "-----------------------------------------------------\n";


	//// 3. detailed result

	//outFile << logContents;

	//outFile.close();
}

void LogWriter::setStatus(bool bSuccess, std::string message)
{
	isSuccess = bSuccess;
	logObject["isSuccess"] = isSuccess;
	if(!bSuccess)
		logObject["failureLog"] = message;
}

///< Record the start time of conversion
void LogWriter::start()
{
	startTime = getCurrentTimeString();
	logObject["startTime"] = startTime;
	logObject["numberOfFilesToBeConverted"] = 0;
	logObject["numberOfFilesConverted"] = 0;
	logObject["isSuccess"] = true;
	logObject["conversionJobResult"] = Json::Value(Json::arrayValue);
}

///< Record the finishing time of conversion
void LogWriter::finish()
{
	endTime = getCurrentTimeString();
	logObject["endTime"] = endTime;
	logObject["numberOfFilesToBeConverted"] = numberOfFilesToBeConverted;
	logObject["numberOfFilesConverted"] = numberOfFilesConverted;
}

///< Check conversion is started or not
bool LogWriter::isStarted()
{
	return !(startTime.empty());
}

///< Get current system time
std::string LogWriter::getCurrentTimeString()
{
	auto nowTime = std::chrono::system_clock::now();
	std::time_t currentTime = std::chrono::system_clock::to_time_t(nowTime);

	char timeStringLine[256];
	memset(timeStringLine, 0x00, 256);
	std::strftime(timeStringLine, 256, "%Y-%m-%d %H:%M:%S", std::localtime(&currentTime));

	std::string timeString(timeStringLine);
	/*if (timeString.find_last_of(std::string("\n")) != std::string::npos)
		timeString = timeString.substr(0, timeString.find_last_of(std::string("\n")));*/

	return timeString;
}

void LogWriter::createNewConversionJobLog(std::string fileName, std::string fullPath)
{
	if (isConversionJobGoing)
		return;

	currentConversionJob["fileName"] = fileName;
	currentConversionJob["fullPath"] = fullPath;
	currentConversionJob["startTime"] = getCurrentTimeString();
	currentConversionJob["resultStatus"] = std::string("success");
	currentConversionJob["message"] = std::string("");

	isConversionJobGoing = true;
}

void LogWriter::changeCurrentConversionJobStatus(CONVERSION_JOB_STATUS jobStatus)
{
	if (!isConversionJobGoing)
		return;

	switch (jobStatus)
	{
	case success:
		currentConversionJob["resultStatus"] = std::string("success");
		break;
	case warning:
		currentConversionJob["resultStatus"] = std::string("warning");
		break;
	case failure:
		currentConversionJob["resultStatus"] = std::string("failure");
		break;
	}
}

void LogWriter::addDescriptionToCurrentConversionJobLog(std::string content)
{
	if (!isConversionJobGoing)
		return;

	if (currentConversionJob["message"].asString().empty())
		currentConversionJob["message"] = content;
	else
		currentConversionJob["message"] = currentConversionJob["message"].asString() + std::string(" | ") + content;
}

void LogWriter::closeCurrentConversionJobLog()
{
	if (!isConversionJobGoing)
		return;

	currentConversionJob["endTime"] = getCurrentTimeString();
	logObject["conversionJobResult"].append(currentConversionJob);
	currentConversionJob.clear();

	isConversionJobGoing = false;
}

void LogWriter::setGeoReferencingInfo(bool bHasInfo, double minx, double miny, double minz, double maxx, double maxy, double maxz)
{
	if (bHasInfo)
	{
		currentConversionJob["bGeoReferenced"] = true;
	}
	else
	{
		currentConversionJob["bGeoReferenced"] = false;

		Json::Value bbox(Json::arrayValue);
		bbox.append(minx);
		bbox.append(miny);
		bbox.append(minz);
		bbox.append(maxx);
		bbox.append(maxy);
		bbox.append(maxz);

		currentConversionJob["bbox"] = bbox;
	}
}
