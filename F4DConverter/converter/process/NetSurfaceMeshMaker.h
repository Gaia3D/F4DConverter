#pragma once

#include <map>
#include <vector>

namespace gaia3d
{
	class TrianglePolyhedron;
	class OctreeBox;
}

class NetSurfaceMeshSetting;
class SceneControlVariables;

class NetSurfaceMeshMaker
{
public:
	NetSurfaceMeshMaker();
	~NetSurfaceMeshMaker();

public:
	void makeNetSurfaceMesh(std::vector<gaia3d::OctreeBox*>& octrees,
							NetSurfaceMeshSetting* setting,
							SceneControlVariables* scv,
							unsigned int shaderProgramDepthDetection,
							unsigned int shaderProgramTexture,
							std::map<std::string, unsigned int>& bindingResult,
							std::map<unsigned char, gaia3d::TrianglePolyhedron*>& netSurfaceMeshes,
							std::map<unsigned char, unsigned char*>& netSurfaceTextures,
							std::map<unsigned char, int>& netSurfaceTextureWidth,
							std::map<unsigned char, int>& netSurfaceTextureHeight);

	void makeNetSurfaceMesh(std::vector<gaia3d::OctreeBox*>& octrees,
							NetSurfaceMeshSetting* setting,
							std::map<unsigned char, gaia3d::TrianglePolyhedron*>& netSurfaceMeshes);

};

