#pragma once

class CKK_Rectangle
{
public:

	double m_minX, m_maxX, m_minY, m_maxY;

	CKK_Rectangle();
	~CKK_Rectangle();

	void Add_Point(double x, double y);
	void Add_Rectangle(CKK_Rectangle *rect);
	void CopyFrom(CKK_Rectangle *rect);
	double Get_Height();
	double Get_Width();
	double Get_Perimeter();
	bool Intersection_withPoint(double x, double y, double error);
	bool Intersection_withRectangle(CKK_Rectangle *rect);
	bool Intersection_withRectangle(CKK_Rectangle *rect, double error);
	void Set(double minX, double minY, double maxX, double maxY);
	void Set_Init(double x, double y);
};

