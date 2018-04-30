

#include "stdafx.h"

#include "utility.h"

#include "../geometry/TrianglePolyhedron.h"

#define EdgeLengthComparisonTolerance 10E-5
#define RotationAxisVectorMagnitudeTolerance 10E-7
#define PointCoincidenceTolerance 10E-4
#define AngleComparisonTolerance 10E-9


namespace gaia3d
{
	void GeometryUtility::calculatePlaneNormal(double& x0, double& y0, double& z0,
												double& x1, double& y1, double& z1,
												double& x2, double& y2, double& z2,
												double& nX, double& nY, double& nZ,
												bool bNormalize)
	{
		double edgeX1 = x1 - x0, edgeY1 = y1 - y0, edgeZ1 = z1 - z0;
		double edgeX2 = x2 - x0, edgeY2 = y2 - y0, edgeZ2 = z2 - z0;

		crossProduct(edgeX1, edgeY1, edgeZ1, edgeX2, edgeY2, edgeZ2, nX, nY, nZ);

		if(bNormalize)
		{
			double magnitude = sqrt(nX*nX + nY*nY + nZ*nZ);
			nX /= magnitude;
			nY /= magnitude;
			nZ /= magnitude;
		}
	}

	void GeometryUtility::crossProduct(double x1, double y1, double z1,
										double x2, double y2, double z2,
										double& nX, double& nY, double& nZ)
	{
		nX = y1*z2 - z1*y2;
		nY = z1*x2 - x1*z2;
		nZ = x1*y2 - y1*x2;
	}

