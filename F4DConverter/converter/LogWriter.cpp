
#include "stdafx.h"

#include "LogWriter.h"

#include <fstream>
#include <chrono>
#include <ctime>

LogWriter LogWriter::logWriter; 

LogWriter::LogWriter()
{
	numberOfFilesToBeConverted = 0;

	numberOfFilesConverted = 0;
}

LogWriter::~LogWriter()
{
}

void LogWriter::setFullPath(std::string& path)
{
	fullPath = path;
}

void LogWriter::addContents(std::string& contents, bool newLine)
{
	logContents += contents;
	if(newLine)
		logContents += std::string("\n");
}

void LogWriter::clearContents()
{
	logContents.clear();
}

void LogWriter::save()
{
	std::ofstream outFile;
	outFile.open(fullPath);

	// 1. result
	char stringLine[1024];
	memset(stringLine, 0x00, sizeof(char)* 1024);
	sprintf(stringLine, "%u of %u files have been converted.\n", numberOfFilesConverted, numberOfFilesToBeConverted);
	outFile << stringLine;

	outFile << "-----------------------------------------------------\n";

	// 2. conversion time
	outFile << "start time : " << startTime << "\n";
	outFile << "end time   : " << endTime << "\n";

	outFile << "-----------------------------------------------------\n";


	// 3. detailed result

	outFile << logContents;

	outFile.close();
}

void LogWriter::setStatus(bool bSuccess)
{
	isSuccess = bSuccess;
}

void LogWriter::start()
{
	startTime = getCurrentTimeString();
}

void LogWriter::finish()
{
	endTime = getCurrentTimeString();
}

bool LogWriter::isStarted()
{
	return !(startTime.empty());
}

std::string LogWriter::getCurrentTimeString()
{
	auto nowTime = std::chrono::system_clock::now();
	std::time_t currentTime = std::chrono::system_clock::to_time_t(nowTime);

	return std::string(std::ctime(&currentTime));
}