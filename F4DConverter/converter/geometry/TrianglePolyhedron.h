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
	
	///< TrianglePolyhedron은 vertices와 surfaces로 이루어져 있다.
	class TrianglePolyhedron
	{
	public:
		TrianglePolyhedron();

		virtual ~TrianglePolyhedron();

		
		struct REFERENCE_INFO
		{
			///< 어떤 모델을 원본으로 가지는가
			TrianglePolyhedron* model;
			Matrix4 mat;
			size_t modelIndex;
		};

	protected:
		std::vector<Vertex*> vertices;
		std::vector<Surface*> surfaces;

		///< 추가로 설정할 속성들을 (이름, 값)의 쌍으로 저장한다. 
		std::map<std::string, std::string> stringAttributes;

		///< texture가 있으면 normal이 거의 의미가 없다.
		bool hasNormals;
		bool hasTextureCoordinates;
		ColorMode colorMode; ///< non/single/colors on vertices

		ColorU4 singleColor;

		REFERENCE_INFO refInfo;

		BoundingBox bbox;
		///< writing 직전에 vbo형태로 쓰게 된다.
		std::vector<Vbo*> vbos;
		///< 메쉬를 구분하는 unique 숫자
		size_t id;

		

	public:
		///< surface의 리스트를 가져온다
		std::vector<Surface*>& getSurfaces() {return surfaces;}
		///< vertex의 리스트를 가져온다
		std::vector<Vertex*>& getVertices() {return vertices;}
		///< trianglepolyhedron에 해당 속성과 값을 추가한다
		void addStringAttribute(std::string keyString, std::string valueString);
		///< trianglepolyhedron이 해당 속성을 가지고 있는지 확인한다
		bool doesStringAttributeExist(std::string keyString);
		///< trianglepolyhedron에서 해당 속성의 값을 가져온다
		std::string getStringAttribute(std::string keyString);
		///< trianglepolyhedron이 가지고 있는 속성값을 가져온다
		std::map<std::string, std::string>& getStringAttributes() { return stringAttributes; }
		
		///< trianglepolyhedron이 normal값을 가지고 있는지를 설정한다
		void setHasNormals(bool bHas) {hasNormals = bHas;}
		///< trianglepolyhedron이 normal값을 가지고 있는지를 확인한다
		bool doesThisHaveNormals() {return hasNormals;} 
		///< trianglepolyhedron이 texture coordinate를 가지고 있다고 설정한다
		void setHasTextureCoordinates (bool bHas) {hasTextureCoordinates = bHas;}
		///< trianglepolyhedron에 texture coordinate가 설정되어있는지의 여부를 얻는다
		bool doesThisHaveTextureCoordinates() {return hasTextureCoordinates;}
		///< trianglepolyhedron에 colormode를 설정한다
		void setColorMode(ColorMode mode) {colorMode = mode;}
		///< trianglepolyhedron에 설정된 colormode 값을 얻는다
		ColorMode getColorMode() {return colorMode;}
		///< trianglepolyhedron에 색깔을 설정한다. 
		void setSingleColor(ColorU4 color) {singleColor = color;}
		///< trianglepolyhedron에 설정된 색깔을 얻는다
		ColorU4 getSingleColor() {return singleColor;}
		///< reference model 정보를 얻는다
		gaia3d::TrianglePolyhedron::REFERENCE_INFO& getReferenceInfo() {return refInfo;}
		///< reference model을 설정한다 
		void setReferenceModel(TrianglePolyhedron* model) {refInfo.model = model;}
		///< 원래의 referenceModel를 얻으려면 이 메트릭스로 연산하면 된다
		void setReferenceMatrix(Matrix4& matrix)
		{
			refInfo.mat.set(matrix);
		}
		///< ReferenceModel의 id를 index로 정한다
		void setReferenceModelIndex(size_t id) {refInfo.modelIndex = id;}
		///< trianglepolyheron의 boundingbox를 얻는다
		BoundingBox& getBoundingBox() {return bbox;}
		///< 이 trianglepolyhedron을 이루는 vbo들의 리스트를 얻는다
		std::vector<Vbo*>& getVbos() {return vbos;}
		///< 고유 identifier를 설정한다
		void setId(size_t indexNumber) {id = indexNumber;}
		///< 고유 identifier를 준다
		size_t getId() {return id;}
		///< 바깥으로 노출된 Surface가 있는가
		bool doesHaveAnyExteriorSurface();
	};

	
}