#pragma once

class NetSurfaceMeshSetting
{
public:
	NetSurfaceMeshSetting();
	~NetSurfaceMeshSetting();

public:
	unsigned char lod; // lod
	float maxAngleChangeOfNormalVectorForAllowingEdgeCollapse; // threshold value of normal vector angle change on edge collapse(for all edge types) (degree)
	float maxAngleChangeOfFrontierEdgeForAllowingEdgeCollapse; // threshold value of edge angle change on edge collpase(for only frontier edge) (degree)
	float maxAngleDifferenceBetweenNeighborFrontierEdgesForCollapse; // threshold value of angle btw two adjacent frontier edges for collapse (degree)
	float maxLengthForAllowingInnerEdgeSkirting;// threshold value for making skirting triangles or not(for inner edge) (meter)
	float maxLengthForAllowingFrontierEdgeSkirting; // threshold value for making skirting triangles or not(for frontier edge) (meter)
	float netCellSize; // grid size for net (meter)
	float subBoxSize; // box edge length by which a target is to be divided into sub boxes (meter)
	int netSurfaceMeshTextureWidth; // texture image width for output net surface mesh
	int netSurfaceMeshTextureHeight; // texture image height for output net surface mesh

public:
	static NetSurfaceMeshSetting* getNetSurfaceMeshSetting(unsigned char settingIndex, unsigned char lodNumber);
};

