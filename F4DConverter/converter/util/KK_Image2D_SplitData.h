#pragma once


class CKK_Rectangle;
namespace gaia3d
{
	class Triangle;
	class Vertex;
}


#include <vector>
#include <map>

class CKK_Image2D_SplitData
{
public:

	CKK_Rectangle *m_rectangleImage_original; // region of the original image.***
	CKK_Rectangle *m_rectangleImage_splitted; // the size is equal to "original", only differs in the position in the new image.***
	std::vector<gaia3d::Triangle*> m_vec_triangles;

	CKK_Image2D_SplitData();
	~CKK_Image2D_SplitData();

	void Get_Vertices(std::vector<gaia3d::Vertex*> &vec_vertices);

	bool TEST__AreTexCoordsInside_rectangleOriginal();
};

