#include "stdafx.h"
#include "NetSurfaceMeshMaker.h"

#include "glew.h"

#include "../geometry/OctreeBox.h"
#include "../geometry/TrianglePolyhedron.h"
#include "../util/utility.h"
#include "NetSurfaceMeshSetting.h"
#include "SceneControlVariables.h"

struct Hedge;
struct HedgeVertex
{
	gaia3d::Point3D position;
	std::vector<Hedge*> hedgesFromThisVertex;
	bool bOnGeometrySurface;
};

struct HedgeTriangle
{
	HedgeVertex* vertex0;
	HedgeVertex* vertex1;
	HedgeVertex* vertex2;
	gaia3d::Point3D normal;
	Hedge* hedge;
};

struct Hedge
{
	HedgeVertex* startVertex;
	Hedge* next;
	Hedge* twin;
	HedgeTriangle* face;
	gaia3d::Point3D direction;
};

enum TYPE_CUBEFACE
{
	CUBEFACE_UNKNOWN, CUBEFACE_TOP, CUBEFACE_BOTTOM, CUBEFACE_FRONT, CUBEFACE_REAR, CUBEFACE_LEFT, CUBEFACE_RIGHT
};

struct NetSurface
{
	std::vector<std::vector<HedgeVertex*>*> netVertices;
	gaia3d::TrianglePolyhedron* nsm;
	std::vector<Hedge*> hedges;
	std::vector<std::vector<HedgeTriangle*>*> netTriangles;
	gaia3d::Point3D normalVector;
	gaia3d::BoundingBox domainBox;
	float gridSize;
	TYPE_CUBEFACE cubeFace;

	unsigned char* texture;
	int textureWidth, textureHeight;
};

void makeNetSurfacesFromBox(gaia3d::BoundingBox& bbox, float gridSize, std::vector<NetSurface*>& result);
void makeModelProjectedOnTargetSurface(std::vector<NetSurface*>& netSurfaces,
										std::vector<gaia3d::TrianglePolyhedron*>& meshes,
										bool bOnlyExterior,
										SceneControlVariables* scv,
										unsigned int shaderProgramDepthDetection,
										unsigned int shaderProgramTexture,
										std::map<std::string, unsigned int>& bindingResult);
void makeNetSurfaceMeshes(NetSurface* netSurface, float maxLengthForInnerEdgeSkirting, float maxLengthForFrontierEdgeSkirting);
void triangleReduction(NetSurface* netSurface, float cosNormalAngleChangeLimit, float cosEdgeAngleChangeLimit, float cosAngleBtwFrontierEdges);
void changeNetSurfacesIntoTrianglePolyhedron(NetSurface* netSurface, std::map<HedgeVertex*, gaia3d::Vertex*>& vertexMapper);
void clearHalfEdgeSystem(NetSurface* netSurface);
void makeNetSurfaceTextures(std::vector<NetSurface*>& netSurfaces,
							NetSurfaceMeshSetting* setting,
							unsigned char** textureData,
							int& textureWidth,
							int& textureHeight);
gaia3d::TrianglePolyhedron* mergePolyhedrons(std::vector<gaia3d::TrianglePolyhedron*> meshes);
NetSurface* makeNetSurfaceMeshFromOriginalDirectly(gaia3d::TrianglePolyhedron* mesh, std::map<HedgeVertex*, gaia3d::Vertex*>& vertexMapper);

NetSurfaceMeshMaker::NetSurfaceMeshMaker()
{
}


NetSurfaceMeshMaker::~NetSurfaceMeshMaker()
{
}

void NetSurfaceMeshMaker::makeNetSurfaceMesh(std::vector<gaia3d::OctreeBox*>& octrees,
											NetSurfaceMeshSetting* setting,
											SceneControlVariables* scv,
											unsigned int shaderProgramDepthDetection,
											unsigned int shaderProgramTexture,
											std::map<std::string, unsigned int>& bindingResult,
											std::map<unsigned char, gaia3d::TrianglePolyhedron*>& netSurfaceMeshes,
											std::map<unsigned char, unsigned char*>& netSurfaceTextures,
											std::map<unsigned char, int>& netSurfaceTextureWidth,
											std::map<unsigned char, int>& netSurfaceTextureHeight)
{
	size_t octreeCount = octrees.size();
	
	std::vector<NetSurface*> allNetSurfaces;
	std::map<size_t, std::vector<NetSurface*>*> octreeNetSurfaceRelations;
	for(size_t h = 0; h < octreeCount; h++)
	{
		gaia3d::SpatialOctreeBox* octree = (gaia3d::SpatialOctreeBox*)octrees[h];
		if (octree->meshes.empty())
			continue;

		// make netSurface vertices
		gaia3d::BoundingBox octreeBbox;
		octreeBbox.addPoint(octree->minX, octree->minY, octree->minZ);
		octreeBbox.addPoint(octree->maxX, octree->maxY, octree->maxZ);
		float octreeSize = (float)octreeBbox.getMaxLength();

		int boxEdgeDivisionCount;
		if (setting->subBoxSize > octreeSize)
			boxEdgeDivisionCount = 1;
		else
			boxEdgeDivisionCount = (int)floorf(octreeSize / setting->subBoxSize);

		std::vector<gaia3d::BoundingBox> dividedBoxes;
		octreeBbox.divideBbox(boxEdgeDivisionCount, dividedBoxes);

		size_t dividedBoxCount = dividedBoxes.size();
		std::vector<NetSurface*>* netSurfacesInThisOctree = new std::vector<NetSurface*>;
		octreeNetSurfaceRelations[h] = netSurfacesInThisOctree;
		for (size_t i = 0; i < dividedBoxCount; i++)
		{
			dividedBoxes[i].expand(10E-5);
			std::vector<NetSurface*> newlyCreatedNetSurfaces;
			makeNetSurfacesFromBox(dividedBoxes[i], setting->netCellSize, newlyCreatedNetSurfaces);

			for (size_t j = 0; j < newlyCreatedNetSurfaces.size(); j++)
			{
				newlyCreatedNetSurfaces[j]->texture = NULL;
				newlyCreatedNetSurfaces[j]->textureWidth = setting->netSurfaceMeshTextureWidth;
				newlyCreatedNetSurfaces[j]->textureHeight = setting->netSurfaceMeshTextureHeight;
			}

			//bool bOnlyExterior = setting->lod == 2 ? false : true;
			bool bOnlyExterior = true;
			makeModelProjectedOnTargetSurface(newlyCreatedNetSurfaces, octree->meshes, bOnlyExterior, scv, shaderProgramDepthDetection, shaderProgramTexture, bindingResult);

			for (size_t j = 0; j < newlyCreatedNetSurfaces.size(); j++)
			{
				netSurfacesInThisOctree->push_back(newlyCreatedNetSurfaces[j]);
				allNetSurfaces.push_back(newlyCreatedNetSurfaces[j]);
			}
		}

		// change netSurface vertices into mesh
		size_t netSurfaceCount = netSurfacesInThisOctree->size();
		for (size_t i = 0; i < netSurfaceCount; i++)
			makeNetSurfaceMeshes((*netSurfacesInThisOctree)[i],
								setting->maxLengthForAllowingInnerEdgeSkirting,
								setting->maxLengthForAllowingFrontierEdgeSkirting);

		// process on triangle reduction
		float cosNormalAngleChangeLimit = cosf(setting->maxAngleChangeOfNormalVectorForAllowingEdgeCollapse * ((float)M_PI) / 180.0f);
		float cosEdgeAngleChangeLimit = cosf(setting->maxAngleChangeOfFrontierEdgeForAllowingEdgeCollapse * ((float)M_PI) / 180.0f);
		float cosAngleBtwFrontierEdges = cosf(setting->maxAngleDifferenceBetweenNeighborFrontierEdgesForCollapse * ((float)M_PI) / 180.0f);
		for (size_t i = 0; i < netSurfaceCount; i++)
			triangleReduction((*netSurfacesInThisOctree)[i], cosNormalAngleChangeLimit, cosEdgeAngleChangeLimit, cosAngleBtwFrontierEdges);		

		// change net surface mesh into triangle polyhedron
		std::map<HedgeVertex*, gaia3d::Vertex*> dummy;
		for (size_t i = 0; i < netSurfaceCount; i++)
			changeNetSurfacesIntoTrianglePolyhedron((*netSurfacesInThisOctree)[i], dummy);

		// delete allocated memory for half edge system in all net surfaces
		for (size_t i = 0; i < netSurfaceCount; i++)
			clearHalfEdgeSystem((*netSurfacesInThisOctree)[i]);
	}

	// make mosaic texture and allocate texture coordinates on built polyhedrons
	int textureWidth = 0, textureHeight = 0;
	unsigned char* textureData = NULL;
	makeNetSurfaceTextures(allNetSurfaces, setting, &textureData, textureWidth, textureHeight);
	netSurfaceTextures[setting->lod] = textureData;
	netSurfaceTextureWidth[setting->lod] = textureWidth;
	netSurfaceTextureHeight[setting->lod] = textureHeight;

	// clear each texture on all net surface meshes
	size_t netSurfaceCount = allNetSurfaces.size();
	for (size_t i = 0; i < netSurfaceCount; i++)
		delete allNetSurfaces[i]->texture;

	// make result polyhedron on each spatial octree
	std::map<size_t, gaia3d::TrianglePolyhedron*> resultMeshes;
	std::map<size_t, std::vector<NetSurface*>*>::iterator iterOctreeNSM = octreeNetSurfaceRelations.begin();
	for (; iterOctreeNSM != octreeNetSurfaceRelations.end(); iterOctreeNSM++)
	{
		std::vector<NetSurface*>* netSurfaces = iterOctreeNSM->second;
		std::vector<gaia3d::TrianglePolyhedron*> meshes;
		size_t netSurfaceCount = netSurfaces->size();
		for (size_t i = 0; i < netSurfaceCount; i++)
			meshes.push_back((*netSurfaces)[i]->nsm);

		resultMeshes[iterOctreeNSM->first] = mergePolyhedrons(meshes);

		for (size_t i = 0; i < netSurfaceCount; i++)
			delete(*netSurfaces)[i]->nsm;

		delete netSurfaces;
	}

	// clear net surfaces
	netSurfaceCount = allNetSurfaces.size();
	for (size_t i = 0; i < netSurfaceCount; i++)
		delete allNetSurfaces[i];

	// push result meshes into container
	if (setting->lod == 2)
	{
		std::map<size_t, gaia3d::TrianglePolyhedron*>::iterator iterResultMeshes = resultMeshes.begin();
		for (; iterResultMeshes != resultMeshes.end(); iterResultMeshes++)
			((gaia3d::SpatialOctreeBox*)(octrees[iterResultMeshes->first]))->netSurfaceMesh = iterResultMeshes->second;
	}
	else
	{
		// in lod 3 or higher, merge all polyhedrons into one polyhedron
		std::vector<gaia3d::TrianglePolyhedron*> meshes;
		std::map<size_t, gaia3d::TrianglePolyhedron*>::iterator iterResultMeshes = resultMeshes.begin();
		for (; iterResultMeshes != resultMeshes.end(); iterResultMeshes++)
			meshes.push_back(iterResultMeshes->second);

		netSurfaceMeshes[setting->lod] = mergePolyhedrons(meshes);

		iterResultMeshes = resultMeshes.begin();
		for (; iterResultMeshes != resultMeshes.end(); iterResultMeshes++)
			delete iterResultMeshes->second;
	}
}

void NetSurfaceMeshMaker::makeNetSurfaceMesh(std::vector<gaia3d::OctreeBox*>& octrees,
											NetSurfaceMeshSetting* setting,
											std::map<unsigned char, gaia3d::TrianglePolyhedron*>& netSurfaceMeshes)
{
	// process configuration
	float cosNormalAngleChangeLimit = cosf(setting->maxAngleChangeOfNormalVectorForAllowingEdgeCollapse * ((float)M_PI) / 180.0f);
	float cosEdgeAngleChangeLimit = cosf(setting->maxAngleChangeOfFrontierEdgeForAllowingEdgeCollapse * ((float)M_PI) / 180.0f);
	float cosAngleBtwFrontierEdges = cosf(setting->maxAngleDifferenceBetweenNeighborFrontierEdgesForCollapse * ((float)M_PI) / 180.0f);

	// process
	size_t octreeCount = octrees.size();
	std::map<size_t, std::vector<NetSurface*>*> octreeNetSurfaceRelations;
	for (size_t h = 0; h < octreeCount; h++)
	{
		gaia3d::SpatialOctreeBox* octree = (gaia3d::SpatialOctreeBox*)octrees[h];
		if (octree->meshes.empty())
			continue;

		std::vector<NetSurface*>* netSurfacesInThisOctree = new std::vector<NetSurface*>;
		octreeNetSurfaceRelations[h] = netSurfacesInThisOctree;

		size_t meshCount = octree->meshes.size();
		for (size_t i = 0; i < meshCount; i++)
		{
			gaia3d::TrianglePolyhedron* mesh = octree->meshes[i];

			// make half edge system from original mesh
			std::map<HedgeVertex*, gaia3d::Vertex*> vertexMapper;
			NetSurface* netSurface = makeNetSurfaceMeshFromOriginalDirectly(mesh, vertexMapper);

			// triangle reduction
			triangleReduction(netSurface, cosNormalAngleChangeLimit, cosEdgeAngleChangeLimit, cosAngleBtwFrontierEdges);

			// convert result to polyhedron mesh
			changeNetSurfacesIntoTrianglePolyhedron(netSurface, vertexMapper);

			// clear half edge system
			clearHalfEdgeSystem(netSurface);

			// collect result along each octree
			netSurfacesInThisOctree->push_back(netSurface);
		}
	}

	// make result polyhedron on each spatial octree
	std::map<size_t, gaia3d::TrianglePolyhedron*> resultMeshes;
	std::map<size_t, std::vector<NetSurface*>*>::iterator iterOctreeNSM = octreeNetSurfaceRelations.begin();
	for (; iterOctreeNSM != octreeNetSurfaceRelations.end(); iterOctreeNSM++)
	{
		std::vector<NetSurface*>* netSurfaces = iterOctreeNSM->second;
		std::vector<gaia3d::TrianglePolyhedron*> meshes;
		size_t netSurfaceCount = netSurfaces->size();
		for (size_t i = 0; i < netSurfaceCount; i++)
			meshes.push_back((*netSurfaces)[i]->nsm);

		resultMeshes[iterOctreeNSM->first] = mergePolyhedrons(meshes);

		for (size_t i = 0; i < netSurfaceCount; i++)
		{
			delete(*netSurfaces)[i]->nsm;
			delete (*netSurfaces)[i];
		}

		delete netSurfaces;
	}

	// push result meshes into container
	if (setting->lod == 2)
	{
		std::map<size_t, gaia3d::TrianglePolyhedron*>::iterator iterResultMeshes = resultMeshes.begin();
		for (; iterResultMeshes != resultMeshes.end(); iterResultMeshes++)
			((gaia3d::SpatialOctreeBox*)(octrees[iterResultMeshes->first]))->netSurfaceMesh = iterResultMeshes->second;
	}
	else
	{
		// in lod 3 or higher, merge all polyhedrons into one polyhedron
		std::vector<gaia3d::TrianglePolyhedron*> meshes;
		std::map<size_t, gaia3d::TrianglePolyhedron*>::iterator iterResultMeshes = resultMeshes.begin();
		for (; iterResultMeshes != resultMeshes.end(); iterResultMeshes++)
			meshes.push_back(iterResultMeshes->second);

		netSurfaceMeshes[setting->lod] = mergePolyhedrons(meshes);

		iterResultMeshes = resultMeshes.begin();
		for (; iterResultMeshes != resultMeshes.end(); iterResultMeshes++)
			delete iterResultMeshes->second;
	}
}