	bool GeometryUtility::areTwoCongruentWithEachOther(void* geom1, void* geom2, void* transform, double tolerance, GeomType geomType)
	{
		// TODO(khj 20170317) : 나중에 geometry 타입의 종속성이 없도록 제거해야 한다.
		switch(geomType)
		{
		case POLYHEDRON:
			{
				TrianglePolyhedron* candidate = (gaia3d::TrianglePolyhedron*)geom1;
				TrianglePolyhedron* original = (gaia3d::TrianglePolyhedron*)geom2;

				// check if both vertex counts are same with each other
				size_t candidateVertexCount = candidate->getVertices().size();
				size_t originalVertexCount = original->getVertices().size();
				if(candidateVertexCount != originalVertexCount)
					return false;

				// check if two triangles composed of first three vertices in both polyhedrons respectively are congruent with each other
				Triangle tr1, tr2;
				Vertex v11, v12, v13, v21, v22, v23;
				v11.position = candidate->getVertices()[0]->position;
				v12.position = candidate->getVertices()[1]->position;
				v13.position = candidate->getVertices()[2]->position;
				v21.position = original->getVertices()[0]->position;
				v22.position = original->getVertices()[1]->position;
				v23.position = original->getVertices()[2]->position;
				tr1.setVertices(&v11, &v12, &v13);
				tr2.setVertices(&v21, &v22, &v23);
				if(!areTwoCongruentWithEachOther(&tr1, &tr2, NULL, EdgeLengthComparisonTolerance, TRIANGLE))
					return false;

				// check if we can make the first triangle of first geom coincide with that of second geom
				// by two rotational transforms and one translational transform
				Point3D rotAxis1, rotAxis2;
				double angle1, angle2; // rotational angles
				bool isFirstAxisZero = true, isSecondAxisZero = true;
				Matrix4 rotMat1, rotMat2;

				// first rotation axis for first rotational transform is made by cross product between first edges in both triangles.
				Point3D firstEdgeVec1, firstEdgeVec2;
				firstEdgeVec1.set(tr1.getVertices()[1]->position.x - tr1.getVertices()[0]->position.x,
								tr1.getVertices()[1]->position.y - tr1.getVertices()[0]->position.y,
								tr1.getVertices()[1]->position.z - tr1.getVertices()[0]->position.z);

				firstEdgeVec2.set(tr2.getVertices()[1]->position.x - tr2.getVertices()[0]->position.x,
								tr2.getVertices()[1]->position.y - tr2.getVertices()[0]->position.y,
								tr2.getVertices()[1]->position.z - tr2.getVertices()[0]->position.z);

				rotAxis1 = firstEdgeVec1 ^ firstEdgeVec2;
				double magnitude1 = rotAxis1.magnitude();
				if( magnitude1 >= RotationAxisVectorMagnitudeTolerance || magnitude1 <= -RotationAxisVectorMagnitudeTolerance)
					isFirstAxisZero = false;

				if (isFirstAxisZero)
				{
					// check if angle is zero or 180 degrees
					if (firstEdgeVec1.x * firstEdgeVec2.x + firstEdgeVec1.y*firstEdgeVec2.y + firstEdgeVec1.z*firstEdgeVec2.z > 0)
						rotMat1.identity();
					else
					{
						// at this case(angle == 180), set a vector which is parallel to triangle1's normal and goes through vertex1 of triangle1 as rot axis
						Point3D tri1Normal;
						calculatePlaneNormal(v11.position.x, v11.position.y, v11.position.z,
											v12.position.x, v12.position.y, v12.position.z,
											v13.position.x, v13.position.y, v13.position.z,
											tri1Normal.x, tri1Normal.y, tri1Normal.z, true);
						rotMat1.rotation(M_PI, &tri1Normal);
					}
				}
				else
				{
					rotAxis1.normalize();
					angle1 = angleBetweenTwoVectors(firstEdgeVec2.x, firstEdgeVec2.y, firstEdgeVec2.z, firstEdgeVec1.x, firstEdgeVec1.y, firstEdgeVec1.z);
					rotMat1.rotation(angle1, &rotAxis1);
				}

				// rotate second triangle. then first edges in both triangles will be parallel to each other.
				tr2.getVertices()[0]->position = rotMat1 * tr2.getVertices()[0]->position;
				tr2.getVertices()[1]->position = rotMat1 * tr2.getVertices()[1]->position;
				tr2.getVertices()[2]->position = rotMat1 * tr2.getVertices()[2]->position;

				// second rotation axis for second rotational transform is made by cross product between normal vectors of both triangles.
				calculatePlaneNormal(tr1.getVertices()[0]->position.x, tr1.getVertices()[0]->position.y, tr1.getVertices()[0]->position.z,
									tr1.getVertices()[1]->position.x, tr1.getVertices()[1]->position.y, tr1.getVertices()[1]->position.z,
									tr1.getVertices()[2]->position.x, tr1.getVertices()[2]->position.y, tr1.getVertices()[2]->position.z,
									tr1.getNormal()->x, tr1.getNormal()->y, tr1.getNormal()->z,
									true);
				calculatePlaneNormal(tr2.getVertices()[0]->position.x, tr2.getVertices()[0]->position.y, tr2.getVertices()[0]->position.z,
									tr2.getVertices()[1]->position.x, tr2.getVertices()[1]->position.y, tr2.getVertices()[1]->position.z,
									tr2.getVertices()[2]->position.x, tr2.getVertices()[2]->position.y, tr2.getVertices()[2]->position.z,
									tr2.getNormal()->x, tr2.getNormal()->y, tr2.getNormal()->z,
									true);
				Point3D* normal1 = tr1.getNormal();
				Point3D* normal2 = tr2.getNormal();
				rotAxis2 = *normal1 ^ *normal2;

				double magnitude2 = rotAxis2.magnitude();
				if(magnitude2 >= RotationAxisVectorMagnitudeTolerance || magnitude2 <= -RotationAxisVectorMagnitudeTolerance)
					isSecondAxisZero = false;

				if (isSecondAxisZero)
				{
					// check if angle is zero or 180 degrees
					if(normal1->x * normal2->x + normal1->y * normal2->y + normal1->z * normal2->z > 0)
						rotMat2.identity();
					else
					{
						// at this case(angle = 180), set the first edge of triangle1 as a rot axis
						firstEdgeVec1.normalize();
						rotMat2.rotation(M_PI, &firstEdgeVec1);
					}
				}
				else
				{
					rotAxis2.normalize();
					angle2 = angleBetweenTwoVectors(normal2->x, normal2->y, normal2->z, normal1->x, normal1->y, normal1->z);
					rotMat2.rotation(angle2, &rotAxis2);
				}
					

				// rotate second triangle. then the plane of second triangle will be parallel to that of first triangle.
				tr2.getVertices()[0]->position = rotMat2 * tr2.getVertices()[0]->position;
				tr2.getVertices()[1]->position = rotMat2 * tr2.getVertices()[1]->position;
				tr2.getVertices()[2]->position = rotMat2 * tr2.getVertices()[2]->position;

				// at this time, two triangles are parallel to each other.
				// let's find translational transform to make second trangle coincide with first triangle.
				Point3D transVec = tr1.getVertices()[0]->position - tr2.getVertices()[0]->position;
				Matrix4 transMat, *finalTransform;
				transMat.translation(&transVec);
				finalTransform = (Matrix4*)transform;

				*finalTransform = (rotMat1 * rotMat2) * transMat; // final transform matrix

				// finally, check if all original mesh vertices multiplied by final transform coincide with all candidate mesh vertices
				Point3D translatedPoint;
				for(size_t i = 3; i < originalVertexCount; i++)
				{
					translatedPoint = *finalTransform * (original->getVertices()[i]->position);
					if(!areTwoCongruentWithEachOther(&(candidate->getVertices()[i]->position), &translatedPoint, NULL, PointCoincidenceTolerance, POINT))
						return false;
				}
			}
			break;
		case TRIANGLE:
			{
				gaia3d::Triangle* tr1 = (gaia3d::Triangle*)geom1;
				gaia3d::Triangle* tr2 = (gaia3d::Triangle*)geom2;

				double edgeLength1[3], edgeLength2[3];
				edgeLength1[0] = sqrt(tr1->getVertices()[0]->position.squaredDistanceTo(tr1->getVertices()[1]->position));
				edgeLength1[1] = sqrt(tr1->getVertices()[1]->position.squaredDistanceTo(tr1->getVertices()[2]->position));
				edgeLength1[2] = sqrt(tr1->getVertices()[2]->position.squaredDistanceTo(tr1->getVertices()[0]->position));
				edgeLength2[0] = sqrt(tr2->getVertices()[0]->position.squaredDistanceTo(tr2->getVertices()[1]->position));
				edgeLength2[1] = sqrt(tr2->getVertices()[1]->position.squaredDistanceTo(tr2->getVertices()[2]->position));
				edgeLength2[2] = sqrt(tr2->getVertices()[2]->position.squaredDistanceTo(tr2->getVertices()[0]->position));

				if( edgeLength1[0] >= edgeLength2[0] + tolerance || edgeLength1[0] <= edgeLength2[0] - tolerance ||
					edgeLength1[1] >= edgeLength2[1] + tolerance || edgeLength1[1] <= edgeLength2[1] - tolerance ||
					edgeLength1[2] >= edgeLength2[2] + tolerance || edgeLength1[2] <= edgeLength2[2] - tolerance )
					return false;
			}
			break;
		case POINT:
			{
				Point3D* point1 = (Point3D*)geom1;
				Point3D* point2 = (Point3D*)geom2;

				if( point1->x >= point2->x + tolerance || point1->x <= point2->x - tolerance ||
					point1->y >= point2->y + tolerance || point1->y <= point2->y - tolerance ||
					point1->z >= point2->z + tolerance || point1->z <= point2->z - tolerance )
					return false;

			}
			break;
		}

		return true;
	}

