# F4DConverter
This application, F4DConverter, is for converting popular 3D model formats into F4D format
which is devised for Mago3D - 3D web geo-platform. (www.mago3d.com).
This project is of Microsoft Visual Studio 2015 C++ project.

## developer's comments before reading more ##
- F4DConverter runs only in Windows 7 or later version of 64-bit OS.
- LOD numbering in F4D spec is reversed in comparison with conventional LOD numbering.
- Officially released window installer(SetupF4DConverter.msi on www.mago3d.com) and source codes of this project are for conversion of 3D models of geographically normal size.
So, it takes so long to convert 3D models of geographically large size. Of course, so many parameters were introduced and defined, which are used in controlling conversion process
and you can control conversion time by modifing these parameters regardless of geographical size of 3D models. Just we didn't expose them through API or arguments passed into
this console application, because they are so complicated. As we mentioned in 『stuffs under development or to be developed』, we will offer ways to access to and modify these
parameters on configuration script files.
- Recently we changed this converter very much and opened this Github repository with the repository of previous version deprecated.
  This converter runs in pure console mode and makes newer version of F4D.
- We discarded Lego structure and introduced NSM(Net Surface Mesh) for rougher LOD data structure. Detailed information about NSM will be released in www.mago3d.com
- Resource files of proj4(proj4 parameters for EPSG code, datum files, and etc) should be in '[executable full path]/proj' when you set up this Visual Studio solution.
This means that you have to copy the folder of proj4 resouce into the binary output folder manually.(Of course, this resource folder is included in the .msi file.)
This situation is not so recommended in the point of development style. But we don't want mandatory proj4 installation before installing the converter.
(We are still considering whether we have to insert step of proj4 installation into the whole installation process or not.)
- Conversion from citygml to F4D is prototype-level. Currently, there are 3 things to be kept in mind.  
 1. Only geometries of most detailed LOD are converted to F4D.  
 2. Geometries without any id in original files are assigned randomly-created id by libcitygml and the converter accepts them as they are.  
 3. A geometry can have multiple texture themes but the converter accepts only 1-st theme by the '1 mesh on 1 texture material' rule in F4D spec.
- When we built libcitygml with CMake, we dropped GDAL dependency and selected static library mode(.lib) instead of dynamic library mode(.dll). 

## supported input formats ##
- .ifc
- .3ds
- .obj
- .dae
- .gml / .xml / .citygml

> Beside above formats, other formats which are supported by Assimp may be supported.(NOT TESTED!!)
>
> In this version, .JT(Jupiter Tessellation, a kind of cad design format) is not included.
> 
> As you know, the file extension .gml and .xml are not only used in citygml. So we are considering if we have to limit the file extension for citygml.

## necessary libraries for F4DConverter ##
- OpenSceneGraph 3.4.0 : http://www.openscenegraph.org
- ifcplusplus : https://github.com/ifcquery/ifcplusplus
- Carve : https://github.com/ifcquery/ifcplusplus
- Assimp 3.2 : http://assimp.sourceforge.net/main_downloads.html
- boost 1.62 : http://www.boost.org/users/history/version_1_62_0.html
- glew 2.0 : http://glew.sourceforge.net/
- proj4 4.9.3 : https://proj4.org
- libcitygml-2.0.9 : https://github.com/jklimke/libcitygml
- xerces-c-3.2.2 : http://xerces.apache.org/xerces-c/

> ifcplusplus, Assimp, libcitygml, and glew are for F4DConverter directly.  
>
> Carve, boost, and OpenSceneGraph are for ifcplusplus.
>
> xerces-c is for libcitygml.


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
- #meshType [one of 0, 1, 2] : type of 3D mesh in raw data. 0 is for semantic data, 1 is for large-sized single realistic mesh, 2 is for large-sized splitted realistic meshes. Realistic mesh means irregularily networked triangles such like point cloud.
- #isYAxisUp [one of Y, y, N, n] : Some 3D models support left-handed coordinate system so that y-axis is toward ceil of a building. this arguments describes wheter y-axis of raw data is toward ceil or not. "No" is default.
- #referenceLonLat [lon,lat] : Matched longitude and latitude of origin of input data files in WGS84 in the case that all input data files are described in same coordinate system.   
- #epsg [public EPSG code] : EPSG code of input data file in the case that all input data files are described in same EPSG code.

### precautions for contraints ###
- At least one of "#inputFolder" and "#indexing" is mandatory. Both arguments can be used together.
- "#outputFolder", "#log", and "#meshType"  are mandatory when "#inputFolder" is used.
- "#outputFolder" is mandatory when "#indexing" is used. (So "#outputFolder" is mandatory in any case.)
- It takes very looooooong time to create visibility indices. If "#oc y" is used, 99% of total conversion time is used in creaing visibility indices.
- When "#idPrefix" and/or "#idSuffix" are used, the name of created F4D folder is F4D_|prefix|originalDataFileName|suffix|.
- In this version, only "#meshType 0" is available. You can use other values with 1 line source code modification in 'F4DConverter.cpp'.
- "#meshType 1" is for raw data of a single mesh of irregular triangle network in wide area(about (250-350)m x (250-300)m) with SINGLE texture file such like .obj files based on LIDAR data on city block.
- "#meshType 2" is for raw data of splitted meshes of irregular triangle network in wide area(about (250-350)m x (250-300)m) with texture files of each meshes such like .obj files based on LIDAR data on city block.
- "#referenceLonLat" should be used like "#referenceLonLat -58.78785,-62.2233". Separation with comma & no blank allowed.
- When "#epsg" and "#referenceLonLat" are used at same time, "#referenceLonLat" is ignored.
- When "#epsg" or "#referenceLonLat" are used, 3D models are moved to their origins along x-y plane so that their bounding box centers coincide with thier origin in x-y plane, and
  'lonsLats.json' is created, in which geo-referencing information of each F4D are written.
- When converting citygml, 'lonsLats.json' is created.
- When "#epsg" or "#referenceLonLat" are used in converting citygml, the embedded SRS information in citygml is ignored.
- All folder paths injected MUST exist before running the converter. F4DConverter doesn't create folders automatically.

## stuffs under development or to be developed ##
> Priority is not considered.
- We are considering changing the way of start this converter from passing arguments into this converter to loading a configuration file. As number of parameters used in controlling conversion processes increases, we see the necessity to introduce a configuration file like .ini file to offer much control point for end users.
- We have plans to extend formats of input data supported by our converter for point cloud and .rvt
- Data packing including tiling will be supported by F4D spec and request/response protocol of mago3d to reduce network traffic.
- Flexible LOD will be supported to handle 3D models of various geometric sizes.
- As F4DConverter is developed for pure CLI mode, more screen logs will be supported.
- Replacing curerent version of IfcPlusPlus library with latest version is undergoing.
