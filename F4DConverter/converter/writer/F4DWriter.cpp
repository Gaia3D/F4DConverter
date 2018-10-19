
#include "stdafx.h"

#include "F4DWriter.h"
#include <direct.h>
#include <io.h>
#include <sys/stat.h>
#include <algorithm>
#include "../process/ConversionProcessor.h"
#include "../LogWriter.h"
#include "../geometry/ColorU4.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../util/stb_image_write.h"


F4DWriter::F4DWriter(ConversionProcessor* conversionResult)
:processor(conversionResult)
{
	version = "0.0.1";
	guid = "abcdefghi";
	guidLength = 9;
}

F4DWriter::~F4DWriter()
{}

bool F4DWriter::write()
{
	// make target root folder
	bool outputFolderExist = false;
	std::string resultPath = folder + "/F4D_" + processor->getAttribute(F4DID);
	if (_access(resultPath.c_str(), 0) == 0)
	{
		struct stat status;
		stat(resultPath.c_str(), &status);
		if (status.st_mode & S_IFDIR)
			outputFolderExist = true;
	}
	if (!outputFolderExist)
	{
		if (_mkdir(resultPath.c_str()) != 0)
		{
			LogWriter::getLogWriter()->addContents(std::string(ERROR_FLAG), false);
			LogWriter::getLogWriter()->addContents(std::string(CANNOT_CREATE_DIRECTORY), false);
			return false;
		}
	}

	std::string headerPath = resultPath + "/HeaderAsimetric.hed";
	FILE* file = fopen(headerPath.c_str(), "wb");
	// write header and get order of texture recorded
	std::map<std::string, size_t> textureIndices;
	writeHeader(file, textureIndices);
	fclose(file);

	// create reference directory
	outputFolderExist = false;
	std::string referencePath = resultPath + "/References";
	if (_access(referencePath.c_str(), 0) == 0)
	{
		struct stat status;
		stat(referencePath.c_str(), &status);
		if (status.st_mode & S_IFDIR)
			outputFolderExist = true;
	}
	if (!outputFolderExist)
	{
		if (_mkdir(referencePath.c_str()) != 0)
		{
			LogWriter::getLogWriter()->addContents(std::string(ERROR_FLAG), false);
			LogWriter::getLogWriter()->addContents(std::string(CANNOT_CREATE_DIRECTORY), false);
			return false;
		}
	}

	// create lod2 geometries directory
	outputFolderExist = false;
	std::string lod2Path = resultPath + "/Bricks";
	if (_access(lod2Path.c_str(), 0) == 0)
	{
		struct stat status;
		stat(lod2Path.c_str(), &status);
		if (status.st_mode & S_IFDIR)
			outputFolderExist = true;
	}
	if (!outputFolderExist)
	{
		if (_mkdir(lod2Path.c_str()) != 0)
		{
			LogWriter::getLogWriter()->addContents(std::string(ERROR_FLAG), false);
			LogWriter::getLogWriter()->addContents(std::string(CANNOT_CREATE_DIRECTORY), false);
			return false;
		}
	}

	// create model directory
	outputFolderExist = false;
	std::string modelPath = resultPath + "/Models";
	if (_access(modelPath.c_str(), 0) == 0)
	{
		struct stat status;
		stat(modelPath.c_str(), &status);
		if (status.st_mode & S_IFDIR)
			outputFolderExist = true;
	}
	if (!outputFolderExist)
	{
		if (_mkdir(modelPath.c_str()) != 0)
		{
			LogWriter::getLogWriter()->addContents(std::string(ERROR_FLAG), false);
			LogWriter::getLogWriter()->addContents(std::string(CANNOT_CREATE_DIRECTORY), false);
			return false;
		}
	}

	writeReferencesAndModels(referencePath, modelPath, lod2Path, textureIndices);

	// create image directory
	if (!processor->getTextureInfo().empty())
	{
		outputFolderExist = false;
		std::string imagePath = resultPath + "/Images_Resized";
		if (_access(imagePath.c_str(), 0) == 0)
		{
			struct stat status;
			stat(imagePath.c_str(), &status);
			if (status.st_mode & S_IFDIR)
				outputFolderExist = true;
		}
		if (!outputFolderExist)
		{
			if (_mkdir(imagePath.c_str()) != 0)
			{
				LogWriter::getLogWriter()->addContents(std::string(ERROR_FLAG), false);
				LogWriter::getLogWriter()->addContents(std::string(CANNOT_CREATE_DIRECTORY), false);
				return false;
			}
		}
		writeTextures(imagePath);
	}

	// net surface mesh lod 3~5
	std::map<unsigned char, gaia3d::TrianglePolyhedron*>::iterator iterNetSurfaceMesh = processor->getNetSurfaceMeshes().begin();
	for (; iterNetSurfaceMesh != processor->getNetSurfaceMeshes().end(); iterNetSurfaceMesh++)
	{ 
		unsigned char lod = iterNetSurfaceMesh->first;
		gaia3d::TrianglePolyhedron* netSurfaceMesh = iterNetSurfaceMesh->second;
		std::string filePath = resultPath + "/lod" + std::to_string(lod);
		file = fopen(filePath.c_str(), "wb");
		writeNetSurfaceMesh(netSurfaceMesh, file);
		fclose(file);
	}

	// mosaic textures for net surface mesh lod 2 ~ 5
	writeNetSurfaceTextures(resultPath);

	return true;
}

