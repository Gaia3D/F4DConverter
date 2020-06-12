#include "stdafx.h"

#ifdef AVEVAREVIEWFORMAT

#include "AvevaRevReader.h"

#include "../util/utility.h"

#include "../geometry/TrianglePolyhedron.h"

#include "../LogWriter.h"

// surface definition in rev files
class RevSubSurface
{
public:
	std::vector<gaia3d::Vertex*> vertices;
};

class RevSurface
{
public:

	~RevSurface()
	{
		for (size_t i = 0; i < subSurfaces.size(); i++)
		{
			delete subSurfaces[i];
			subSurfaces[i] = NULL;
		}

		subSurfaces.clear();
	}

	std::vector<RevSubSurface*> subSurfaces;
};

// PRIM definition in rev files
class RevPrim
{
public:
	RevPrim()
	{
		primType = NONE;
	}

	~RevPrim()
	{
		for (size_t i = 0; i < surfaces.size(); i++)
		{
			delete surfaces[i];
			surfaces[i] = NULL;
		}

		surfaces.clear();
	}

	enum PRIM_TYPE {NONE, UNKNOWN, BROKEN, TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7, TYPE8, TYPE9, TYPE10, TYPE11};

public:
	PRIM_TYPE primType; // one of 1, 2, 4, 7, 8, 10, 11. still can't understand what this means
	gaia3d::BoundingBox bbox;
	std::vector<RevSurface*> surfaces;
	std::string unparsedInfo;
};

// node definition in rev files
class RevNode
{
public:
	RevNode()
	{
		parent = NULL;
	}

	~RevNode()
	{
		clearChildren();

		for (size_t i = 0; i < prims.size(); i++)
		{
			delete prims[i];
			prims[i] = NULL;
		}

		prims.clear();
	}

	void clearChildren()
	{
		for (size_t i = 0; i < children.size(); i++)
		{
			delete children[i];
			children[i] = NULL;
		}

		children.clear();
	}

public:
	std::string id;
	std::string bboxCenter;
	std::string obst;
	std::string matrix;
	std::string bbox;
	std::vector<RevPrim*> prims;

	RevNode* parent;
	std::vector<RevNode*> children;
};

bool createNode(FILE* file, RevNode*& rootNode, RevNode*& currentNode, std::vector<RevNode*>& createdRootNodes);
bool readPrimInfo(FILE* file, RevNode* node);
bool readObstInfo(FILE* file, RevNode* node);
void extractGeometryInformation(RevNode* node, std::vector<gaia3d::TrianglePolyhedron*>& container);
void tokenizeFloatingNumbers(char buffer[], std::vector<double>& receiver);

#define LineLengthMax 1024
size_t readLineCount = 0;
unsigned char readingMode = 0; // 0 : none, 1: creating node, 2 : reading prim, 3 : reading obst
bool readALine(char buffer[], FILE*&file)
{
	memset(buffer, 0x00, LineLengthMax);
	fgets(buffer, LineLengthMax, file);
	readLineCount++;

	if (readingMode != 0)
	{
		if (std::string(buffer).find(std::string("CNTB")) != std::string::npos ||
			std::string(buffer).find(std::string("CNTE")) != std::string::npos ||
			std::string(buffer).find(std::string("PRIM")) != std::string::npos ||
			std::string(buffer).find(std::string("OBST")) != std::string::npos)
		{
			printf("[ERROR]Parsing Status Broken. line number : %zd\n", readLineCount);
			// new log
			char log[1024];
			memset(log, 0x00, 1024);
			sprintf(log, "parsing status broken. line number : %zd\n", readLineCount);
			LogWriter::getLogWriter()->changeCurrentConversionJobStatus(LogWriter::failure);
			LogWriter::getLogWriter()->addDescriptionToCurrentConversionJobLog(std::string("(AvevaRevReader.cpp)readALine : ") + std::string(log));
			return false;
		}
	}

	return true;
}

