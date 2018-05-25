#pragma once

#include <string>

#define ERROR_FLAG					"[ERROR]"
#define WARNING_FLAG				"[WARNING]"
#define NO_BLOCK_ID					"[No block ID]"
#define NO_OBJECT_ID				"[No object ID]"
#define NO_TRANSFORM_MATRIX			"[No Transform Matrix]"
#define NO_VERTEX_COUNT				"[No vertex count]"
#define NO_VERTEX_ARRAY				"[No vertex array]"
#define NO_INDEX_COUNT				"[No index count]"
#define NO_INDEX_ARRAY				"[No index array]"
#define NO_NORMAL_COUNT				"[No normal vector count]"
#define NO_NORMAL_ARRAY				"[No normal vector array]"
#define INVALID_TRIANGLE_COUNT		"[Invalid triangle count]"
#define UNKNOWN_NODE_TYPE			"[Unknown node type]"
#define NO_DATA_OR_INVALID_PATH		"[No raw data or invalid data path]"
#define UNSUPPORTED_FORMAT			"[Unsupported raw data format]"
#define CANNOT_LOAD_FILE			"[Unable to read data file]"
#define NO_DATA_IN_RAW_DATA			"[No data in raw data file]"
#define CONVERSION_FAILURE			"[Conversion Failed]"
#define CANNOT_INITIALIZE			"[Unable to initialize OpenGL]"
#define CANNOT_CHOOSE_PF			"[Unable to choose appropriate pixel format for device context]"
#define CANNOT_SET_PF				"[Unable to set up pixel format for device context]"
#define CANNOT_CREATE_GL_CONTEXT	"[Unable to create OpenGL context for device context]"
#define CANNOT_CONNECT_GLC_TO_DC	"[Unable to connect OpenGL context to device context]"
#define CANNOT_INITIALIZE_GLEW		"[Unable to initialize GLEW]"
#define CANNOT_INITIALIZE_WND		"[Unable to initialize window]"
#define CANNOT_INITIALIZE_DC		"[Unable to initialize device context]"
#define CANNOT_CREATE_DIRECTORY		"[Unable to create the conversion result directory]"
#define UNLOADABLE_MESH_EXISTS		"[At least 1 unloadable mesh exists]"
#define NO_REFERENCE_FILE			"[No reference file exists]"

class LogWriter
{
private:
	LogWriter();

public:
	virtual ~LogWriter();

	unsigned int numberOfFilesToBeConverted;

	unsigned int numberOfFilesConverted;

	

private:
	static LogWriter logWriter;

	std::string startTime;

	std::string endTime;

	std::string logContents;

	std::string fullPath;

	bool isSuccess;

public:
	static LogWriter* getLogWriter() {return &logWriter;}

	void setFullPath(std::string& path);

	void addContents(std::string& contents, bool newLine);

	void clearContents();

	void save();

	void setStatus(bool bSuccess);

	void start();

	void finish();

	bool isStarted();

private:
	std::string getCurrentTimeString();
};