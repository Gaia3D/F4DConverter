# F4DConverter
This application, F4DConverter, is for converting popular 3D model formats into F4D format
which is devised for Mago3D - 3D web geo-platform. (www.mago3d.com).
This project is of Microsoft Visual Studio 2015 C++ project.

## developer's comments before reading more ##
- F4DConverter runs only in Windows 7 or later version of 64-bit OS.
- Recently we changed this converter very much and opened this Github repository with the repository of previous version deprecated.
  This converter runs in pure console mode and makes newer version of F4D.
- We discarded Lego structure and introduced NSM(Net Surface Mesh) for rougher LOD data structure. Detailed information about NSM will be released in www.mago3d.com
- Full information and Window installer will be released SOON in www.mago3d.com (Newer specification of F4D and newer version of window installer) 

## supported input formats ##
- .ifc
- .3ds
- .obj
- .dae

> Beside above formats, other formats which are supported by Assimp may be supported.(NOT TESTED!!)
>
> In this version, .JT(Jupiter Tessellation, a kind of cad design format) is not included.

## necessary libraries for F4DConverter ##
- OpenSceneGraph 3.4.0 : http://www.openscenegraph.org
- ifcplusplus : https://github.com/ifcquery/ifcplusplus
- Carve : https://github.com/ifcquery/ifcplusplus
- Assimp 3.2 : http://assimp.sourceforge.net/main_downloads.html
- boost 1.62 : http://www.boost.org/users/history/version_1_62_0.html
- glew 2.0 : http://glew.sourceforge.net/

> ifcplusplus, Assimp, and glew are for F4DConverter directly.
>
> Carve, boost, and OpenSceneGraph are for ifcplusplus.

## how to use ##
### sample arguments ###
- #inputFolder D:/data/data_3ds/DC_library_del_3DS #outputFolder D:/data/conversionResult #log D:/data/conversionResult/logTest.txt #idPrefix design_ #idSuffix _2017 #meshType 0 #oc y #usf 0.01
- #outputFolder D:/dataConverted #indexing y
### detailed information ###
- #inputFolder [rawDataFolder] : an absolute path of the folder where raw data files to be converted are.
- #outputFolder [F4DFolder] : an absolute path of the folder where conversion results(F4D datasets) are created.
- #log [logFileFullPath] : an absolute path of a log file which is created after finishing conversion processes.
- #idPrefix [prefix] : a prefix used in name of an F4D folder.
- #idSuffix [suffix] : a suffix used in name of an F4D folder.
- #oc [one of Y, y, N, n] : whether visibility indices for occlusion culling should be created or not. "NOT created" is default.
- #usf [numericValue] : unit scale factor. Geometries in F4D are described in meter. That is, the unit scale factor of raw data described in centimeter is 0.01 for example.
- #indexing [one of Y, y, N, n] : wheter objectIndexFile.ihe should be created or not. "NOT created" is default.
- #meshType [one of 0, 1, 2] : type of 3D mesh in raw data. 0 is for semantic data, 1 is for large-sized single realistic mesh, 2 is for large-sized splitted realistic meshes. Realistic mesh means irregularily networked triangles such like point cloud
### precautions for contraints ###
- At least one of "#inputFolder" and "#indexing" is mandatory. Both arguments can be used together.
- "#outputFolder", "#log", and "#meshType"  are mandatory when "#inputFolder" is used.
- "#outputFolder" is mandatory when "#indexing" is used. (So "#outputFolder" is mandatory in any case.)
- It takes very looooooong time to create visibility indices. If "#oc y" is used, 99% of total conversion time is used in creaing visibility indices.
- When "#idPrefix" and/or "#idSuffix" are used, the name of created F4D folder is F4D_|prefix|originalDataFileName|suffix|.
- "#meshType 1" is for raw data of a single mesh of irregular triangle network in wide area(about 250~350m x 250~300m) with SINGLE texture file such like .obj files based on LIDAR data on city block.
- "#meshType 2" is for raw data of splitted meshes of irregular triangle network in wide area(about 250~350m x 250~300m) with texture files of each meshes such like .obj files based on LIDAR data on city block.
- All folder paths injected MUST exist before running the converter. F4DConverter doesn't create folders automatically.

## stuffs under development or to be developed ##
> Priority is not considered.
- A version for Linux family will be released. A prototype is already made and we are trimming it.
- We are considering changing the way of start this converter from passing arguments into this converter to loading a configuration file. As number of parameters used in controlling conversion processes increases, we see the necessity to introduce a configuration file like .ini file to offer much control point for end users.
- We have plans to extend formats of input data supported by our converter for point cloud and .rvt
- Data packing including tiling will be supported by F4D spec and request/response protocol of mago3d to reduce network traffic.
- When converts multiple raw data with same coordinate system, to support matching local coordinates to GIS environment, coordinates on GIS environment will be exported.
- Flexible LOD will be supported to handle 3D models of various geometric sizes.
- As F4DConverter is developed for pure CLI mode, more screen logs will be supported.