bool F4DWriter::writeHeader(FILE* f, std::map<std::string, size_t>& textureIndices)
{
	// versoin div : start(20170323)
	// version
	fwrite(version.c_str(), sizeof(char), 5, f);
	// guid length
	fwrite(&guidLength, sizeof(int), 1, f);
	// guid
	fwrite(guid.c_str(), sizeof(char), guidLength, f);
	// representative longitude, latitude, altitude
	double longitude = processor->getLongitude(), latitude = processor->getLatitude();
	float altitude = processor->getAltitude();
	fwrite(&longitude, sizeof(double), 1, f);
	fwrite(&latitude, sizeof(double), 1, f);
	fwrite(&altitude, sizeof(float), 1, f);
	// bounding box
	float minX = (float)processor->getBoundingBox().minX, minY = (float)processor->getBoundingBox().minY, minZ = (float)processor->getBoundingBox().minZ;
	float maxX = (float)processor->getBoundingBox().maxX, maxY = (float)processor->getBoundingBox().maxY, maxZ = (float)processor->getBoundingBox().maxZ;
	fwrite(&minX, sizeof(float), 1, f); fwrite(&minY, sizeof(float), 1, f); fwrite(&minZ, sizeof(float), 1, f);
	fwrite(&maxX, sizeof(float), 1, f); fwrite(&maxY, sizeof(float), 1, f); fwrite(&maxZ, sizeof(float), 1, f);

	// spatial octree info
	writeOctreeInfo(processor->getSpatialOctree(), f);

	// material info
	unsigned int textureCount = (unsigned int)processor->getTextureInfo().size();
	fwrite(&textureCount, sizeof(unsigned int), 1, f);
	std::map<std::string, std::string>::iterator iterTexture = processor->getTextureInfo().begin();
	std::string materialType("diffuse");
	unsigned int typeLength = (unsigned int)materialType.size();
	for (size_t i = 0; iterTexture != processor->getTextureInfo().end(); iterTexture++, i++)
	{
		std::string textureFileName = iterTexture->first;
		unsigned int textureFileNameLength = (unsigned int)textureFileName.size();
		
		fwrite(&typeLength, sizeof(unsigned int), 1, f);
		fwrite(materialType.c_str(), sizeof(char), typeLength, f);
		fwrite(&textureFileNameLength, sizeof(int), 1, f);
		fwrite(textureFileName.c_str(), sizeof(char), textureFileNameLength, f);

		textureIndices[textureFileName] = i;
	}

	// lod info
	unsigned char geomLodCount = 6;
	fwrite(&geomLodCount, sizeof(unsigned char), 1, f);
	unsigned char lod;
	bool bDividedBySpatialOctree;
	std::string lodFileName;
	unsigned char lodFileNameLength;
	std::string lodTextureFileName;
	unsigned char lodTextureFileNameLength;
	for (lod = 0; lod < geomLodCount; lod++)
	{
		// lod
		fwrite(&lod, sizeof(unsigned char), 1, f);
		
		// if data is divided by spatial octrees or not
		if (lod < 3)
			bDividedBySpatialOctree = true;
		else
			bDividedBySpatialOctree = false;
		fwrite(&bDividedBySpatialOctree, sizeof(bool), 1, f);

		// merged lod data file name
		if (lod > 2)
		{
			lodFileName = "lod" + std::to_string(lod);
			lodFileNameLength = (unsigned char)lodFileName.length();
			fwrite(&lodFileNameLength, sizeof(unsigned char), 1, f);
			fwrite(lodFileName.c_str(), sizeof(char), lodFileNameLength, f);
		}

		// merged lod data texture file name
		if (lod > 1)
		{
			lodTextureFileName = "mosaicTextureLod" + std::to_string(lod) + ".jpg";
			lodTextureFileNameLength = (unsigned char)lodTextureFileName.length();
			fwrite(&lodTextureFileNameLength, sizeof(unsigned char), 1, f);
			fwrite(lodTextureFileName.c_str(), sizeof(char), lodTextureFileNameLength, f);
		}
	}
	
	// end marker
	char endMarker = 0;
	fwrite(&endMarker, sizeof(char), 1, f);

	return true;
}