AvevaRevReader::AvevaRevReader()
{
}


AvevaRevReader::~AvevaRevReader()
{
	clear();
}

bool AvevaRevReader::readRawDataFile(std::string& filePath)
{
	// tag definition
	std::string cntbTag("CNTB");
	std::string cnteTag("CNTE");
	std::string primTag("PRIM");
	std::string obstTag("OBST");

	// file open
	FILE* file = NULL;
	file = fopen(filePath.c_str(), "rt");
	if (file == NULL)
	{
		printf("[ERROR]Unable to open the file : %s\n", filePath.c_str());
		// new log
		LogWriter::getLogWriter()->changeCurrentConversionJobStatus(LogWriter::failure);
		LogWriter::getLogWriter()->addDescriptionToCurrentConversionJobLog(std::string("AvevaRevReader::readRawDataFile : unable to open file"));
		return false;
	}

	char line[LineLengthMax];
	readLineCount = 0;

	size_t beginTagCount = 0;
	size_t endTagCount = 0;
	
	// codes for skipping header parts
	std::vector<RevNode*> createdRootNodes;
	RevNode* rootNode = NULL;
	RevNode* currentNode = NULL;
	while (feof(file) == 0)
	{
		if (!readALine(line, file))
			return false;
		
		std::string aLine = std::string(line);

		if (aLine.find(cntbTag) != std::string::npos)  // enter into a new node
		{
			beginTagCount++;
			if (!createNode(file, rootNode, currentNode, createdRootNodes))
			{
				for (size_t i = 0; i < createdRootNodes.size(); i++)
					delete createdRootNodes[i];

				if (currentNode != NULL)
					delete currentNode;

				fclose(file);

				return false;
			}

			continue;
		}

		if (aLine.find(cnteTag) != std::string::npos)  // go out of a node
		{
			endTagCount++;
			if (currentNode->parent == NULL)
				rootNode = currentNode = NULL;
			else
				currentNode = currentNode->parent;

			continue;
		}

		if (aLine.find(primTag) != std::string::npos)  // meet a geometry info
		{
			if (!readPrimInfo(file, currentNode))
			{
				for (size_t i = 0; i < createdRootNodes.size(); i++)
					delete createdRootNodes[i];

				if (currentNode != NULL)
					delete currentNode;

				fclose(file);

				return false;
			}

			continue;
		}

		if (aLine.find(obstTag) != std::string::npos)  // meet a unknown(right now, but maybe geometry) tag
		{
			if (!readObstInfo(file, currentNode))
			{
				for (size_t i = 0; i < createdRootNodes.size(); i++)
					delete createdRootNodes[i];

				if (currentNode != NULL)
					delete currentNode;

				fclose(file);

				return false;
			}

			continue;
		}
	}
	fclose(file);

	for (size_t i = 0; i < createdRootNodes.size(); i++)
	{
		extractGeometryInformation(createdRootNodes[i], container);

		delete createdRootNodes[i];
		createdRootNodes[i] = NULL;
	}
	createdRootNodes.clear();

	return true;
}

void AvevaRevReader::clear()
{
	container.clear();

	textureContainer.clear();
}

