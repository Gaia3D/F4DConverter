#pragma once

#define InputFolderW		L"#inputFolder"	///< The path of the Input Folder.
#define OutputFolderW		L"#outputFolder"///< The path of the Output Folder.
#define LogFilePathW		L"#log"			///< The path of the log file.
#define CreateIndexW		L"#indexing"	///< The argument for creating a index file for mago3D.
#define IdPrefixW			L"#idPrefix"	///< The prefix which will be attached in front of the name of F4D folder name.
#define IdSuffixW			L"#idSuffix"	///< The postfix which will be attached at the end of the name of F4D folder name.
#define PerformOCW			L"#oc"			///< The argument for asking whether Occlusion culling is done or not.
#define UnitScaleFactorW	L"#usf"			///< Unit scale factor. 
#define SkinLevelNsmW		L"#skinLevel"	///< This is the scale unit of the surface of geometry data. Default value is 4 for single building.
											///< Smaller value is used for smaller surface(Usually 3).
#define IsYAxisUpW			L"#isYAxisUp"	///< For specific axis this argument is used. If this argument is set then y axis and z axis values are swapped.
#define ReferenceLonLatW	L"#referenceLonLat" ///< Origin coordinate of geometry data. 
#define MeshTypeW			L"#meshType"	///< MeshType 0 : The data with syntax and inner structure. 
#define AlignToW			L"#alignTo"		///< Move the origin of the data to the specific position.
#define EpsgW				L"#epsg"		///< If the geometry data is written in specific CRS then this argument is used.
#define OffsetXW			L"#offsetX"		///< If this is set then the offset value is added to the x coordinate value. 
#define OffsetYW			L"#offsetY"		///< If this is set then the offset value is added to the y coordinate value. 
#define OffsetZW			L"#offsetZ"		///< If this is set then the offset value is added to the z coordinate value. 
#define ProjectNameW		L"#projectName"	///< The sub folder is created with this name under output folder.
#define SplitFilterW		L"#splitFilter"	///< If the data has the identifier and that is matched with this filter then the converter create single F4D
											///< and put that data into the single F4D file.

#define ProgramPath		"programPath"		
#define InputFolder		"#inputFolder"		
#define OutputFolder	"#outputFolder"
#define LogFilePath		"#log"
#define CreateIndex		"#indexing"
#define IdPrefix		"#idPrefix"
#define IdSuffix		"#idSuffix"
#define PerformOC		"#oc"
#define UnitScaleFactor	"#usf"
#define SkinLevelNsm	"#skinLevel"
#define IsYAxisUp		"#isYAxisUp"
#define ReferenceLonLat	"#referenceLonLat"
#define MeshType		"#meshType"
#define AlignTo			"#alignTo"
#define Epsg			"#epsg"
#define OffsetX			"#offsetX"
#define OffsetY			"#offsetY"
#define OffsetZ			"#offsetZ"
#define ProjectName		"#projectName"
#define SplitFilter		"#splitFilter"
