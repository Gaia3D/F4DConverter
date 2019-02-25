#pragma once

#include <vector>
#include <map>
#include "../geometry/TrianglePolyhedron.h"
#include "../geometry/BoundingBox.h"
#include "../geometry/ColorU4.h"
#include "../geometry/OctreeBox.h"
#include "ProcessSetting.h"


class SceneControlVariables;

class ConversionProcessor
{
public:
	ConversionProcessor();

	virtual ~ConversionProcessor();

protected:
	ProcessSetting settings;

public:
	void setNsmSettingIndex(unsigned char index) { settings.netSurfaceMeshSettingIndex = index; }
	void setUseNsm(bool bUse) { settings.bUseNsm = bUse; }
	void setExteriorExtraction(bool bDo) { settings.bExtractExterior = bDo; }
	void setVisibilityIndexing(bool bDo) { settings.bOcclusionCulling = bDo; }
	void setLeafSpatialOctreeSize(float fSize) { settings.leafSpatialOctreeSize = fSize; }
	void setTextureCoordinateVFlip(bool bDo) { settings.bFlipTextureCoordinateV = bDo; }
	void setInteriorVisibilityIndexingCameraStep(float step) { settings.interiorVisibilityIndexingCameraStep = step; }
	void setExteriorVisibilityIndexingCameraStep(float step) { settings.exteriorVisibilityIndexingCameraStep = step; }
	void setInteriorVisibilityIndexingOctreeDepth(unsigned char depth) { settings.interiorVisibilityIndexingOctreeDepth = depth; }
	void setExteriorVisibilityIndexingOctreeDepth(unsigned char depth) { settings.exteriorVisibilityIndexingOctreeDepth = depth; }
	void clearNsmSettings() { settings.clearNsmSettings(); }
	void setSkinLevel(unsigned char level) { settings.netSurfaceMeshSettingIndex = level; }
	void setAlignPostionToCenter(bool bAlign) { settings.bAlignPositionToCenter = bAlign; }
	void setMeshType(int type) { settings.meshType = type; }
	int getMeshType() { return settings.meshType; }

protected:
	SceneControlVariables* scv;

	std::vector<gaia3d::TrianglePolyhedron*> allMeshes;

	std::map<std::string, std::string> allTextureInfo;
	std::map<std::string, unsigned char*> resizedTextures;
	std::map<std::string, int> allTextureWidths;
	std::map<std::string, int> allTextureHeights;

	gaia3d::BoundingBox fullBbox;

	gaia3d::SpatialOctreeBox thisSpatialOctree;

	std::map<unsigned char, gaia3d::TrianglePolyhedron*> netSurfaceMeshes;
	std::map<unsigned char, unsigned char*> netSurfaceTextures;
	std::map<unsigned char, int> netSurfaceTextureWidth;
	std::map<unsigned char, int> netSurfaceTextureHeight;

	gaia3d::Matrix4 bboxCenterToLocalOrigin;

	std::map<std::string, std::string> attributes;

	double longitude, latitude;
	float altitude;

public:
	bool initialize();

	void uninitialize();

	void clear();

	void defaultSpaceSetupForVisualization(int width, int height);

	bool proceedConversion(std::vector<gaia3d::TrianglePolyhedron*>& originalMeshes,
							std::map<std::string, std::string>& originalTextureInfo);

	gaia3d::SpatialOctreeBox* getSpatialOctree() {return &thisSpatialOctree;}

	std::map<std::string, std::string>& getTextureInfo() { return allTextureInfo; }

	void getMatrixForBboxCenterToLocalOrigin(gaia3d::Matrix4* matrix) {matrix->set(&bboxCenterToLocalOrigin);}

	void setMatrixForBboxCenterToLocalOrigin(gaia3d::Matrix4* matrix) {bboxCenterToLocalOrigin.set(matrix);}

	void changeSceneControlVariables();

	SceneControlVariables* getSceneControlVariables() {return scv;}

	void addAttribute(std::string key, std::string value);

	std::map<std::string, std::string>& getAttributes() {return attributes;}

	std::string getAttribute(std::string key) {return attributes[key];}

	bool doesAttributeExist(std::string key) { return attributes.find(key) != attributes.end(); }