bool createNode(FILE* file, RevNode*& rootNode, RevNode*& currentNode, std::vector<RevNode*>& createdRootNodes)
{
	readingMode = 1;

	char line[LineLengthMax];
	RevNode* parentNode = NULL;

	if (rootNode == NULL)
	{
		if (currentNode != NULL)
		{
			printf("[ERROR]Rev node heiararchy broken!!\n");
			for (size_t i = 0; i < createdRootNodes.size(); i++)
				delete createdRootNodes[i];
			delete currentNode;

			readingMode = 0;

			// new log
			LogWriter::getLogWriter()->changeCurrentConversionJobStatus(LogWriter::failure);
			LogWriter::getLogWriter()->addDescriptionToCurrentConversionJobLog(std::string("(AvevaRevReader.cpp)createNode : Rev node heiararchy broken"));

			return false;
		}

		rootNode = currentNode = new RevNode;
		createdRootNodes.push_back(rootNode);
	}
	else
	{
		if (currentNode == NULL)
		{
			printf("[ERROR]Rev node heiararchy broken!!\n");
			for (size_t i = 0; i < createdRootNodes.size(); i++)
				delete createdRootNodes[i];

			readingMode = 0;

			// new log
			LogWriter::getLogWriter()->changeCurrentConversionJobStatus(LogWriter::failure);
			LogWriter::getLogWriter()->addDescriptionToCurrentConversionJobLog(std::string("(AvevaRevReader.cpp)createNode : Rev node heiararchy broken"));
			return false;
		}

		parentNode = currentNode;
		currentNode = new RevNode;
		currentNode->parent = parentNode;
		parentNode->children.push_back(currentNode);
	}

	// read 4 lines
	if (!readALine(line, file))  // dummy 2 numbers
		return false;

	if (!readALine(line, file)) // id of this node
		return false; 
	//currentNode->id = gaia3d::StringUtility::convertMultibyteToUtf8(std::string(line));
	currentNode->id =std::string(strtok(line, "\n"));

	if (!readALine(line, file))  // geometric center of this node
		return false;
	currentNode->bboxCenter = std::string(line);

	if (!readALine(line, file)) // dummy 1 number
		return false;

	readingMode = 0;
	return true;
}