void makeNet(NetSurface* netSurface,
			gaia3d::Point3D& leftBottom,
			gaia3d::Point3D& rightBottom,
			gaia3d::Point3D& leftTop,
			gaia3d::Point3D& rightTop)
{
	double leftEdgeLength = sqrt(leftBottom.squaredDistanceTo(leftTop));
	double bottomEdgeLength = sqrt(leftBottom.squaredDistanceTo(rightBottom));
	double rightEdgeLength = sqrt(rightBottom.squaredDistanceTo(rightTop));
	double topEdgeLength = sqrt(leftTop.squaredDistanceTo(rightTop));

	int nCols = (int)(bottomEdgeLength / netSurface->gridSize);
	int nRows = (int)(leftEdgeLength / netSurface->gridSize);
	if (nCols == 0) nCols = 1;
	if (nRows == 0) nRows = 1;

	for (int i = 0; i <= nRows; i++)
	{
		std::vector<HedgeVertex*>* verticesOnCurrentLine = new std::vector<HedgeVertex*>;
		
		gaia3d::Point3D leftestPoint, rightestPoint;
		leftestPoint.set( (leftBottom.x*(nRows-i) + leftTop.x*i)/nRows,
						(leftBottom.y*(nRows - i) + leftTop.y*i) / nRows,
						(leftBottom.z*(nRows - i) + leftTop.z*i) / nRows );
		rightestPoint.set( (rightBottom.x*(nRows - i) + rightTop.x*i) / nRows,
						(rightBottom.y*(nRows - i) + rightTop.y*i) / nRows,
						(rightBottom.z*(nRows - i) + rightTop.z*i) / nRows );

		for (int j = 0; j <= nCols; j++)
		{
			gaia3d::Point3D currentPoint;
			currentPoint.set( (leftestPoint.x*(nCols-j) + rightestPoint.x*j)/nCols,
							(leftestPoint.y*(nCols - j) + rightestPoint.y*j) / nCols,
							(leftestPoint.z*(nCols - j) + rightestPoint.z*j) / nCols );

			HedgeVertex* hVertex = new HedgeVertex;
			hVertex->position = currentPoint;
			verticesOnCurrentLine->push_back(hVertex);
		}


		netSurface->netVertices.push_back(verticesOnCurrentLine);
	}
}

void makeNetSurfacesFromBox(gaia3d::BoundingBox& bbox, float gridSize, std::vector<NetSurface*>& result)
{
	gaia3d::Point3D leftTop, leftBottom, rightTop, rightBottom;

	// top.*********************************************************************************************
	leftBottom.set(bbox.minX, bbox.minY, bbox.maxZ);
	rightBottom.set(bbox.maxX, bbox.minY, bbox.maxZ);
	rightTop.set(bbox.maxX, bbox.maxY, bbox.maxZ);
	leftTop.set(bbox.minX, bbox.maxY, bbox.maxZ);
	NetSurface* netSurface = new NetSurface;
	netSurface->cubeFace = CUBEFACE_TOP;
	netSurface->normalVector.set(0.0, 0.0, 1.0);
	netSurface->nsm = NULL;
	netSurface->gridSize = gridSize;
	netSurface->domainBox.addBox(bbox);
	makeNet(netSurface, leftBottom, rightBottom, leftTop, rightTop);
	result.push_back(netSurface);

	// bottom (xRot = 180).*******************************************************************************************
	leftBottom.set(bbox.minX, bbox.maxY, bbox.minZ);
	rightBottom.set(bbox.maxX, bbox.maxY, bbox.minZ);
	rightTop.set(bbox.maxX, bbox.minY, bbox.minZ);
	leftTop.set(bbox.minX, bbox.minY, bbox.minZ);
	netSurface = new NetSurface;
	netSurface->cubeFace = CUBEFACE_BOTTOM;
	netSurface->normalVector.set(0.0, 0.0, -1.0);
	netSurface->nsm = NULL;
	netSurface->gridSize = gridSize;
	netSurface->domainBox.addBox(bbox);
	makeNet(netSurface, leftBottom, rightBottom, leftTop, rightTop);
	result.push_back(netSurface);

	// front (xRot = -90).*******************************************************************************************
	leftBottom.set(bbox.minX, bbox.minY, bbox.minZ);
	rightBottom.set(bbox.maxX, bbox.minY, bbox.minZ);
	rightTop.set(bbox.maxX, bbox.minY, bbox.maxZ);
	leftTop.set(bbox.minX, bbox.minY, bbox.maxZ);
	netSurface = new NetSurface;
	netSurface->cubeFace = CUBEFACE_FRONT;
	netSurface->normalVector.set(0.0, -1.0, 0.0);
	netSurface->nsm = NULL;
	netSurface->gridSize = gridSize;
	netSurface->domainBox.addBox(bbox);
	makeNet(netSurface, leftBottom, rightBottom, leftTop, rightTop);
	result.push_back(netSurface);

	// rear (xRot = 90).*******************************************************************************************
	leftBottom.set(bbox.minX, bbox.maxY, bbox.maxZ);
	rightBottom.set(bbox.maxX, bbox.maxY, bbox.maxZ);
	rightTop.set(bbox.maxX, bbox.maxY, bbox.minZ);
	leftTop.set(bbox.minX, bbox.maxY, bbox.minZ);
	netSurface = new NetSurface;
	netSurface->cubeFace = CUBEFACE_REAR;
	netSurface->normalVector.set(0.0, 1.0, 0.0);
	netSurface->nsm = NULL;
	netSurface->gridSize = gridSize;
	netSurface->domainBox.addBox(bbox);
	makeNet(netSurface, leftBottom, rightBottom, leftTop, rightTop);
	result.push_back(netSurface);

	// left (yRot = 90).*******************************************************************************************
	leftBottom.set(bbox.minX, bbox.minY, bbox.minZ);
	rightBottom.set(bbox.minX, bbox.minY, bbox.maxZ);
	rightTop.set(bbox.minX, bbox.maxY, bbox.maxZ);
	leftTop.set(bbox.minX, bbox.maxY, bbox.minZ);
	netSurface = new NetSurface;
	netSurface->cubeFace = CUBEFACE_LEFT;
	netSurface->normalVector.set(-1.0, 0.0, 0.0);
	netSurface->nsm = NULL;
	netSurface->gridSize = gridSize;
	netSurface->domainBox.addBox(bbox);
	makeNet(netSurface, leftBottom, rightBottom, leftTop, rightTop);
	result.push_back(netSurface);

	// right (yRot = -90).*******************************************************************************************
	leftBottom.set(bbox.maxX, bbox.minY, bbox.maxZ);
	rightBottom.set(bbox.maxX, bbox.minY, bbox.minZ);
	rightTop.set(bbox.maxX, bbox.maxY, bbox.minZ);
	leftTop.set(bbox.maxX, bbox.maxY, bbox.maxZ);
	netSurface = new NetSurface;
	netSurface->cubeFace = CUBEFACE_RIGHT;
	netSurface->normalVector.set(1.0, 0.0, 0.0);
	netSurface->nsm = NULL;
	netSurface->gridSize = gridSize;
	netSurface->domainBox.addBox(bbox);
	makeNet(netSurface, leftBottom, rightBottom, leftTop, rightTop);
	result.push_back(netSurface);
}

void drawMeshesForDepthDetection(std::vector<gaia3d::TrianglePolyhedron*>& meshes,
				unsigned int shaderProgram,
				bool bOnlyExterior)
{
	// do this only once.
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	glEnable(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Shader.
	int oneVertexBuffSize = 3 * 4 + 3 * 4 + 2 * 4;
	GLfloat mv_matrix[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, mv_matrix);

	GLfloat proj_matrix[16];
	glGetFloatv(GL_PROJECTION_MATRIX, proj_matrix);

	GLfloat mvProj_matrix[16];

	gaia3d::Matrix4 mv_mat, proj_mat, mvProj_mat, mv_invMat, mv_invTransposedMat;

	mv_mat.set(mv_matrix[0], mv_matrix[4], mv_matrix[8], mv_matrix[12],
		mv_matrix[1], mv_matrix[5], mv_matrix[9], mv_matrix[13],
		mv_matrix[2], mv_matrix[6], mv_matrix[10], mv_matrix[14],
		mv_matrix[3], mv_matrix[7], mv_matrix[11], mv_matrix[15]);

	proj_mat.set(proj_matrix[0], proj_matrix[4], proj_matrix[8], proj_matrix[12],
		proj_matrix[1], proj_matrix[5], proj_matrix[9], proj_matrix[13],
		proj_matrix[2], proj_matrix[6], proj_matrix[10], proj_matrix[14],
		proj_matrix[3], proj_matrix[7], proj_matrix[11], proj_matrix[15]);

	mvProj_mat = proj_mat*mv_mat;
	mvProj_mat.getFloatArray(mvProj_matrix);

	mv_invMat = mv_mat.inverse();
	mv_invTransposedMat = mv_invMat.transpose();
	float normalMat3[9];
	mv_invMat.getOnlyRotationFloatArray(normalMat3);

	glUseProgram(shaderProgram);

	int normalMatrix_location = glGetUniformLocation(shaderProgram, "uNMatrix");
	int mvMatrix_location = glGetUniformLocation(shaderProgram, "ModelViewProjectionMatrix");

	glUniformMatrix4fv(mvMatrix_location, 1, false, mvProj_matrix);
	glUniformMatrix3fv(normalMatrix_location, 1, false, normalMat3);

	int position_location = glGetAttribLocation(shaderProgram, "position");
	int texCoord_location = glGetAttribLocation(shaderProgram, "aTextureCoord");
	int normal_location = glGetAttribLocation(shaderProgram, "aVertexNormal");

	glEnableVertexAttribArray(position_location);
	glDisableVertexAttribArray(texCoord_location);
	glEnableVertexAttribArray(normal_location);

	int samplerUniform = glGetUniformLocation(shaderProgram, "uSampler");
	glUniform1i(samplerUniform, 0);

	int hasTexture_location = glGetUniformLocation(shaderProgram, "hasTexture");
	int colorAux_location = glGetUniformLocation(shaderProgram, "vColorAux");
	int bUseLighting_location = glGetUniformLocation(shaderProgram, "useLighting");

	glUniform1i(bUseLighting_location, false);
	glUniform1i(hasTexture_location, false);

	//glUniform1i(hasTexture_location, true);

	size_t meshCount = meshes.size();
	for (size_t i = 0; i < meshCount; i++)
	{
		gaia3d::TrianglePolyhedron* polyhedron = meshes[i];

		if (bOnlyExterior && !polyhedron->doesHaveAnyExteriorSurface())
			continue;

		size_t vboCount = polyhedron->getVbos().size();
		// filter out empty mesh
		if (vboCount == 0)
			continue;

		GLfloat* vertices;
		GLushort* indices;
		GLfloat* normals;
		for (size_t j = 0; j < vboCount; j++)
		{
			vertices = normals = NULL;
			indices = NULL;

			gaia3d::Vbo* vbo = polyhedron->getVbos()[j];

			size_t vertexCount = vbo->vertices.size();
			if (vertexCount == 0)
				continue;

			// make vertex array
			vertices = new GLfloat[vertexCount * 3];
			memset(vertices, 0x00, sizeof(GLfloat) * 3 * vertexCount);
			for (size_t k = 0; k < vertexCount; k++)
			{
				vertices[k * 3] = float(vbo->vertices[k]->position.x);
				vertices[k * 3 + 1] = float(vbo->vertices[k]->position.y);
				vertices[k * 3 + 2] = float(vbo->vertices[k]->position.z);
			}

			// make index array
			size_t indexCount = vbo->indices.size();
			indices = new GLushort[indexCount];
			memset(indices, 0x00, sizeof(GLushort)*indexCount);
			for (size_t k = 0; k < indexCount; k++)
				indices[k] = vbo->indices[k];

			// make normal array
			normals = new GLfloat[vertexCount * 3];
			memset(normals, 0x00, sizeof(GLfloat) * 3 * vertexCount);
			for (size_t k = 0; k < vertexCount; k++)
			{
				normals[k * 3] = float(vbo->vertices[k]->normal.x);
				normals[k * 3 + 1] = float(vbo->vertices[k]->normal.y);
				normals[k * 3 + 2] = float(vbo->vertices[k]->normal.z);
			}

			// at this point, all necessary arrays are made

			// bind vertex array
			unsigned int vertexArrayId;
			glGenBuffers(1, &vertexArrayId);
			glBindBuffer(GL_ARRAY_BUFFER, vertexArrayId);
			glBufferData(GL_ARRAY_BUFFER, vertexCount * 3 * 4, vertices, GL_STATIC_DRAW);
			glVertexAttribPointer(position_location, 3, GL_FLOAT, GL_FALSE, 3 * 4, (void*)0);    //send positions on pipe 0
			delete[] vertices;

			// bind index array
			unsigned int indexArrayId;
			glGenBuffers(1, &indexArrayId);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexArrayId);// bind VBO in order to use
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * 2, indices, GL_STATIC_DRAW);// upload data to VBO
			delete[] indices;

			// bind normal array
			unsigned int normalArrayId;
			glGenBuffers(1, &normalArrayId);
			glBindBuffer(GL_ARRAY_BUFFER, normalArrayId);
			glBufferData(GL_ARRAY_BUFFER, vertexCount * 3 * 4, normals, GL_STATIC_DRAW);
			glVertexAttribPointer(normal_location, 3, GL_FLOAT, GL_FALSE, 3 * 4, (void*)0);
			delete[] normals;

			// color
			float colorR = 0.5f, colorG = 0.5f, colorB = 0.5f;
			glUniform3f(colorAux_location, colorR, colorG, colorB);

			glDrawElements(GL_TRIANGLES, (int)indexCount, GL_UNSIGNED_SHORT, 0);

			glDeleteBuffers(1, &vertexArrayId);
			glDeleteBuffers(1, &indexArrayId);
			glDeleteBuffers(1, &normalArrayId);
		}
	}

	glDisableVertexAttribArray(position_location);
	glDisableVertexAttribArray(normal_location);

	glUseProgram(0);
}

void drawTrianglesForTexture(std::vector<gaia3d::TrianglePolyhedron*>& meshes, std::map<std::string, unsigned int>& bindingResult)
{
	glColor3f(0.0f, 0.0f, 1.0f);
	glEnable(GL_TEXTURE_2D);
	
	size_t meshCount = meshes.size();
	for (size_t i = 0; i < meshCount; i++)
	{
		gaia3d::TrianglePolyhedron* mesh = meshes[i];
		unsigned int textureId = 0;
		if (bindingResult.find(mesh->getStringAttribute(TextureName)) != bindingResult.end())
		{
			textureId = bindingResult[mesh->getStringAttribute(TextureName)];
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
			glBindTexture(GL_TEXTURE_2D, textureId);
		}
		glBegin(GL_TRIANGLES);
		size_t surfaceCount = mesh->getSurfaces().size();
		for (size_t j = 0; j < surfaceCount; j++)
		{
			gaia3d::Surface* surface = mesh->getSurfaces()[j];
			size_t triangleCount = surface->getTriangles().size();
			for (size_t k = 0; k < triangleCount; k++)
			{
				gaia3d::Triangle *tri = surface->getTriangles()[k];

				if(textureId != 0)
					glTexCoord2f((float)tri->getVertices()[0]->textureCoordinate[0], (float)tri->getVertices()[0]->textureCoordinate[1]);
				glVertex3f((float)tri->getVertices()[0]->position.x, (float)tri->getVertices()[0]->position.y, (float)tri->getVertices()[0]->position.z);
				if (textureId != 0)
					glTexCoord2f((float)tri->getVertices()[1]->textureCoordinate[0], (float)tri->getVertices()[1]->textureCoordinate[1]);
				glVertex3f((float)tri->getVertices()[1]->position.x, (float)tri->getVertices()[1]->position.y, (float)tri->getVertices()[1]->position.z);
				if (textureId != 0)
					glTexCoord2f((float)tri->getVertices()[2]->textureCoordinate[0], (float)tri->getVertices()[2]->textureCoordinate[1]);
				glVertex3f((float)tri->getVertices()[2]->position.x, (float)tri->getVertices()[2]->position.y, (float)tri->getVertices()[2]->position.z);
				
			}
		}
		glEnd();
	}
	
	glBindTexture(GL_TEXTURE_2D, 0);
}

