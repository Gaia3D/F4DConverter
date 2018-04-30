#include "stdafx.h"
#include "ProcessSetting.h"

#include "NetSurfaceMeshSetting.h"


ProcessSetting::ProcessSetting()
{
	netSurfaceMeshSettingIndex = 2; // default index to template process setting

	bExtractExterior = true; // if extract exteriors or not
	bOcclusionCulling = false; // if do visibility indexing or not
	leafSpatialOctreeSize = 12.0f; // deepest spatial octree edge length
	bFlipTextureCoordinateU = false; // if flip texture coordinate u or not
	interiorVisibilityIndexingCameraStep = 1.8f; // camera position step for interior visibility indexing
	exteriorVisibilityIndexingCameraStep = 20.0f; // camera position step for exterior visibility indexing
	interiorVisibilityIndexingOctreeDepth = 2; // visibility octree depth for interior
	exteriorVisibilityIndexingOctreeDepth = 1; // visibility octree depth for exterior
}


ProcessSetting::~ProcessSetting()
{
	clearNsmSettings();
}

void ProcessSetting::fillNsmSettings(unsigned char settingIndex)
{ 
	clearNsmSettings();

	NetSurfaceMeshSetting* lod2 = NetSurfaceMeshSetting::getNetSurfaceMeshSetting(settingIndex, 2);
	nsmSettings[2] = lod2;
	NetSurfaceMeshSetting* lod3 = NetSurfaceMeshSetting::getNetSurfaceMeshSetting(settingIndex, 3);
	nsmSettings[3] = lod3;
	NetSurfaceMeshSetting* lod4 = NetSurfaceMeshSetting::getNetSurfaceMeshSetting(settingIndex, 4);
	nsmSettings[4] = lod4;
	NetSurfaceMeshSetting* lod5 = NetSurfaceMeshSetting::getNetSurfaceMeshSetting(settingIndex, 5);
	nsmSettings[5] = lod5;
}

void ProcessSetting::clearNsmSettings()
{
	std::map<unsigned char, NetSurfaceMeshSetting*>::iterator iter = nsmSettings.begin();
	for (; iter != nsmSettings.end(); iter++)
		delete iter->second;
	nsmSettings.clear();
}