bool readPrimInfo(FILE* file, RevNode* node)
{
	readingMode = 2;

	char line[LineLengthMax];

	if (!readALine(line, file)) // dummy 2 numbers
		return false;

	if (!readALine(line, file)) // prim type
		return false;

	char* token = strtok(line, " \t\n");
	if (token == NULL)
	{
		readingMode = 0;
		// new log
		LogWriter::getLogWriter()->changeCurrentConversionJobStatus(LogWriter::failure);
		LogWriter::getLogWriter()->addDescriptionToCurrentConversionJobLog(std::string("(AvevaRevReader.cpp)ReadPrimInfo : no prim type"));
		return false;
	}

	std::string aPrimType = std::string(token);
	RevPrim::PRIM_TYPE primType = RevPrim::PRIM_TYPE::UNKNOWN;
	if (aPrimType.compare(std::string("1")) == 0)
		primType = RevPrim::PRIM_TYPE::TYPE1;

	else if (aPrimType.compare(std::string("2")) == 0)
		primType = RevPrim::PRIM_TYPE::TYPE2;

	else if (aPrimType.compare(std::string("3")) == 0)
		primType = RevPrim::PRIM_TYPE::TYPE3;

	else if (aPrimType.compare(std::string("4")) == 0)
		primType = RevPrim::PRIM_TYPE::TYPE4;

	else if (aPrimType.compare(std::string("5")) == 0)
		primType = RevPrim::PRIM_TYPE::TYPE5;

	else if (aPrimType.compare(std::string("6")) == 0)
		primType = RevPrim::PRIM_TYPE::TYPE6;

	else if (aPrimType.compare(std::string("7")) == 0)
		primType = RevPrim::PRIM_TYPE::TYPE7;

	else if (aPrimType.compare(std::string("8")) == 0)
		primType = RevPrim::PRIM_TYPE::TYPE8;

	else if (aPrimType.compare(std::string("9")) == 0)
		primType = RevPrim::PRIM_TYPE::TYPE9;

	else if (aPrimType.compare(std::string("10")) == 0)
		primType = RevPrim::PRIM_TYPE::TYPE10;

	else if (aPrimType.compare(std::string("11")) == 0)
		primType = RevPrim::PRIM_TYPE::TYPE11;

	else
	{
		printf("[ERROR]Unknown Prim Type : %s, line number : %zd\n", aPrimType.c_str(), readLineCount);
		return true;
	}

	RevPrim* prim = new RevPrim;
	prim->primType = primType;
	node->prims.push_back(prim);

	std::vector<double> tokenizedNumbers;

	// read 3 lines for transform matrix
	double mat[4][4]; // [col][row]

	if (!readALine(line, file)) // 1st row of matrix
		return false;

	tokenizeFloatingNumbers(line, tokenizedNumbers);
	for (size_t i = 0; i < 4; i++)
		mat[i][0] = tokenizedNumbers[i];
	tokenizedNumbers.clear();

	if (!readALine(line, file)) // 2nd row of matrix
		return false;

	tokenizeFloatingNumbers(line, tokenizedNumbers);
	for (size_t i = 0; i < 4; i++)
		mat[i][1] = tokenizedNumbers[i];
	tokenizedNumbers.clear();

	if (!readALine(line, file)) // 3rd row of matrix
		return false;

	tokenizeFloatingNumbers(line, tokenizedNumbers);
	for (size_t i = 0; i < 4; i++)
		mat[i][2] = tokenizedNumbers[i];
	tokenizedNumbers.clear();

	gaia3d::Matrix4 transMat;
	transMat.set(mat[0][0], mat[0][1], mat[0][2], 0.0,
				mat[1][0], mat[1][1], mat[1][2], 0.0, 
				mat[2][0], mat[2][1], mat[2][2], 0.0, 
				mat[3][0], mat[3][1], mat[3][2], 1.0);

	// read bounding box
	if(!readALine(line, file)) // min Bounding Point
		return false;

	tokenizeFloatingNumbers(line, tokenizedNumbers);
	gaia3d::Point3D minBoundingPoint;
	minBoundingPoint.set(tokenizedNumbers[0], tokenizedNumbers[1], tokenizedNumbers[2]);
	tokenizedNumbers.clear();
	minBoundingPoint = transMat * minBoundingPoint;

	// read maximum bounding point
	if (!readALine(line, file)) // max Bounding Point
		return false;
	tokenizeFloatingNumbers(line, tokenizedNumbers);
	gaia3d::Point3D maxBoundingPoint;
	maxBoundingPoint.set(tokenizedNumbers[0], tokenizedNumbers[1], tokenizedNumbers[2]);
	tokenizedNumbers.clear();
	maxBoundingPoint = transMat * maxBoundingPoint;

	prim->bbox.addPoint(minBoundingPoint.x, minBoundingPoint.y, minBoundingPoint.z);
	prim->bbox.addPoint(maxBoundingPoint.x, maxBoundingPoint.y, maxBoundingPoint.z);

	switch (prim->primType)
	{
	case RevPrim::PRIM_TYPE::UNKNOWN:
	{
		printf("[ERROR]Unknwon Prim Type\n");
		_ASSERT(false);
	}
	break;
	case RevPrim::PRIM_TYPE::TYPE1:
	{
		if (!readALine(line, file))
			return false; // read dummy line
		if (!readALine(line, file))
			return false; // read dummy line
	}
	break;
	case RevPrim::PRIM_TYPE::TYPE2:
	{
		if (!readALine(line, file))
			return false; // read dummy line
	}
	break;
	case RevPrim::PRIM_TYPE::TYPE3:
	{
		if (!readALine(line, file))
			return false; // read dummy line
	}
	break;
	case RevPrim::PRIM_TYPE::TYPE4:
	{
		if (!readALine(line, file))
			return false; // read dummy line
	}
	break;
	case RevPrim::PRIM_TYPE::TYPE5:
	{
		if (!readALine(line, file))
			return false; // read dummy line
	}
	break;
	case RevPrim::PRIM_TYPE::TYPE6:
	{
		if (!readALine(line, file))
			return false; // read dummy line
	}
	break;
	case RevPrim::PRIM_TYPE::TYPE7:
	{
		if (!readALine(line, file))
			return false; // read dummy line
		if (!readALine(line, file))
			return false; // read dummy line
	}
	break;
	case RevPrim::PRIM_TYPE::TYPE8:
	{
		if (!readALine(line, file))
			return false; // read dummy line
	}
	break;
	case RevPrim::PRIM_TYPE::TYPE9:
	{
		if (!readALine(line, file))
			return false; // read dummy line
	}
	break;
	case RevPrim::PRIM_TYPE::TYPE10:
	{
		if (!readALine(line, file))
			return false; // read dummy line
	}
	break;
	case RevPrim::PRIM_TYPE::TYPE11:
	{
		// read surface count
		if (!readALine(line, file))
			return false;// surface count

		std::string aSurfaceCount = std::string(line);
		int surfaceCount = 0;
		try
		{
			surfaceCount = std::stoi(aSurfaceCount);

			if (surfaceCount < 1)
			{
				printf("[ERROR]Surface Count Not Positive\n");
				prim->primType = RevPrim::PRIM_TYPE::BROKEN;
				readingMode = 0;
				return true;
			}
		}
		catch (const std::invalid_argument& error)
		{
			std::string errorMessage = error.what();
			printf("[ERROR]Invalid Surface Count : %s.\n", errorMessage.c_str());
			readingMode = 0;
			return true;
		}
		catch (const std::out_of_range& error)
		{
			std::string errorMessage = error.what();
			printf("[ERROR]Invalid Surface Count : %s.\n", errorMessage.c_str());
			readingMode = 0;
			return true;
		}

		// read surfaces
		RevSurface* surface = NULL;
		int subSurfaceCount = -1;
		for (int i = 0; i < surfaceCount; i++)
		{
			surface = new RevSurface;
			prim->surfaces.push_back(surface);

			if (!readALine(line, file))
				return false;// sub-surface count

			try
			{
				subSurfaceCount = std::stoi(std::string(line));

				if (subSurfaceCount < 1)
				{
					printf("[ERROR]Sub-surface Count Not Positive\n");
					prim->primType = RevPrim::PRIM_TYPE::BROKEN;
					readingMode = 0;
					return true;
				}
			}
			catch (const std::invalid_argument& error)
			{
				std::string errorMessage = error.what();
				printf("[ERROR]Invalid Sub-surface Count : %s.\n", errorMessage.c_str());
				readingMode = 0;
				return true;
			}
			catch (const std::out_of_range& error)
			{
				std::string errorMessage = error.what();
				printf("[ERROR]Invalid Sub-surface Count : %s.\n", errorMessage.c_str());
				readingMode = 0;
				return true;
			}

			RevSubSurface* subSurface = NULL;
			int pointCount = -1;
			for (int j = 0; j < subSurfaceCount; j++)
			{
				if (!readALine(line, file))
					return false;// point count

				try
				{
					pointCount = std::stoi(std::string(line));

					if (pointCount < 1)
					{
						printf("[ERROR]Point Count Not Positive\n");
						prim->primType = RevPrim::PRIM_TYPE::BROKEN;
						readingMode = 0;
						return true;
					}

					if (pointCount < 3)
					{
						printf("[WARNING]Surface of point count less than 3\n");
						for (int j = 0; j < pointCount; j++)
						{
							if (!readALine(line, file))
								return false;// abandoned point geometry
							if (!readALine(line, file))
								return false;// abandoned point normal
						}
						continue;
					}
				}
				catch (const std::invalid_argument& error)
				{
					std::string errorMessage = error.what();
					printf("[ERROR]Invalid Point Count : %s.\n", errorMessage.c_str());
					readingMode = 0;
					return true;
				}
				catch (const std::out_of_range& error)
				{
					std::string errorMessage = error.what();
					printf("[ERROR]Invalid Point Count : %s.\n", errorMessage.c_str());
					readingMode = 0;
					return true;
				}

				subSurface = new RevSubSurface;
				surface->subSurfaces.push_back(subSurface);
				gaia3d::Vertex* vertex = NULL;
				for (int j = 0; j < pointCount; j++)
				{
					vertex = new gaia3d::Vertex;
					subSurface->vertices.push_back(vertex);

					if (!readALine(line, file))
						return false;// point geometry
					tokenizeFloatingNumbers(line, tokenizedNumbers);
					vertex->position.set(tokenizedNumbers[0], tokenizedNumbers[1], tokenizedNumbers[2]);
					tokenizedNumbers.clear();
					vertex->position = transMat * (vertex->position);

					if (!readALine(line, file))
						return false;// point normal
					tokenizeFloatingNumbers(line, tokenizedNumbers);
					vertex->normal.set(tokenizedNumbers[0], tokenizedNumbers[1], tokenizedNumbers[2]);
					tokenizedNumbers.clear();
					transMat.applyOnlyRotationOnPoint(vertex->normal);
					vertex->normal.normalize();

					
				}
			}
		}
	}
	break;
	}
	
	readingMode = 0;

	return true;
}

