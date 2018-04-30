#pragma once

#define WindowClassNameForOpenGL L"Window for OpenGL"
#define WindowWidthForOpenGL 1024
#define WindowHeightForOpenGL 1024
#define MemoryDeviceContextEdgeLength 512

#define ExteriorDetectionThreshold 6

#define DisplayListIdForOcclusionCulling 1
#define OcclusionCullingScreenSize 512

#define BigObjectThreshold 5.0
#define MediumObjectThreshold 2.0
#define BoneObjectThreshold 5.0

#define SpatialIndexingDepth ((unsigned char)3)

#define MaxLod 5

#define VboVertexMaxCount 65532

#define MaxUnsignedLong 65532

#define TriangleSizeLevels 4
#define TriangleSizeThresholds {1.0, 0.5, 0.1, 0.05}

// attribute key names
#define ObjectGuid "objectGuid"
#define TextureName "textureName"
#define F4DID	"id"