	double getLongitude() {return longitude;}
	void setLongitude(double lon) {longitude = lon;}
	double getLatitude() {return latitude;}
	void setLatitude(double lat) {latitude = lat;}
	float getAltitude() {return altitude;}
	void setAltitude(float alt) {altitude = alt;}
	gaia3d::BoundingBox& getBoundingBox() {return fullBbox;}
	std::vector<gaia3d::TrianglePolyhedron*>& getAllMeshes() {return allMeshes;}

	std::map<std::string, unsigned char*>& getResizedTextures() { return resizedTextures; }
	std::map<std::string, int>& getAllTextureWidths() { return allTextureWidths; }
	std::map<std::string, int>& getAllTextureHeights() { return allTextureHeights; }

	std::map<unsigned char, gaia3d::TrianglePolyhedron*>& getNetSurfaceMeshes() { return netSurfaceMeshes; }
	std::map<unsigned char, unsigned char*>&  getNetSurfaceTextures() { return netSurfaceTextures; }
	std::map<unsigned char, int>& getNetSurfaceTextureWidth() { return netSurfaceTextureWidth; }
	std::map<unsigned char, int>& getNetSurfaceTextureHeight() { return netSurfaceTextureHeight; }

protected:
	// main processing steps - start
	void convertPointCloud(std::vector<gaia3d::TrianglePolyhedron*>& originalMeshes);

	void convertSemanticData(std::vector<gaia3d::TrianglePolyhedron*>& originalMeshes,
							std::map<std::string, std::string>& originalTextureInfo);

	void convertSingleRealisticMesh(std::vector<gaia3d::TrianglePolyhedron*>& originalMeshes,
									std::map<std::string, std::string>& originalTextureInfo);

	void convertSplittedRealisticMesh(std::vector<gaia3d::TrianglePolyhedron*>& originalMeshes,
									std::map<std::string, std::string>& originalTextureInfo);

	void trimVertexNormals(std::vector<gaia3d::TrianglePolyhedron*>& meshes);

	void calculateBoundingBox(std::vector<gaia3d::TrianglePolyhedron*>& meshes, gaia3d::BoundingBox& bbox);

	void makeVboObjects(std::vector<gaia3d::TrianglePolyhedron*>& meshes, bool bBind = false);

	void determineWhichSurfacesAreExterior(std::vector<gaia3d::TrianglePolyhedron*>& meshes, gaia3d::BoundingBox& bbox);

	void determineModelAndReference(std::vector<gaia3d::TrianglePolyhedron*>& meshes);

	void assignReferencesIntoExteriorAndInterior(std::vector<gaia3d::TrianglePolyhedron*>& meshes,
												std::vector<gaia3d::TrianglePolyhedron*>& interiors,
												std::vector<gaia3d::TrianglePolyhedron*>& exteriors);

	void assignReferencesIntoEachSpatialOctrees(gaia3d::SpatialOctreeBox& spatialOctree,
												std::vector<gaia3d::TrianglePolyhedron*>& meshes,
												gaia3d::BoundingBox& bbox,
												bool bFixedDepth = true,
												double leafBoxSize = 0.0,
												bool bRefOnOnlyOneLeaf = false);

	void splitOriginalMeshIntoEachSpatialOctrees(gaia3d::SpatialOctreeBox& spatialOctree,
												std::vector<gaia3d::TrianglePolyhedron*>& meshes,
												gaia3d::BoundingBox& bbox,
												bool bFixedDepth,
												double leafBoxSize,
												bool bAllowDuplication);

	void assignObjectsIntoEachCubeInPyramid(gaia3d::SpatialOctreeBox& spatialOctree,
											std::vector<gaia3d::TrianglePolyhedron*>& meshes,
											gaia3d::BoundingBox& bbox,
											double leafBoxSize,
											bool bAllowDuplication,
											bool bBasedOnMesh);

	void makeOcclusionInformation(std::vector<gaia3d::TrianglePolyhedron*>& meshes,
									gaia3d::VisionOctreeBox& interiorOcclusionOctree,
									gaia3d::VisionOctreeBox& exteriorOcclusionOctree,
									gaia3d::BoundingBox& interiorBbox,
									gaia3d::BoundingBox& exteriorBbox);

	void applyOcclusionInformationOnSpatialOctree(gaia3d::SpatialOctreeBox& spatialOctree,
													gaia3d::VisionOctreeBox& interiorOcclusionOctree,
													gaia3d::VisionOctreeBox& exteriorOcclusionOctree);