bool readObstInfo(FILE* file, RevNode* node)
{
	readingMode = 3;

	char line[LineLengthMax];

	if (!readALine(line, file))
		return false;// dummy 2 numbers

	if (!readALine(line, file))
		return false;// obst type

	// read 3 lines for transform matrix
	if (!readALine(line, file))
		return false;// 1st row of matrix
	if (!readALine(line, file))
		return false;// 2nd row of matrix
	if (!readALine(line, file))
		return false;// 3rd row of matrix

	// read minimum bounding point
	if (!readALine(line, file))
		return false;// min Bounding Point

	// read maximum bounding point
	if (!readALine(line, file))
		return false;// max Bounding Point

	if (!readALine(line, file))
		return false;// read dummy line
	if (!readALine(line, file))
		return false;// read dummy line

	readingMode = 0;
}

void extractGeometryInformation(RevNode* node, std::vector<gaia3d::TrianglePolyhedron*>& container)
{
	if (!node->prims.empty())
	{
		RevPrim* prim;
		gaia3d::TrianglePolyhedron* polyhedron;
		for (size_t i = 0; i < node->prims.size(); i++)
		{
			if (node->prims[i]->primType != RevPrim::PRIM_TYPE::TYPE11)
				continue;

			prim = node->prims[i];
			polyhedron = new gaia3d::TrianglePolyhedron;
			
			RevSurface* surface;
			for (size_t j = 0; j < prim->surfaces.size(); j++)
			{
				surface = prim->surfaces[j];

				size_t subSurfaceCount = surface->subSurfaces.size();
				gaia3d::Surface* f4dSurface;
				if (subSurfaceCount > 1)  // case of polygon with inner holes
				{
					// sub surface 0 : exterior
					// sub surface 1 ~ n : inner holes

					std::vector<size_t> pointCountOfAllRings;
					double** xss = new double*[subSurfaceCount];
					memset(xss, 0x00, sizeof(double*)*subSurfaceCount);
					double** yss = new double*[subSurfaceCount];
					memset(yss, 0x00, sizeof(double*)*subSurfaceCount);
					double** zss = new double*[subSurfaceCount];
					memset(zss, 0x00, sizeof(double*)*subSurfaceCount);

					RevSubSurface* subSurface;
					size_t vertexCount, totalVertexCount = 0;
					for (size_t k = 0; k < subSurfaceCount; k++)
					{
						subSurface = surface->subSurfaces[k];
						vertexCount = subSurface->vertices.size();
						pointCountOfAllRings.push_back(vertexCount);
						totalVertexCount += vertexCount;

						xss[k] = new double[vertexCount];
						memset(xss[k], 0x00, sizeof(double)*vertexCount);
						yss[k] = new double[vertexCount];
						memset(yss[k], 0x00, sizeof(double)*vertexCount);
						zss[k] = new double[vertexCount];
						memset(zss[k], 0x00, sizeof(double)*vertexCount);

						for (size_t m = 0; m < vertexCount; m++)
						{
							xss[k][m] = subSurface->vertices[m]->position.x;
							yss[k][m] = subSurface->vertices[m]->position.y;
							zss[k][m] = subSurface->vertices[m]->position.z;
						}
					}

					std::vector<std::pair<size_t, size_t>> earCutResult;
					gaia3d::GeometryUtility::earCut(xss, yss, zss, pointCountOfAllRings, earCutResult);

					if (earCutResult.empty())
					{
						for (size_t k = 0; k < subSurfaceCount; k++)
						{
							delete[] xss[k];
							delete[] yss[k];
							delete[] zss[k];
						}
						delete[] xss;
						delete[] yss;
						delete[] zss;

						for (size_t k = 0; k < subSurfaceCount; k++)
						{
							subSurface = surface->subSurfaces[k];
							vertexCount = subSurface->vertices.size();

							for (size_t m = 0; m < vertexCount; m++)
								delete subSurface->vertices[m];
						}

						printf("[WARNING] Ear cutting failed. : %s\n", node->id.c_str());

						continue;
					}

					size_t indexOffset = polyhedron->getVertices().size();
					for (size_t k = 0; k < subSurfaceCount; k++)
					{
						subSurface = surface->subSurfaces[k];
						vertexCount = subSurface->vertices.size();

						for (size_t m = 0; m < vertexCount; m++)
							polyhedron->getVertices().push_back(subSurface->vertices[m]);
					}

					double* xs = new double[totalVertexCount];
					memset(xs, 0x00, sizeof(double)*totalVertexCount);
					double* ys = new double[totalVertexCount];
					memset(ys, 0x00, sizeof(double)*totalVertexCount);
					double* zs = new double[totalVertexCount];
					memset(zs, 0x00, sizeof(double)*totalVertexCount);

					size_t arrayPosOffset = 0;
					for (size_t k = 0; k < subSurfaceCount; k++)
					{
						if (k != 0)
							arrayPosOffset += pointCountOfAllRings[k-1];

						memcpy(xs + arrayPosOffset, xss[k], sizeof(double) * pointCountOfAllRings[k]);
						memcpy(ys + arrayPosOffset, yss[k], sizeof(double) * pointCountOfAllRings[k]);
						memcpy(zs + arrayPosOffset, zss[k], sizeof(double) * pointCountOfAllRings[k]);
					}

					for (size_t k = 0; k < subSurfaceCount; k++)
					{
						delete[] xss[k];
						delete[] yss[k];
						delete[] zss[k];
					}
					delete[] xss;
					delete[] yss;
					delete[] zss;

					std::vector<size_t> polygonIndices;
					size_t polygonPointIndexOffset;
					for (size_t k = 0; k < earCutResult.size(); k++)
					{
						polygonPointIndexOffset = 0;
						for (size_t m = 0; m < earCutResult[k].first; m++)
							polygonPointIndexOffset += pointCountOfAllRings[m];

						polygonIndices.push_back(polygonPointIndexOffset + earCutResult[k].second);
					}

					std::vector<size_t> triangleIndices;
					gaia3d::GeometryUtility::tessellate(xs, ys, zs, totalVertexCount, polygonIndices, triangleIndices);
					delete[] xs;
					delete[] ys;
					delete[] zs;

					if (triangleIndices.empty())
						continue;

					f4dSurface = new gaia3d::Surface;
					gaia3d::Triangle* triangle;
					for (size_t m = 0; m < triangleIndices.size() / 3; m++)
					{
						triangle = new gaia3d::Triangle;

						triangle->setVertices(  polyhedron->getVertices()[indexOffset + triangleIndices[3 * m]],
												polyhedron->getVertices()[indexOffset + triangleIndices[3 * m + 1]],
												polyhedron->getVertices()[indexOffset + triangleIndices[3 * m + 2]])  ;
						triangle->setVertexIndices( indexOffset + triangleIndices[3 * m],
													indexOffset + triangleIndices[3 * m + 1],
													indexOffset + triangleIndices[3 * m + 2] );

						f4dSurface->getTriangles().push_back(triangle);
					}

					polyhedron->getSurfaces().push_back(f4dSurface);
				}
				else if(subSurfaceCount == 1)// case of polygon having only exterior closed linestring
				{
					RevSubSurface* subSurface;
					size_t vertexCount;
					size_t indexOffset;

					subSurface = surface->subSurfaces[0];
					vertexCount = subSurface->vertices.size();
					indexOffset = polyhedron->getVertices().size();

					f4dSurface = new gaia3d::Surface;

					double* xs = new double[vertexCount];
					memset(xs, 0x00, sizeof(double)*vertexCount);
					double* ys = new double[vertexCount];
					memset(ys, 0x00, sizeof(double)*vertexCount);
					double* zs = new double[vertexCount];
					memset(zs, 0x00, sizeof(double)*vertexCount);
					std::vector<size_t> polygonIndices;
					for (size_t m = 0; m < vertexCount; m++)
					{
						polyhedron->getVertices().push_back(subSurface->vertices[m]);

						xs[m] = subSurface->vertices[m]->position.x;
						ys[m] = subSurface->vertices[m]->position.y;
						zs[m] = subSurface->vertices[m]->position.z;

						polygonIndices.push_back(m);
					}
					std::vector<size_t> triangleIndices;

					gaia3d::GeometryUtility::tessellate(xs, ys, zs, vertexCount, polygonIndices, triangleIndices);
					delete[] xs;
					delete[] ys;
					delete[] zs;

					gaia3d::Triangle* triangle;
					for (size_t m = 0; m < triangleIndices.size() / 3; m++)
					{
						triangle = new gaia3d::Triangle;

						triangle->setVertices(  polyhedron->getVertices()[indexOffset + triangleIndices[3 * m]],
												polyhedron->getVertices()[indexOffset + triangleIndices[3 * m + 1]],
												polyhedron->getVertices()[indexOffset + triangleIndices[3 * m + 2]]  );
						triangle->setVertexIndices( indexOffset + triangleIndices[3 * m],
													indexOffset + triangleIndices[3 * m + 1],
													indexOffset + triangleIndices[3 * m + 2] );

						f4dSurface->getTriangles().push_back(triangle);
					}

					polyhedron->getSurfaces().push_back(f4dSurface);
				}
			}

			if (polyhedron->getVertices().empty() || polyhedron->getSurfaces().empty())
			{
				delete polyhedron;
				return;
			}

			polyhedron->setId(container.size());
			polyhedron->addStringAttribute(std::string(ObjectGuid), node->id);
			polyhedron->setHasNormals(true);
			polyhedron->setColorMode(gaia3d::ColorMode::SingleColor);
			polyhedron->setSingleColor(DefaultColor);
			container.push_back(polyhedron);
		}
	}

	for (size_t i = 0; i < node->children.size(); i++)
		extractGeometryInformation(node->children[i], container);
}

void tokenizeFloatingNumbers(char buffer[], std::vector<double>& receiver)
{
	char* token = strtok(buffer, " \t\n");

	while (token != NULL)
	{
		try
		{
			double parsedValue = std::stod(std::string(token));

			receiver.push_back(parsedValue);
		}
		catch (const std::invalid_argument& error)
		{
			std::string errorMessage = error.what();
			printf("[ERROR][Invalid Floating Number] Value in REV file : %s.\n", errorMessage.c_str());
			return;
		}
		catch (const std::out_of_range& error)
		{
			std::string errorMessage = error.what();
			printf("[ERROR][Invalid Floating Number] Value in REV file : %s.\n", errorMessage.c_str());
			return;
		}

		token = strtok(NULL, " \t\n");
	}
}

#endif