bool F4DWriter::writeModels(FILE* f, std::vector<gaia3d::TrianglePolyhedron*>& models)
{
	fwrite(&version, sizeof(char), 5, f);

	unsigned int modelCount = (unsigned int)models.size();
	fwrite(&modelCount, sizeof(unsigned int), 1, f);

	gaia3d::TrianglePolyhedron* model;
	unsigned int modelIndex;
	float minX, minY, minZ, maxX, maxY, maxZ;
	unsigned int vboCount, vertexCount, indexCount;
	unsigned char sizeLevels;
	float thresholds[TriangleSizeLevels];
	float x, y, z;
	char nx, ny, nz;
	bool bInterleavedMode = false;
	char padding = 0;
	unsigned short index;
	for(size_t i = 0; i < modelCount; i++)
	{
		model = models[i];
		modelIndex = (unsigned int)model->getReferenceInfo().modelIndex;
		fwrite(&modelIndex, sizeof(unsigned int), 1, f);

		// bounding box
		minX = (float)model->getBoundingBox().minX; minY = (float)model->getBoundingBox().minY; minZ = (float)model->getBoundingBox().minZ;
		maxX = (float)model->getBoundingBox().maxX; maxY = (float)model->getBoundingBox().maxY; maxZ = (float)model->getBoundingBox().maxZ;
		fwrite(&minX, sizeof(float), 1, f); fwrite(&minY, sizeof(float), 1, f); fwrite(&minZ, sizeof(float), 1, f);
		fwrite(&maxX, sizeof(float), 1, f); fwrite(&maxY, sizeof(float), 1, f); fwrite(&maxZ, sizeof(float), 1, f);

		// vbo count
		vboCount = (unsigned int)model->getVbos().size();
		fwrite(&vboCount, sizeof(unsigned int), 1, f);

		// vbo
		for(unsigned int j = 0; j < vboCount; j++)
		{
			vertexCount = (unsigned int)model->getVbos()[j]->vertices.size();
			// vertex count
			fwrite(&vertexCount, sizeof(unsigned int), 1, f);

			// vertex positions
			for(unsigned int k = 0; k < vertexCount; k++)
			{
				x = (float)model->getVbos()[j]->vertices[k]->position.x;
				y = (float)model->getVbos()[j]->vertices[k]->position.y;
				z = (float)model->getVbos()[j]->vertices[k]->position.z;
				fwrite(&x, sizeof(float), 1, f); fwrite(&y, sizeof(float), 1, f); fwrite(&z, sizeof(float), 1, f);
			}

			// normal count
			fwrite(&vertexCount, sizeof(unsigned int), 1, f);

			// normals
			for(unsigned int k = 0; k < vertexCount; k++)
			{
				nx = (char)(127.0f * model->getVbos()[j]->vertices[k]->normal.x);
				ny = (char)(127.0f * model->getVbos()[j]->vertices[k]->normal.y);
				nz = (char)(127.0f * model->getVbos()[j]->vertices[k]->normal.z);
				fwrite(&nx, sizeof(char), 1, f); fwrite(&ny, sizeof(char), 1, f); fwrite(&nz, sizeof(char), 1, f);
				if(bInterleavedMode)
					fwrite(&padding, sizeof(char), 1, f);
			}

			// index count
			indexCount =  (unsigned int)model->getVbos()[j]->indices.size();
			fwrite(&indexCount, sizeof(unsigned int), 1, f); // 전체 index개수
			sizeLevels = TriangleSizeLevels; // 삼각형을 크기별로 정렬할 때 적용된 크기 개수
			fwrite(&sizeLevels, sizeof(unsigned char), 1, f);
			for(unsigned char k = 0; k < sizeLevels; k++)
				thresholds[k] = (float)model->getVbos()[j]->triangleSizeThresholds[k]; // 삼각형을 크기별로 정렬할 때 적용된 크기들
			fwrite(thresholds, sizeof(float), sizeLevels, f);
			fwrite(model->getVbos()[j]->indexMarker, sizeof(unsigned int), sizeLevels, f); // 정렬된 vertex들의 index에서 크기 기준이 변경되는 최초 삼각형의 vertex의 index 위치 marker
			for(size_t k = 0; k < indexCount; k++)
			{
				index = models[i]->getVbos()[j]->indices[k];
				fwrite(&index, sizeof(unsigned short), 1, f);
			}
		}

		bool bLegoExist = false;
		fwrite(&bLegoExist, sizeof(bool), 1, f);
	}

	return true;
}

