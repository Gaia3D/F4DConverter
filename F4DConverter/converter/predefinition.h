#pragma once

#define WindowClassNameForOpenGL L"Window for OpenGL"
///< 더미 윈도우의 크기(px)
#define WindowWidthForOpenGL 1024
#define WindowHeightForOpenGL 1024
///< Deffered rendering시 찍는 이미지의 가로 세로 사이즈(px)
#define MemoryDeviceContextEdgeLength 512

///< Deffered rendering시 해당 픽셀 이상 외부(Exterior)에 노출되어 있으면 이는 외부에 있는 객체라고 할 수 있다.
#define ExteriorDetectionThreshold 6

#define DisplayListIdForOcclusionCulling 1
///< Occlusion Culling 시 찍는 이미지의 가로 세로 사이즈(px)
#define OcclusionCullingScreenSize 512

///< 큰 물체라고 인식할 수 있는 threshold값
#define BigObjectThreshold 5.0
///< 중간 물체라고 인식할 수 있는 threshold값 
#define MediumObjectThreshold 2.0
#define BoneObjectThreshold 5.0
#define SpatialIndexingDepth ((unsigned char)3)

#define MaxLod 5

#define VboVertexMaxCount 65532

#define MaxUnsignedLong 65532

///< 삼각형의 기본 사이즈 레벨
#define TriangleSizeLevels 4
///< 삼각형의 기본 사이즈 레벨 구간. 0.05이하, 1.0 이상을 포함해 5개의 구간이 있다.
#define TriangleSizeThresholds {1.0, 0.5, 0.1, 0.05}

// attribute key names
#define ObjectGuid "objectGuid"
#define TextureName "textureName"
#define F4DID	"id"