	void normalizeTextures(std::map<std::string, std::string>& textureInfo);
	// main processing steps - end

	void calculateBoundingBox(gaia3d::TrianglePolyhedron* mesh);

	void setupPerspectiveViewSetting(gaia3d::BoundingBox& bbox);

	void checkIfEachSurfaceIsExterior(std::vector<gaia3d::Surface*>& surfaces, std::vector<gaia3d::ColorU4>& colors);

	void drawSurfacesWithIndexColor(std::vector<gaia3d::Surface*>& surfaces, std::vector<gaia3d::ColorU4>& colors);

	void makeDisplayListOfMeshes(std::vector<gaia3d::TrianglePolyhedron*>& meshes, std::vector<gaia3d::ColorU4>& colors);

	void renderMesh(gaia3d::TrianglePolyhedron* mesh, bool bNormal, bool bTextureCoordinate);

	void makeVisibilityIndices(gaia3d::VisionOctreeBox& octree, std::vector<gaia3d::TrianglePolyhedron*>& meshes,
								float scanStepX, float scanStepY, float scanStepZ,
								gaia3d::VisionOctreeBox* excludedBox);

	void drawAndDetectVisibleColorIndices(std::map<size_t, size_t>& container);

	void extractMatchedReferencesFromOcclusionInfo(gaia3d::VisionOctreeBox* receiver,
													gaia3d::VisionOctreeBox& info,
													std::vector<gaia3d::TrianglePolyhedron*>& meshesToBeCompared);

	void sortTrianglesBySize(std::vector<gaia3d::Triangle*>& inputTriangles,
							unsigned char sizeLevels,
							double* sizeArray,
							std::vector<gaia3d::Triangle*>& outputTriangles,
							unsigned int* sizeIndexMarkers);

	void loadAndBindTextures(std::map<std::string, unsigned char*>& textures,
							std::map<std::string, int>& textureWidths,
							std::map<std::string, int>& textureHeights,
							std::map<std::string, unsigned int>& bindingResult);

	void drawMeshesWithTextures(std::vector<gaia3d::TrianglePolyhedron*>& meshes, std::map<std::string, unsigned int>& bindingResult, unsigned int shaderProgram);

	void unbindTextures(std::map<std::string, unsigned int>& bindingResult);

	unsigned int makeShaders(unsigned int shaderType);

	void deleteShaders(unsigned int programId);

	void makeNetSurfaceMeshes(gaia3d::SpatialOctreeBox& octrees,
							std::map<std::string, unsigned char*>& textures,
							std::map<std::string, int>& textureWidths,
							std::map<std::string, int>& textureHeights,
							std::map<unsigned char, unsigned char>& lodUsingOriginalMesh);

	void normalizeMosiacTextures(std::map<unsigned char, unsigned char*>& mosaicTextures,
								std::map<unsigned char, int>& mosaicTextureWidth,
								std::map<unsigned char, int>& mosaicTextureHeight);

	void changeXYPlaneCoordinateToRelativeCoordinateToBoundingBoxFootprintCenter(std::vector<gaia3d::TrianglePolyhedron*>& meshes, gaia3d::BoundingBox& bbox);

	void dropTrianglesOfSmallSizedEdge(std::vector<gaia3d::TrianglePolyhedron*>& meshes, double edgeMinSize);

	void removeDuplicatedVerticesAndOverlappingTriangles(std::vector<gaia3d::TrianglePolyhedron*>& meshes, bool bCompareTexCoord, bool bCompareNormal);

	void makeLodTextureUsingOriginalTextureDirectly(unsigned char* originalTexture, int originalWidth, int originalHeight,
													std::map<unsigned char, unsigned char>& lodMadeOfOriginalMesh,
													std::map<unsigned char, unsigned char*>& netSurfaceTextures,
													std::map<unsigned char, int>& netSurfaceTextureWidth,
													std::map<unsigned char, int>& netSurfaceTextureHeight);

	void divideOriginalTextureIntoSmallerSize(unsigned char* originalTexture, int originalWidth, int originalHeight,
											gaia3d::SpatialOctreeBox& octree,
											std::map<std::string, unsigned char*>& results,
											std::map<std::string, int>& resultWidths,
											std::map<std::string, int>& resultHeights,
											std::map<std::string, std::string>& resultTextureInfo);

	void reuseOriginalMeshForRougherLods(gaia3d::SpatialOctreeBox& octree);
};