bool F4DWriter::writeReferencesAndModels(std::string& referencePath, std::string& modelPath, std::string& lod2Path, std::map<std::string, size_t>& textureIndices)
{
	std::vector<gaia3d::OctreeBox*> leafBoxes;
	gaia3d::OctreeBox* leafBox;
	processor->getSpatialOctree()->getAllLeafBoxes(leafBoxes, true);
	size_t leafCount = leafBoxes.size();
	std::string referenceFilePath; // each reference file full path
	std::string modelFilePath; // each model file full path
	std::string lod2FilePath; // each lod 2 file full path
	std::vector<gaia3d::TrianglePolyhedron*> models;
	size_t modelCount;
	FILE* file = NULL;
	unsigned int referenceCount; // total reference count
	size_t meshCount = processor->getAllMeshes().size();
	unsigned int referenceId, modelId; // referenc id, block id which reference is refering to
	unsigned char objectIdLength;
	std::string objectId;
	gaia3d::TrianglePolyhedron* reference;
	gaia3d::TrianglePolyhedron* model;
	bool bFound;
	unsigned int vertexCount; // vertex count in a reference
	float m;	// element of transform matrix by which referend block is transformed into current reference position
	bool bColor; // if color exists
	unsigned short valueType; // array value type
	unsigned char colorDimension; // color channel count
	bool bTextureCoordinate; // if texture coordinate exists
	unsigned int textureIndex;
	unsigned int totalTriangleCount;
	for (size_t i = 0; i < leafCount; i++)
	{
		leafBox = leafBoxes[i];
		totalTriangleCount = 0;

		//--------- save reference info of each spatial octree -----------

		referenceFilePath = referencePath + "/" + std::to_string((long long)((gaia3d::SpatialOctreeBox*)leafBoxes[i])->octreeId) + std::string("_Ref");
		file = fopen(referenceFilePath.c_str(), "wb");
		fwrite(&version, sizeof(char), 5, file);

		// reference count in each octrees
		referenceCount = (unsigned int)leafBox->meshes.size();
		fwrite(&referenceCount, sizeof(unsigned int), 1, file);

		for (unsigned int j = 0; j < referenceCount; j++)
		{
			reference = leafBox->meshes[j];

			// extract models
			if (reference->getReferenceInfo().model == NULL)
				model = reference;
			else
				model = reference->getReferenceInfo().model;
			bFound = false;
			modelCount = models.size();
			for (size_t k = 0; k < modelCount; k++)
			{
				if (models[k] == model)
				{
					bFound = true;
					break;
				}
			}

			if (!bFound)
				models.push_back(model);

			// reference id
			referenceId = (unsigned int)reference->getId();
			fwrite(&referenceId, sizeof(unsigned int), 1, file);

			// reference object id
			if (reference->doesStringAttributeExist(std::string(ObjectGuid)))
			{
				objectId = reference->getStringAttribute(std::string(ObjectGuid));
				objectIdLength = (unsigned char)objectId.length();
				fwrite(&objectIdLength, sizeof(unsigned char), 1, file);
				if (objectIdLength > 0)
					fwrite(objectId.c_str(), sizeof(char), objectIdLength, file);
			}
			else
			{
				std::string tmpObjectId = std::to_string(referenceId);
				objectIdLength = (unsigned char)tmpObjectId.length();
				fwrite(&objectIdLength, sizeof(unsigned char), 1, file);
				fwrite(tmpObjectId.c_str(), sizeof(char), objectIdLength, file);
			}


			// model id
			modelId = (unsigned int)reference->getReferenceInfo().modelIndex;
			fwrite(&modelId, sizeof(unsigned int), 1, file);

			// transform matrix
			unsigned char matrixType = reference->getReferenceInfo().mat.getMatrixType(10E-8);
			fwrite(&matrixType, sizeof(unsigned char), 1, file);
			switch (matrixType)
			{
			case 0: // nothing to do
			{
			}break;
			case 1: // translation
			{
				// save translation vector.
				float translation_x = float(reference->getReferenceInfo().mat.m[3][0]);
				float translation_y = float(reference->getReferenceInfo().mat.m[3][1]);
				float translation_z = float(reference->getReferenceInfo().mat.m[3][2]);
				fwrite(&translation_x, sizeof(float), 1, file);// SAVE.*** SAVE.*** SAVE.*** SAVE.*** SAVE.*** SAVE.*** SAVE.***
				fwrite(&translation_y, sizeof(float), 1, file);// SAVE.*** SAVE.*** SAVE.*** SAVE.*** SAVE.*** SAVE.*** SAVE.***
				fwrite(&translation_z, sizeof(float), 1, file);// SAVE.*** SAVE.*** SAVE.*** SAVE.*** SAVE.*** SAVE.*** SAVE.***
			}break;
			case 2: // transform
			{
				for (size_t c = 0; c < 4; c++)
				{
					for (size_t r = 0; r < 4; r++)
					{
						m = (float)reference->getReferenceInfo().mat.m[c][r];
						fwrite(&m, sizeof(float), 1, file);
					}
				}
			}break;
			}

			vertexCount = (unsigned int)reference->getVertices().size();

			// representative color of this reference
			if (reference->getColorMode() == gaia3d::SingleColor)
			{
				bColor = true;
				fwrite(&bColor, sizeof(bool), 1, file);
				valueType = 5121; // (5120 signed byte), (5121 unsigned byte), (5122 signed short), (5123 unsigned short), (5126 float)
				fwrite(&valueType, sizeof(unsigned short), 1, file);
				colorDimension = 4;
				fwrite(&colorDimension, sizeof(unsigned char), 1, file);
				writeColor(reference->getSingleColor(), valueType, true, file);
			}
			else
			{
				bColor = false;
				fwrite(&bColor, sizeof(bool), 1, file);
			}

			// if vertex color & texture coordinate exist
			bColor = (reference->getColorMode() == gaia3d::ColorsOnVertices) ? true : false;
			fwrite(&bColor, sizeof(bool), 1, file);
			bTextureCoordinate = reference->doesThisHaveTextureCoordinates();
			fwrite(&bTextureCoordinate, sizeof(bool), 1, file);

			// save vertex color & texture coordinate on vbo count loop
			if (bColor || bTextureCoordinate)
			{
				unsigned int vboCount = (unsigned int)reference->getVbos().size();
				fwrite(&vboCount, sizeof(unsigned int), 1, file);

				for (unsigned int k = 0; k < vboCount; k++)
				{
					gaia3d::Vbo* vbo = reference->getVbos()[k];
					unsigned int vboVertexCount = (unsigned int)vbo->vertices.size();

					if (bColor)
					{
						valueType = 5121;
						fwrite(&valueType, sizeof(unsigned short), 1, file);
						colorDimension = 3;
						fwrite(&colorDimension, sizeof(unsigned char), 1, file);

						fwrite(&vboVertexCount, sizeof(unsigned int), 1, file);
						for (unsigned int l = 0; l < vboVertexCount; l++)
							writeColor(vbo->vertices[l]->color, valueType, false, file);
					}

					if (bTextureCoordinate)
					{
						valueType = 5126;
						fwrite(&valueType, sizeof(unsigned short), 1, file);

						fwrite(&vboVertexCount, sizeof(unsigned int), 1, file);
						for (unsigned int l = 0; l < vboVertexCount; l++)
						{
							float tx = (float)vbo->vertices[l]->textureCoordinate[0];
							float ty = (float)vbo->vertices[l]->textureCoordinate[1];
							fwrite(&tx, sizeof(float), 1, file);
							fwrite(&ty, sizeof(float), 1, file);
						}
					}
				}
			}

			if (bTextureCoordinate)
			{
				textureIndex = (unsigned int)textureIndices[reference->getStringAttribute(TextureName)];
				fwrite(&textureIndex, sizeof(unsigned int), 1, file);
			}
			else
			{
				int tmpTextureIndex = -1;
				fwrite(&tmpTextureIndex, sizeof(int), 1, file);
			}

			size_t surfaceCount = reference->getSurfaces().size();
			for (size_t k = 0; k < surfaceCount; k++)
			{
				gaia3d::Surface* surface = reference->getSurfaces()[k];
				size_t triangleCount = surface->getTriangles().size();
				totalTriangleCount += (unsigned int)triangleCount;
			}
		}

		// total triangle count
		fwrite(&totalTriangleCount, sizeof(unsigned int), 1, file);

		// visibility indices
		writeVisibilityIndices(file, (static_cast<gaia3d::SpatialOctreeBox*>(leafBox))->exteriorOcclusionInfo);
		writeVisibilityIndices(file, (static_cast<gaia3d::SpatialOctreeBox*>(leafBox))->interiorOcclusionInfo);
		fclose(file);

		//--------- save lod 2 geometry of each spatial octree -----------
		if (((gaia3d::SpatialOctreeBox*)leafBox)->netSurfaceMesh != NULL)
		{
			lod2FilePath = lod2Path + "/" + std::to_string((long long)((gaia3d::SpatialOctreeBox*)leafBoxes[i])->octreeId) + "_Brick";
			file = fopen(lod2FilePath.c_str(), "wb");
			writeNetSurfaceMesh(((gaia3d::SpatialOctreeBox*)leafBox)->netSurfaceMesh, file);
			fclose(file);
		}

		//--------- save model of each spatial octree ----------
		modelFilePath = modelPath + "/" + std::to_string((long long)((gaia3d::SpatialOctreeBox*)leafBoxes[i])->octreeId) + "_Model";
		file = fopen(modelFilePath.c_str(), "wb");
		this->writeModels(file, models);
		fclose(file);
		models.clear();
	}

	return true;
}