	double GeometryUtility::angleBetweenTwoVectors(double x1, double y1, double z1, double x2, double y2, double z2)
	{
		double angle;
		double dotProduct, dotProductMax;

		dotProductMax= sqrt(x1*x1 + y1*y1 + z1*z1)*sqrt(x2*x2 + y2*y2 + z2*z2);
		angle=0.0;
		if(dotProductMax >= AngleComparisonTolerance || dotProductMax <= -AngleComparisonTolerance)
		{
			dotProduct= x1*x2 + y1*y2+ z1*z2;
			if(dotProduct/dotProductMax<=-1.0) angle=M_PI;
			else
			{
				if(dotProduct/dotProductMax>=1.0) angle=0.0;
				else angle = acos(dotProduct/dotProductMax);
			}
		}

		return angle;
	}

	bool GeometryUtility::isInsideBox(double x, double y, double z,
									double minX, double minY, double minZ,
									double maxX, double maxY, double maxZ)
	{
		if( x <= minX || x >= maxX || y <= minY || y >= maxY || z <= minZ || z >= maxZ)
			return false;

		return true;
	}

	bool GeometryUtility::doesTriangleIntersectWithBox(double& x0, double& y0, double& z0,
														double& x1, double& y1, double& z1,
														double& x2, double& y2, double& z2,
														double& minX, double& minY, double& minZ,
														double& maxX, double& maxY, double& maxZ)
	{
		// SAT(Separating Axis Test)

		// 1. 3 axes projection test
		double triMaxX = (x0 > x1) ? ((x0 > x2) ? x0 : x2) : ((x1 > x2) ? x1 : x2);
		double triMaxY = (y0 > y1) ? ((y0 > y2) ? y0 : y2) : ((y1 > y2) ? y1 : y2);
		double triMaxZ = (z0 > z1) ? ((z0 > z2) ? z0 : z2) : ((z1 > z2) ? z1 : z2);
		double triMinX = (x0 > x1) ? ((x1 > x2) ? x2 : x1) : ((x0 > x2) ? x2 : x0);
		double triMinY = (y0 > y1) ? ((y1 > y2) ? y2 : y1) : ((y0 > y2) ? y2 : y0);
		double triMinZ = (z0 > z1) ? ((z1 > z2) ? z2 : z1) : ((z0 > z2) ? z2 : z0);

		double tolerance = 0.000001;
		if(triMaxX < minX - tolerance || triMinX > maxX + tolerance ||
			triMaxY < minY - tolerance || triMinY > maxY + tolerance ||
			triMaxZ < minZ - tolerance || triMinZ > maxZ + tolerance)
			return false;

		// 2. box projection on triangle normal test
		// 2-1. plane equation of triangle(ax + by + cz + d = 0)
		double a, b, c, d;
		calculatePlaneNormal(x0, y0, z0, x1, y1, z1, x2, y2, z2, a, b, c, true);
		d = -a*x1 -b*y1 -c*z1;
		// 2-2. plane equation tests for 8 corners of box
		double test[8];
		test[0] = a*minX + b*minY + c*minZ + d;
		test[1] = a*maxX + b*minY + c*minZ + d;
		test[2] = a*maxX + b*maxY + c*minZ + d;
		test[3] = a*minX + b*maxY + c*minZ + d;
		test[4] = a*minX + b*minY + c*maxZ + d;
		test[5] = a*maxX + b*minY + c*maxZ + d;
		test[6] = a*maxX + b*maxY + c*maxZ + d;
		test[7] = a*minX + b*maxY + c*maxZ + d;
		bool testPassed = false;
		char countTouchedPoint = 0;
		if (test[0] >= -10E-6 && test[0] <= 10E-6)
			countTouchedPoint++;

		for(int i = 1; i < 8; i++)
		{
			if(test[0]*test[i] < -10E-8)
			{
				testPassed = true;
				break;
			}

			if (test[i] >= -10E-6 && test[i] <= 10E-6)
			{
				countTouchedPoint++;
				if (countTouchedPoint > 2)
				{
					testPassed = true;
					break;
				}
			}
		}

		if(!testPassed)
			return false;

		// 3. test on 9 axes made by cross products between 3 axis vectors and 3 edge vectors of triangle
		// 3-1. edge vectors of triangle
		double edge[3][3];
		edge[0][0] = x1-x0, edge[0][1] = y1-y0, edge[0][2] = z1-z0;
		edge[1][0] = x2-x1, edge[1][1] = y2-y1, edge[1][2] = z2-z1;
		edge[2][0] = x0-x2, edge[2][1] = y0-y2, edge[2][2] = z0-z2;

		// 3-2. 9 axes made by cross product of 3 global axis vectors and 3 edge vectors
		double axis[3][3][3];
		/*
		axis[0][0][0] = 0.0, axis[0][0][1] = -edge[0][2], axis[0][0][2] = edge[0][1];
		axis[0][1][0] = 0.0, axis[0][1][1] = -edge[1][2], axis[0][1][2] = edge[1][1];
		axis[0][2][0] = 0.0, axis[0][2][1] = -edge[2][2], axis[0][2][2] = edge[2][1];

		axis[1][0][0] = edge[0][2], axis[1][0][1] = 0.0, axis[1][0][2] = -edge[0][0];
		axis[1][1][0] = edge[1][2], axis[1][1][1] = 0.0, axis[1][1][2] = -edge[1][0];
		axis[1][2][0] = edge[2][2], axis[1][2][1] = 0.0, axis[1][2][2] = -edge[2][0];

		axis[2][0][0] = -edge[0][1], axis[2][0][1] = edge[0][0], axis[2][0][2] = 0.0;
		axis[2][1][0] = -edge[1][1], axis[2][1][1] = edge[1][0], axis[2][1][2] = 0.0;
		axis[2][2][0] = -edge[2][1], axis[2][2][1] = edge[2][0], axis[2][2][2] = 0.0;
		*/

		// 3 global axis vector loop
		for(int i = 0; i < 3; i++)
		{
			// 3 edge vectors loop
			for(int j = 0; j < 3; j++)
			{
				// x, y, z loop
				for(int k = 0; k < 3; k++)
				{
					if(i == k)
						axis[i][j][k] = 0.0;
					else
					{
						if((i+1)%3 == k)
							axis[i][j][k] = -edge[j][(k+1)%3];
						else
							axis[i][j][k] = edge[j][(i+1)%3];
					}
				}
			}
		}

		// 3-3. center of box
		double cobX = (maxX+minX)/2.0, cobY = (maxY+minY)/2.0, cobZ = (maxZ+minZ)/2.0;

		// 3-4. translated 3 vertices of triangle by vector which translate cob into origin
		double v[3][3];
		v[0][0] = x0 - cobX, v[0][1] = y0 - cobY, v[0][2] = z0 - cobZ;
		v[1][0] = x1 - cobX, v[1][1] = y1 - cobY, v[1][2] = z1 - cobZ;
		v[2][0] = x2 - cobX, v[2][1] = y2 - cobY, v[2][2] = z2 - cobZ;

		// 3-5. half diagonal vector of box
		double hd[3];
		hd[0] = (maxX-minX)/2.0, hd[1] = (maxY-minY)/2.0, hd[2] = (maxZ-minZ)/2.0;

		double radius;
		// first 3 loop of 9 axes
		double vProj1, vProj2;
		for(int i = 0; i < 3; i++)
		{
			// second 3 loop of 9 axes
			for(int j = 0; j < 3; j++)
			{
				radius = hd[(i+1)%3]*fabs(axis[i][j][(i+1)%3]) + hd[(i+2)%3]*fabs(axis[i][j][(i+2)%3]);

				vProj1 = v[j][(i+2)%3]*v[(j+1)%3][(i+1)%3] - v[j][(i+1)%3]*v[(j+1)%3][(i+2)%3];
				vProj2 = (v[(j+1)%3][(i+1)%3] - v[j][(i+1)%3])*v[(j+2)%3][(i+2)%3] - (v[(j+1)%3][(i+2)%3] -v[j][(i+2)%3])*v[(j+2)%3][(i+1)%3];

				if( (vProj1 > vProj2) ? (vProj1 < -radius -10E-8 || vProj2 > radius + 10E-8) : (vProj2 < -radius -10E-8 || vProj1 > radius + 10E-8) )
					return false;
			}
		}

		return true;
	}

#ifdef _WIN32
#include <Windows.h>
#endif
	std::string StringUtility::convertWideStringToUtf8(std::wstring& sourceString)
	{
#ifdef _WIN32
		int neededLength = WideCharToMultiByte(CP_UTF8, 0, sourceString.c_str(), (int)sourceString.size(), NULL, 0, NULL, NULL);
		char* receiver = new char[neededLength + 1];
		memset(receiver, 0x00, sizeof(char)*(neededLength + 1));
		WideCharToMultiByte(CP_UTF8, 0, sourceString.c_str(), (int)sourceString.size(), receiver, neededLength, NULL, NULL);
		std::string newString(receiver);
		delete[] receiver;
		return newString;
#else
		std::string newString(sourceString.begin(), sourceString.end());
		return newString;
#endif
	}

	std::wstring StringUtility::convertUtf8ToWideString(std::string& sourceString)
	{
#ifdef _WIN32
		int neededLength = 0;
		neededLength = MultiByteToWideChar(CP_UTF8, 0, sourceString.c_str(), -1, NULL, 0);
		wchar_t* receiver = new wchar_t[neededLength+1];
		memset(receiver, 0x00, sizeof(wchar_t)*(neededLength+1));
		MultiByteToWideChar(CP_UTF8, 0, sourceString.c_str(), -1, receiver, neededLength);
		std::wstring newString(receiver);
		delete[] receiver;
		return newString;
#else
		std::wstring newString(sourceString.begin(), sourceString.end());
		return newString;
#endif
	}

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../util/stb_image_write.h"
	void ImageUtility::writeMemoryImageToFile(unsigned char* buffer, int width, int height, const char* fullPath)
	{
		stbi_write_jpg(fullPath, width, height, 4, buffer, 0);
	}
}


