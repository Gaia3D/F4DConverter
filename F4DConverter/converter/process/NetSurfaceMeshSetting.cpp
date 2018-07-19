#include "stdafx.h"
#include "NetSurfaceMeshSetting.h"

NetSurfaceMeshSetting::NetSurfaceMeshSetting()
{
}


NetSurfaceMeshSetting::~NetSurfaceMeshSetting()
{
}

NetSurfaceMeshSetting* NetSurfaceMeshSetting::getNetSurfaceMeshSetting(unsigned char settingIndex, unsigned char lodNumber)
{
	NetSurfaceMeshSetting* result = NULL;

	switch (settingIndex)
	{
	case 0: // extremely tight. closest to reality
	{
		switch (lodNumber)
		{
		case 2:
		{
			result = new NetSurfaceMeshSetting;
			result->maxAngleChangeOfNormalVectorForAllowingEdgeCollapse = 20.0f;
			result->maxAngleChangeOfFrontierEdgeForAllowingEdgeCollapse = 5.0f;
			result->maxAngleDifferenceBetweenNeighborFrontierEdgesForCollapse = 14.0f;
			result->netCellSize = 0.03f;
			result->maxLengthForAllowingInnerEdgeSkirting = result->netCellSize * 2.5f;
			result->maxLengthForAllowingFrontierEdgeSkirting = result->netCellSize * 2.5f;
			result->subBoxSize = 3.0f;
			result->netSurfaceMeshTextureWidth = 48;
			result->netSurfaceMeshTextureHeight = 48;
		}
		break;
		case 3:
		{
			result = new NetSurfaceMeshSetting;
			result->maxAngleChangeOfNormalVectorForAllowingEdgeCollapse = 20.0f;
			result->maxAngleChangeOfFrontierEdgeForAllowingEdgeCollapse = 5.0f;
			result->maxAngleDifferenceBetweenNeighborFrontierEdgesForCollapse = 14.0f;
			result->netCellSize = 0.06f;
			result->maxLengthForAllowingInnerEdgeSkirting = result->netCellSize * 2.5f;
			result->maxLengthForAllowingFrontierEdgeSkirting = result->netCellSize * 2.5f;
			result->subBoxSize = 5.0f;
			result->netSurfaceMeshTextureWidth = 48;
			result->netSurfaceMeshTextureHeight = 48;
		}
		break;
		case 4:
		{
			result = new NetSurfaceMeshSetting;
			result->maxAngleChangeOfNormalVectorForAllowingEdgeCollapse = 20.0f;
			result->maxAngleChangeOfFrontierEdgeForAllowingEdgeCollapse = 5.0f;
			result->maxAngleDifferenceBetweenNeighborFrontierEdgesForCollapse = 14.0f;
			result->netCellSize = 0.1f;
			result->maxLengthForAllowingInnerEdgeSkirting = result->netCellSize * 2.5f;
			result->maxLengthForAllowingFrontierEdgeSkirting = result->netCellSize * 2.5f;
			result->subBoxSize = 10.0f;
			result->netSurfaceMeshTextureWidth = 32;
			result->netSurfaceMeshTextureHeight = 32;
		}
		break;
		case 5:
		{
			result = new NetSurfaceMeshSetting;
			result->maxAngleChangeOfNormalVectorForAllowingEdgeCollapse = 20.0f;
			result->maxAngleChangeOfFrontierEdgeForAllowingEdgeCollapse = 5.0f;
			result->maxAngleDifferenceBetweenNeighborFrontierEdgesForCollapse = 14.0f;
			result->netCellSize = 0.2f;
			result->maxLengthForAllowingInnerEdgeSkirting = result->netCellSize * 2.5f;
			result->maxLengthForAllowingFrontierEdgeSkirting = result->netCellSize * 2.5f;
			result->subBoxSize = 20.0f;
			result->netSurfaceMeshTextureWidth = 16;
			result->netSurfaceMeshTextureHeight = 16;
		}
		break;
		}
	}
	break;
	case 1: // for MEP
	{
		switch (lodNumber)
		{
		case 2:
		{
			result = new NetSurfaceMeshSetting;
			result->maxAngleChangeOfNormalVectorForAllowingEdgeCollapse = 20.0f;
			result->maxAngleChangeOfFrontierEdgeForAllowingEdgeCollapse = 5.0f;
			result->maxAngleDifferenceBetweenNeighborFrontierEdgesForCollapse = 14.0f;
			result->netCellSize = 0.05f;
			result->maxLengthForAllowingInnerEdgeSkirting = result->netCellSize * 2.5f;
			result->maxLengthForAllowingFrontierEdgeSkirting = result->netCellSize * 2.5f;
			result->subBoxSize = 2.5f;
			result->netSurfaceMeshTextureWidth = 48;
			result->netSurfaceMeshTextureHeight = 48;
		}
		break;
		case 3:
		{
			result = new NetSurfaceMeshSetting;
			result->maxAngleChangeOfNormalVectorForAllowingEdgeCollapse = 20.0f;
			result->maxAngleChangeOfFrontierEdgeForAllowingEdgeCollapse = 5.0f;
			result->maxAngleDifferenceBetweenNeighborFrontierEdgesForCollapse = 14.0f;
			result->netCellSize = 0.1f;
			result->maxLengthForAllowingInnerEdgeSkirting = result->netCellSize * 2.5f;
			result->maxLengthForAllowingFrontierEdgeSkirting = result->netCellSize * 2.5f;
			result->subBoxSize = 5.0f;
			result->netSurfaceMeshTextureWidth = 48;
			result->netSurfaceMeshTextureHeight = 48;
		}
		break;
		case 4:
		{
			result = new NetSurfaceMeshSetting;
			result->maxAngleChangeOfNormalVectorForAllowingEdgeCollapse = 20.0f;
			result->maxAngleChangeOfFrontierEdgeForAllowingEdgeCollapse = 5.0f;
			result->maxAngleDifferenceBetweenNeighborFrontierEdgesForCollapse = 14.0f;
			result->netCellSize = 0.2f;
			result->maxLengthForAllowingInnerEdgeSkirting = result->netCellSize * 2.5f;
			result->maxLengthForAllowingFrontierEdgeSkirting = result->netCellSize * 2.5f;
			result->subBoxSize = 10.0f;
			result->netSurfaceMeshTextureWidth = 32;
			result->netSurfaceMeshTextureHeight = 32;
		}
		break;
		case 5:
		{
			result = new NetSurfaceMeshSetting;
			result->maxAngleChangeOfNormalVectorForAllowingEdgeCollapse = 20.0f;
			result->maxAngleChangeOfFrontierEdgeForAllowingEdgeCollapse = 5.0f;
			result->maxAngleDifferenceBetweenNeighborFrontierEdgesForCollapse = 14.0f;
			result->netCellSize = 0.4f;
			result->maxLengthForAllowingInnerEdgeSkirting = result->netCellSize * 2.5f;
			result->maxLengthForAllowingFrontierEdgeSkirting = result->netCellSize * 2.5f;
			result->subBoxSize = 20.0f;
			result->netSurfaceMeshTextureWidth = 16;
			result->netSurfaceMeshTextureHeight = 16;
		}
		break;
		}
	}
	break;
	case 2: // for small buildings
	{
		switch (lodNumber)
		{
		case 2:
		{
			result = new NetSurfaceMeshSetting;
			result->maxAngleChangeOfNormalVectorForAllowingEdgeCollapse = 20.0f;
			result->maxAngleChangeOfFrontierEdgeForAllowingEdgeCollapse = 5.0f;
			result->maxAngleDifferenceBetweenNeighborFrontierEdgesForCollapse = 14.0f;
			result->netCellSize = 0.1f;
			result->maxLengthForAllowingInnerEdgeSkirting = result->netCellSize * 2.5f;
			result->maxLengthForAllowingFrontierEdgeSkirting = result->netCellSize * 2.5f;
			result->subBoxSize = 5.0f;
			result->netSurfaceMeshTextureWidth = 48;
			result->netSurfaceMeshTextureHeight = 48;
		}
		break;
		case 3:
		{
			result = new NetSurfaceMeshSetting;
			result->maxAngleChangeOfNormalVectorForAllowingEdgeCollapse = 20.0f;
			result->maxAngleChangeOfFrontierEdgeForAllowingEdgeCollapse = 5.0f;
			result->maxAngleDifferenceBetweenNeighborFrontierEdgesForCollapse = 14.0f;
			result->netCellSize = 0.2f;
			result->maxLengthForAllowingInnerEdgeSkirting = result->netCellSize * 2.5f;
			result->maxLengthForAllowingFrontierEdgeSkirting = result->netCellSize * 2.5f;
			result->subBoxSize = 10.0f;
			result->netSurfaceMeshTextureWidth = 48;
			result->netSurfaceMeshTextureHeight = 48;
		}
		break;
		case 4:
		{
			result = new NetSurfaceMeshSetting;
			result->maxAngleChangeOfNormalVectorForAllowingEdgeCollapse = 20.0f;
			result->maxAngleChangeOfFrontierEdgeForAllowingEdgeCollapse = 5.0f;
			result->maxAngleDifferenceBetweenNeighborFrontierEdgesForCollapse = 14.0f;
			result->netCellSize = 0.4f;
			result->maxLengthForAllowingInnerEdgeSkirting = result->netCellSize * 2.5f;
			result->maxLengthForAllowingFrontierEdgeSkirting = result->netCellSize * 2.5f;
			result->subBoxSize = 20.0f;
			result->netSurfaceMeshTextureWidth = 32;
			result->netSurfaceMeshTextureHeight = 32;
		}
		break;
		case 5:
		{
			result = new NetSurfaceMeshSetting;
			result->maxAngleChangeOfNormalVectorForAllowingEdgeCollapse = 20.0f;
			result->maxAngleChangeOfFrontierEdgeForAllowingEdgeCollapse = 5.0f;
			result->maxAngleDifferenceBetweenNeighborFrontierEdgesForCollapse = 14.0f;
			result->netCellSize = 0.8f;
			result->maxLengthForAllowingInnerEdgeSkirting = result->netCellSize * 2.5f;
			result->maxLengthForAllowingFrontierEdgeSkirting = result->netCellSize * 2.5f;
			result->subBoxSize = 40.0f;
			result->netSurfaceMeshTextureWidth = 16;
			result->netSurfaceMeshTextureHeight = 16;
		}
		break;
		}
	}
	break;
	case 3: // for big buildings
	{
		switch (lodNumber)
		{
		case 2:
		{
			result = new NetSurfaceMeshSetting;
			result->maxAngleChangeOfNormalVectorForAllowingEdgeCollapse = 20.0f;
			result->maxAngleChangeOfFrontierEdgeForAllowingEdgeCollapse = 5.0f;
			result->maxAngleDifferenceBetweenNeighborFrontierEdgesForCollapse = 14.0f;
			result->netCellSize = 0.15f;
			result->maxLengthForAllowingInnerEdgeSkirting = result->netCellSize * 2.5f;
			result->maxLengthForAllowingFrontierEdgeSkirting = result->netCellSize * 2.5f;
			result->subBoxSize = 6.0f;
			result->netSurfaceMeshTextureWidth = 48;
			result->netSurfaceMeshTextureHeight = 48;
		}
		break;
		case 3:
		{
			result = new NetSurfaceMeshSetting;
			result->maxAngleChangeOfNormalVectorForAllowingEdgeCollapse = 20.0f;
			result->maxAngleChangeOfFrontierEdgeForAllowingEdgeCollapse = 5.0f;
			result->maxAngleDifferenceBetweenNeighborFrontierEdgesForCollapse = 14.0f;
			result->netCellSize = 0.3f;
			result->maxLengthForAllowingInnerEdgeSkirting = result->netCellSize * 2.5f;
			result->maxLengthForAllowingFrontierEdgeSkirting = result->netCellSize * 2.5f;
			result->subBoxSize = 10.0f;
			result->netSurfaceMeshTextureWidth = 48;
			result->netSurfaceMeshTextureHeight = 48;
		}
		break;
		case 4:
		{
			result = new NetSurfaceMeshSetting;
			result->maxAngleChangeOfNormalVectorForAllowingEdgeCollapse = 20.0f;
			result->maxAngleChangeOfFrontierEdgeForAllowingEdgeCollapse = 5.0f;
			result->maxAngleDifferenceBetweenNeighborFrontierEdgesForCollapse = 14.0f;
			result->netCellSize = 0.7f;
			result->maxLengthForAllowingInnerEdgeSkirting = result->netCellSize * 2.5f;
			result->maxLengthForAllowingFrontierEdgeSkirting = result->netCellSize * 2.5f;
			result->subBoxSize = 20.0f;
			result->netSurfaceMeshTextureWidth = 32;
			result->netSurfaceMeshTextureHeight = 32;
		}
		break;
		case 5:
		{
			result = new NetSurfaceMeshSetting;
			result->maxAngleChangeOfNormalVectorForAllowingEdgeCollapse = 20.0f;
			result->maxAngleChangeOfFrontierEdgeForAllowingEdgeCollapse = 5.0f;
			result->maxAngleDifferenceBetweenNeighborFrontierEdgesForCollapse = 14.0f;
			result->netCellSize = 1.5f;
			result->maxLengthForAllowingInnerEdgeSkirting = result->netCellSize * 2.5f;
			result->maxLengthForAllowingFrontierEdgeSkirting = result->netCellSize * 2.5f;
			result->subBoxSize = 100.0f;
			result->netSurfaceMeshTextureWidth = 16;
			result->netSurfaceMeshTextureHeight = 16;
		}
		break;
		}
	}
	break;
	case 4: // extra large
	{
		switch (lodNumber)
		{
		case 2:
		{
			result = new NetSurfaceMeshSetting;
			result->maxAngleChangeOfNormalVectorForAllowingEdgeCollapse = 20.0f;
			result->maxAngleChangeOfFrontierEdgeForAllowingEdgeCollapse = 5.0f;
			result->maxAngleDifferenceBetweenNeighborFrontierEdgesForCollapse = 14.0f;
			result->netCellSize = 0.5f;
			result->maxLengthForAllowingInnerEdgeSkirting = result->netCellSize * 2.5f;
			result->maxLengthForAllowingFrontierEdgeSkirting = result->netCellSize * 2.5f;
			result->subBoxSize = 25.0f;
			result->netSurfaceMeshTextureWidth = 48;
			result->netSurfaceMeshTextureHeight = 48;
		}
		break;
		case 3:
		{
			result = new NetSurfaceMeshSetting;
			result->maxAngleChangeOfNormalVectorForAllowingEdgeCollapse = 20.0f;
			result->maxAngleChangeOfFrontierEdgeForAllowingEdgeCollapse = 5.0f;
			result->maxAngleDifferenceBetweenNeighborFrontierEdgesForCollapse = 14.0f;
			result->netCellSize = 1.0f;
			result->maxLengthForAllowingInnerEdgeSkirting = result->netCellSize * 2.5f;
			result->maxLengthForAllowingFrontierEdgeSkirting = result->netCellSize * 2.5f;
			result->subBoxSize = 50.0f;
			result->netSurfaceMeshTextureWidth = 48;
			result->netSurfaceMeshTextureHeight = 48;
		}
		break;
		case 4:
		{
			result = new NetSurfaceMeshSetting;
			result->maxAngleChangeOfNormalVectorForAllowingEdgeCollapse = 20.0f;
			result->maxAngleChangeOfFrontierEdgeForAllowingEdgeCollapse = 5.0f;
			result->maxAngleChangeOfFrontierEdgeForAllowingEdgeCollapse = 30.0f;
			result->maxAngleDifferenceBetweenNeighborFrontierEdgesForCollapse = 14.0f;
			result->netCellSize = 2.0f;
			result->maxLengthForAllowingInnerEdgeSkirting = result->netCellSize * 2.5f;
			result->maxLengthForAllowingFrontierEdgeSkirting = result->netCellSize * 2.5f;
			result->subBoxSize = 100.0f;
			result->netSurfaceMeshTextureWidth = 32;
			result->netSurfaceMeshTextureHeight = 32;
		}
		break;
		case 5:
		{
			result = new NetSurfaceMeshSetting;
			result->maxAngleChangeOfNormalVectorForAllowingEdgeCollapse = 20.0f;
			result->maxAngleChangeOfFrontierEdgeForAllowingEdgeCollapse = 5.0f;
			result->maxAngleDifferenceBetweenNeighborFrontierEdgesForCollapse = 14.0f;
			result->netCellSize = 4.0f;
			result->maxLengthForAllowingInnerEdgeSkirting = result->netCellSize * 2.5f;
			result->maxLengthForAllowingFrontierEdgeSkirting = result->netCellSize * 2.5f;
			result->subBoxSize = 200.0f;
			result->netSurfaceMeshTextureWidth = 16;
			result->netSurfaceMeshTextureHeight = 16;
		}
		break;
		}
	}
	break;
	case 5: // extra extra large
	{
		switch (lodNumber)
		{
		case 2:
		{
			result = new NetSurfaceMeshSetting;
			result->maxAngleChangeOfNormalVectorForAllowingEdgeCollapse = 20.0f;
			result->maxAngleChangeOfFrontierEdgeForAllowingEdgeCollapse = 5.0f;
			result->maxAngleDifferenceBetweenNeighborFrontierEdgesForCollapse = 14.0f;
			result->netCellSize = 1.5f;
			result->maxLengthForAllowingInnerEdgeSkirting = result->netCellSize * 2.5f;
			result->maxLengthForAllowingFrontierEdgeSkirting = result->netCellSize * 2.5f;
			result->subBoxSize = 50.0f;
			result->netSurfaceMeshTextureWidth = 48;
			result->netSurfaceMeshTextureHeight = 48;
		}
		break;
		case 3:
		{
			result = new NetSurfaceMeshSetting;
			result->maxAngleChangeOfNormalVectorForAllowingEdgeCollapse = 20.0f;
			result->maxAngleChangeOfFrontierEdgeForAllowingEdgeCollapse = 5.0f;
			result->maxAngleDifferenceBetweenNeighborFrontierEdgesForCollapse = 14.0f;
			result->netCellSize = 3.0f;
			result->maxLengthForAllowingInnerEdgeSkirting = result->netCellSize * 2.5f;
			result->maxLengthForAllowingFrontierEdgeSkirting = result->netCellSize * 2.5f;
			result->subBoxSize = 100.0f;
			result->netSurfaceMeshTextureWidth = 48;
			result->netSurfaceMeshTextureHeight = 48;
		}
		break;
		case 4:
		{
			result = new NetSurfaceMeshSetting;
			result->maxAngleChangeOfNormalVectorForAllowingEdgeCollapse = 20.0f;
			result->maxAngleChangeOfFrontierEdgeForAllowingEdgeCollapse = 5.0f;
			result->maxAngleDifferenceBetweenNeighborFrontierEdgesForCollapse = 14.0f;
			result->netCellSize = 6.0f;
			result->maxLengthForAllowingInnerEdgeSkirting = result->netCellSize * 2.5f;
			result->maxLengthForAllowingFrontierEdgeSkirting = result->netCellSize * 2.5f;
			result->subBoxSize = 300.0f;
			result->netSurfaceMeshTextureWidth = 32;
			result->netSurfaceMeshTextureHeight = 32;
		}
		break;
		case 5:
		{
			result = new NetSurfaceMeshSetting;
			result->maxAngleChangeOfNormalVectorForAllowingEdgeCollapse = 20.0f;
			result->maxAngleChangeOfFrontierEdgeForAllowingEdgeCollapse = 5.0f;
			result->maxAngleDifferenceBetweenNeighborFrontierEdgesForCollapse = 14.0f;
			result->netCellSize = 10.0f;
			result->maxLengthForAllowingInnerEdgeSkirting = result->netCellSize * 2.5f;
			result->maxLengthForAllowingFrontierEdgeSkirting = result->netCellSize * 2.5f;
			result->subBoxSize = 600.0f;
			result->netSurfaceMeshTextureWidth = 16;
			result->netSurfaceMeshTextureHeight = 16;
		}
		break;
		}
	}
	break;
	case 6: // a kind of tests
	{
		switch (lodNumber)
		{
		case 2:
		{
			result = new NetSurfaceMeshSetting;
			result->maxAngleChangeOfNormalVectorForAllowingEdgeCollapse = 30.0f;
			result->maxAngleChangeOfFrontierEdgeForAllowingEdgeCollapse = 30.0f;
			result->maxAngleDifferenceBetweenNeighborFrontierEdgesForCollapse = 14.0f;
			result->netCellSize = 5.0f;
			result->maxLengthForAllowingInnerEdgeSkirting = result->netCellSize * 2.5f;
			result->maxLengthForAllowingFrontierEdgeSkirting = result->netCellSize * 2.5f;
			result->subBoxSize = 50.0f;
			result->netSurfaceMeshTextureWidth = 48;
			result->netSurfaceMeshTextureHeight = 48;
		}
		break;
		case 3:
		{
			result = new NetSurfaceMeshSetting;
			result->maxAngleChangeOfNormalVectorForAllowingEdgeCollapse = 30.0f;
			result->maxAngleChangeOfFrontierEdgeForAllowingEdgeCollapse = 30.0f;
			result->maxAngleDifferenceBetweenNeighborFrontierEdgesForCollapse = 14.0f;
			result->netCellSize = 10.0f;
			result->maxLengthForAllowingInnerEdgeSkirting = result->netCellSize * 2.5f;
			result->maxLengthForAllowingFrontierEdgeSkirting = result->netCellSize * 2.5f;
			result->subBoxSize = 100.0f;
			result->netSurfaceMeshTextureWidth = 48;
			result->netSurfaceMeshTextureHeight = 48;
		}
		break;
		case 4:
		{
			result = new NetSurfaceMeshSetting;
			result->maxAngleChangeOfNormalVectorForAllowingEdgeCollapse = 30.0f;
			result->maxAngleChangeOfFrontierEdgeForAllowingEdgeCollapse = 30.0f;
			result->maxAngleDifferenceBetweenNeighborFrontierEdgesForCollapse = 14.0f;
			result->netCellSize = 15.0f;
			result->maxLengthForAllowingInnerEdgeSkirting = result->netCellSize * 2.5f;
			result->maxLengthForAllowingFrontierEdgeSkirting = result->netCellSize * 2.5f;
			result->subBoxSize = 300.0f;
			result->netSurfaceMeshTextureWidth = 32;
			result->netSurfaceMeshTextureHeight = 32;
		}
		break;
		case 5:
		{
			result = new NetSurfaceMeshSetting;
			result->maxAngleChangeOfNormalVectorForAllowingEdgeCollapse = 30.0f;
			result->maxAngleChangeOfFrontierEdgeForAllowingEdgeCollapse = 30.0f;
			result->maxAngleDifferenceBetweenNeighborFrontierEdgesForCollapse = 14.0f;
			result->netCellSize = 20.0f;
			result->maxLengthForAllowingInnerEdgeSkirting = result->netCellSize * 2.5f;
			result->maxLengthForAllowingFrontierEdgeSkirting = result->netCellSize * 2.5f;
			result->subBoxSize = 600.0f;
			result->netSurfaceMeshTextureWidth = 16;
			result->netSurfaceMeshTextureHeight = 16;
		}
		break;
		}
	}
	break;
	case 50: // for large single realistic mesh
	{
		switch (lodNumber)
		{
		case 2:
		{
			result = new NetSurfaceMeshSetting;
			result->maxAngleChangeOfNormalVectorForAllowingEdgeCollapse = 20.0f;
			result->maxAngleChangeOfFrontierEdgeForAllowingEdgeCollapse = 5.0f;
			result->maxAngleDifferenceBetweenNeighborFrontierEdgesForCollapse = 15.0f;
			result->netCellSize = 0.04f;
			result->maxLengthForAllowingInnerEdgeSkirting = result->netCellSize * 2.5f;
			result->maxLengthForAllowingFrontierEdgeSkirting = result->netCellSize * 2.5f;
			result->subBoxSize = 3.0f;
			result->netSurfaceMeshTextureWidth = 48;
			result->netSurfaceMeshTextureHeight = 48;
		}
		break;
		case 3:
		{
			result = new NetSurfaceMeshSetting;
			result->maxAngleChangeOfNormalVectorForAllowingEdgeCollapse = 40.0f;
			result->maxAngleChangeOfFrontierEdgeForAllowingEdgeCollapse = 15.0f;
			result->maxAngleDifferenceBetweenNeighborFrontierEdgesForCollapse = 30.0f;
			result->netCellSize = 0.1f;
			result->maxLengthForAllowingInnerEdgeSkirting = result->netCellSize * 2.5f;
			result->maxLengthForAllowingFrontierEdgeSkirting = result->netCellSize * 2.5f;
			result->subBoxSize = 5.0f;
			result->netSurfaceMeshTextureWidth = 48;
			result->netSurfaceMeshTextureHeight = 48;
		}
		break;
		case 4:
		{
			result = new NetSurfaceMeshSetting;
			result->maxAngleChangeOfNormalVectorForAllowingEdgeCollapse = 50.0f;
			result->maxAngleChangeOfFrontierEdgeForAllowingEdgeCollapse = 30.0f;
			result->maxAngleDifferenceBetweenNeighborFrontierEdgesForCollapse = 40.0f;
			result->netCellSize = 0.3f;
			result->maxLengthForAllowingInnerEdgeSkirting = result->netCellSize * 2.5f;
			result->maxLengthForAllowingFrontierEdgeSkirting = result->netCellSize * 2.5f;
			result->subBoxSize = 10.0f;
			result->netSurfaceMeshTextureWidth = 32;
			result->netSurfaceMeshTextureHeight = 32;
		}
		break;
		case 5:
		{
			result = new NetSurfaceMeshSetting;
			result->maxAngleChangeOfNormalVectorForAllowingEdgeCollapse = 35.0f;
			result->maxAngleChangeOfFrontierEdgeForAllowingEdgeCollapse = 5.0f;
			result->maxAngleDifferenceBetweenNeighborFrontierEdgesForCollapse = 14.0f;
			result->netCellSize = 0.7f;
			result->maxLengthForAllowingInnerEdgeSkirting = result->netCellSize * 200.0f;
			result->maxLengthForAllowingFrontierEdgeSkirting = result->netCellSize * 100.f;
			result->subBoxSize = 8.0f;
			result->netSurfaceMeshTextureWidth = 16;
			result->netSurfaceMeshTextureHeight = 16;
		}
		break;
		}
	}
	break;
	case 51: // for large splitted realistic mesh
	{
		switch (lodNumber)
		{
		case 2:
		{
			result = new NetSurfaceMeshSetting;
			result->maxAngleChangeOfNormalVectorForAllowingEdgeCollapse = 35.0f;
			result->maxAngleChangeOfFrontierEdgeForAllowingEdgeCollapse = 5.0f;
			result->maxAngleDifferenceBetweenNeighborFrontierEdgesForCollapse = 14.0f;
			result->netCellSize = 0.5f;
			result->maxLengthForAllowingInnerEdgeSkirting = result->netCellSize * 200.f;
			result->maxLengthForAllowingFrontierEdgeSkirting = result->netCellSize * 100.f;
			result->subBoxSize = 40.0f;
			result->netSurfaceMeshTextureWidth = 96;
			result->netSurfaceMeshTextureHeight = 96;
		}
		break;
		case 3:
		{
			result = new NetSurfaceMeshSetting;
			result->maxAngleChangeOfNormalVectorForAllowingEdgeCollapse = 35.0f;
			result->maxAngleChangeOfFrontierEdgeForAllowingEdgeCollapse = 5.0f;
			result->maxAngleDifferenceBetweenNeighborFrontierEdgesForCollapse = 14.0f;
			result->netCellSize = 1.0f;
			result->maxLengthForAllowingInnerEdgeSkirting = result->netCellSize * 200.f;
			result->maxLengthForAllowingFrontierEdgeSkirting = result->netCellSize * 100.f;
			result->subBoxSize = 40.0f;
			result->netSurfaceMeshTextureWidth = 96;
			result->netSurfaceMeshTextureHeight = 96;
		}
		break;
		case 4:
		{
			result = new NetSurfaceMeshSetting;
			result->maxAngleChangeOfNormalVectorForAllowingEdgeCollapse = 35.0f;
			result->maxAngleChangeOfFrontierEdgeForAllowingEdgeCollapse = 5.0f;
			result->maxAngleDifferenceBetweenNeighborFrontierEdgesForCollapse = 14.0f;
			result->netCellSize = 2.0f;
			result->maxLengthForAllowingInnerEdgeSkirting = result->netCellSize * 200.f;
			result->maxLengthForAllowingFrontierEdgeSkirting = result->netCellSize * 100.f;
			result->subBoxSize = 40.0f;
			result->netSurfaceMeshTextureWidth = 64;
			result->netSurfaceMeshTextureHeight = 64;
		}
		break;
		case 5:
		{
			result = new NetSurfaceMeshSetting;
			result->maxAngleChangeOfNormalVectorForAllowingEdgeCollapse = 35.0f;
			result->maxAngleChangeOfFrontierEdgeForAllowingEdgeCollapse = 5.0f;
			result->maxAngleDifferenceBetweenNeighborFrontierEdgesForCollapse = 14.0f;
			result->netCellSize = 4.0f;
			result->maxLengthForAllowingInnerEdgeSkirting = result->netCellSize * 200.0f;
			result->maxLengthForAllowingFrontierEdgeSkirting = result->netCellSize * 100.f;
			result->subBoxSize = 40.0f;
			result->netSurfaceMeshTextureWidth = 32;
			result->netSurfaceMeshTextureHeight = 32;
		}
		break;
		}
	}
	break;
	case 255: // test case
	{
		switch (lodNumber)
		{
		case 2:
		{
			result = new NetSurfaceMeshSetting;
			result->maxAngleChangeOfNormalVectorForAllowingEdgeCollapse = 20.0f;
			result->maxAngleChangeOfFrontierEdgeForAllowingEdgeCollapse = 5.0f;
			result->maxAngleDifferenceBetweenNeighborFrontierEdgesForCollapse = 14.0f;
			result->netCellSize = 0.15f;
			result->maxLengthForAllowingInnerEdgeSkirting = result->netCellSize * 2.5f;
			result->maxLengthForAllowingFrontierEdgeSkirting = result->netCellSize * 2.5f;
			result->subBoxSize = 15.0f;
			result->netSurfaceMeshTextureWidth = 48;
			result->netSurfaceMeshTextureHeight = 48;
		}
		break;
		case 3:
		{
			result = new NetSurfaceMeshSetting;
			result->maxAngleChangeOfNormalVectorForAllowingEdgeCollapse = 20.0f;
			result->maxAngleChangeOfFrontierEdgeForAllowingEdgeCollapse = 5.0f;
			result->maxAngleDifferenceBetweenNeighborFrontierEdgesForCollapse = 14.0f;
			result->netCellSize = 0.3f;
			result->maxLengthForAllowingInnerEdgeSkirting = result->netCellSize * 2.5f;
			result->maxLengthForAllowingFrontierEdgeSkirting = result->netCellSize * 2.5f;
			result->subBoxSize = 15.0f;
			result->netSurfaceMeshTextureWidth = 48;
			result->netSurfaceMeshTextureHeight = 48;
		}
		break;
		case 4:
		{
			result = new NetSurfaceMeshSetting;
			result->maxAngleChangeOfNormalVectorForAllowingEdgeCollapse = 20.0f;
			result->maxAngleChangeOfFrontierEdgeForAllowingEdgeCollapse = 5.0f;
			result->maxAngleDifferenceBetweenNeighborFrontierEdgesForCollapse = 14.0f;
			result->netCellSize = 0.7f;
			result->maxLengthForAllowingInnerEdgeSkirting = result->netCellSize * 2.5f;
			result->maxLengthForAllowingFrontierEdgeSkirting = result->netCellSize * 2.5f;
			result->subBoxSize = 20.0f;
			result->netSurfaceMeshTextureWidth = 32;
			result->netSurfaceMeshTextureHeight = 32;
		}
		break;
		case 5:
		{
			result = new NetSurfaceMeshSetting;
			result->maxAngleChangeOfNormalVectorForAllowingEdgeCollapse = 20.0f;
			result->maxAngleChangeOfFrontierEdgeForAllowingEdgeCollapse = 5.0f;
			result->maxAngleDifferenceBetweenNeighborFrontierEdgesForCollapse = 14.0f;
			result->netCellSize = 1.5f;
			result->maxLengthForAllowingInnerEdgeSkirting = result->netCellSize * 2.5f;
			result->maxLengthForAllowingFrontierEdgeSkirting = result->netCellSize * 2.5f;
			result->subBoxSize = 100.0f;
			result->netSurfaceMeshTextureWidth = 16;
			result->netSurfaceMeshTextureHeight = 16;
		}
		break;
		}
	}
	break;
	}

	if (result != NULL)
		result->lod = lodNumber;

	return result;
}