bool F4DWriter::writeVisibilityIndices(FILE* f, gaia3d::OctreeBox* octree)
{
	bool bRoot = octree->parent == NULL ? true : false;
	fwrite(&bRoot, sizeof(bool), 1, f);
	if (bRoot)
	{
		float minX, minY, minZ, maxX, maxY, maxZ;

		minX = (float)octree->minX;
		minY = (float)octree->minY;
		minZ = (float)octree->minZ;
		maxX = (float)octree->maxX;
		maxY = (float)octree->maxY;
		maxZ = (float)octree->maxZ;
		fwrite(&minX, sizeof(float), 1, f); fwrite(&maxX, sizeof(float), 1, f);
		fwrite(&minY, sizeof(float), 1, f); fwrite(&maxY, sizeof(float), 1, f);
		fwrite(&minZ, sizeof(float), 1, f); fwrite(&maxZ, sizeof(float), 1, f);
	}

	unsigned int childCount = (unsigned int)octree->children.size();
	fwrite(&childCount, sizeof(unsigned int), 1, f);

	if (childCount == 0)
	{
		unsigned int referenceCount = (unsigned int)octree->meshes.size();
		fwrite(&referenceCount, sizeof(unsigned int), 1, f);

		unsigned int referenceId;
		for (unsigned int i = 0; i < referenceCount; i++)
		{
			referenceId = (unsigned int)octree->meshes[i]->getId();
			fwrite(&referenceId, sizeof(unsigned int), 1, f);
		}
	}
	else
	{
		for (unsigned int i = 0; i < childCount; i++)
		{
			writeVisibilityIndices(f, octree->children[i]);
		}
	}

	return true;
}

