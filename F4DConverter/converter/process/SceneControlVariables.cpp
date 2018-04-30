
#include "stdafx.h"

#include "SceneControlVariables.h"

#include "../geometry/Matrix4.h"

SceneControlVariables::SceneControlVariables(void)
{
	m_hWnd = 0;
	m_myhDC = 0;
	m_hRC = 0;
	m_width = m_height = 0;
	this->m_tp_pantalla_action = GESTIO_PANTALLA_CAP;
	//m_system_state = SYSTEM_NORMAL;
	//m_navigation_mode = NAVIGATION_FIRSTPERSON;
	this->m_navigation_mode = NAVIGATION_WORLDMOVE;
	//m_tp_render= RENDER_SHADE_WIREFRAME;
	this->m_tp_select_and_move = SELECT_AND_MOVE_INDIVIDUALOBJECT;
	this->m_tp_geometry_create = NO_CREATE_GEOMETRY;
	//m_tp_display_on_screen = DISPLAY_NORMAL;

	this->m_xRot= 0.0; this->m_yRot= 0.0; this->m_zRot= 0.0; 
	this->m_xRotIni=0.0; this->m_yRotIni=0.0; this->m_zRotIni=0.0;
	this->m_xPos=0.0; this->m_yPos=-10.0; this->m_zPos=-20.5; // Original.***
	//m_xPos=0.0; m_yPos=0.0; m_zPos=-16000000.0; // For visualise the globe.***
	this->m_xNextPos=0.0; this->m_yNextPos=0.0; this->m_zNextPos=-60.0;

	this->m_xPosIni=0.0; this->m_yPosIni=0.0; this->m_zPosIni=0.0;

	this->m_xRot_aditional=0.0; this->m_yRot_aditional=0.0; this->m_zRot_aditional=0.0; // Test. delete after test.***

	//-------------------------------------------------------------
	//*************************************************************
	gaia3d::Matrix4 mat_xrot, mat_yrot, mat_zrot;
	mat_xrot.rotation(this->m_xRot*this->MPI_Div_180, 1.0, 0.0, 0.0);
	mat_yrot.rotation(this->m_yRot*this->MPI_Div_180, 0.0, 1.0, 0.0);
	mat_zrot.rotation(this->m_zRot*this->MPI_Div_180, 0.0, 0.0, 1.0);
	this->mat_rot= (mat_xrot*mat_yrot)*mat_zrot;
	//*************************************************************
	//-------------------------------------------------------------
	this->m_viewing_direction.set(0.0, 0.0, -1.0);
	this->m_viewing_direction = this->mat_rot*this->m_viewing_direction;

	//m_frustum= new CKK_Frustum();

	this->m_Zoom=0.0; this->m_ZoomIni=0.0;
	this->m_nRange=1300;
	this->m_LButtonDown=false;
	this->m_MButtonDown=false;
	this->m_RButtonDown=false;

	this->m_xDifPos_Model=0.0; this->m_yDifPos_Model=0.0; this->m_zDifPos_Model=0.0;
	//selector=new CSelector();

	this->m_xAnteriorPos_Model=0.0;
	this->m_yAnteriorPos_Model=0.0;
	this->m_zAnteriorPos_Model=0.0;

	this->m_xRot_speed=0.0; this->m_yRot_speed=0.0; this->m_zRot_speed=0.0;
	this->m_speed=0.0; this->m_lateral_speed=0.0; this->m_vertical_speed=0.0;
	this->m_speed_value=0.2;
	this->m_key_down=false;

	this->m_perspective_angle=90.0;
	this->m_perspective_near=0.01;
	this->m_perspective_far=8000000.0;

	this->MPI_Div_180= 3.14159265358979323846/180.0;
}


SceneControlVariables::~SceneControlVariables(void)
{
}