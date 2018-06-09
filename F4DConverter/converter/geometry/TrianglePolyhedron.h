#pragma once

#include <vector>
#include <map>

#include "Surface.h"
#include "Matrix4.h"
#include "ColorU4.h"

#include "BoundingBox.h"
#include "Vbo.h"

namespace gaia3d
{
	

	class TrianglePolyhedron
	{
	public:
		TrianglePolyhedron();

		virtual ~TrianglePolyhedron();

		struct REFERENCE_INFO
		{
			TrianglePolyhedron* model;
			Matrix4 mat;
			size_t modelIndex;
		};

	protected:
		std::vector<Vertex*> vertices;
		std::vector<Surface*> surfaces;

		std::map<std::string, std::string> stringAttributes;

		bool hasNormals;
		bool hasTextureCoordinates;
		ColorMode colorMode;

		ColorU4 singleColor;

		REFERENCE_INFO refInfo;

		BoundingBox bbox;
		std::vector<Vbo*> vbos;
		size_t id;

		

	public:
		std::vector<Surface*>& getSurfaces() {return surfaces;}
		std::vector<Vertex*>& getVertices() {return vertices;}
		void addStringAttribute(std::string keyString, std::string valueString);
		bool doesStringAttributeExist(std::string keyString);
		std::string getStringAttribute(std::string keyString);
		std::map<std::string, std::string>& getStringAttributes() { return stringAttributes; }

		void setHasNormals(bool bHas) {hasNormals = bHas;}
		bool doesThisHaveNormals() {return hasNormals;} 
		void setHasTextureCoordinates (bool bHas) {hasTextureCoordinates = bHas;}
		bool doesThisHaveTextureCoordinates() {return hasTextureCoordinates;}
		void setColorMode(ColorMode mode) {colorMode = mode;}
		ColorMode getColorMode() {return colorMode;}
		void setSingleColor(ColorU4 color) {singleColor = color;}
		ColorU4 getSingleColor() {return singleColor;}

		gaia3d::TrianglePolyhedron::REFERENCE_INFO& getReferenceInfo() {return refInfo;}
		void setReferenceModel(TrianglePolyhedron* model) {refInfo.model = model;}
		void setReferenceMatrix(Matrix4& matrix)
		{
			refInfo.mat.set(matrix);
		}
		void setReferenceModelIndex(size_t id) {refInfo.modelIndex = id;}

		BoundingBox& getBoundingBox() {return bbox;}
		std::vector<Vbo*>& getVbos() {return vbos;}
		void setId(size_t indexNumber) {id = indexNumber;}
		size_t getId() {return id;}

		bool doesHaveAnyExteriorSurface();
	};

	
}