bool F4DWriter::writeOctreeInfo(gaia3d::OctreeBox* octree, FILE* f)
{
	unsigned int level = octree->level;
	fwrite(&level, sizeof(unsigned int), 1, f);

	float minX, minY, minZ, maxX, maxY, maxZ;
	if(level == 0)
	{
		minX = (float)octree->minX;
		minY = (float)octree->minY;
		minZ = (float)octree->minZ;
		maxX = (float)octree->maxX;
		maxY = (float)octree->maxY;
		maxZ = (float)octree->maxZ;
		fwrite(&minX, sizeof(float), 1, f); fwrite(&maxX, sizeof(float), 1, f);
		fwrite(&minY, sizeof(float), 1, f); fwrite(&maxY, sizeof(float), 1, f);
		fwrite(&minZ, sizeof(float), 1, f); fwrite(&maxZ, sizeof(float), 1, f);
	}

	unsigned char childCount = (unsigned char)octree->children.size();
	fwrite(&childCount, sizeof(unsigned char), 1, f);

	unsigned int triangleCount = 0;
	size_t meshCount = octree->meshes.size();
	size_t surfaceCount;
	for(size_t i = 0; i < meshCount; i++)
	{
		surfaceCount = octree->meshes[i]->getSurfaces().size();
		for(size_t j = 0; j < surfaceCount; j++)
			triangleCount += (unsigned int)octree->meshes[i]->getSurfaces()[j]->getTriangles().size();
	}
	fwrite(&triangleCount, sizeof(unsigned int), 1, f);

	for(unsigned char i = 0; i < childCount; i++)
	{
		writeOctreeInfo(octree->children[i], f);
	}

	return true;
}

void F4DWriter::writeColor(unsigned long color, unsigned short type, bool bAlpha, FILE* file)
{
	// color type : (5120 signed byte), (5121 unsigned byte), (5122 signed short), (5123 unsigned short), (5126 float)

	unsigned char red = GetRedValue(color);
	unsigned char green = GetGreenValue(color);
	unsigned char blue = GetBlueValue(color);
	unsigned char alpha = 255;

	switch(type)
	{
	case 5121: // unsigned char mode
		{
			fwrite(&red, sizeof(unsigned char), 1, file);
			fwrite(&green, sizeof(unsigned char), 1, file);
			fwrite(&blue, sizeof(unsigned char), 1, file);
			if(bAlpha)
				fwrite(&alpha, sizeof(unsigned char), 1, file);
		}
		break;
	case 5126: // float mode
		{
			float fRed = red / 255.0f;
			float fGreen = green / 255.0f;
			float fBlue = blue / 255.0f;
			fwrite(&fRed, sizeof(float), 1, file);
			fwrite(&fGreen, sizeof(float), 1, file);
			fwrite(&fBlue, sizeof(float), 1, file);
			if(bAlpha)
			{
				float fAlpha = 1.0f;
				fwrite(&fAlpha, sizeof(float), 1, file);
			}
		}
		break;
	default: // TODO(khj 20170324) : NYI the other color type
		break;
	}
}

