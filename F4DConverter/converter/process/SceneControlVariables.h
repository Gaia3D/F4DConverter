#pragma once

#include <Windows.h>
#include <gl/GL.h>

#include "../geometry/Point3D.h"
#include "../geometry/Matrix4.h"

enum TYPE_PANTALLA_ACTION
{  GESTIO_PANTALLA_CAP, GESTIO_PANTALLA_PAN, GESTIO_PANTALLA_ROT, GESTIO_PANTALLA_ZOOM  };

enum TYPE_NAVIGATION_MODE
{
    NAVIGATION_FIRSTPERSON, NAVIGATION_WORLDMOVE, NAVIGATION_WALKINGMODE
};

enum TYPE_PROJECTION
{ PROJECTION_ORTHO, PROJECTION_PERSPECTIVE };

enum TYPE_GEOMETRY_CREATE	
{  NO_CREATE_GEOMETRY, CREATE_GEOMETRY_POLYGON, CREATE_EXTRUDE};

enum TYPE_SELECT_AND_MOVE
{
	SELECT_AND_MOVE_INDIVIDUALOBJECT, SELECT_AND_MOVE_STOREY, SELECT_AND_MOVE_STOREY_STACK
};

class SceneControlVariables
{
public:
	SceneControlVariables();

	virtual ~SceneControlVariables();

public:

	int m_mouse_x, m_mouse_y;

	int nCount;
#ifdef _WIN32
	HWND m_hWnd;
	HGLRC m_hRC; // Permanent Rendering Context
	HDC m_myhDC; // Private GDI Device Context
#else
#endif
	//unsigned long long m_myhDC; // Private GDI Device Context
	int m_height; // Stores the height of the View
	int m_width; // Stores the width of the view
	float m_nRange;
	TYPE_PANTALLA_ACTION m_tp_pantalla_action;

	TYPE_NAVIGATION_MODE m_navigation_mode;
	TYPE_SELECT_AND_MOVE m_tp_select_and_move;
	TYPE_GEOMETRY_CREATE m_tp_geometry_create;
	

	double m_xRot, m_yRot, m_zRot, m_xRotIni, m_yRotIni, m_zRotIni;
	double m_xPos, m_yPos, m_zPos, m_xPosIni, m_yPosIni, m_zPosIni;
	double m_xNextPos, m_yNextPos, m_zNextPos; 
	double m_Zoom, m_ZoomIni;

	double m_xRot_aditional, m_yRot_aditional, m_zRot_aditional; // Test. delete after test.***

	bool m_LButtonDown, m_MButtonDown, m_RButtonDown;
	//CRect m_Rect;
	int CODIFILTRE;
	int senCodi[100], selProfund[100], senCodi_aux[100], selProfund_aux[100];

	float m_xPosLookAt, m_yPosLookAt, m_zPosLookAt;
	double m_speed, m_lateral_speed, m_vertical_speed, m_xRot_speed, m_yRot_speed, m_zRot_speed;
	double m_speed_value;
	bool m_key_down;
	double m_perspective_angle, m_perspective_near, m_perspective_far;

	float ambientLight[4];
	float diffuseLight[4];
	float specular[4];
	float lightPos[4];
	float specref[4];
	float ClearColor[4];

	double m_xDifPos_Model, m_yDifPos_Model, m_zDifPos_Model;
	double m_xAnteriorPos_Model, m_yAnteriorPos_Model, m_zAnteriorPos_Model;
	double MPI_Div_180;

	TYPE_PROJECTION tp_projection;
	gaia3d::Matrix4 mat_rot;
	gaia3d::Point3D m_viewing_direction;
};