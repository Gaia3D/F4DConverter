#pragma once

class CKK_Rectangle;
class CKK_Image2D_SplitData;
class CKK_Image2D;
namespace gaia3d
{
	class Triangle;
}

#include <vector>
#include <map>

class CKK_Image2D_Utils
{
public:

	std::vector<CKK_Image2D_SplitData*> m_vec_image2DSplitDatas;


	CKK_Image2D_Utils();
	~CKK_Image2D_Utils();

	int* Get_AreaOnImage_OfNormalizedRectangle(CKK_Image2D *image, CKK_Rectangle *normalizedRectangle);

	void MULTISPLITTIMAGE_Add_ImageRectangle(CKK_Rectangle* imageRectangle, gaia3d::Triangle *triangle);
	void MULTISPLITTIMAGE_Delete_Image2DSplitDatas();
	void MULTISPLITTIMAGE_Make_SplittedMosaic();
	bool MULTISPLITTIMAGE_Recombine_ImageRectangles();
	void InsertImage_ARGB(CKK_Image2D *image, CKK_Rectangle *rectSplitter, CKK_Image2D *imageToInsert_RGBA);
	void InsertImage_RGBA(CKK_Image2D *image, CKK_Rectangle *rectSplitter, CKK_Image2D *imageToInsert_RGBA);
	void Get_Region(CKK_Image2D *image, CKK_Rectangle *rectSplitter, CKK_Image2D *resultSplittedImage_RGBA);

	bool TEST__AllImage2DSplidatas_AreTexCoordsInside_rectangleOriginal();

private:
	void MULTISPLITTIMAGE_Get_BestPositionMosaic(std::vector<CKK_Image2D_SplitData*> &vec_splitDatasMosaic, CKK_Image2D_SplitData *splitData_toPutInMosaic, double *posX, double *posY);
	bool MULTISPLITTIMAGE_IntersectsRectangle(std::vector<CKK_Rectangle*> &vec_rectangles, CKK_Rectangle *rectangle);
	bool MULTISPLITTIMAGE_TryToRecombine_ImageRectangle(CKK_Image2D_SplitData *image2dSplitData);
};