bool F4DWriter::writeIndexFile()
{
	_finddata_t fd;
	long long handle;
    int result = 1;
	std::string structureJtFilter = folder + std::string("/*.*");
	handle = _findfirst(structureJtFilter.c_str(), &fd);

	if(handle == -1)
	{
		return false;
	}

	std::vector<std::string> convertedDataFolders;
	while(result != -1)
	{
		if((fd.attrib & _A_SUBDIR) == _A_SUBDIR)
		{
			if(std::string(fd.name) != "." && std::string(fd.name) != "..")
				convertedDataFolders.push_back(std::string(fd.name));
		}
		result = _findnext(handle, &fd);
	}

	_findclose(handle);

	if(convertedDataFolders.size() == 0)
		return false;

	std::string targetFilePath = folder + "/objectIndexFile.ihe";
	FILE* f;
	f = fopen(targetFilePath.c_str(), "wb");
	
	unsigned int dataFolderCount = (unsigned int)convertedDataFolders.size();
	fwrite(&dataFolderCount, sizeof(unsigned int), 1, f);

	std::string eachDataHeader;
	char version[6];	
	int guidLength;
	char guid[256];
	memset(guid, 0x00, 256);
	double longitude, latitude;
	float altitude;
	float minX, minY, minZ, maxX, maxY, maxZ;
	unsigned int dataFolderNameLength;
	for(size_t i = 0; i < dataFolderCount; i++)
	{
		eachDataHeader = folder + "/" + convertedDataFolders[i] + "/HeaderAsimetric.hed";
		FILE* header = NULL;
		header = fopen(eachDataHeader.c_str(), "rb");

		if (header == NULL)
			continue;

		// version
		memset(version, 0x00, 6);
		fread(version, sizeof(char), 5, header);
		// guid length
		fread(&guidLength, sizeof(int), 1, header);
		// guid
		memset(guid, 0x00, 256);
		fread(guid, sizeof(char), guidLength, header);
		// representative longitude, latitude, altitude
		fread(&longitude, sizeof(double), 1, header);
		fread(&latitude, sizeof(double), 1, header);
		fread(&altitude, sizeof(float), 1, header);
		// bounding box
		fread(&minX, sizeof(float), 1, header); fread(&minY, sizeof(float), 1, header); fread(&minZ, sizeof(float), 1, header);
		fread(&maxX, sizeof(float), 1, header); fread(&maxY, sizeof(float), 1, header); fread(&maxZ, sizeof(float), 1, header);

		fclose(header);

		dataFolderNameLength = (unsigned int)convertedDataFolders[i].length();
		fwrite(&dataFolderNameLength, sizeof(unsigned int), 1, f);
		fwrite(convertedDataFolders[i].c_str(), sizeof(char), dataFolderNameLength, f);

		fwrite(&longitude, sizeof(double), 1, f);
		fwrite(&latitude, sizeof(double), 1, f);
		fwrite(&altitude, sizeof(float), 1, f);

		fwrite(&minX, sizeof(float), 1, f); fwrite(&minY, sizeof(float), 1, f); fwrite(&minZ, sizeof(float), 1, f);
		fwrite(&maxX, sizeof(float), 1, f); fwrite(&maxY, sizeof(float), 1, f); fwrite(&maxZ, sizeof(float), 1, f);
	}


	fclose(f);

	return true;
}

void F4DWriter::writeTextures(std::string imagePath)
{
	std::map<std::string, unsigned char*>::iterator itr = processor->getResizedTextures().begin();
	std::string fileName, fileExt, targetFullPath;
	int bpp = 4, width, height;
	for (; itr != processor->getResizedTextures().end(); itr++)
	{
		fileName = itr->first;

		std::string::size_type dotPosition = fileName.rfind(".");
		if (dotPosition == std::string::npos)
			continue;

		fileExt = fileName.substr(dotPosition + 1, fileName.length() - dotPosition - 1);
		std::transform(fileExt.begin(), fileExt.end(), fileExt.begin(), towlower);

		targetFullPath = imagePath + "/" + fileName;
		width = processor->getAllTextureWidths()[fileName];
		height = processor->getAllTextureHeights()[fileName];
		unsigned char* texture = itr->second;

		if (fileExt.compare("jpg") == 0 || fileExt.compare("jpeg") == 0 || fileExt.compare("jpe") == 0)
		{
			stbi_write_jpg(targetFullPath.c_str(), width, height, bpp, texture, 0);
		}
		else if (fileExt.compare("png") == 0)
		{
			stbi_write_png(targetFullPath.c_str(), width, height, bpp, texture, 0);
		}
		else if (fileExt.compare("bmp"))
		{
			stbi_write_tga(targetFullPath.c_str(), width, height, bpp, texture);
		}
		else if (fileExt.compare("tga"))
		{
			stbi_write_tga(targetFullPath.c_str(), width, height, bpp, texture);
		}
		else
			continue;
	}
}

