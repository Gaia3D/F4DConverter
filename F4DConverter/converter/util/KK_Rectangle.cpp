

#include "stdafx.h"

#include "KK_Rectangle.h"



CKK_Rectangle::CKK_Rectangle()
{
	m_minX = 0.0;
	m_maxX = 0.0; 
	m_minY = 0.0; 
	m_maxY = 0.0;
}


CKK_Rectangle::~CKK_Rectangle()
{
}

double CKK_Rectangle::Get_Width()
{
	return m_maxX - m_minX;
}

double CKK_Rectangle::Get_Height()
{
	return m_maxY - m_minY;
}

double CKK_Rectangle::Get_Perimeter()
{
	return (this->Get_Width() * 2 + this->Get_Height() * 2);
}

void CKK_Rectangle::Set_Init(double x, double y)
{
	// use rectangle as bounding rectangle.***
	m_minX = x;
	m_maxX = x;
	m_minY = y;
	m_maxY = y;

	if (m_minX < 0.0 || m_maxX < 0.0 || m_minY < 0.0 || m_maxY < 0.0)
	{
		int hola = 0;
	}
}

void CKK_Rectangle::Set(double minX, double minY, double maxX, double maxY)
{
	// use rectangle as bounding rectangle.***
	m_minX = minX;
	m_maxX = maxX;
	m_minY = minY;
	m_maxY = maxY;

	if (m_minX < 0.0 || m_maxX < 0.0 || m_minY < 0.0 || m_maxY < 0.0)
	{
		int hola = 0;
	}
}

void CKK_Rectangle::Add_Point(double x, double y)
{
	// use rectangle as bounding rectangle.***
	if (x < m_minX)
		m_minX = x;
	else if (x > m_maxX)
		m_maxX = x;

	if (y < m_minY)
		m_minY = y;
	else if (y > m_maxY)
		m_maxY = y;

	if (m_minX < 0.0 || m_maxX < 0.0 || m_minY < 0.0 || m_maxY < 0.0)
	{
		int hola = 0;
	}
}

void CKK_Rectangle::Add_Rectangle(CKK_Rectangle *rect)
{
	// use rectangle as bounding rectangle.***
	if (rect->m_minX < this->m_minX)
		this->m_minX = rect->m_minX;

	if (rect->m_maxX > this->m_maxX)
		this->m_maxX = rect->m_maxX;

	if (rect->m_minY < this->m_minY)
		this->m_minY = rect->m_minY;

	if (rect->m_maxY > this->m_maxY)
		this->m_maxY = rect->m_maxY;

	if (m_minX < 0.0 || m_maxX < 0.0 || m_minY < 0.0 || m_maxY < 0.0)
	{
		int hola = 0;
	}
}

void CKK_Rectangle::CopyFrom(CKK_Rectangle *rect)
{
	this->m_minX = rect->m_minX;
	this->m_maxX = rect->m_maxX;
	this->m_minY = rect->m_minY;
	this->m_maxY = rect->m_maxY;

	if (m_minX < 0.0 || m_maxX < 0.0 || m_minY < 0.0 || m_maxY < 0.0)
	{
		int hola = 0;
	}
}

bool CKK_Rectangle::Intersection_withPoint(double x, double y, double error)
{
	bool interscets = true;

	if (x > this->m_maxX + error)
		return false;
	else if (x < this->m_minX - error)
		return false;
	else if (y > this->m_maxY + error)
		return false;
	else if (y < this->m_minY - error)
		return false;

	return interscets;
}

bool CKK_Rectangle::Intersection_withRectangle(CKK_Rectangle *rect)
{
	bool interscets = true;

	if (rect->m_minX > this->m_maxX)
		return false;
	else if (rect->m_maxX < this->m_minX)
		return false;
	else if (rect->m_minY > this->m_maxY)
		return false;
	else if (rect->m_maxY < this->m_minY)
		return false;

	return interscets;
}

bool CKK_Rectangle::Intersection_withRectangle(CKK_Rectangle *rect, double error)
{
	bool interscets = true;

	if (rect->m_minX > this->m_maxX - error)
		return false;
	else if (rect->m_maxX < this->m_minX + error)
		return false;
	else if (rect->m_minY > this->m_maxY - error)
		return false;
	else if (rect->m_maxY < this->m_minY + error)
		return false;

	return interscets;
}
