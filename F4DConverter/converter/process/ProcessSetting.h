﻿#pragma once

#include <map>

class NetSurfaceMeshSetting;

class ProcessSetting
{
public:
	ProcessSetting();
	~ProcessSetting();

public:
	unsigned char netSurfaceMeshSettingIndex; // index to template NSM setting

	bool bUseNsm;		// if make net surface mesh or not
	bool bExtractExterior; // if extract exteriors or not
	bool bOcclusionCulling; // if do visibility indexing or not
	float leafSpatialOctreeSize; // deepest spatial octree edge length(meter)
	bool bFlipTextureCoordinateV; // if flip texture coordinate v or not
	float interiorVisibilityIndexingCameraStep; // camera position step for interior visibility indexing(meter)
	float exteriorVisibilityIndexingCameraStep; // camera position step for exterior visibility indexing(meter)
	unsigned char interiorVisibilityIndexingOctreeDepth; // visibility octree depth for interior
	unsigned char exteriorVisibilityIndexingOctreeDepth; // visibility octree depth for exterior
	bool bAlignPositionToCenter; // if positions of result F4D is relative to it's center
	int meshType; // type of original mesh
					// 0 : semantic mesh, 1 : single(merged) realistic mesh, 2 : splitted realistic mesh

	std::map<unsigned char, NetSurfaceMeshSetting*> nsmSettings; // net surface mesh setting for each lod

	void fillNsmSettings(unsigned char settingIndex);
	void clearNsmSettings();
};