void F4DWriter::writeNetSurfaceMesh(gaia3d::TrianglePolyhedron* mesh, FILE* f)
{
	// bounding box
	float min_x = float(mesh->getBoundingBox().minX);
	float min_y = float(mesh->getBoundingBox().minY);
	float min_z = float(mesh->getBoundingBox().minZ);

	float max_x = float(mesh->getBoundingBox().maxX);
	float max_y = float(mesh->getBoundingBox().maxY);
	float max_z = float(mesh->getBoundingBox().maxZ);

	fwrite(&min_x, sizeof(float), 1, f);
	fwrite(&min_y, sizeof(float), 1, f);
	fwrite(&min_z, sizeof(float), 1, f);

	fwrite(&max_x, sizeof(float), 1, f);
	fwrite(&max_y, sizeof(float), 1, f);
	fwrite(&max_z, sizeof(float), 1, f);

	// vertex count
	unsigned int vertexCount = 0;
	size_t surfaceCount = mesh->getSurfaces().size();
	for (size_t i = 0; i < surfaceCount; i++)
	{
		gaia3d::Surface* surface = mesh->getSurfaces()[i];
		size_t triangleCount = surface->getTriangles().size();
		vertexCount += (unsigned int)(triangleCount * 3);
	}
	fwrite(&vertexCount, sizeof(unsigned int), 1, f);

	// vertex position
	float x, y, z;
	for (size_t i = 0; i < surfaceCount; i++)
	{
		gaia3d::Surface* surface = mesh->getSurfaces()[i];
		size_t triangleCount = surface->getTriangles().size();
		for (size_t j = 0; j < triangleCount; j++)
		{
			gaia3d::Triangle* triangle = surface->getTriangles()[j];
			for (int k = 0; k < 3; k++)
			{
				x = (float)triangle->getVertices()[k]->position.x;
				y = (float)triangle->getVertices()[k]->position.y;
				z = (float)triangle->getVertices()[k]->position.z;
				fwrite(&x, sizeof(float), 1, f);
				fwrite(&y, sizeof(float), 1, f);
				fwrite(&z, sizeof(float), 1, f);
			}
		}
	}

	// vertex normal
	bool bNormal = mesh->doesThisHaveNormals();
	fwrite(&bNormal, sizeof(bool), 1, f);
	if (bNormal)
	{
		fwrite(&vertexCount, sizeof(unsigned int), 1, f);
		char nx, ny, nz;
		int tnx, tny, tnz;
		for (size_t i = 0; i < surfaceCount; i++)
		{
			gaia3d::Surface* surface = mesh->getSurfaces()[i];
			size_t triangleCount = surface->getTriangles().size();
			for (size_t j = 0; j < triangleCount; j++)
			{
				gaia3d::Triangle* triangle = surface->getTriangles()[j];
				for (int k = 0; k < 3; k++)
				{
					tnx = (int)(triangle->getVertices()[k]->normal.x * 127); if (tnx > 127) tnx = 127; if (tnx < -128) tnx = -128;
					tny = (int)(triangle->getVertices()[k]->normal.y * 127); if (tny > 127) tny = 127; if (tny < -128) tny = -128;
					tnz = (int)(triangle->getVertices()[k]->normal.z * 127); if (tnz > 127) tnz = 127; if (tnz < -128) tnz = -128;
					nx = (char)tnx; ny = (char)tny; nz = (char)tnz;
					fwrite(&nx, sizeof(char), 1, f);
					fwrite(&ny, sizeof(char), 1, f);
					fwrite(&nz, sizeof(char), 1, f);
				}
			}
		}
	}

	// vertex color
	bool bColor = mesh->getColorMode() == gaia3d::ColorsOnVertices ? true : false;
	fwrite(&bColor, sizeof(bool), 1, f);
	if (bColor)
	{
		fwrite(&vertexCount, sizeof(unsigned int), 1, f);
		unsigned char r, g, b, a = 255;
		for (size_t i = 0; i < surfaceCount; i++)
		{
			gaia3d::Surface* surface = mesh->getSurfaces()[i];
			size_t triangleCount = surface->getTriangles().size();
			for (size_t j = 0; j < triangleCount; j++)
			{
				gaia3d::Triangle* triangle = surface->getTriangles()[j];
				for (int k = 0; k < 3; k++)
				{
					r = GetRedValue(triangle->getVertices()[k]->color);
					g = GetGreenValue(triangle->getVertices()[k]->color);
					b = GetBlueValue(triangle->getVertices()[k]->color);
					fwrite(&r, sizeof(unsigned char), 1, f);
					fwrite(&g, sizeof(unsigned char), 1, f);
					fwrite(&b, sizeof(unsigned char), 1, f);
					fwrite(&a, sizeof(unsigned char), 1, f);
				}
			}
		}
	}

	// vertex texture coordinates
	bool bTextureCoordinate = mesh->doesThisHaveTextureCoordinates();
	fwrite(&bTextureCoordinate, sizeof(bool), 1, f);
	if (bTextureCoordinate)
	{
		unsigned short type = 5126; // float.***
		fwrite(&type, sizeof(unsigned short), 1, f);

		fwrite(&vertexCount, sizeof(unsigned int), 1, f);
		float u, v;
		for (size_t i = 0; i < surfaceCount; i++)
		{
			gaia3d::Surface* surface = mesh->getSurfaces()[i];
			size_t triangleCount = surface->getTriangles().size();
			for (size_t j = 0; j < triangleCount; j++)
			{
				gaia3d::Triangle* triangle = surface->getTriangles()[j];
				for (int k = 0; k < 3; k++)
				{
					u = (float)triangle->getVertices()[k]->textureCoordinate[0];
					v = (float)triangle->getVertices()[k]->textureCoordinate[1];
					fwrite(&u, sizeof(float), 1, f);
					fwrite(&v, sizeof(float), 1, f);
				}
			}
		}
	}
}

void F4DWriter::writeNetSurfaceTextures(std::string resultPath)
{
	std::map<unsigned char, unsigned char*>::iterator iterTexture = processor->getNetSurfaceTextures().begin();
	std::string filePath;
	int bpp = 4;
	for (; iterTexture != processor->getNetSurfaceTextures().end(); iterTexture++)
	{
		unsigned char lod = iterTexture->first;
		unsigned char* texture = iterTexture->second;
		unsigned int width = processor->getNetSurfaceTextureWidth()[lod];
		unsigned int height = processor->getNetSurfaceTextureHeight()[lod];
		filePath = resultPath + "/mosaicTextureLod" + std::to_string(lod) + ".jpg";

		stbi_write_jpg(filePath.c_str(), width, height, bpp, texture, 0);
	}
}