void drawMeshesForTexture(std::vector<gaia3d::TrianglePolyhedron*>& meshes,
						unsigned int shaderProgram,
						std::map<std::string, unsigned int>& bindingResult)
{
	if (meshes.empty())
		return;

	// do this only once.
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	glEnable(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Shader.
	int oneVertexBuffSize = 3 * 4 + 3 * 4 + 2 * 4;
	GLfloat mv_matrix[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, mv_matrix);

	GLfloat proj_matrix[16];
	glGetFloatv(GL_PROJECTION_MATRIX, proj_matrix);

	GLfloat mvProj_matrix[16];

	gaia3d::Matrix4 mv_mat, proj_mat, mvProj_mat, mv_invMat, mv_invTransposedMat;

	mv_mat.set(mv_matrix[0], mv_matrix[4], mv_matrix[8], mv_matrix[12],
		mv_matrix[1], mv_matrix[5], mv_matrix[9], mv_matrix[13],
		mv_matrix[2], mv_matrix[6], mv_matrix[10], mv_matrix[14],
		mv_matrix[3], mv_matrix[7], mv_matrix[11], mv_matrix[15]);

	proj_mat.set(proj_matrix[0], proj_matrix[4], proj_matrix[8], proj_matrix[12],
		proj_matrix[1], proj_matrix[5], proj_matrix[9], proj_matrix[13],
		proj_matrix[2], proj_matrix[6], proj_matrix[10], proj_matrix[14],
		proj_matrix[3], proj_matrix[7], proj_matrix[11], proj_matrix[15]);

	mvProj_mat = proj_mat*mv_mat;
	mvProj_mat.getFloatArray(mvProj_matrix);

	mv_invMat = mv_mat.inverse();
	mv_invTransposedMat = mv_invMat.transpose();
	float normalMat3[9];
	mv_invMat.getOnlyRotationFloatArray(normalMat3);

	glUseProgram(shaderProgram);

	int normalMatrix_location = glGetUniformLocation(shaderProgram, "uNMatrix");
	int mvMatrix_location = glGetUniformLocation(shaderProgram, "ModelViewProjectionMatrix");

	glUniformMatrix4fv(mvMatrix_location, 1, false, mvProj_matrix);
	glUniformMatrix3fv(normalMatrix_location, 1, false, normalMat3);

	int position_location = glGetAttribLocation(shaderProgram, "position");
	int texCoord_location = glGetAttribLocation(shaderProgram, "aTextureCoord");
	int normal_location = glGetAttribLocation(shaderProgram, "aVertexNormal");

	glEnableVertexAttribArray(position_location);
	glEnableVertexAttribArray(texCoord_location);
	glDisableVertexAttribArray(normal_location);

	int samplerUniform = glGetUniformLocation(shaderProgram, "uSampler");
	glUniform1i(samplerUniform, 0);

	int hasTexture_location = glGetUniformLocation(shaderProgram, "hasTexture");
	int colorAux_location = glGetUniformLocation(shaderProgram, "vColorAux");
	int bUseLighting_location = glGetUniformLocation(shaderProgram, "useLighting");

	glUniform1i(bUseLighting_location, false);

	size_t meshCount = meshes.size();
	for (size_t i = 0; i < meshCount; i++)
	{
		gaia3d::TrianglePolyhedron* polyhedron = meshes[i];

		size_t vboCount = polyhedron->getVbos().size();

		if (vboCount == 0)
			continue;

		GLfloat* vertices, *textureCoordinates;
		GLushort* indices;
		unsigned int textureCoordinateArrayId;
		for (size_t j = 0; j < vboCount; j++)
		{
			vertices = textureCoordinates = NULL;
			indices = NULL;

			gaia3d::Vbo* vbo = polyhedron->getVbos()[j];

			// make vertex array
			size_t vertexCount = vbo->vertices.size();
			if (vertexCount == 0)
				continue;

			vertices = new GLfloat[vertexCount * 3];
			memset(vertices, 0x00, sizeof(GLfloat) * 3 * vertexCount);

			for (size_t k = 0; k < vertexCount; k++)
			{
				vertices[k * 3] = float(vbo->vertices[k]->position.x);
				vertices[k * 3 + 1] = float(vbo->vertices[k]->position.y);
				vertices[k * 3 + 2] = float(vbo->vertices[k]->position.z);
			}

			// make index array
			size_t indexCount = vbo->indices.size();
			indices = new GLushort[indexCount];
			memset(indices, 0x00, sizeof(GLushort)*indexCount);
			for (size_t k = 0; k < indexCount; k++)
				indices[k] = vbo->indices[k];

			// at this point, all necessary arrays are made

			// bind vertex array
			unsigned int vertexArrayId;
			glGenBuffers(1, &vertexArrayId);
			glBindBuffer(GL_ARRAY_BUFFER, vertexArrayId);
			glBufferData(GL_ARRAY_BUFFER, vertexCount * 3 * 4, vertices, GL_STATIC_DRAW);
			glVertexAttribPointer(position_location, 3, GL_FLOAT, GL_FALSE, 3 * 4, (void*)0);    //send positions on pipe 0
			delete[] vertices;

			// bind index array
			unsigned int indexArrayId;
			glGenBuffers(1, &indexArrayId);// generate a new VBO and get the associated ID
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexArrayId);// bind VBO in order to use
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * 2, indices, GL_STATIC_DRAW);// upload data to VBO
			delete[] indices;

			if (polyhedron->doesThisHaveTextureCoordinates())
			{
				// 이 polyhedron을 그릴 때 필요한 texture를 activate 시킨다.
				std::string textureName = polyhedron->getStringAttribute(std::string(TextureName));
				if (bindingResult.find(textureName) != bindingResult.end())
				{
					unsigned int textureId = bindingResult[textureName];
					glBindTexture(GL_TEXTURE_2D, textureId);
				}

				glUniform1i(hasTexture_location, true);

				// make texture coordinate array
				textureCoordinates = new GLfloat[vertexCount * 2];
				memset(textureCoordinates, 0x00, sizeof(GLfloat) * 2 * vertexCount);
				for (size_t k = 0; k < vertexCount; k++)
				{
					textureCoordinates[k * 2] = float(vbo->vertices[k]->textureCoordinate[0]);
					textureCoordinates[k * 2 + 1] = float(vbo->vertices[k]->textureCoordinate[1]);
				}

				// bind texture coordinate array
				glGenBuffers(1, &textureCoordinateArrayId);
				glBindBuffer(GL_ARRAY_BUFFER, textureCoordinateArrayId);
				glBufferData(GL_ARRAY_BUFFER, vertexCount * 2 * 4, textureCoordinates, GL_STATIC_DRAW);
				glVertexAttribPointer(texCoord_location, 2, GL_FLOAT, GL_FALSE, 2 * 4, (void*)0);
				delete[] textureCoordinates;
			}
			else
			{
				glUniform1i(hasTexture_location, false);
				glDisableVertexAttribArray(texCoord_location);
				float colorR, colorG, colorB;

				if (polyhedron->getColorMode() == gaia3d::SingleColor)
				{
					colorR = GetRedValue(polyhedron->getSingleColor()) / 255.0f;
					colorG = GetGreenValue(polyhedron->getSingleColor()) / 255.0f;
					colorB = GetBlueValue(polyhedron->getSingleColor()) / 255.0f;
				}
				else
				{
					colorR = GetRedValue(polyhedron->getVertices()[0]->color) / 255.0f;
					colorG = GetGreenValue(polyhedron->getVertices()[0]->color) / 255.0f;
					colorB = GetBlueValue(polyhedron->getVertices()[0]->color) / 255.0f;
				}

				glUniform3f(colorAux_location, colorR, colorG, colorB);
			}

			glDrawElements(GL_TRIANGLES, (int)indexCount, GL_UNSIGNED_SHORT, 0);

			glDeleteBuffers(1, &vertexArrayId);
			glDeleteBuffers(1, &indexArrayId);
			if (polyhedron->doesThisHaveTextureCoordinates())
				glDeleteBuffers(1, &textureCoordinateArrayId);
		}
	}

	glDisableVertexAttribArray(position_location);
	glDisableVertexAttribArray(texCoord_location);

	glUseProgram(0);
}

bool checkIfPixelRepresentGeometry(GLfloat* depthBuffer,
									GLubyte* colorBuffer,
									int bufferWidth,
									int bufferHeight,
									int pixelX,
									int pixelY,
									unsigned char backgroundColor[3],
									float& zDepth)
{
	bool bIntersection = false;

	int pixelX_corrected, pixelY_corrected;
	int dataRGBIdx = (pixelX + pixelY * bufferWidth) * 4;
	unsigned char r, g, b;
	r = colorBuffer[dataRGBIdx];
	g = colorBuffer[dataRGBIdx + 1];
	b = colorBuffer[dataRGBIdx + 2];

	if (r != backgroundColor[0] || g != backgroundColor[1] || b != backgroundColor[2])
	{
		bIntersection = true;
		pixelX_corrected = pixelX;
		pixelY_corrected = pixelY;
	}
	else
	{
		// in the pixel there are no geometry.
		bIntersection = false;
	}

	// check neighborhood pixels
	if (!bIntersection)
	{
		// check left pixel.***
		if (pixelX > 0)
		{
			dataRGBIdx = (pixelX - 1 + pixelY * bufferWidth) * 4;
			r = colorBuffer[dataRGBIdx];
			g = colorBuffer[dataRGBIdx + 1];
			b = colorBuffer[dataRGBIdx + 2];
			if (r != backgroundColor[0] || g != backgroundColor[1] || b != backgroundColor[2])
			{
				bIntersection = true;
				pixelX_corrected = pixelX - 1;
				pixelY_corrected = pixelY;
			}
			else
			{
				// no geometry in left pixel.****
				bIntersection = false;
			}
		}
	}

	if (!bIntersection)
	{
		// check right pixel.***
		if (pixelX < bufferWidth)
		{
			dataRGBIdx = (pixelX + 1 + pixelY * bufferWidth) * 4;
			r = colorBuffer[dataRGBIdx];
			g = colorBuffer[dataRGBIdx + 1];
			b = colorBuffer[dataRGBIdx + 2];
			if (r != backgroundColor[0] || g != backgroundColor[1] || b != backgroundColor[2])
			{
				bIntersection = true;
				pixelX_corrected = pixelX + 1;
				pixelY_corrected = pixelY;
			}
			else
			{
				// no geometry in left pixel.****
				bIntersection = false;
			}
		}
	}

	if (!bIntersection)
	{
		// check top pixel.***
		if (pixelY < bufferHeight)
		{
			dataRGBIdx = (pixelX + (pixelY + 1) * bufferWidth) * 4;
			r = colorBuffer[dataRGBIdx];
			g = colorBuffer[dataRGBIdx + 1];
			b = colorBuffer[dataRGBIdx + 2];
			if (r != backgroundColor[0] || g != backgroundColor[1] || b != backgroundColor[2])
			{
				bIntersection = true;
				pixelX_corrected = pixelX;
				pixelY_corrected = pixelY + 1;
			}
			else
			{
				// no geometry in left pixel.****
				bIntersection = false;
			}
		}
	}

	if (!bIntersection)
	{
		// check bottom pixel.***
		if (pixelY > 0)
		{
			dataRGBIdx = (pixelX + (pixelY - 1) * bufferWidth) * 4;
			r = colorBuffer[dataRGBIdx];
			g = colorBuffer[dataRGBIdx + 1];
			b = colorBuffer[dataRGBIdx + 2];
			if (r != backgroundColor[0] || g != backgroundColor[1] || b != backgroundColor[2])
			{
				bIntersection = true;
				pixelX_corrected = pixelX;
				pixelY_corrected = pixelY - 1;
			}
			else
			{
				// no geometry in left pixel.****
				bIntersection = false;
			}
		}
	}

	if (bIntersection)
	{
		int dataIdx;
		dataIdx = pixelX_corrected + pixelY_corrected * bufferWidth;
		zDepth = depthBuffer[dataIdx];
	}

	return bIntersection;
}

void makeProjectedModel(NetSurface* netSurface,
						std::vector<gaia3d::TrianglePolyhedron*>& meshes,
						bool bOnlyExterior,
						SceneControlVariables* scv,
						unsigned int shaderProgramDepthDetection,
						unsigned int shaderProgramTexture,
						std::map<std::string, unsigned int>& bindingResult)
{
	if (netSurface->netVertices.size() < 2)
		return;

	if (netSurface->netVertices[0]->size() < 0)
		return;

	TYPE_CUBEFACE faceType = netSurface->cubeFace;
	gaia3d::BoundingBox& bbox = netSurface->domainBox;

	// setup viewport
	int screenSize = MemoryDeviceContextEdgeLength;
	scv->tp_projection = PROJECTION_ORTHO;
	glViewport(0, 0, screenSize, screenSize);

	gaia3d::Point3D centerPoint; bbox.getCenterPoint(centerPoint.x, centerPoint.y, centerPoint.z);
	float drawingBufferWidth = (float)screenSize, drawingBufferHeight = (float)screenSize;

	int wa = screenSize, ha = screenSize;

	// Mini define_space_of_visualitzation.***********************************************************************************************
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// FaceType (0 = Top), (1 = Bottom), (2 = Front), (3 = Rear), (4 = Left), (5 = Right).***
	float frustumNear;
	float frustumFar;
	float frustumTop, frustumBottom, frustumLeft, frustumRight;
	switch (faceType)
	{
	case CUBEFACE_TOP:
	case CUBEFACE_BOTTOM:
	{
		frustumNear = (float)(-(bbox.getZLength() / 2.0));
		frustumFar = (float)(bbox.getZLength() / 2.0);

		frustumBottom = (float)(-(bbox.getYLength() / 2.0));
		frustumTop = (float)(bbox.getYLength() / 2.0);

		frustumLeft = (float)(-(bbox.getXLength() / 2.0));
		frustumRight = (float)(bbox.getXLength() / 2.0);
	}
	break;
	case CUBEFACE_FRONT:
	case CUBEFACE_REAR:
	{
		frustumNear = (float)(-(bbox.getYLength() / 2.0));
		frustumFar = (float)(bbox.getYLength() / 2.0);

		frustumBottom = (float)(-(bbox.getZLength() / 2.0));
		frustumTop = (float)(bbox.getZLength() / 2.0);

		frustumLeft = (float)(-(bbox.getXLength() / 2.0));
		frustumRight = (float)(bbox.getXLength() / 2.0);
	}
	break;
	case CUBEFACE_LEFT:
	case CUBEFACE_RIGHT:
	{
		frustumNear = (float)(-(bbox.getXLength() / 2.0));
		frustumFar = (float)(bbox.getXLength() / 2.0);

		frustumBottom = (float)(-(bbox.getZLength() / 2.0));
		frustumTop = (float)(bbox.getZLength() / 2.0);

		frustumLeft = (float)(-(bbox.getYLength() / 2.0));
		frustumRight = (float)(bbox.getYLength() / 2.0);
	}
	break;
	}

	wglMakeCurrent(scv->m_myhDC, scv->m_hRC);
	glViewport(0, 0, (GLsizei)screenSize, (GLsizei)screenSize); // Set the viewport 
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//glOrtho(-scv->m_nRange*wa / ha, scv->m_nRange*wa / ha, -scv->m_nRange, scv->m_nRange, frustumNear, frustumFar); // Note that wa/ha = 1.***
	glOrtho(frustumLeft*wa / ha, frustumRight*wa / ha, frustumBottom, frustumTop, frustumNear, frustumFar); // Note that wa/ha = 1.***
	glMatrixMode(GL_MODELVIEW);
	// End Mini define_space_of_visualitzation.--------------------------------------------------------------------------------------------

	glClearColor(scv->ClearColor[0], scv->ClearColor[1], scv->ClearColor[2], scv->ClearColor[3]);
	// Clear the screen and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Reset the model matrix
	//glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// setup target position
	double xRot = 0.0, yRot = 0.0, zRot = 0.0;
	switch (faceType)
	{
	case CUBEFACE_TOP:
	{
		xRot = 0.0; yRot = 0.0; zRot = 0.0;
	}
	break;
	case CUBEFACE_BOTTOM:
	{
		xRot = 180.0; yRot = 0.0; zRot = 0.0;
	}
	break;
	case CUBEFACE_FRONT:
	{
		xRot = -90.0; yRot = 0.0; zRot = 0.0;
	}
	break;
	case CUBEFACE_REAR:
	{
		xRot = 90.0; yRot = 0.0; zRot = 0.0;
	}
	break;
	case CUBEFACE_LEFT:
	{
		xRot = 0.0; yRot = 90.0; zRot = 0.0;
	}
	break;
	case CUBEFACE_RIGHT:
	{
		xRot = 0.0; yRot = -90.0; zRot = 0.0;
	}
	break;
	}
	scv->m_xRot = xRot; scv->m_yRot = yRot; scv->m_zRot = zRot;
	scv->m_xPos = -centerPoint.x;
	scv->m_yPos = -centerPoint.y;
	scv->m_zPos = -centerPoint.z;

	glReadBuffer(GL_BACK);
	glDrawBuffer(GL_BACK);
	glDisable(GL_LIGHTING);

	glRotatef((float)scv->m_xRot, 1.0f, 0.0f, 0.0f);
	glRotatef((float)scv->m_yRot, 0.0f, 1.0f, 0.0f);
	glRotatef((float)scv->m_zRot, 0.0f, 0.0f, 1.0f);

	glTranslatef((float)scv->m_xPos, (float)scv->m_yPos, (float)scv->m_zPos);

	// render target
	drawMeshesForDepthDetection(meshes, shaderProgramDepthDetection, bOnlyExterior);

	// retrieve color & depth buffer from rendered result
	GLfloat* depthBuffer = new GLfloat[wa*ha];
	memset(depthBuffer, 0x00, sizeof(GLfloat)*wa*ha);
	GLubyte* colorBuffer = new GLubyte[wa*ha * 4];
	memset(colorBuffer, 0x00, sizeof(GLubyte)*wa*ha * 4);
	unsigned char backgroundColor[3] = {255, 255, 255};
	glReadPixels(0, 0, wa, ha, GL_DEPTH_COMPONENT, GL_FLOAT, depthBuffer); // read depth buffer
	glReadPixels(0, 0, wa, ha, GL_RGBA, GL_UNSIGNED_BYTE, colorBuffer); // read color buffer

	// at this point, make netSurfacTexture
	netSurface->texture = new unsigned char[netSurface->textureWidth * netSurface->textureHeight * 4];
	memset(netSurface->texture, 0x00, netSurface->textureWidth * netSurface->textureHeight * 4);
	glViewport(0, 0, (GLsizei)netSurface->textureWidth, (GLsizei)netSurface->textureHeight); // Set the viewport 
	glClearColor(0.6f, 0.6f, 0.6f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	drawMeshesForTexture(meshes, shaderProgramTexture, bindingResult);
	//drawTrianglesForTexture(meshes, bindingResult);
	glReadPixels(0, 0, netSurface->textureWidth, netSurface->textureHeight, GL_RGBA, GL_UNSIGNED_BYTE, netSurface->texture);

	// move vertices on netSurface onto geometry surface
	gaia3d::Matrix4 mv_mat, mv_invMat, mv_invTransposedMat;
	GLfloat mv_matrix[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, mv_matrix);
	mv_mat.set(mv_matrix[0], mv_matrix[1], mv_matrix[2], mv_matrix[3],
				mv_matrix[4], mv_matrix[5], mv_matrix[6], mv_matrix[7],
				mv_matrix[8], mv_matrix[9], mv_matrix[10], mv_matrix[11],
				mv_matrix[12], mv_matrix[13], mv_matrix[14], mv_matrix[15]);
	mv_invMat = mv_mat.inverse();

	float planeWidth = frustumRight * 2.0f;
	float planeHeight = frustumTop * 2.0f;

	size_t rowCount = netSurface->netVertices.size();
	for (size_t i = 0; i < rowCount; i++)
	{
		std::vector<HedgeVertex*>* verticesInRow = netSurface->netVertices[i];
		size_t vertexCount = verticesInRow->size();
		for (size_t j = 0; j < vertexCount; j++)
		{
			HedgeVertex* hVertex = (*verticesInRow)[j];

			gaia3d::Point3D posWorldCoord = hVertex->position;

			// calculate posCamCoord
			gaia3d::Point3D posCamCoord = mv_mat * posWorldCoord;

			// at this point, all netSurface grid points are on near frustum.

			// translate posCamCoord to pixel coordinate
			int pixelX = (int)(posCamCoord.x * drawingBufferWidth / planeWidth);
			int pixelY = (int)(posCamCoord.y * drawingBufferHeight / planeHeight);

			pixelX = (int)(pixelX + drawingBufferWidth / 2 - 1);
			pixelY = (int)(pixelY + drawingBufferHeight / 2 - 1);

			if (pixelX < 0)
				pixelX = 0;

			if (pixelY < 0)
				pixelY = 0;

			if (pixelX == wa)
				pixelX = wa - 1;

			if (pixelY == ha)
				pixelY = ha - 1;

			// check if this pixel represents geometry or background and calculate depth onto geometry surface
			float zDepth = 0.0f;
			hVertex->bOnGeometrySurface = checkIfPixelRepresentGeometry(depthBuffer, colorBuffer, wa, ha, pixelX, pixelY, backgroundColor, zDepth);

			float ndcZ = (2 * zDepth) - 1; // normalized device coordinate.
			float camZ = ((ndcZ + (frustumFar + frustumNear) / (frustumFar - frustumNear))*(frustumFar - frustumNear)) / -2;

			// do a little correction in the zValue.***
			camZ -= 0.007f; // rest 7mm

			// posCamCoord of vertex projected onto geometry surface
			posCamCoord.z = camZ;

			// now calculate posWorldCoord of projected vertex
			hVertex->position = mv_invMat * posCamCoord;
		}
	}

	delete[] depthBuffer;
	delete[] colorBuffer;

	glEnable(GL_LIGHTING);
	glPopMatrix();
}

void makeModelProjectedOnTargetSurface(std::vector<NetSurface*>& netSurfaces,
										std::vector<gaia3d::TrianglePolyhedron*>& meshes,
										bool bOnlyExterior,
										SceneControlVariables* scv,
										unsigned int shaderProgramDepthDetection,
										unsigned int shaderProgramTexture,
										std::map<std::string, unsigned int>& bindingResult)
{
	double backupXRot = scv->m_xRot, backupYRot = scv->m_yRot, backupZRot = scv->m_zRot;
	double backupXPos = scv->m_xPos, backupYPos = scv->m_yPos, backupZPos = scv->m_zPos;
	float backupNRange = scv->m_nRange;

	size_t netSurfaceCount = netSurfaces.size();
	for (size_t i = 0; i < netSurfaceCount; i++)
		makeProjectedModel(netSurfaces[i], meshes, bOnlyExterior, scv, shaderProgramDepthDetection, shaderProgramTexture, bindingResult);

	scv->m_xRot = backupXRot; scv->m_yRot = backupYRot; scv->m_zRot = backupZRot;
	scv->m_xPos = backupXPos; scv->m_yPos = backupYPos; scv->m_zPos = backupZPos;
	scv->m_nRange  = backupNRange;
}

bool makeHedgeTriangle(HedgeVertex* vertex0, HedgeVertex* vertex1, HedgeVertex* vertex2,
								std::vector<HedgeTriangle*>* triangleList,
								std::vector<Hedge*>& hedges,
								gaia3d::Point3D& netSurfaceNormal,
								bool isCurTriangleFrontier,
								float maxSquaredDist, float maxSquaredDistFrontier)
{
	// filtering case where any vertex is not on geometry surface
	if (!vertex0->bOnGeometrySurface || !vertex1->bOnGeometrySurface || !vertex2->bOnGeometrySurface)
		return false;

	// filtering case where angle btw triangle normal and netsurface normal is larger than limit
	double maxAngleDeviation = M_PI / 3.0;
	double cosMaxAngleDeviation = cos(maxAngleDeviation);
	gaia3d::Point3D normal;
	gaia3d::GeometryUtility::calculatePlaneNormal(vertex0->position.x, vertex0->position.y, vertex0->position.z,
												vertex1->position.x, vertex1->position.y, vertex1->position.z,
												vertex2->position.x, vertex2->position.y, vertex2->position.z,
												normal.x, normal.y, normal.z, true);

	double cosAngle = normal.x*netSurfaceNormal.x + normal.y*netSurfaceNormal.y + normal.z*netSurfaceNormal.z;
	if (cosAngle < cosMaxAngleDeviation)
		return false;

	// filtering case where any triangle edge length is larger than limit
	double length0 = vertex0->position.squaredDistanceTo(vertex1->position);
	double length1 = vertex0->position.squaredDistanceTo(vertex2->position);
	double length2 = vertex1->position.squaredDistanceTo(vertex2->position);
	if (isCurTriangleFrontier)
		if (length0 >= maxSquaredDistFrontier || length1 >= maxSquaredDistFrontier || length2 >= maxSquaredDistFrontier)
			return false;
	else
		if (length0 >= maxSquaredDist|| length1 >= maxSquaredDist|| length2 >= maxSquaredDist)
			return false;

	// let's make the mesh

	HedgeTriangle* triangle = new HedgeTriangle;
	triangle->vertex0 = vertex0; triangle->vertex1 = vertex1; triangle->vertex2 = vertex2;

	Hedge* hedge0 = new Hedge;
	Hedge* hedge1 = new Hedge;
	Hedge* hedge2 = new Hedge;

	hedge0->face = triangle;
	hedge0->next = hedge1;
	hedge0->startVertex = vertex0;
	hedge0->twin = NULL;
	vertex0->hedgesFromThisVertex.push_back(hedge0);

	hedge1->face = triangle;
	hedge1->next = hedge2;
	hedge1->startVertex = vertex1;
	hedge1->twin = NULL;
	vertex1->hedgesFromThisVertex.push_back(hedge1);

	hedge2->face = triangle;
	hedge2->next = hedge0;
	hedge2->startVertex = vertex2;
	hedge2->twin = NULL;
	vertex2->hedgesFromThisVertex.push_back(hedge2);

	hedge0->direction = vertex1->position - vertex0->position;
	hedge1->direction = vertex2->position - vertex1->position;
	hedge2->direction = vertex0->position - vertex2->position;
	hedge0->direction.normalize();
	hedge1->direction.normalize();
	hedge2->direction.normalize();

	triangle->hedge = hedge0;
	triangle->normal = normal;

	triangleList->push_back(triangle);
	hedges.push_back(hedge0);
	hedges.push_back(hedge1);
	hedges.push_back(hedge2);

	return true;
}

void makeNetSurfaceMeshes(NetSurface* netSurface,
						float maxLengthForInnerEdgeSkirting,
						float maxLengthForFrontierEdgeSkirting)
{
	// this function makes a halfEdge based mesh.***
	// triangles.***************************************
	//
	//        B0----B1----B2----B3----B4-   -BN    <- vertexListB
	//         |\ T1 |\ T3 |\ T5 |\ T7 |\    |
	//         | \   | \   | \   | \   |     |
	//         |  \  |  \  |  \  |  \  |     |
	//         |   \ |   \ |   \ |   \ |     |
	//         | T0 \| T2 \| T4 \| T6 \|    \|
	//        A0----A1----A2----A3----A4-    AN    <- vertexListA
	//
	// Triangle_0 = A0, A1, B0
	// Triangle_1 = A1, B1, B0

	if (netSurface->netVertices.size() < 2)
		return;

	// squared distance for allowing skirting
	float maxSquaredDistFrontier = maxLengthForFrontierEdgeSkirting * maxLengthForFrontierEdgeSkirting;
	float maxSquaredDist = maxLengthForInnerEdgeSkirting * maxLengthForInnerEdgeSkirting;
	bool isCurTriangleFrontier = false;

	// 1. make mesh without setting 'twins' of hedges
	size_t rowCount = netSurface->netVertices.size();
	for (size_t i = 0; i < rowCount - 1; i++)
	{
		isCurTriangleFrontier = false;

		std::vector<HedgeVertex*>* vertexListA = netSurface->netVertices[i];
		size_t vertexCount = vertexListA->size();
		if (vertexCount < 2)
			return;

		if (i == 0 || i == rowCount - 2)
			isCurTriangleFrontier = true;

		std::vector<HedgeVertex*>* vertexListB = netSurface->netVertices[i+1];
		std::vector<HedgeTriangle*>* triangleList = new std::vector<HedgeTriangle*>;

		for (size_t j = 0; j < vertexCount - 1; j++)
		{
			if (j == 0 || j == vertexCount - 2)
				isCurTriangleFrontier = true;

			HedgeVertex* vertexA0 = (*vertexListA)[j];
			HedgeVertex* vertexA1 = (*vertexListA)[j + 1];
			HedgeVertex* vertexB0 = (*vertexListB)[j];
			HedgeVertex* vertexB1 = (*vertexListB)[j + 1];

			bool triACreated = false, triBCreated = false;
			HedgeTriangle* triB = NULL;

			triACreated = makeHedgeTriangle(vertexA0, vertexA1, vertexB0, triangleList, netSurface->hedges, netSurface->normalVector, isCurTriangleFrontier, maxSquaredDist, maxSquaredDistFrontier);
			triBCreated = makeHedgeTriangle(vertexA1, vertexB1, vertexB0, triangleList, netSurface->hedges, netSurface->normalVector, isCurTriangleFrontier, maxSquaredDist, maxSquaredDistFrontier);

			if (!triACreated && !triBCreated)
			{
				triACreated = makeHedgeTriangle(vertexA0, vertexA1, vertexB1, triangleList, netSurface->hedges, netSurface->normalVector, isCurTriangleFrontier, maxSquaredDist, maxSquaredDistFrontier);
				if(!triACreated)
					makeHedgeTriangle(vertexA0, vertexB1, vertexB0, triangleList, netSurface->hedges, netSurface->normalVector, isCurTriangleFrontier, maxSquaredDist, maxSquaredDistFrontier);
			}
		}

		netSurface->netTriangles.push_back(triangleList);
	}

	// HalfEdges of the triangles.***
	// Triangle A = (A0, A1, B0). Triangle B = (A1, B1, B0).
	// B0------------------B1
	// |\    <-------      |
	// |  \       1        |
	// |    \  \         ^ |
	// |      \  \2      | |
	// | |  ^   \  \    0| |
	// | |2   \   \  v   | |
	// | |     1\   \      |
	// | v        \   \    |
	// |       0        \  |
	// |     ------->     \|
	// A0------------------A1

	// now set the halfEdge's "twin".
	size_t triangleListCount = netSurface->netTriangles.size();
	for (size_t i = 0; i < triangleListCount; i++)
	{
		std::vector<HedgeTriangle*>* curTriangleList = netSurface->netTriangles[i];
		std::vector<HedgeTriangle*>* prevTriangleList = NULL;
		if (i != 0)
			prevTriangleList = netSurface->netTriangles[i-1];

		size_t triangleCount = curTriangleList->size();
		for (size_t j = 0; j < triangleCount; j++)
		{
			HedgeTriangle* curTriangle = (*curTriangleList)[j];

			// make twin relationship btw adjacent triangles in same row
			if (j != triangleCount - 1)
			{
				HedgeTriangle* nextTriangle = (*curTriangleList)[j + 1];

				Hedge* curHedgeCurTriangle = curTriangle->hedge;
				Hedge* curHedgeNextTriangle = nextTriangle->hedge;
				bool notYetFound = true;
				while (notYetFound)
				{
					if (curHedgeCurTriangle->twin == NULL)
					{
						while (notYetFound)
						{
							if (curHedgeNextTriangle->twin == NULL)
							{
								if (curHedgeCurTriangle->startVertex == curHedgeNextTriangle->next->startVertex &&
									curHedgeCurTriangle->next->startVertex == curHedgeNextTriangle->startVertex)
								{
									curHedgeCurTriangle->twin = curHedgeNextTriangle;
									curHedgeNextTriangle->twin = curHedgeCurTriangle;
									notYetFound = false;
									break;
								}
							}

							curHedgeNextTriangle = curHedgeNextTriangle->next;
							if (curHedgeNextTriangle == nextTriangle->hedge)
								break;
						}
					}

					curHedgeCurTriangle = curHedgeCurTriangle->next;
					if (curHedgeCurTriangle == curTriangle->hedge)
						break;
				}
			}

			// make twin relationship btw adjacent triangles in same column
			if (prevTriangleList == NULL)
				continue;

			bool notYetFound = true;
			size_t prevRowTriangleCount = prevTriangleList->size();
			for (size_t k = 0; k < prevRowTriangleCount; k++)
			{
				HedgeTriangle* prevRowTriangle = (*prevTriangleList)[k];
				Hedge* curHedgeCurTriangle = curTriangle->hedge;
				Hedge* curHedgePrevRowTriangle = prevRowTriangle->hedge;

				while (notYetFound)
				{
					if (curHedgeCurTriangle->twin == NULL)
					{
						while (notYetFound)
						{
							if (curHedgePrevRowTriangle->twin == NULL)
							{
								if (curHedgeCurTriangle->startVertex == curHedgePrevRowTriangle->next->startVertex &&
									curHedgeCurTriangle->next->startVertex == curHedgePrevRowTriangle->startVertex)
								{
									curHedgeCurTriangle->twin = curHedgePrevRowTriangle;
									curHedgePrevRowTriangle->twin = curHedgeCurTriangle;
									notYetFound = false;
									break;
								}
							}

							curHedgePrevRowTriangle = curHedgePrevRowTriangle->next;
							if (curHedgePrevRowTriangle == prevRowTriangle->hedge)
								break;
						}
					}

					curHedgeCurTriangle = curHedgeCurTriangle->next;
					if (curHedgeCurTriangle == curTriangle->hedge)
						break;
				}

				if (!notYetFound)
					break;
			}
		}
	}

	// delete unused vertices(in case of bOnGeometrySurface == false)
	rowCount = netSurface->netVertices.size();
	for (size_t i = 0; i < rowCount; i++)
	{
		std::vector<HedgeVertex*>* vertexList = netSurface->netVertices[i];
		std::vector<HedgeVertex*> newList;
		size_t vertexCount = vertexList->size();
		for (size_t j = 0; j < vertexCount; j++)
		{
			if ((*vertexList)[j]->bOnGeometrySurface)
				newList.push_back((*vertexList)[j]);
			else
				delete (*vertexList)[j];
		}

		vertexList->clear();
		vertexList->assign(newList.begin(), newList.end());
	}
}

// NoCollapse : not collapsable, Type1 : move startVertex to endVertex, Type2 : move endVertex to startVertex
enum EdgeCollpaseType {NoCollapse, Type1, Type2};

bool collapseHedge(Hedge* hedge,
					std::map<Hedge*, size_t>& hedgeMap, std::vector<size_t>& indicesOfDeletedHedges,
					std::map<std::map<HedgeVertex*, size_t>*, size_t>& vertexMap, std::vector<size_t>& indicesOfDeletedVertices,
					std::map<std::map<HedgeTriangle*, size_t>*, size_t>& triangleMap, std::vector<size_t>& indicesOfDeletedTriangles,
					EdgeCollpaseType type)
{
	if (type == NoCollapse)
		return false;

	

	HedgeVertex* startVertex;
	HedgeVertex* endVertex;
	switch (type)
	{
	case Type1:
	{
		startVertex = hedge->startVertex;
		endVertex = hedge->next->startVertex;
	}
	break;
	case Type2:
	{
		startVertex = hedge->next->startVertex;
		endVertex = hedge->startVertex;
	}
	break;
	default:
		return false;
	}

	bool bEdgeHasTwin = hedge->twin == NULL ? false : true;
	Hedge* twinHedge = NULL;
	if (bEdgeHasTwin)
		twinHedge = hedge->twin;

	// edge collapse process is just removing this edge by moving startVertex to endVertex
	// during this process, followings are removed.
	// 1. triangle including this edge and 3 hedges of it
	// 2. triangle including twin edge of this edge and 3 edges of it if twin edge of this edge exists
	// 3. vertex to be moved(startVertex)

	// step 0 : basic preparation

	// set triangles to be deleted
	HedgeTriangle* triangleToBeDeleted = hedge->face;
	HedgeTriangle* twinTriangleToBeDeleted = NULL;
	if (bEdgeHasTwin)
		twinTriangleToBeDeleted = twinHedge->face;

	// collect triangles involved with startVertex and endVertex respectively
	std::vector<HedgeTriangle*> trianglesInvolvedWithStartVertex;
	size_t outwardHedgeCount = startVertex->hedgesFromThisVertex.size();
	for (size_t i = 0; i < outwardHedgeCount; i++)
	{
		if (bEdgeHasTwin)
		{
			if (startVertex->hedgesFromThisVertex[i]->face != triangleToBeDeleted && startVertex->hedgesFromThisVertex[i]->face != twinTriangleToBeDeleted)
				trianglesInvolvedWithStartVertex.push_back(startVertex->hedgesFromThisVertex[i]->face);
		}
		else
		{
			if (startVertex->hedgesFromThisVertex[i]->face != triangleToBeDeleted)
				trianglesInvolvedWithStartVertex.push_back(startVertex->hedgesFromThisVertex[i]->face);
		}
	}

	std::vector<HedgeTriangle*> trianglesInvolvedWithEndVertex;
	outwardHedgeCount = endVertex->hedgesFromThisVertex.size();
	for (size_t i = 0; i < outwardHedgeCount; i++)
	{
		if (bEdgeHasTwin)
		{
			if (endVertex->hedgesFromThisVertex[i]->face != triangleToBeDeleted && endVertex->hedgesFromThisVertex[i]->face != twinTriangleToBeDeleted)
				trianglesInvolvedWithEndVertex.push_back(endVertex->hedgesFromThisVertex[i]->face);
		}
		else
		{
			if (endVertex->hedgesFromThisVertex[i]->face != triangleToBeDeleted)
				trianglesInvolvedWithEndVertex.push_back(endVertex->hedgesFromThisVertex[i]->face);
		}
	}

	// step 1. remove outward hedges from vertices on triangles to be deleted
	std::vector<Hedge*> updatedOutward;
	outwardHedgeCount = triangleToBeDeleted->vertex0->hedgesFromThisVertex.size();
	for (size_t i = 0; i < outwardHedgeCount; i++)
	{
		if (triangleToBeDeleted->vertex0->hedgesFromThisVertex[i] != triangleToBeDeleted->hedge)
			updatedOutward.push_back(triangleToBeDeleted->vertex0->hedgesFromThisVertex[i]);
	}
	triangleToBeDeleted->vertex0->hedgesFromThisVertex.clear();
	triangleToBeDeleted->vertex0->hedgesFromThisVertex.assign(updatedOutward.begin(), updatedOutward.end());
	
	updatedOutward.clear();
	outwardHedgeCount = triangleToBeDeleted->vertex1->hedgesFromThisVertex.size();
	for (size_t i = 0; i < outwardHedgeCount; i++)
	{
		if (triangleToBeDeleted->vertex1->hedgesFromThisVertex[i] != triangleToBeDeleted->hedge->next)
			updatedOutward.push_back(triangleToBeDeleted->vertex1->hedgesFromThisVertex[i]);
	}
	triangleToBeDeleted->vertex1->hedgesFromThisVertex.clear();
	triangleToBeDeleted->vertex1->hedgesFromThisVertex.assign(updatedOutward.begin(), updatedOutward.end());
	
	updatedOutward.clear();
	outwardHedgeCount = triangleToBeDeleted->vertex2->hedgesFromThisVertex.size();
	for (size_t i = 0; i < outwardHedgeCount; i++)
	{
		if (triangleToBeDeleted->vertex2->hedgesFromThisVertex[i] != triangleToBeDeleted->hedge->next->next)
			updatedOutward.push_back(triangleToBeDeleted->vertex2->hedgesFromThisVertex[i]);
	}
	triangleToBeDeleted->vertex2->hedgesFromThisVertex.clear();
	triangleToBeDeleted->vertex2->hedgesFromThisVertex.assign(updatedOutward.begin(), updatedOutward.end());

	if (bEdgeHasTwin)
	{
		updatedOutward.clear();
		outwardHedgeCount = twinTriangleToBeDeleted->vertex0->hedgesFromThisVertex.size();
		for (size_t i = 0; i < outwardHedgeCount; i++)
		{
			if (twinTriangleToBeDeleted->vertex0->hedgesFromThisVertex[i] != twinTriangleToBeDeleted->hedge)
				updatedOutward.push_back(twinTriangleToBeDeleted->vertex0->hedgesFromThisVertex[i]);
		}
		twinTriangleToBeDeleted->vertex0->hedgesFromThisVertex.clear();
		twinTriangleToBeDeleted->vertex0->hedgesFromThisVertex.assign(updatedOutward.begin(), updatedOutward.end());

		updatedOutward.clear();
		outwardHedgeCount = twinTriangleToBeDeleted->vertex1->hedgesFromThisVertex.size();
		for (size_t i = 0; i < outwardHedgeCount; i++)
		{
			if (twinTriangleToBeDeleted->vertex1->hedgesFromThisVertex[i] != twinTriangleToBeDeleted->hedge->next)
				updatedOutward.push_back(twinTriangleToBeDeleted->vertex1->hedgesFromThisVertex[i]);
		}
		twinTriangleToBeDeleted->vertex1->hedgesFromThisVertex.clear();
		twinTriangleToBeDeleted->vertex1->hedgesFromThisVertex.assign(updatedOutward.begin(), updatedOutward.end());

		updatedOutward.clear();
		outwardHedgeCount = twinTriangleToBeDeleted->vertex2->hedgesFromThisVertex.size();
		for (size_t i = 0; i < outwardHedgeCount; i++)
		{
			if (twinTriangleToBeDeleted->vertex2->hedgesFromThisVertex[i] != twinTriangleToBeDeleted->hedge->next->next)
				updatedOutward.push_back(twinTriangleToBeDeleted->vertex2->hedgesFromThisVertex[i]);
		}
		twinTriangleToBeDeleted->vertex2->hedgesFromThisVertex.clear();
		twinTriangleToBeDeleted->vertex2->hedgesFromThisVertex.assign(updatedOutward.begin(), updatedOutward.end());
	}

	// step 2 : For all triangles sharing statVertex, replace vertex which is identical to startVertex with endVertex.
	// For all edges in triangles whose start vertex is identical to starVertex, replace start vertex with endVertex.
	// add edges whose start vertex was changed into outward hedge list of new start vertex(endVertex).
	// recalculate plane normals of modified triangles and direction of modified hedges.
	size_t involvedTriangleCount = trianglesInvolvedWithStartVertex.size();
	for (size_t i = 0; i < involvedTriangleCount; i++)
	{
		HedgeTriangle* triangle = trianglesInvolvedWithStartVertex[i];
		Hedge* targetHedge;
		if (triangle->vertex0 == startVertex)
		{
			targetHedge = triangle->hedge;
			triangle->vertex0 = endVertex;
		}
		else if (triangle->vertex1 == startVertex)
		{
			targetHedge = triangle->hedge->next;
			triangle->vertex1 = endVertex;
		}
		else if (triangle->vertex2 == startVertex)
		{
			targetHedge = triangle->hedge->next->next;
			triangle->vertex2 = endVertex;
		}
		else
			continue;

		targetHedge->startVertex = endVertex;
		endVertex->hedgesFromThisVertex.push_back(targetHedge);
		gaia3d::GeometryUtility::calculatePlaneNormal(triangle->vertex0->position.x, triangle->vertex0->position.y, triangle->vertex0->position.z,
													triangle->vertex1->position.x, triangle->vertex1->position.y, triangle->vertex1->position.z,
													triangle->vertex2->position.x, triangle->vertex2->position.y, triangle->vertex2->position.z,
													triangle->normal.x, triangle->normal.y, triangle->normal.z, true);
		targetHedge->direction = targetHedge->next->startVertex->position - targetHedge->startVertex->position;
		targetHedge->direction.normalize();
		targetHedge->next->next->direction = targetHedge->startVertex->position - targetHedge->next->next->startVertex->position;
		targetHedge->next->next->direction.normalize();
	}

	// step 3 : delete startVertex and mark it on indicesOfDeletedVertices using vertexMap
	std::map<std::map<HedgeVertex*, size_t>*, size_t>::iterator iterVertexList = vertexMap.begin();
	for (; iterVertexList != vertexMap.end(); iterVertexList++)
	{
		std::map<HedgeVertex*, size_t>* vertexListMap = iterVertexList->first;
		if (vertexListMap->find(startVertex) == vertexListMap->end())
			continue;

		indicesOfDeletedVertices.push_back(iterVertexList->second);
		indicesOfDeletedVertices.push_back((*vertexListMap)[startVertex]);
		delete startVertex;
		break;
	}

	// step 4 : delete triangles and 3 hedges on it, and mark it on indicesOfDeletedTriangles and indicesOfDeletedTriangles using hedgeMap and triangleMap
	// collect edges whose twin was removed to use them for twin rebuilding
	std::vector<Hedge*> edgesWithTwinRemoved;
	indicesOfDeletedHedges.push_back(hedgeMap[hedge->next->next]);
	if (hedge->next->next->twin != NULL)
	{
		hedge->next->next->twin->twin = NULL;
		edgesWithTwinRemoved.push_back(hedge->next->next->twin);
	}
	delete hedge->next->next;
	indicesOfDeletedHedges.push_back(hedgeMap[hedge->next]);
	if (hedge->next->twin != NULL)
	{
		hedge->next->twin->twin = NULL;
		edgesWithTwinRemoved.push_back(hedge->next->twin);
	}
	delete hedge->next;
	indicesOfDeletedHedges.push_back(hedgeMap[hedge]);
	delete hedge;

	std::map<std::map<HedgeTriangle*, size_t>*, size_t>::iterator iterTriangleList = triangleMap.begin();
	for (; iterTriangleList != triangleMap.end(); iterTriangleList++)
	{
		std::map<HedgeTriangle*, size_t>* triangleListMap = iterTriangleList->first;
		if (triangleListMap->find(triangleToBeDeleted) == triangleListMap->end())
			continue;

		indicesOfDeletedTriangles.push_back(iterTriangleList->second);
		indicesOfDeletedTriangles.push_back((*triangleListMap)[triangleToBeDeleted]);
		delete triangleToBeDeleted;
		break;
	}

	if (bEdgeHasTwin)
	{
		indicesOfDeletedHedges.push_back(hedgeMap[twinHedge->next->next]);
		if (twinHedge->next->next->twin != NULL)
		{
			twinHedge->next->next->twin->twin = NULL;
			edgesWithTwinRemoved.push_back(twinHedge->next->next->twin);
		}
		delete twinHedge->next->next;
		indicesOfDeletedHedges.push_back(hedgeMap[twinHedge->next]);
		if (twinHedge->next->twin != NULL)
		{
			twinHedge->next->twin->twin = NULL;
			edgesWithTwinRemoved.push_back(twinHedge->next->twin);
		}
		delete twinHedge->next;
		indicesOfDeletedHedges.push_back(hedgeMap[twinHedge]);
		delete twinHedge;

		std::map<std::map<HedgeTriangle*, size_t>*, size_t>::iterator iterTriangleList = triangleMap.begin();
		for (; iterTriangleList != triangleMap.end(); iterTriangleList++)
		{
			std::map<HedgeTriangle*, size_t>* triangleListMap = iterTriangleList->first;
			if (triangleListMap->find(twinTriangleToBeDeleted) == triangleListMap->end())
				continue;

			indicesOfDeletedTriangles.push_back(iterTriangleList->second);
			indicesOfDeletedTriangles.push_back((*triangleListMap)[twinTriangleToBeDeleted]);
			delete twinTriangleToBeDeleted;
			break;
		}
	}

	//	step 5 : rebuild twin relationship of twin-removed hedges
	size_t twinRemovedEdgeCount = edgesWithTwinRemoved.size();
	for (size_t i = 0; i < twinRemovedEdgeCount; i++)
	{
		if (edgesWithTwinRemoved[i]->twin != NULL)
			continue;

		for (size_t j = i + 1; j < twinRemovedEdgeCount; j++)
		{
			if (edgesWithTwinRemoved[i]->startVertex != edgesWithTwinRemoved[j]->next->startVertex ||
				edgesWithTwinRemoved[i]->next->startVertex != edgesWithTwinRemoved[j]->startVertex)
				continue;

			edgesWithTwinRemoved[i]->twin = edgesWithTwinRemoved[j];
			edgesWithTwinRemoved[j]->twin = edgesWithTwinRemoved[i];
			break;
		}
	}

	return true;
}

bool isHedgeCollapsable(Hedge* hedge, bool toBeReversed, float cosNormalAngleChangeLimit, float cosEdgeAngleChangeLimit, float cosAngleBtwFrontierEdges)
{
	HedgeVertex* startVertex;
	HedgeVertex* endVertex;
	if (toBeReversed)
	{
		startVertex = hedge->next->startVertex;
		endVertex = hedge->startVertex;
	}
	else
	{
		startVertex = hedge->startVertex;
		endVertex = hedge->next->startVertex;
	}

	std::vector<HedgeTriangle*> involvedTriangles;
	size_t outwardHedgeCount = startVertex->hedgesFromThisVertex.size();
	for (size_t i = 0; i < outwardHedgeCount; i++)
		involvedTriangles.push_back(startVertex->hedgesFromThisVertex[i]->face);

	size_t triangleCount = involvedTriangles.size();
	for (size_t i = 0; i < triangleCount; i++)
	{
		HedgeTriangle* triangle = involvedTriangles[i];
		if (triangle == hedge->face)
			continue;

		if (hedge->twin != NULL && hedge->twin->face == triangle)
			continue;

		HedgeVertex* vertex0 = NULL, *vertex1 = NULL, *vertex2 = NULL;
		if (triangle->vertex0 == startVertex)
		{
			vertex0 = endVertex; vertex1 = triangle->vertex1; vertex2 = triangle->vertex2;
		}
		else if (triangle->vertex1 == startVertex)
		{
			vertex0 = triangle->vertex0; vertex1 = endVertex; vertex2 = triangle->vertex2;
		}
		else if (triangle->vertex2 == startVertex)
		{
			vertex0 = triangle->vertex0; vertex1 = triangle->vertex1; vertex2 = endVertex;
		}

		gaia3d::Point3D afterNormal, beforeNormal;
		gaia3d::GeometryUtility::calculatePlaneNormal(vertex0->position.x, vertex0->position.y, vertex0->position.z,
														vertex1->position.x, vertex1->position.y, vertex1->position.z,
														vertex2->position.x, vertex2->position.y, vertex2->position.z,
														afterNormal.x, afterNormal.y, afterNormal.z, false);

		// checking triangle area after edge collapse
		if ( !afterNormal.normalize())
			return false;

		// checking angle change of this triangle's normal
		if (afterNormal.x*triangle->normal.x + afterNormal.y*triangle->normal.y + afterNormal.z*triangle->normal.z < cosNormalAngleChangeLimit)
			return false;

		Hedge* hedgeOnTriangle = triangle->hedge;
		while (true)
		{
			if (hedgeOnTriangle->twin == NULL)
			{
				if (hedgeOnTriangle->startVertex == startVertex || hedgeOnTriangle->next->startVertex == startVertex)
				{
					if (hedge->direction.x * hedgeOnTriangle->direction.x +
						hedge->direction.y * hedgeOnTriangle->direction.y +
						hedge->direction.z * hedgeOnTriangle->direction.z < cosAngleBtwFrontierEdges)
						return false;

					gaia3d::Point3D afterDirection;
					if (hedgeOnTriangle->startVertex == startVertex)
						afterDirection = hedgeOnTriangle->next->startVertex->position - endVertex->position;
					else if (hedgeOnTriangle->next->startVertex == startVertex)
						afterDirection = endVertex->position - hedgeOnTriangle->startVertex->position;

					afterDirection.normalize();
					if (afterDirection.x*hedgeOnTriangle->direction.x +
						afterDirection.y*hedgeOnTriangle->direction.y +
						afterDirection.z*hedgeOnTriangle->direction.z < cosEdgeAngleChangeLimit)
						return false;
				}
			}

			hedgeOnTriangle = hedgeOnTriangle->next;
			if (hedgeOnTriangle == triangle->hedge)
				break;
		}
	}

	return true;
}

EdgeCollpaseType getCollapseType(Hedge* hedge, float cosNormalAngleChangeLimit, float cosEdgeAngleChangeLimit, float cosAngleBtwFrontierEdges)
{
	// there are 2 collpase options
	// first, move startVertex to endVertex
	// second, move endVertex to startVertex
	// this function will find out which option should be applied.

	// check if which option should be checked
	bool bType1 = false;
	bool bType2 = false;
	if (hedge->twin == NULL) // this hedge is a frontier edge
	{
		if (hedge->next->twin == NULL)
			bType1 = true;
		else
		{
			if (hedge->next->next->twin == NULL)
				bType2 = true;
			else
				bType1 = bType2 = true;
		}
	}
	else // this hedge is an inner edge
	{
		if (hedge->next->next->twin != NULL)
		{
			if (hedge->twin->next->twin == NULL)
				return NoCollapse;
			else
				bType1 = true;
		}
		else
			return NoCollapse;
	}

	if (bType1 && isHedgeCollapsable(hedge, false, cosNormalAngleChangeLimit, cosEdgeAngleChangeLimit, cosAngleBtwFrontierEdges))
		return Type1;

	if (bType2 && isHedgeCollapsable(hedge, true, cosNormalAngleChangeLimit, cosEdgeAngleChangeLimit, cosAngleBtwFrontierEdges))
		return Type2;

	return NoCollapse;
}

bool triangleReduction(std::vector<Hedge*>& hedges,
						std::vector<std::vector<HedgeVertex*>*>& vertices,
						std::vector<std::vector<HedgeTriangle*>*>& triangles,
						float cosNormalAngleChangeLimit, float cosEdgeAngleChangeLimit, float cosAngleBtwFrontierEdges)
{
	size_t originalHedgeCount = hedges.size();
	std::map<Hedge*, size_t> hedgeMap;
	for (size_t i = 0; i < originalHedgeCount; i++)
		hedgeMap.insert(std::map<Hedge*, size_t>::value_type(hedges[i], i));

	std::map<std::map<HedgeVertex*, size_t>*, size_t> vertexMap;
	size_t vertexListCount = vertices.size();
	for (size_t i = 0; i < vertexListCount; i++)
	{
		std::vector<HedgeVertex*>* vertexList = vertices[i];
		size_t vertexCount = vertexList->size();
		std::map<HedgeVertex*, size_t>* vertexListMap = new std::map<HedgeVertex*, size_t>;
		for (size_t j = 0; j < vertexCount; j++)
			vertexListMap->insert(std::map<HedgeVertex*, size_t>::value_type((*vertexList)[j], j));

		vertexMap.insert(std::map<std::map<HedgeVertex*, size_t>*, size_t>::value_type(vertexListMap, i));
	}

	std::map<std::map<HedgeTriangle*, size_t>*, size_t> triangleMap;
	size_t triangleListCount = triangles.size();
	for (size_t i = 0; i < triangleListCount; i++)
	{
		std::vector<HedgeTriangle*>* triangleList = triangles[i];
		size_t triangleCount = triangleList->size();
		std::map<HedgeTriangle*, size_t>* triangleListMap = new std::map<HedgeTriangle*, size_t>;
		for (size_t j = 0; j < triangleCount; j++)
			triangleListMap->insert(std::map<HedgeTriangle*, size_t>::value_type((*triangleList)[j], j));

		triangleMap.insert(std::map<std::map<HedgeTriangle*, size_t>*, size_t>::value_type(triangleListMap, i));
	}

	std::vector<size_t> indicesOfDeletedHedges;
	std::vector<size_t> indicesOfDeletedVertices;
	std::vector<size_t> indicesOfDeletedTriangles;
	for (size_t i = 0; i < originalHedgeCount; i++)
	{
		Hedge* hedge = hedges[i];
		if (hedge == NULL)
			continue;

		bool bTwin = hedge->twin == NULL ? false : true;
		EdgeCollpaseType collapseType = getCollapseType(hedge, cosNormalAngleChangeLimit, cosEdgeAngleChangeLimit, cosAngleBtwFrontierEdges);
		if (collapseType == NoCollapse)
			continue;

		if (collapseHedge(hedge,
			hedgeMap, indicesOfDeletedHedges,
			vertexMap, indicesOfDeletedVertices,
			triangleMap, indicesOfDeletedTriangles,
			collapseType))
		{
			size_t deletedHedgeCount = indicesOfDeletedHedges.size();
			for(size_t j = 0; j < deletedHedgeCount; j++)
				hedges[indicesOfDeletedHedges[j]] = NULL;
			indicesOfDeletedHedges.clear();

			size_t deletedVertexCount = indicesOfDeletedVertices.size();
			for (size_t j = 0; j < deletedVertexCount; j += 2)
				(*(vertices[ indicesOfDeletedVertices[j] ] ))[ indicesOfDeletedVertices[j + 1] ] = NULL;
			indicesOfDeletedVertices.clear();

			size_t deletedTriangleCount = indicesOfDeletedTriangles.size();
			for (size_t j = 0; j < deletedTriangleCount; j += 2)
				(*(triangles[ indicesOfDeletedTriangles[j] ]))[ indicesOfDeletedTriangles[j + 1] ] = NULL;
			indicesOfDeletedTriangles.clear();
		}
	}

	std::vector<Hedge*> newHedges;
	for (size_t i = 0; i < originalHedgeCount; i++)
	{
		if (hedges[i] == NULL)
			continue;

		newHedges.push_back(hedges[i]);
	}

	size_t newHedgeCount = newHedges.size();
	hedges.clear();
	hedges.assign(newHedges.begin(), newHedges.end());

	for (size_t i = 0; i < vertexListCount; i++)
	{
		std::vector<HedgeVertex*> newVertices;
		std::vector<HedgeVertex*>* vertexList = vertices[i];
		size_t vertexCount = vertexList->size();
		for (size_t j = 0; j < vertexCount; j++)
		{
			if ((*vertexList)[j] == NULL)
				continue;

			newVertices.push_back((*vertexList)[j]);
		}

		vertexList->clear();
		vertexList->assign(newVertices.begin(), newVertices.end());
	}

	std::map<std::map<HedgeVertex*, size_t>*, size_t>::iterator iterVertex = vertexMap.begin();
	for (; iterVertex != vertexMap.end(); iterVertex++)
		delete iterVertex->first;

	for (size_t i = 0; i < triangleListCount; i++)
	{
		std::vector<HedgeTriangle*> newTriangles;
		std::vector<HedgeTriangle*>* triangleList = triangles[i];
		size_t triangleCount = triangleList->size();
		for (size_t j = 0; j < triangleCount; j++)
		{
			if ((*triangleList)[j] == NULL)
				continue;

			newTriangles.push_back((*triangleList)[j]);
		}

		triangleList->clear();
		triangleList->assign(newTriangles.begin(), newTriangles.end());
	}

	std::map<std::map<HedgeTriangle*, size_t>*, size_t>::iterator iterTriangle = triangleMap.begin();
	for (; iterTriangle != triangleMap.end(); iterTriangle++)
		delete iterTriangle->first;

	return (originalHedgeCount == newHedgeCount);
}

void smootheFrontierEdges(std::vector<Hedge*>& hedges, float cellSize)
{
	if (hedges.size() < 12)
		return;

	double squaredMaxDist = cellSize * cellSize * 9.0;

	size_t hedgeCount = hedges.size();
	for (size_t i = 0; i < hedgeCount; i++)
	{
		if (hedges[i]->twin != NULL)
			continue;

		Hedge* hedge = hedges[i];
		
		// find out previous frontier hedge
		Hedge* prevFrontierHedge = NULL;
		size_t outwardHedgeCount = hedge->startVertex->hedgesFromThisVertex.size();
		for (size_t j = 0; j < outwardHedgeCount; j++)
		{
			Hedge* outwardHedge = hedge->startVertex->hedgesFromThisVertex[j];

			if (hedge == outwardHedge)
				continue;

			if (outwardHedge->next->next->twin != NULL)
				continue;

			prevFrontierHedge = outwardHedge->next->next;
			if (prevFrontierHedge->next->startVertex != hedge->startVertex)
				prevFrontierHedge = NULL;

			break;
		}

		if (prevFrontierHedge == NULL)
			continue;

		// set 3 chain vertex on frontier hedges
		HedgeVertex* prevVertex = prevFrontierHedge->startVertex;
		HedgeVertex* curVertex = hedge->startVertex;
		HedgeVertex* nextVertex = hedge->next->startVertex;

		// hedge length test
		double squaredPrevDist = curVertex->position.squaredDistanceTo(prevVertex->position);
		double squaredNextDist = nextVertex->position.squaredDistanceTo(curVertex->position);

		if (squaredPrevDist >= squaredMaxDist || squaredNextDist >= squaredMaxDist)
			continue;

		// area test of modified frontier triangles
		gaia3d::Point3D newCurPos;
		newCurPos.set((prevVertex->position.x + nextVertex->position.x) / 2.0,
					(prevVertex->position.y + nextVertex->position.y) / 2.0,
					(prevVertex->position.z + nextVertex->position.z) / 2.0);

		gaia3d::Point3D newPrevNormal, newNextNormal;
		gaia3d::GeometryUtility::calculatePlaneNormal(newCurPos.x, newCurPos.y, newCurPos.z,
			prevFrontierHedge->next->next->startVertex->position.x, prevFrontierHedge->next->next->startVertex->position.y, prevFrontierHedge->next->next->startVertex->position.z,
			prevFrontierHedge->startVertex->position.x, prevFrontierHedge->startVertex->position.y, prevFrontierHedge->startVertex->position.z,
			newPrevNormal.x, newPrevNormal.y, newPrevNormal.z, false);

		if (!newPrevNormal.normalize())
			continue;

		gaia3d::GeometryUtility::calculatePlaneNormal(newCurPos.x, newCurPos.y, newCurPos.z,
			hedge->next->startVertex->position.x, hedge->next->startVertex->position.y, hedge->next->startVertex->position.z,
			hedge->next->next->startVertex->position.x, hedge->next->next->startVertex->position.y, hedge->next->next->startVertex->position.z,
			newNextNormal.x, newNextNormal.y, newNextNormal.z, false);

		if (!newNextNormal.normalize())
			continue;

		// update position of current vertex
		hedge->startVertex->position = newCurPos;

		// recalculate normals and directions of triangles and hedges associated with current vertex
		for (size_t j = 0; j < outwardHedgeCount; j++)
		{
			Hedge* outwardHedge = hedge->startVertex->hedgesFromThisVertex[j];
			outwardHedge->direction = outwardHedge->next->startVertex->position - outwardHedge->startVertex->position;
			outwardHedge->direction.normalize();
			Hedge* inwardHedge = outwardHedge->next->next;
			inwardHedge->direction = inwardHedge->next->startVertex->position - inwardHedge->startVertex->position;
			inwardHedge->direction.normalize();
			
			HedgeTriangle* triangle = outwardHedge->face;
			if (triangle == prevFrontierHedge->face)
			{
				triangle->normal = newPrevNormal;
				continue;
			}
			if (triangle == hedge->face)
			{
				triangle->normal = newNextNormal;
				continue;
			}
	
			gaia3d::GeometryUtility::calculatePlaneNormal(
				triangle->vertex0->position.x, triangle->vertex0->position.y, triangle->vertex0->position.z,
				triangle->vertex1->position.x, triangle->vertex1->position.y, triangle->vertex1->position.z,
				triangle->vertex2->position.x, triangle->vertex2->position.y, triangle->vertex2->position.z,
				triangle->normal.x, triangle->normal.y, triangle->normal.z,
				true
			);
		}
	}
}

void smootheSmallFrontierEdges(std::vector<Hedge*>& hedges, float cellSize)
{
	if (hedges.size() < 12)
		return;

	double squaredMaxDist = cellSize * cellSize * 16.0;
	double cosMaxAngle = cos(M_PI / 6.0);

	size_t hedgeCount = hedges.size();
	for (size_t i = 0; i < hedgeCount; i++)
	{
		if (hedges[i]->twin != NULL) // filter out inner edges
			continue;

		Hedge* hedge = hedges[i];
		HedgeVertex* curVertex = hedge->startVertex;
		HedgeVertex* nextVertex = hedge->next->startVertex;

		double squaredHedgeLength = curVertex->position.squaredDistanceTo(nextVertex->position);
		if (squaredHedgeLength >= squaredMaxDist) // check length of target frontier edge
			continue;
		
		// find out previous frontier hedge
		Hedge* prevFrontierHedge = NULL;
		size_t outwardHedgeCount = hedge->startVertex->hedgesFromThisVertex.size();
		for (size_t j = 0; j < outwardHedgeCount; j++)
		{
			Hedge* outwardHedge = hedge->startVertex->hedgesFromThisVertex[j];

			if (hedge == outwardHedge)
				continue;

			if (outwardHedge->next->next->twin != NULL)
				continue;

			prevFrontierHedge = outwardHedge->next->next;
			if (prevFrontierHedge->next->startVertex != hedge->startVertex)
				prevFrontierHedge = NULL;

			break;
		}

		if (prevFrontierHedge == NULL)
			continue;

		// find out next frontier hedge
		Hedge* nextFrontierHedge = NULL;
		outwardHedgeCount = hedge->next->startVertex->hedgesFromThisVertex.size();
		for (size_t j = 0; j < outwardHedgeCount; j++)
		{
			Hedge* outwardHedge = hedge->next->startVertex->hedgesFromThisVertex[j];

			if (outwardHedge->twin != NULL)
				continue;

			nextFrontierHedge = outwardHedge;
			break;
		}

		if (nextFrontierHedge == NULL)
			continue;

		// check direction angle difference test between prev and next frontier edges
		if (prevFrontierHedge->direction.x * nextFrontierHedge->direction.x +
			prevFrontierHedge->direction.y * nextFrontierHedge->direction.y +
			prevFrontierHedge->direction.z * nextFrontierHedge->direction.z < cosMaxAngle)
			continue;

		// check if current hedge is normal to line between prev and next vertices before moving current vertex
		HedgeVertex* prevVertex = prevFrontierHedge->startVertex;

		gaia3d::Point3D refLineVector = nextVertex->position - prevVertex->position;
		refLineVector.normalize();

		if (refLineVector.x*hedge->direction.x + refLineVector.y*hedge->direction.y + refLineVector.z*hedge->direction.z < 1E-8)
			continue;

		// finally project current vertex on the line between prev and next vertices
		gaia3d::Point3D frontierHedgeVector = curVertex->position - prevVertex->position;
		double refLineComponentOfFrontierHedgeVector =
			frontierHedgeVector.x*refLineVector.x + frontierHedgeVector.y*refLineVector.y + frontierHedgeVector.z*refLineVector.z;

		curVertex->position = prevVertex->position + refLineVector * refLineComponentOfFrontierHedgeVector;

		// recalculate plane normals of triangles and directions of hedges associated with current vertex
		outwardHedgeCount = curVertex->hedgesFromThisVertex.size();
		for (size_t j = 0; j < outwardHedgeCount; j++)
		{
			Hedge* outwardHedge = hedge->startVertex->hedgesFromThisVertex[j];
			outwardHedge->direction = outwardHedge->next->startVertex->position - outwardHedge->startVertex->position;
			outwardHedge->direction.normalize();
			Hedge* inwardHedge = outwardHedge->next->next;
			inwardHedge->direction = inwardHedge->next->startVertex->position - inwardHedge->startVertex->position;
			inwardHedge->direction.normalize();

			HedgeTriangle* triangle = outwardHedge->face;
			gaia3d::GeometryUtility::calculatePlaneNormal(
				triangle->vertex0->position.x, triangle->vertex0->position.y, triangle->vertex0->position.z,
				triangle->vertex1->position.x, triangle->vertex1->position.y, triangle->vertex1->position.z,
				triangle->vertex2->position.x, triangle->vertex2->position.y, triangle->vertex2->position.z,
				triangle->normal.x, triangle->normal.y, triangle->normal.z,
				true
			);
		}
	}
}

void triangleReduction(NetSurface* netSurface, float cosNormalAngleChangeLimit, float cosEdgeAngleChangeLimit, float cosAngleBtwFrontierEdges)
{
	// basic triangle reduction
	int loopCount = 0;
	while (!triangleReduction(netSurface->hedges, netSurface->netVertices, netSurface->netTriangles,
			cosNormalAngleChangeLimit, cosEdgeAngleChangeLimit, cosAngleBtwFrontierEdges))
	{
		loopCount++;
	}

	// change positions of 3 vertices on 2 adjacent frontier edges so that 3 vertices make a straight line
	// when lengths of both frontier edges are under threshold
	smootheFrontierEdges(netSurface->hedges, netSurface->gridSize);

	// re-do basic triangle reduction on the result
	loopCount = 0;
	while (!triangleReduction(netSurface->hedges, netSurface->netVertices, netSurface->netTriangles,
			cosNormalAngleChangeLimit, cosEdgeAngleChangeLimit, cosAngleBtwFrontierEdges))
	{
		loopCount++;
	}
	
	// drop relatively short frontier edges with comparison to neighbour fronier edges 
	smootheSmallFrontierEdges(netSurface->hedges, netSurface->gridSize);

	// re-do basic triangle reduction on the result
	loopCount = 0;
	while (!triangleReduction(netSurface->hedges, netSurface->netVertices, netSurface->netTriangles,
			cosNormalAngleChangeLimit, cosEdgeAngleChangeLimit, cosAngleBtwFrontierEdges))
	{
		loopCount++;
	}
}

void changeNetSurfacesIntoTrianglePolyhedron(NetSurface* netSurface, std::map<HedgeVertex*, gaia3d::Vertex*>& vertexMapper)
{
	netSurface->nsm = new gaia3d::TrianglePolyhedron;
	gaia3d::Surface* surface = new gaia3d::Surface;
	netSurface->nsm->getSurfaces().push_back(surface);

	size_t triangleListCount = netSurface->netTriangles.size();
	std::map<HedgeVertex*, size_t> vertexDuplicationChecker;
	for (size_t i = 0; i < triangleListCount; i++)
	{
		std::vector<HedgeTriangle*>* triangleList = netSurface->netTriangles[i];
		size_t triangleCount = triangleList->size();
		for (size_t j = 0; j < triangleCount; j++)
		{
			HedgeTriangle* hTriangle = (*triangleList)[j];
			gaia3d::Triangle* triangle = new gaia3d::Triangle;

			// make vertices and vertex normals
			gaia3d::Vertex* vertex0, *vertex1, *vertex2;
			size_t vertexIndex0, vertexIndex1, vertexIndex2;
			if (vertexDuplicationChecker.find(hTriangle->vertex0) == vertexDuplicationChecker.end())
			{
				vertex0 = new gaia3d::Vertex;
				vertex0->position = hTriangle->vertex0->position;
				vertexIndex0 = netSurface->nsm->getVertices().size();
				if (!vertexMapper.empty())
				{
					vertex0->textureCoordinate[0] = vertexMapper[hTriangle->vertex0]->textureCoordinate[0];
					vertex0->textureCoordinate[1] = vertexMapper[hTriangle->vertex0]->textureCoordinate[1];
				}

				// calculate vertex normal
				// TODO(khj 20180412) : NYI A vertex normal should be calcualted by average on 'weighted' plane normals of triangles sharing this vertex
				//						Now, the vertex normal is calculated by only average of plane normals
				vertex0->normal.set(0.0, 0.0, 0.0);
				size_t outwardHedgeCount = hTriangle->vertex0->hedgesFromThisVertex.size();
				for (size_t k = 0; k < outwardHedgeCount; k++)
				{
					Hedge* outwardHedge = hTriangle->vertex0->hedgesFromThisVertex[k];
					vertex0->normal += outwardHedge->face->normal;
				}
				vertex0->normal.normalize();

				netSurface->nsm->getVertices().push_back(vertex0);
				vertexDuplicationChecker.insert(std::map<HedgeVertex*, size_t>::value_type(hTriangle->vertex0, vertexIndex0));
			}
			else
			{
				vertexIndex0 = vertexDuplicationChecker[hTriangle->vertex0];
				vertex0 = netSurface->nsm->getVertices()[vertexIndex0];
			}

			if (vertexDuplicationChecker.find(hTriangle->vertex1) == vertexDuplicationChecker.end())
			{
				vertex1 = new gaia3d::Vertex;
				vertex1->position = hTriangle->vertex1->position;
				vertexIndex1 = netSurface->nsm->getVertices().size();
				if (!vertexMapper.empty())
				{
					vertex1->textureCoordinate[0] = vertexMapper[hTriangle->vertex1]->textureCoordinate[0];
					vertex1->textureCoordinate[1] = vertexMapper[hTriangle->vertex1]->textureCoordinate[1];
				}

				// calculate vertex normal
				// TODO(khj 20180412) : NYI A vertex normal should be calcualted by average on 'weighted' plane normals of triangles sharing this vertex
				//						Now, the vertex normal is calculated by only average of plane normals
				vertex1->normal.set(0.0, 0.0, 0.0);
				size_t outwardHedgeCount = hTriangle->vertex1->hedgesFromThisVertex.size();
				for (size_t k = 0; k < outwardHedgeCount; k++)
				{
					Hedge* outwardHedge = hTriangle->vertex1->hedgesFromThisVertex[k];
					vertex1->normal += outwardHedge->face->normal;
				}
				vertex1->normal.normalize();

				netSurface->nsm->getVertices().push_back(vertex1);
				vertexDuplicationChecker.insert(std::map<HedgeVertex*, size_t>::value_type(hTriangle->vertex1, vertexIndex1));
			}
			else
			{
				vertexIndex1 = vertexDuplicationChecker[hTriangle->vertex1];
				vertex1 = netSurface->nsm->getVertices()[vertexIndex1];
			}

			if (vertexDuplicationChecker.find(hTriangle->vertex2) == vertexDuplicationChecker.end())
			{
				vertex2 = new gaia3d::Vertex;
				vertex2->position = hTriangle->vertex2->position;
				vertexIndex2 = netSurface->nsm->getVertices().size();
				if (!vertexMapper.empty())
				{
					vertex2->textureCoordinate[0] = vertexMapper[hTriangle->vertex2]->textureCoordinate[0];
					vertex2->textureCoordinate[1] = vertexMapper[hTriangle->vertex2]->textureCoordinate[1];
				}

				// calculate vertex normal
				// TODO(khj 20180412) : NYI A vertex normal should be calcualted by average on 'weighted' plane normals of triangles sharing this vertex
				//						Now, the vertex normal is calculated by only average of plane normals
				vertex2->normal.set(0.0, 0.0, 0.0);
				size_t outwardHedgeCount = hTriangle->vertex2->hedgesFromThisVertex.size();
				for (size_t k = 0; k < outwardHedgeCount; k++)
				{
					Hedge* outwardHedge = hTriangle->vertex2->hedgesFromThisVertex[k];
					vertex2->normal += outwardHedge->face->normal;
				}
				vertex2->normal.normalize();

				netSurface->nsm->getVertices().push_back(vertex2);
				vertexDuplicationChecker.insert(std::map<HedgeVertex*, size_t>::value_type(hTriangle->vertex2, vertexIndex2));
			}
			else
			{
				vertexIndex2 = vertexDuplicationChecker[hTriangle->vertex2];
				vertex2 = netSurface->nsm->getVertices()[vertexIndex2];
			}

			triangle->setVertices(vertex0, vertex1, vertex2);
			triangle->setVertexIndices(vertexIndex0, vertexIndex1, vertexIndex2);
			triangle->setNormal(hTriangle->normal.x, hTriangle->normal.y, hTriangle->normal.z);
			surface->getTriangles().push_back(triangle);
		}
	}

	netSurface->nsm->setColorMode(gaia3d::NoColor);
	netSurface->nsm->setHasNormals(true);
}

void clearHalfEdgeSystem(NetSurface* netSurface)
{
	size_t vertexListCount = netSurface->netVertices.size();
	for (size_t j = 0; j < vertexListCount; j++)
	{
		std::vector<HedgeVertex*>* vertexList = netSurface->netVertices[j];
		size_t vertexCount = vertexList->size();
		for (size_t k = 0; k < vertexCount; k++)
			delete (*vertexList)[k];

		delete vertexList;
	}
	netSurface->netVertices.clear();

	size_t triangleListCount = netSurface->netTriangles.size();
	for (size_t j = 0; j < triangleListCount; j++)
	{
		std::vector<HedgeTriangle*>* triangleList = netSurface->netTriangles[j];
		size_t triangleCount = triangleList->size();
		for (size_t k = 0; k < triangleCount; k++)
			delete (*triangleList)[k];

		delete triangleList;
	}
	netSurface->netTriangles.clear();

	size_t hedgeCount = netSurface->hedges.size();
	for (size_t j = 0; j < hedgeCount; j++)
		delete netSurface->hedges[j];
	netSurface->hedges.clear();
}

void calculateTextureCoordinates(gaia3d::TrianglePolyhedron* polyhedron,
								TYPE_CUBEFACE cubeface,
								float minS, float maxS, float minT, float maxT,
								float minX, float maxX, float minY, float maxY, float minZ, float maxZ)
{
	float mosaic_sRange = maxS - minS;
	float mosaic_tRange = maxT - minT;

	size_t surfaceCount = polyhedron->getSurfaces().size();
	for (size_t h = 0; h < surfaceCount; h++)
	{
		gaia3d::Surface* surface = polyhedron->getSurfaces()[h];
		size_t triangleCount = surface->getTriangles().size();

		for (size_t i = 0; i < triangleCount; i++)
		{
			gaia3d::Triangle *tri = surface->getTriangles()[i];

			if (cubeface == CUBEFACE_TOP)
			{
				float xRange = maxX - minX;
				float yRange = maxY - minY;

				for (int j = 0; j < 3; j++)
				{
					gaia3d::Vertex* vertex = tri->getVertices()[j];
					
					gaia3d::Point3D *p = &(vertex->position);
					double x = p->x;
					double y = p->y;
					double s = (x - minX) / xRange;
					double t = (y - minY) / yRange;

					// now correct s, t to put inside the mosaic.***
					double mosaicS = minS + s * mosaic_sRange;
					double mosaicT = minT + t * mosaic_tRange;

					vertex->textureCoordinate[0] = mosaicS; vertex->textureCoordinate[1] = mosaicT;
				}
			}
			else if (cubeface == CUBEFACE_BOTTOM)
			{
				float xRange = maxX - minX;
				float yRange = maxY - minY;

				for (int j = 0; j < 3; j++)
				{
					gaia3d::Vertex* vertex = tri->getVertices()[j];

					gaia3d::Point3D *p = &(vertex->position);
					double x = p->x;
					double y = p->y;
					double s = (x - minX) / xRange;
					double t = (y - minY) / yRange;
					t = 1.0 - t;

					// now correct s, t to put inside the mosaic.***
					double mosaicS = minS + s * mosaic_sRange;
					double mosaicT = minT + t * mosaic_tRange;
					vertex->textureCoordinate[0] = mosaicS; vertex->textureCoordinate[1] = mosaicT;
				}
			}
			else if (cubeface == CUBEFACE_FRONT)
			{
				float xRange = maxX - minX;
				float zRange = maxZ - minZ;

				for (int j = 0; j < 3; j++)
				{
					gaia3d::Vertex* vertex = tri->getVertices()[j];

					gaia3d::Point3D *p = &(vertex->position);
					double x = p->x;
					double z = p->z;
					double s = (x - minX) / xRange;
					double t = (z - minZ) / zRange;

					// now correct s, t to put inside the mosaic.***
					double mosaicS = minS + s * mosaic_sRange;
					double mosaicT = minT + t * mosaic_tRange;

					vertex->textureCoordinate[0] = mosaicS; vertex->textureCoordinate[1] = mosaicT;
				}
			}
			else if (cubeface == CUBEFACE_REAR)
			{
				float xRange = maxX - minX;
				float zRange = maxZ - minZ;

				for (int j = 0; j < 3; j++)
				{
					gaia3d::Vertex* vertex = tri->getVertices()[j];

					gaia3d::Point3D *p = &(vertex->position);
					double x = p->x;
					double z = p->z;
					double s = (x - minX) / xRange;
					double t = (z - minZ) / zRange;
					t = 1.0 - t;

					// now correct s, t to put inside the mosaic.***
					double mosaicS = minS + s * mosaic_sRange;
					double mosaicT = minT + t * mosaic_tRange;

					vertex->textureCoordinate[0] = mosaicS; vertex->textureCoordinate[1] = mosaicT;
				}
			}
			else if (cubeface == CUBEFACE_LEFT)
			{
				float yRange = maxY - minY;
				float zRange = maxZ - minZ;

				for (int j = 0; j < 3; j++)
				{
					gaia3d::Vertex* vertex = tri->getVertices()[j];

					gaia3d::Point3D *p = &(vertex->position);
					double y = p->y;
					double z = p->z;
					double t = (y - minY) / yRange;
					double s = (z - minZ) / zRange;

					//s = 1.0 - s;
					// now correct s, t to put inside the mosaic.***
					double mosaicS = minS + s * mosaic_sRange;
					double mosaicT = minT + t * mosaic_tRange;

					vertex->textureCoordinate[0] = mosaicS; vertex->textureCoordinate[1] = mosaicT;
				}
			}
			else if (cubeface == CUBEFACE_RIGHT)
			{
				float yRange = maxY - minY;
				float zRange = maxZ - minZ;

				for (int j = 0; j < 3; j++)
				{
					gaia3d::Vertex* vertex = tri->getVertices()[j];

					gaia3d::Point3D *p = &(vertex->position);
					double y = p->y;
					double z = p->z;
					double t = (y - minY) / yRange;
					double s = (z - minZ) / zRange;
					s = 1.0 - s;

					// now correct s, t to put inside the mosaic.***
					double mosaicS = minS + s * mosaic_sRange;
					double mosaicT = minT + t * mosaic_tRange;

					vertex->textureCoordinate[0] = mosaicS; vertex->textureCoordinate[1] = mosaicT;
				}
			}
		}
	}
}

void flipTextureCoordinateY(gaia3d::TrianglePolyhedron* polyhedron)
{
	size_t vertexCount = polyhedron->getVertices().size();
	for (size_t i = 0; i < vertexCount; i++)
	{
		gaia3d::Vertex* vertex = polyhedron->getVertices()[i];
		vertex->textureCoordinate[1] = 1.0 - vertex->textureCoordinate[1];
	}
}

void insertEachNetSurfaceTextureIntoMosaicTexture(unsigned char** mosaicTexture,
												unsigned int mosaicNumCols,
												unsigned mosaicNumRows,
												unsigned int insertCol,
												unsigned int insertRow,
												NetSurface* netSurface)
{
	unsigned char* dataRGBA = netSurface->texture;
	int dataRGBA_width = netSurface->textureWidth;
	int dataRGBA_height = netSurface->textureHeight;

	int mosaicTexturePixelWidth = mosaicNumCols * dataRGBA_width;
	int mosaicTexturePixelHeight = mosaicNumRows * dataRGBA_height;

	// first, find the leftDownCornerPixelPosition of the dataRGBA into the mosaicTexture.***
	int dataRGBA_leftDownPixelCol = insertCol * dataRGBA_width;
	int dataRGBA_leftDownPixelRow = insertRow * dataRGBA_height;

	int currentDataCol = 0;
	int currentDataRow = 0;
	int dataRGBA_size = dataRGBA_width * dataRGBA_height;
	for (int i = 0; i < dataRGBA_size; i++)
	{
		//unsigned char* sourcePixel = dataRGBA + i * 4;
		unsigned char r = dataRGBA[i * 4];
		unsigned char g = dataRGBA[i * 4 + 1];
		unsigned char b = dataRGBA[i * 4 + 2];
		unsigned char a = dataRGBA[i * 4 + 3];

		// must find the pixel position in mosaicTexture.***
		int insertPixelCol = dataRGBA_leftDownPixelCol + currentDataCol;
		int insertPixelRow = dataRGBA_leftDownPixelRow + currentDataRow;
		int mosaicTexPixelIdx = insertPixelCol + insertPixelRow * mosaicTexturePixelWidth;
		
		// change the mosaicTexture's pixel value.***
		//unsigned char* targetPixel = (*mosaicTexture) + mosaicTexPixelIdx * 4;
		//memcpy(targetPixel, sourcePixel, sizeof(unsigned char) * 4);
		(*mosaicTexture)[mosaicTexPixelIdx * 4] = r;
		(*mosaicTexture)[mosaicTexPixelIdx * 4 + 1] = g;
		(*mosaicTexture)[mosaicTexPixelIdx * 4 + 2] = b;
		(*mosaicTexture)[mosaicTexPixelIdx * 4 + 3] = a;

		currentDataCol++;
		if (currentDataCol == dataRGBA_width)
		{
			currentDataCol = 0;
			currentDataRow++;
		}
	}

	// calculate texture coordinates of net surface meshes for mosaic texture
	int textoreCoord_offSet = 5;

	float minS = (float)(dataRGBA_leftDownPixelCol + textoreCoord_offSet) / (float)mosaicTexturePixelWidth;
	float maxS = (float)(dataRGBA_leftDownPixelCol + dataRGBA_width - textoreCoord_offSet) / (float)mosaicTexturePixelWidth;
	float minT = (float)(dataRGBA_leftDownPixelRow + textoreCoord_offSet) / (float)mosaicTexturePixelHeight;
	float maxT = (float)(dataRGBA_leftDownPixelRow + dataRGBA_height - textoreCoord_offSet) / (float)mosaicTexturePixelHeight;
	calculateTextureCoordinates(netSurface->nsm, netSurface->cubeFace,
								minS, maxS, minT, maxT,
								float(netSurface->domainBox.minX), float(netSurface->domainBox.maxX),
								float(netSurface->domainBox.minY), float(netSurface->domainBox.maxY),
								float(netSurface->domainBox.minZ), float(netSurface->domainBox.maxZ));
	flipTextureCoordinateY(netSurface->nsm);
}

void makeNetSurfaceTextures(std::vector<NetSurface*>& netSurfaces,
							NetSurfaceMeshSetting* setting,
							unsigned char** textureData,
							int& textureWidth,
							int& textureHeight)
{
	if (netSurfaces.empty())
		return;

	// now, calculate texture mosaic width & height.***
	size_t netSurfaceCount = netSurfaces.size();
	int textureMosaicNumCols, textureMosaicNumRows;
	float netSurfacesCount_sqrt = sqrt(float(netSurfaceCount));
	int numRows = int(floorf(netSurfacesCount_sqrt));
	int numCols = numRows; // init with the same number.***
	bool find = false;
	while (!find)
	{
		if (numRows * numCols >= netSurfaceCount)
		{
			find = true;
			textureMosaicNumRows = numRows;
			textureMosaicNumCols = numCols;
		}
		numCols++;
	}

	// create total texture
	int netSurfTexWidth = setting->netSurfaceMeshTextureWidth;
	int netSurfTexHeight = setting->netSurfaceMeshTextureHeight;
	int netSurfTexSize = netSurfTexWidth * netSurfTexHeight;
	int totalTextureSize = textureMosaicNumRows * textureMosaicNumCols * netSurfTexSize * 4;
	*textureData = new unsigned char[totalTextureSize];
	memset(*textureData, 0x00, sizeof(unsigned char)*totalTextureSize);
	textureWidth = netSurfTexWidth * textureMosaicNumCols;
	textureHeight = netSurfTexHeight * textureMosaicNumRows;

	int currentRow = 0, currentCol = 0;
	for (size_t i = 0; i < netSurfaceCount; i++)
	{
		NetSurface* netSurface = netSurfaces[i];
		insertEachNetSurfaceTextureIntoMosaicTexture(textureData, textureMosaicNumCols, textureMosaicNumRows, currentCol, currentRow, netSurface);
		// after inserting data, increment currentCol.***
		currentCol++;

		if (currentCol >= textureMosaicNumCols)
		{
			currentCol = 0;
			currentRow++;
		}
	}
}

gaia3d::TrianglePolyhedron* mergePolyhedrons(std::vector<gaia3d::TrianglePolyhedron*> meshes)
{
	gaia3d::TrianglePolyhedron* result = new gaia3d::TrianglePolyhedron;
	gaia3d::Surface* resultSurface = new gaia3d::Surface;
	result->getSurfaces().push_back(resultSurface);
	
	size_t meshCount = meshes.size();
	for (size_t i = 0; i < meshCount; i++)
	{
		size_t vertexIndexOffset = result->getVertices().size();
		gaia3d::TrianglePolyhedron* mesh = meshes[i];

		size_t vertexCount = mesh->getVertices().size();
		for (size_t j = 0; j < vertexCount; j++)
			result->getVertices().push_back(mesh->getVertices()[j]);

		size_t surfaceCount = mesh->getSurfaces().size();
		for (size_t j = 0; j < surfaceCount; j++)
		{
			gaia3d::Surface* surface = mesh->getSurfaces()[j];
			size_t triangleCount = surface->getTriangles().size();
			for (size_t k = 0; k < triangleCount; k++)
			{
				gaia3d::Triangle* triangle = surface->getTriangles()[k];
				size_t* vertexIndices = triangle->getVertexIndices();
				triangle->setVertexIndices(vertexIndices[0] + vertexIndexOffset, vertexIndices[1] + vertexIndexOffset, vertexIndices[2] + vertexIndexOffset);
				resultSurface->getTriangles().push_back(triangle);
			}

			surface->getTriangles().clear();
		}
		
		mesh->getVertices().clear();
	}

	result->setHasNormals(true);
	result->setHasTextureCoordinates(true);
	result->setColorMode(gaia3d::NoColor);
	result->setId(0);
	
	return result;
}

NetSurface* makeNetSurfaceMeshFromOriginalDirectly(gaia3d::TrianglePolyhedron* mesh, std::map<HedgeVertex*, gaia3d::Vertex*>& vertexMapper)
{
	NetSurface* netSurface = new NetSurface;

	// make half edge vertices
	std::vector<HedgeVertex*>* hVertices = new std::vector<HedgeVertex*>;
	netSurface->netVertices.push_back(hVertices);
	size_t vertexCount = mesh->getVertices().size();
	for (size_t i = 0; i < vertexCount; i++)
	{
		HedgeVertex* hVertex = new HedgeVertex;
		hVertex->bOnGeometrySurface = true;
		hVertex->position = mesh->getVertices()[i]->position;
		hVertices->push_back(hVertex);
		vertexMapper[hVertex] = mesh->getVertices()[i];
	}

	// collect vertex-sharing triangles along each vertex
	std::map<gaia3d::Vertex*, std::vector<gaia3d::Triangle*>> vertexUseCase;
	size_t surfaceCount = mesh->getSurfaces().size();
	for (size_t j = 0; j < surfaceCount; j++)
	{
		gaia3d::Surface* surface = mesh->getSurfaces()[j];
		size_t triangleCount = surface->getTriangles().size();
		for (size_t k = 0; k < triangleCount; k++)
		{
			gaia3d::Triangle* triangle = surface->getTriangles()[k];

			for (size_t m = 0; m < 3; m++)
			{
				gaia3d::Vertex* vertex = triangle->getVertices()[m];
				if (vertexUseCase.find(vertex) == vertexUseCase.end())
				{
					std::vector<gaia3d::Triangle*> trianglesUsingSameVertex;
					trianglesUsingSameVertex.push_back(triangle);
					vertexUseCase[vertex] = trianglesUsingSameVertex;
				}
				else
					vertexUseCase[vertex].push_back(triangle);
			}
		}
	}

	// build half edge triangles
	std::vector<HedgeTriangle*>* hTriangles = new std::vector<HedgeTriangle*>;
	netSurface->netTriangles.push_back(hTriangles);
	vertexCount = mesh->getVertices().size();
	for (size_t i = 0; i < vertexCount; i++)
	{
		gaia3d::Vertex* vertex = mesh->getVertices()[i];

		size_t triangleCount = vertexUseCase[vertex].size();
		std::vector<HedgeTriangle*> vertexSharingHTriangles;
		for (size_t j = 0; j < triangleCount; j++)
		{
			gaia3d::Triangle* rawTriangle = vertexUseCase[vertex][j];

			HedgeVertex* vertex0 = (*hVertices)[rawTriangle->getVertexIndices()[0]];
			HedgeVertex* vertex1 = (*hVertices)[rawTriangle->getVertexIndices()[1]];
			HedgeVertex* vertex2 = (*hVertices)[rawTriangle->getVertexIndices()[2]];

			HedgeTriangle* triangle = new HedgeTriangle;
			triangle->vertex0 = vertex0; triangle->vertex1 = vertex1; triangle->vertex2 = vertex2;

			Hedge* hedge0 = new Hedge;
			Hedge* hedge1 = new Hedge;
			Hedge* hedge2 = new Hedge;

			hedge0->face = triangle;
			hedge0->next = hedge1;
			hedge0->startVertex = vertex0;
			hedge0->twin = NULL;
			vertex0->hedgesFromThisVertex.push_back(hedge0);

			hedge1->face = triangle;
			hedge1->next = hedge2;
			hedge1->startVertex = vertex1;
			hedge1->twin = NULL;
			vertex1->hedgesFromThisVertex.push_back(hedge1);

			hedge2->face = triangle;
			hedge2->next = hedge0;
			hedge2->startVertex = vertex2;
			hedge2->twin = NULL;
			vertex2->hedgesFromThisVertex.push_back(hedge2);

			hedge0->direction = vertex1->position - vertex0->position;
			hedge1->direction = vertex2->position - vertex1->position;
			hedge2->direction = vertex0->position - vertex2->position;
			hedge0->direction.normalize();
			hedge1->direction.normalize();
			hedge2->direction.normalize();

			triangle->hedge = hedge0;
			gaia3d::Point3D normal;
			gaia3d::GeometryUtility::calculatePlaneNormal(vertex0->position.x, vertex0->position.y, vertex0->position.z,
				vertex1->position.x, vertex1->position.y, vertex1->position.z,
				vertex2->position.x, vertex2->position.y, vertex2->position.z,
				normal.x, normal.y, normal.z, true);
			triangle->normal = normal;

			hTriangles->push_back(triangle);
			netSurface->hedges.push_back(hedge0);
			netSurface->hedges.push_back(hedge1);
			netSurface->hedges.push_back(hedge2);

			vertexSharingHTriangles.push_back(triangle);
		}

		// build twin relationship
		for (size_t j = 0; j < triangleCount; j++)
		{
			HedgeTriangle* hTriangleA = vertexSharingHTriangles[j];
			for (size_t k = j + 1; k < triangleCount; k++)
			{
				HedgeTriangle* hTriangleB = vertexSharingHTriangles[k];

				Hedge* curHedgeA = hTriangleA->hedge;
				for (char m = 0; m < 3; m++)
				{
					curHedgeA = curHedgeA->next;

					if (curHedgeA->twin != NULL)
						continue;

					Hedge* curHedgeB = hTriangleB->hedge;
					for (char n = 0; n < 3; n++)
					{
						curHedgeB = curHedgeB->next;

						if (curHedgeB->twin != NULL)
							continue;

						if (curHedgeA->startVertex == curHedgeB->next->startVertex &&
							curHedgeA->next->startVertex == curHedgeB->startVertex)
						{
							curHedgeA->twin = curHedgeB;
							curHedgeB->twin = curHedgeA;
						}
					}
				}
			}
		}
	}

	return netSurface;
}