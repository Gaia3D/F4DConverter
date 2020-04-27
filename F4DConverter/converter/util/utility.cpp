

#include "stdafx.h"

#include "utility.h"

#include "../geometry/TrianglePolyhedron.h"

///< micrometer까지 지원하기 때문에
#define EdgeLengthComparisonTolerance 10E-5
///< magnitude가 작은 벡터를 구분할 때 쓴다. 
#define RotationAxisVectorMagnitudeTolerance 10E-7
///< 두 점이 같은가?
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

	void GeometryUtility::wgs84ToAbsolutePosition(double&lon, double& lat, double& alt, double* result)
	{
		//WGS 84.***************************************************
		// Extracted from WikiPedia "Geodetic datum".
		// WGS 84 Defining Parameters
		// semi-major axis	a	6378137.0 m
		// Reciprocal of flattening	1/f	298.257223563

		// WGS 84 derived geometric constants
		// Semi-minor axis	b = a(1 − f)	6356752.3142 m
		// First eccentricity squared	e2 = (1 − b2/a2 = 2f − f2) =	6.69437999014 x 10−3
		// Second eccentricity squared	e′2	= (a2/b2 − 1 = f(2 − f)/(1 − f)2) = 6.73949674228 x 10−3
		//----------------------------------------------------------

		// defined in the LINZ standard LINZS25000 (Standard for New Zealand Geodetic Datum 2000)
		// https://www.linz.govt.nz/data/geodetic-system/coordinate-conversion/geodetic-datum-conversions/equations-used-datum
		// a = semi-major axis.
		// e2 = firstEccentricitySquared.
		// v = a / sqrt(1 - e2 * sin2(lat)).
		// x = (v+h)*cos(lat)*cos(lon).
		// y = (v+h)*cos(lat)*sin(lon).
		// z = [v*(1-e2)+h]*sin(lat).

		double degToRadFactor = 0.017453292519943296; // 3.141592653589793 / 180.0;
		double equatorialRadius = 6378137.0;
		double firstEccentricitySquared = 6.69437999014E-3;
		double lonRad = lon *degToRadFactor;
		double latRad = lat * degToRadFactor;
		double cosLon = cos(lonRad);
		double cosLat = cos(latRad);
		double sinLon = sin(lonRad);
		double sinLat = sin(latRad);
		double a = equatorialRadius;
		double e2 = firstEccentricitySquared;
		double v = a / sqrt(1.0 - e2 * sinLat * sinLat);
		double h = alt;

		result[0] = (v + h)*cosLat*cosLon;
		result[1] = (v + h)*cosLat*sinLon;
		result[2] = (v*(1.0 - e2) + h)*sinLat;
	}

	void GeometryUtility::normalAtAbsolutePosition(double& x, double& y, double& z, double* result)
	{
		gaia3d::Point3D normal;

		double equatorialRadiusSquared = 40680631590769.0;
		double polarRadiusSquared = 40408299984087.05552164;

		normal.set(x / equatorialRadiusSquared, y / equatorialRadiusSquared, z / polarRadiusSquared);
		normal.normalize();

		result[0] = normal.x;
		result[1] = normal.y;
		result[2] = normal.z;
	}

	void GeometryUtility::transformMatrixAtAbsolutePosition(double& x, double& y, double& z, double* m)
	{
		gaia3d::Matrix4 matrix;

		gaia3d::Point3D xAxis, yAxis, zAxis;

		double normal[3];
		normalAtAbsolutePosition(x, y, z, normal);
		zAxis.set(normal[0], normal[1], normal[2]);

		xAxis.set(-y, x, 0.0);
		xAxis.normalize();

		yAxis = zAxis ^ xAxis;
		yAxis.normalize();

		matrix.set(	xAxis.x,	xAxis.y,	xAxis.z,	0.0,
					yAxis.x,	yAxis.y,	yAxis.z,	0.0,
					zAxis.x,	zAxis.y,	zAxis.z,	0.0,
					x,			y,			z,			1.0);

		matrix.getDoubleArray(m);
	}

	void findConcavePointsAndNormal(double* pxs, double* pys,
									std::vector<size_t>& indices,
									std::vector<size_t>& concavePointIndicesOnAllPoints,
									std::vector<size_t>& concavePointIndicesOnThisPolygon,
									int& normal)
	{
		double crossProd, dotProd, angle;
		size_t count = indices.size();
		size_t prevIndex, nextIndex;
		gaia3d::Point3D prevVector, nextVector;
		double lfNormal = 0.0;
		double tolerance = 1E-6;
		for (size_t i = 0; i < count; i++)
		{
			prevIndex = (i == 0) ? count - 1 : i - 1;
			nextIndex = (i == count - 1) ? 0 : i + 1;

			prevVector.set(pxs[indices[i]] - pxs[indices[prevIndex]], pys[indices[i]] - pys[indices[prevIndex]], 0.0);
			nextVector.set(pxs[indices[nextIndex]] - pxs[indices[i]], pys[indices[nextIndex]] - pys[indices[i]], 0.0);

			if (!prevVector.normalize())
				continue;
			if (!nextVector.normalize())
				continue;

			crossProd = prevVector.x*nextVector.y - prevVector.y*nextVector.x;
			dotProd = prevVector.x * nextVector.x + prevVector.y * nextVector.y;
			if (crossProd > tolerance)
			{
				crossProd = 1.0;
			}
			else if (crossProd < -tolerance)
			{
				crossProd = -1.0;
				concavePointIndicesOnAllPoints.push_back(indices[i]);
				concavePointIndicesOnThisPolygon.push_back(i);
			}
			else
				continue;

			if (dotProd > 1.0)
				dotProd = 1.0;
			
			if (dotProd < -1.0)
				dotProd = -1.0;

			angle = acos(dotProd);
			
			lfNormal += (crossProd * angle);
		}

		normal = (lfNormal > tolerance) ? 1 :((lfNormal < -tolerance) ? -1 : 0);
	}

	bool intersectionTestOnTwoLineSegments( double x1Start, double y1Start, double x1End, double y1End,
											double x2Start, double y2Start, double x2End, double y2End)
	{
		gaia3d::Point3D normalVector;
		double dotProdLine1, dotProdLine2Start, dotProdLine2End;
		double dotProdLine2, dotProdLine1Start, dotProdLine1End;
		double tolerance = 1E-7;

		// find vector normal to line 1
		normalVector.set(-(y1End - y1Start), x1End - x1Start, 0.0);
		normalVector.normalize();

		// dot products of 4 nodes in 2 lines on this normal vector
		// (dot products of 2 nodes in line 1 on this normal vector are same)
		dotProdLine1 = normalVector.x * x1Start + normalVector.y * y1Start;
		dotProdLine2Start = normalVector.x * x2Start + normalVector.y * y2Start;
		dotProdLine2End = normalVector.x * x2End + normalVector.y * y2End;
		if ((dotProdLine1 > dotProdLine2Start - tolerance && dotProdLine1 > dotProdLine2End - tolerance) ||
			(dotProdLine1 < dotProdLine2Start + tolerance && dotProdLine1 < dotProdLine2End + tolerance))
			return false;

		// find vector normal to line 2
		normalVector.set(-(y2End - y2Start), x2End - x2Start, 0.0);
		normalVector.normalize();

		// dot products of 4 nodes in 2 lines on this normal vector
		// (dot products of 2 nodes in line 2 on this normal vector are same)
		dotProdLine2 = normalVector.x * x2Start + normalVector.y * y2Start;
		dotProdLine1Start = normalVector.x * x1Start + normalVector.y * y1Start;
		dotProdLine1End = normalVector.x * x1End + normalVector.y * y1End;
		if ((dotProdLine2 > dotProdLine1Start - tolerance && dotProdLine2 > dotProdLine1End - tolerance) ||
			(dotProdLine2 < dotProdLine1Start + tolerance && dotProdLine2 < dotProdLine1End + tolerance))
			return false;

		return true;
	}

	void tessellateIntoSubPolygons(double* pxs, double* pys,
									std::vector<size_t>& polygonVertexIndices,
									std::vector<size_t>& concavePointIndicesOnAllPoints,
									std::vector<size_t>& concavePointIndicesOnThisPolygon,
									int thisPolygonNormal,
									std::vector<std::vector<size_t>>& subPolygons)
	{
		// 0. find concave point to use it as starting of ear-cut, which DOESN't appear multple times in polygon point array.
		size_t concavePointIndexOnAllPoints = concavePointIndicesOnAllPoints[0];
		size_t concavePointIndexOnThisPolygon = concavePointIndicesOnThisPolygon[0];

		size_t polygonPointCount = polygonVertexIndices.size();
		gaia3d::Point3D concavePoint;
		concavePoint.set(pxs[concavePointIndexOnAllPoints], pys[concavePointIndexOnAllPoints], 0.0);

		// 1. sort all points by acsending distance from the concave point except the concave point and its 2 neighbor points.
		std::map<double, std::vector<size_t>> sortedPointIndicesOnAllPointsMap, sortedPointIndicesOnThisPolygonMap;
		size_t prevIndexOfConcavePointOnThisPolygon = (concavePointIndexOnThisPolygon == 0) ? polygonPointCount - 1 : concavePointIndexOnThisPolygon - 1;
		size_t nextIndexOfConcavePointOnThisPolygon = (concavePointIndexOnThisPolygon == polygonPointCount - 1) ? 0 : concavePointIndexOnThisPolygon + 1;
		gaia3d::Point3D targetPoint;
		double squaredDist;
		for (size_t i = 0; i < polygonPointCount; i++)
		{
			if(polygonVertexIndices[i] == concavePointIndexOnAllPoints ||
				polygonVertexIndices[(i+1)%polygonPointCount] == concavePointIndexOnAllPoints ||
				polygonVertexIndices[(i + polygonPointCount - 1) % polygonPointCount] == concavePointIndexOnAllPoints ||
				polygonVertexIndices[i] == polygonVertexIndices[prevIndexOfConcavePointOnThisPolygon] ||
				polygonVertexIndices[i] == polygonVertexIndices[nextIndexOfConcavePointOnThisPolygon])
				continue;

			targetPoint.set(pxs[polygonVertexIndices[i]], pys[polygonVertexIndices[i]], 0.0);
			squaredDist = targetPoint.squaredDistanceTo(concavePoint);
			if (sortedPointIndicesOnAllPointsMap.find(squaredDist) == sortedPointIndicesOnAllPointsMap.end())
			{
				sortedPointIndicesOnAllPointsMap[squaredDist] = std::vector<size_t>();
				sortedPointIndicesOnThisPolygonMap[squaredDist] = std::vector<size_t>();
			}

			sortedPointIndicesOnAllPointsMap[squaredDist].push_back(polygonVertexIndices[i]);
			sortedPointIndicesOnThisPolygonMap[squaredDist].push_back(i);
		}
		
		std::vector<size_t> sortedPointIndicesOnAllPoints;
		std::vector<size_t> sortedPointIndicesOnThisPolygon;
		for (std::map<double, std::vector<size_t>>::iterator itr1 = sortedPointIndicesOnAllPointsMap.begin(), itr2 = sortedPointIndicesOnThisPolygonMap.begin();
			itr1 != sortedPointIndicesOnAllPointsMap.end();
			itr1++, itr2++)
		{
			for (size_t i = 0; i < itr1->second.size(); i++)
			{
				sortedPointIndicesOnAllPoints.push_back((itr1->second)[i]);
				sortedPointIndicesOnThisPolygon.push_back((itr2->second)[i]);
			}
		}
			

		// 2. find a point of this polygon where line segment composed of the point and concave point can slices this polygon
		double slicerStartX = pxs[concavePointIndexOnAllPoints], slicerStartY = pys[concavePointIndexOnAllPoints];
		double slicerEndX, slicerEndY;
		double targetStartX, targetStartY, targetEndX, targetEndY;
		for (size_t i = 0; i < sortedPointIndicesOnAllPoints.size(); i++)
		{
			// 2.1 line interseciton test between the slicer and each edge of polygon
			slicerEndX = pxs[sortedPointIndicesOnAllPoints[i]];
			slicerEndY = pys[sortedPointIndicesOnAllPoints[i]];
			bool bIntersected = false;
			for (size_t j = 0; j < polygonPointCount; j++)
			{
				targetStartX = pxs[polygonVertexIndices[j]];
				targetStartY = pys[polygonVertexIndices[j]];
				targetEndX = pxs[polygonVertexIndices[(j + 1)% polygonPointCount]];
				targetEndY = pys[polygonVertexIndices[(j + 1)% polygonPointCount]];

				if (intersectionTestOnTwoLineSegments(  slicerStartX, slicerStartY, slicerEndX, slicerEndY,
														targetStartX, targetStartY, targetEndX, targetEndY  ) )
				{
					bIntersected = true;
					break;
				}
			}

			if (bIntersected)
				continue;

			// 2.2 test if the sliced 2 sub-polygons have same plane normal directions with original polygon
			std::vector<size_t> firstSubPolygonIndicesOnAllPoints;
			firstSubPolygonIndicesOnAllPoints.push_back(concavePointIndexOnAllPoints);
			firstSubPolygonIndicesOnAllPoints.push_back(polygonVertexIndices[sortedPointIndicesOnThisPolygon[i]]);
			for (size_t j = 1; j < polygonPointCount; j++)
			{
				firstSubPolygonIndicesOnAllPoints.push_back(polygonVertexIndices[(sortedPointIndicesOnThisPolygon[i] + j) % polygonPointCount]);
				if ((sortedPointIndicesOnThisPolygon[i] + j) % polygonPointCount == prevIndexOfConcavePointOnThisPolygon)
					break;
			}
			std::vector<size_t> secondSubPolygonIndicesOnAllPoints;
			secondSubPolygonIndicesOnAllPoints.push_back(polygonVertexIndices[sortedPointIndicesOnThisPolygon[i]]);
			secondSubPolygonIndicesOnAllPoints.push_back(concavePointIndexOnAllPoints);
			for (size_t j = 1; j < polygonPointCount; j++)
			{
				secondSubPolygonIndicesOnAllPoints.push_back(polygonVertexIndices[(concavePointIndexOnThisPolygon + j) % polygonPointCount]);
				if ((concavePointIndexOnThisPolygon + j) % polygonPointCount ==
					(sortedPointIndicesOnThisPolygon[i] + polygonPointCount - 1) % polygonPointCount)
					break;
			}
	
			int firstPolygonNormal, secondPolygonNormal;
			std::vector<size_t> concavePointsOfSubPolygon1IndicesOnAllPoints, concavePointsOfSubPolygon1IndicesOnSubPolygon1;
			std::vector<size_t> concavePointsOfSubPolygon2IndicesOnAllPoints, concavePointsOfSubPolygon2IndicesOnSubPolygon2;

			findConcavePointsAndNormal(pxs, pys,
										firstSubPolygonIndicesOnAllPoints,
										concavePointsOfSubPolygon1IndicesOnAllPoints,
										concavePointsOfSubPolygon1IndicesOnSubPolygon1,
										firstPolygonNormal);
			findConcavePointsAndNormal(pxs, pys,
										secondSubPolygonIndicesOnAllPoints,
										concavePointsOfSubPolygon2IndicesOnAllPoints,
										concavePointsOfSubPolygon2IndicesOnSubPolygon2,
										secondPolygonNormal);
			if (thisPolygonNormal != firstPolygonNormal || thisPolygonNormal != secondPolygonNormal)
				continue;

			// 2.3 make sub-polygons or tessellate more
			if (concavePointsOfSubPolygon1IndicesOnAllPoints.empty())
				subPolygons.push_back(firstSubPolygonIndicesOnAllPoints);
			else
				tessellateIntoSubPolygons(pxs, pys,
										firstSubPolygonIndicesOnAllPoints,
										concavePointsOfSubPolygon1IndicesOnAllPoints,
										concavePointsOfSubPolygon1IndicesOnSubPolygon1,
										firstPolygonNormal,
										subPolygons);

			if (concavePointsOfSubPolygon2IndicesOnAllPoints.empty())
				subPolygons.push_back(secondSubPolygonIndicesOnAllPoints);
			else
				tessellateIntoSubPolygons(pxs, pys,
										secondSubPolygonIndicesOnAllPoints,
										concavePointsOfSubPolygon2IndicesOnAllPoints,
										concavePointsOfSubPolygon2IndicesOnSubPolygon2,
										secondPolygonNormal,
										subPolygons);
			break;
		}
	}

	void GeometryUtility::tessellate(double* xs, double* ys, double* zs, size_t vertexCount, std::vector<size_t>& polygonIndices, std::vector<size_t>& indices)
	{
		// 0. basic validation
		size_t count = polygonIndices.size();
		if (count < 3)
			return;

		if (count == 3)
		{
			indices.push_back(polygonIndices[0]);
			indices.push_back(polygonIndices[1]);
			indices.push_back(polygonIndices[2]);
			return;
		}

		// 1. calculate normal of this polygon
		gaia3d::Point3D normal, crossProd, prevVector, nextVector;
		double dotProd, angle;
		size_t prevIndex, curIndex, nextIndex;
		normal.set(0.0, 0.0, 0.0);
		for (size_t i = 0; i < count; i++)
		{
			prevIndex = (i == 0) ? polygonIndices[count - 1] : polygonIndices[i - 1];
			curIndex = polygonIndices[i];
			nextIndex = (i == count - 1) ? polygonIndices[0] : polygonIndices[i + 1];

			prevVector.set(xs[curIndex] - xs[prevIndex], ys[curIndex] - ys[prevIndex], zs[curIndex] - zs[prevIndex]);
			nextVector.set(xs[nextIndex] - xs[curIndex], ys[nextIndex] - ys[curIndex], zs[nextIndex] - zs[curIndex]);

			if (!prevVector.normalize())
				continue;

			if (!nextVector.normalize())
				continue;

			crossProd = prevVector ^ nextVector;
			if (!crossProd.normalize())
				continue;

			dotProd = prevVector.x * nextVector.x + prevVector.y * nextVector.y + prevVector.z * nextVector.z;
			angle = acos(dotProd);

			normal += (crossProd * angle);
		}

		if (!normal.normalize())
			return;

		// 2. make projected polygon
		unsigned char projectionType; // 0 : onto x-y plane, 1 : onto y-z plane, 2 : onto z-x plane
		double nx = abs(normal.x);
		double ny = abs(normal.y);
		double nz = abs(normal.z);

		projectionType = (nz > nx) ? ((nz > ny) ? 0 : 2) : ((nx > ny) ? 1 : 2);
		double* pxs = new double[vertexCount];
		memset(pxs, 0x00, sizeof(double)*vertexCount);
		double* pys = new double[vertexCount];
		memset(pys, 0x00, sizeof(double)*vertexCount);

		switch (projectionType)
		{
		case 0:
		{
			if (normal.z > 0)
			{
				for (size_t i = 0; i < vertexCount; i++)
				{
					pxs[i] = xs[i] - xs[0];
					pys[i] = ys[i] - ys[0];
				}
			}
			else
			{
				for (size_t i = 0; i < vertexCount; i++)
				{
					pxs[i] = xs[i];
					pys[i] = -ys[i];
				}
			}
		}
		break;
		case 1:
		{
			if (normal.x > 0)
			{
				for (size_t i = 0; i < vertexCount; i++)
				{
					pxs[i] = ys[i];
					pys[i] = zs[i];
				}
			}
			else
			{
				for (size_t i = 0; i < vertexCount; i++)
				{
					pxs[i] = ys[i];
					pys[i] = -zs[i];
				}
			}
		}
		break;
		case 2:
		{
			if (normal.y > 0)
			{
				for (size_t i = 0; i < vertexCount; i++)
				{
					pxs[i] = zs[i];
					pys[i] = xs[i];
				}
			}
			else
			{
				for (size_t i = 0; i < vertexCount; i++)
				{
					pxs[i] = zs[i];
					pys[i] = -xs[i];
				}
			}
		}
		break;
		}

		// 3. find concave points and normal of this projected polygon
		std::vector<size_t> concavePointIndicesOnAllPoints;
		std::vector<size_t> concavePointIndicesOnThisPolygon;
		int projectedPolygonNormal;
		std::vector<size_t> polygonVertexIndices;
		for (size_t i = 0; i < count; i++)
			polygonVertexIndices.push_back(polygonIndices[i]);

		findConcavePointsAndNormal(pxs, pys, polygonVertexIndices, concavePointIndicesOnAllPoints, concavePointIndicesOnThisPolygon, projectedPolygonNormal);

		if (concavePointIndicesOnAllPoints.empty())
		{
			for (size_t i = 1; i < count - 1; i++)
			{
				indices.push_back(polygonIndices[0]);
				indices.push_back(polygonIndices[i]);
				indices.push_back(polygonIndices[i + 1]);
			}

			delete[] pxs;
			delete[] pys;
			return;
		}

		// 4. split this polygon into convex sub-polygons
		std::vector<std::vector<size_t>> subPolygons;
		tessellateIntoSubPolygons(pxs, pys, polygonVertexIndices, concavePointIndicesOnAllPoints, concavePointIndicesOnThisPolygon, projectedPolygonNormal, subPolygons);

		delete[] pxs;
		delete[] pys;

		// 5. split sub-polygons into triangles
		for (size_t i = 0; i < subPolygons.size(); i++)
		{
			for (size_t j = 1; j < (subPolygons[i]).size() - 1; j++)
			{
				indices.push_back((subPolygons[i])[0]);
				indices.push_back((subPolygons[i])[j]);
				indices.push_back((subPolygons[i])[j+1]);
			}
		}
	}

	void find2DPlaneNormal(double* pxs, double* pys, size_t count, int& normal) {

		double crossProdResult, dotProdResult, angle;
		size_t prevIndex, nextIndex;
		Point3D prevVector, nextVector;
		double lfNormal = 0.0;
		double tolerance = 1E-7;
		for (size_t i = 0; i < count; i++)
		{
			prevIndex = (i == 0) ? count - 1 : i - 1;
			nextIndex = (i == count - 1) ? 0 : i + 1;

			prevVector.set(pxs[i] - pxs[prevIndex], pys[i] - pys[prevIndex], 0.0);
			nextVector.set(pxs[nextIndex] - pxs[i], pys[nextIndex] - pys[i], 0.0);

			if (!prevVector.normalize())
				continue;
			if (!nextVector.normalize())
				continue;

			crossProdResult = prevVector.x * nextVector.y - prevVector.y * nextVector.x;
			dotProdResult = prevVector.x * nextVector.x + prevVector.y * nextVector.y;
			if (crossProdResult > tolerance)
			{
				crossProdResult = 1.0;
			}
			else if (crossProdResult < -tolerance)
			{
				crossProdResult = -1.0;

			}
			else
				continue;

			if (dotProdResult > 1.0)
				dotProdResult = 1.0;

			if (dotProdResult < -1.0)
				dotProdResult = -1.0;

			angle = acos(dotProdResult);

			lfNormal += (crossProdResult * angle);
		}

		normal = (lfNormal > 0.0) ? 1 : -1;
	}

	void getSortedRingsByDistFromPointAndMarkedIndices(double** px, double** py,
												std::vector<size_t>& eachSizeOfRing,
												double targetx, double targety,
												std::vector<std::pair<size_t, size_t>>& indices,
												bool bDebug)
	{
		//1. sort rings by distance from the reference point to their points
		// and mark the index of each ring where minimum distance happens 
		size_t count = eachSizeOfRing.size();
		std::map<double, std::vector<std::pair<size_t, size_t>> > sortedRingList;
		for (size_t i = 0; i < count; i++)
		{
			std::map<double, std::vector<size_t>> sortedPointListByDistance;
			double tempx, tempy, squaredDist;
			for (size_t j = 0; j < eachSizeOfRing[i]; j++)
			{
				if (targetx == px[i][j] && targety == py[i][j])
				{
					if (sortedPointListByDistance.find(0.0) == sortedPointListByDistance.end())
						sortedPointListByDistance[0.0] = std::vector<size_t>();

					sortedPointListByDistance[0.0].push_back(j);
					continue;
				}

				tempx = px[i][j];
				tempy = py[i][j];
				squaredDist = (targetx - tempx) * (targetx - tempx) + (targety - tempy) * (targety - tempy);
				if (sortedPointListByDistance.find(squaredDist) == sortedPointListByDistance.end())
					sortedPointListByDistance[squaredDist] = std::vector<size_t>();
				sortedPointListByDistance[squaredDist].push_back(j);
			}

			if (sortedRingList.find(sortedPointListByDistance.begin()->first) == sortedRingList.end())
				sortedRingList[sortedPointListByDistance.begin()->first] = std::vector<std::pair<size_t, size_t>>();

			sortedRingList[sortedPointListByDistance.begin()->first].push_back(std::pair<size_t, size_t>(i, (sortedPointListByDistance.begin()->second)[0]));
		}

		//2. push sorted result into the output container
		std::map<double, std::vector<std::pair<size_t, size_t>>>::iterator it = sortedRingList.begin();
		for (; it != sortedRingList.end(); it++)
		{
			for(size_t i = 0; i < it->second.size(); i++)
				indices.push_back((it->second)[i]);
		}
	}

	bool earCutHoleOfPolygon(double** pxs, double** pys, std::vector<size_t>& eachRingPointCount,
							size_t indexOfHoleToBeCut, size_t pointIndexOfCut, bool bReverseInnerRing,
							std::vector<std::pair<size_t, size_t>>& mergedOuterRing)
	{
		// pxs, pys : all 2D point coordinates of a outer ring and inner rings
		// eachRingPointCount : point count of all rings(outer ring index : 0, inner ring(hole) index : 1 ~ n-1)
		// bReverseInnerRings : whether this inner rings should be reversed or not
		// indexOfHoleToBeCut : target inner ring to be ear-cut
		// pointIndexOfCut : ear cut point of the target inner ring
		// outerRing : container which is initially filled with outer ring points and will be filled with ear-cut result finally

		// 1. sort points of outer ring by distance from the ear-cut point of target inner hole
		// with EXCLUSION of duplicated points
		std::vector<char> duplicationMarker;
		for (size_t i = 0; i < mergedOuterRing.size(); i++)
			duplicationMarker.push_back(-1);

		for (size_t i = 0; i < mergedOuterRing.size(); i++)
		{
			if (duplicationMarker[i] != -1)
				continue;

			for (size_t j = i+1; j < mergedOuterRing.size(); j++)
			{
				if (duplicationMarker[j] != -1)
					continue;

				if (mergedOuterRing[i].first == mergedOuterRing[j].first &&
					mergedOuterRing[i].second == mergedOuterRing[j].second)
				{
					duplicationMarker[i] = duplicationMarker[j] = 0;
				}
			}
		}
		std::map<double, std::vector<std::pair<size_t, size_t>>> sortedOuterRingPoints;
		std::map<double, std::vector<size_t>> sortedOuterRingPointIndices;
		double xInnerHoleToBeCut = pxs[indexOfHoleToBeCut][pointIndexOfCut], yInnerHoleToBeCut = pys[indexOfHoleToBeCut][pointIndexOfCut];
		double xOuterRing, yOuterRing;
		double squaredDist;
		for (size_t i = 0; i < mergedOuterRing.size(); i++)
		{
			if (duplicationMarker[i] == 0)
				continue;

			xOuterRing = pxs[mergedOuterRing[i].first][mergedOuterRing[i].second];
			yOuterRing = pys[mergedOuterRing[i].first][mergedOuterRing[i].second];

			squaredDist = (xOuterRing - xInnerHoleToBeCut)*(xOuterRing - xInnerHoleToBeCut) + (yOuterRing - yInnerHoleToBeCut)*(yOuterRing - yInnerHoleToBeCut);

			if (sortedOuterRingPoints.find(squaredDist) == sortedOuterRingPoints.end())
			{
				sortedOuterRingPoints[squaredDist] = std::vector<std::pair<size_t, size_t>>();
				sortedOuterRingPointIndices[squaredDist] = std::vector<size_t>();
			}

			sortedOuterRingPoints[squaredDist].push_back(mergedOuterRing[i]);
			sortedOuterRingPointIndices[squaredDist].push_back(i);
		}

		// 2. find a point on outer ring
		// where line segment composed of this point and point on the inner hole to be cut NEVER intersects with 
		// any edges of outer ring and all inner holes.
		std::map<double, std::vector<std::pair<size_t, size_t>>>::iterator iter1 = sortedOuterRingPoints.begin();
		std::map<double, std::vector<size_t>>::iterator iter2 = sortedOuterRingPointIndices.begin();
		bool bIntersected;
		double xToBeCutStart, yToBeCutStart;
		size_t pointIndexOfOuterRingToBeCut;
		bool bThisInnerHoleCanBeEarCut = false;
		for (; iter1 != sortedOuterRingPoints.end(); iter1++, iter2++)
		{
			
			for (size_t k = 0; k < iter1->second.size(); k++)
			{
				bIntersected = false;

				xToBeCutStart = pxs[(iter1->second)[k].first][(iter1->second)[k].second];
				yToBeCutStart = pys[(iter1->second)[k].first][(iter1->second)[k].second];

				for (size_t i = 0; i < mergedOuterRing.size(); i++)
				{
					size_t holeIndexCur = mergedOuterRing[i].first;
					size_t pointIndexCur = mergedOuterRing[i].second;
					size_t holeIndexNext = mergedOuterRing[(i + 1) % mergedOuterRing.size()].first;
					size_t pointIndexNext = mergedOuterRing[(i + 1) % mergedOuterRing.size()].second;

					if (intersectionTestOnTwoLineSegments(xToBeCutStart, yToBeCutStart, xInnerHoleToBeCut, yInnerHoleToBeCut,
						pxs[holeIndexCur][pointIndexCur], pys[holeIndexCur][pointIndexCur], pxs[holeIndexNext][pointIndexNext], pys[holeIndexNext][pointIndexNext]))
					{
						bIntersected = true;
						break;
					}
				}

				if (!bIntersected)
				{
					for (size_t i = 0; i < eachRingPointCount.size(); i++)
					{
						for (size_t j = 0; j < eachRingPointCount[i]; j++)
						{
							if (intersectionTestOnTwoLineSegments(xToBeCutStart, yToBeCutStart, xInnerHoleToBeCut, yInnerHoleToBeCut,
								pxs[i][j], pys[i][j], pxs[i][(j + 1) % eachRingPointCount[i]], pys[i][(j + 1) % eachRingPointCount[i]]))
							{
								bIntersected = true;
								break;
							}
						}

						if (bIntersected)
							break;
					}
				}

				if (!bIntersected)
				{
					pointIndexOfOuterRingToBeCut = (iter2->second)[k];
					bThisInnerHoleCanBeEarCut = true;
					break;
				}
			}

			if (bThisInnerHoleCanBeEarCut)
				break;
		}

		if (!bThisInnerHoleCanBeEarCut)
			return false;

		// 3. merge outer ring and the target inner hole
		// At this point, mergedOuterRing[pointIndexOfOuterRingToBeCut] is the point of outer ring to be ear cut
		
		std::vector<std::pair<size_t, size_t>> mergedResult;

		for (size_t i = 0; i <= pointIndexOfOuterRingToBeCut; i++)
			mergedResult.push_back(mergedOuterRing[i]);

		if (bReverseInnerRing)
		{
			for (size_t i = 0; i < eachRingPointCount[indexOfHoleToBeCut]; i++)
				mergedResult.push_back(std::pair<size_t, size_t>(indexOfHoleToBeCut, (eachRingPointCount[indexOfHoleToBeCut] + pointIndexOfCut - i) % eachRingPointCount[indexOfHoleToBeCut]));
		}
		else
		{
			for (size_t i = 0; i < eachRingPointCount[indexOfHoleToBeCut]; i++)
				mergedResult.push_back(std::pair<size_t, size_t>(indexOfHoleToBeCut, (i+pointIndexOfCut)%eachRingPointCount[indexOfHoleToBeCut]));
		}
		mergedResult.push_back(std::pair<size_t, size_t>(indexOfHoleToBeCut, pointIndexOfCut));

		for (size_t i = pointIndexOfOuterRingToBeCut; i < mergedOuterRing.size(); i++)
			mergedResult.push_back(mergedOuterRing[i]);

		// 4. final validation - check if this polygon is like infinte simbol(butterfly-shaped)
		//double crossProd, dotProd, angle;
		//size_t count = mergedResult.size();
		//size_t prevIndex, nextIndex;
		//size_t prevIndexHole, nextIndexHole, prevIndexPoint, nextIndexPoint, curIndexHole, curIndexPoint;
		//gaia3d::Point3D prevVector, nextVector;
		//double lfNormal = 0.0;
		//double tolerance = 1E-7;
		//for (size_t i = 0; i < count; i++)
		//{
		//	prevIndex = (i == 0) ? count - 1 : i - 1;
		//	nextIndex = (i == count - 1) ? 0 : i + 1;

		//	curIndexHole = mergedResult[i].first;
		//	curIndexPoint = mergedResult[i].second;
		//	prevIndexHole = mergedResult[prevIndex].first;
		//	prevIndexPoint = mergedResult[prevIndex].second;
		//	nextIndexHole = mergedResult[nextIndex].first;
		//	nextIndexPoint = mergedResult[nextIndex].second;

		//	prevVector.set(pxs[curIndexHole][curIndexPoint] - pxs[prevIndexHole][prevIndexPoint], pys[curIndexHole][curIndexPoint] - pys[prevIndexHole][prevIndexPoint], 0.0);
		//	nextVector.set(pxs[nextIndexHole][nextIndexPoint] - pxs[curIndexHole][curIndexPoint], pys[nextIndexHole][nextIndexPoint] - pys[curIndexHole][curIndexPoint], 0.0);

		//	if (!prevVector.normalize())
		//		continue;
		//	if (!nextVector.normalize())
		//		continue;

		//	crossProd = prevVector.x*nextVector.y - prevVector.y*nextVector.x;
		//	dotProd = prevVector.x * nextVector.x + prevVector.y * nextVector.y;
		//	if (crossProd > tolerance)
		//	{
		//		crossProd = 1.0;
		//	}
		//	else if (crossProd < -tolerance)
		//	{
		//		crossProd = -1.0;
		//	}
		//	else
		//		continue;

		//	if (dotProd > 1.0)
		//		dotProd = 1.0;

		//	if (dotProd < -1.0)
		//		dotProd = -1.0;

		//	angle = acos(dotProd);

		//	lfNormal += (crossProd * angle);
		//}

		//if(lfNormal < 1E-6 && lfNormal > -1E-6)
		//	return false;

		// 5. put the result into container
		mergedOuterRing.clear();
		mergedOuterRing.insert(mergedOuterRing.begin(), mergedResult.begin(), mergedResult.end());

		return true;
	}

	bool GeometryUtility::earCut(double** xs, double** ys, double** zs,
								std::vector<size_t>& eachRingPointCount,
								std::vector<std::pair<size_t, size_t>>& result, bool bDebug)
	{
		// eachRingPointCount[0] : point count of outer ring
		// eachRingPointCount[1] ~ eachRingPointCount[n] : point counts of inner rings
		// xs[0][n-1] : x coordinate of n-th point on outer ring
		// xs[m][n-1] : x coordinate of n-th point on m-th inner ring
		// ys and zs are same with xs


		// 0. basic validation
		if (eachRingPointCount.size() == 0)
			return false;

		if (eachRingPointCount.size() == 1)
		{
			if (eachRingPointCount[0] < 3)
				return false;

			std::pair<size_t, size_t> polygonPointIndex;
			polygonPointIndex.first = 0;
			for (size_t i = 0; i < eachRingPointCount[0]; i++)
			{
				polygonPointIndex.second = i;
				result.push_back(polygonPointIndex);
			}

			return true;
		}

		bool bAllInnerRingsValid = true;
		for (size_t i = 1; i < eachRingPointCount.size(); i++)
		{
			if (eachRingPointCount[i] < 3)
			{
				bAllInnerRingsValid = false;
				break;
			}
		}
		if (!bAllInnerRingsValid)
			return false;
		
		// 1. calculate normal of this polygon
		gaia3d::Point3D normal, crossProd, prevVector, nextVector;
		double dotProd, angle;
		size_t prevIndex, nextIndex;
		normal.set(0.0, 0.0, 0.0);
		for (size_t i = 0; i < eachRingPointCount[0]; i++)
		{
			prevIndex = (i == 0) ? eachRingPointCount[0] - 1 : i - 1;
			nextIndex = (i == eachRingPointCount[0] - 1) ? 0 : i + 1;

			prevVector.set(xs[0][i] - xs[0][prevIndex], ys[0][i] - ys[0][prevIndex], zs[0][i] - zs[0][prevIndex]);
			nextVector.set(xs[0][nextIndex] - xs[0][i], ys[0][nextIndex] - ys[0][i], zs[0][nextIndex] - zs[0][i]);

			if (!prevVector.normalize())
				continue;
			if (!nextVector.normalize())
				continue;

			crossProd = prevVector ^ nextVector;
			if (!crossProd.normalize())
				continue;

			dotProd = prevVector.x * nextVector.x + prevVector.y * nextVector.y + prevVector.z * nextVector.z;
			angle = acos(dotProd);

			normal += (crossProd * angle);
		}

		if (!normal.normalize())
			return false;

		// 2. make projected polygon using the polygon normal
		unsigned char projectionType; // 0 : onto x-y plane, 1 : onto y-z plane, 2 : onto z-x plane
		double nx = abs(normal.x);
		double ny = abs(normal.y);
		double nz = abs(normal.z);

		projectionType = (nz > nx) ? ((nz > ny) ? 0 : 2) : ((nx > ny) ? 1 : 2);
		double** pxs = new double*[eachRingPointCount.size()];
		memset(pxs, 0x00, sizeof(double*) * eachRingPointCount.size());
		double** pys = new double*[eachRingPointCount.size()];
		memset(pys, 0x00, sizeof(double*) * eachRingPointCount.size());

		for (size_t i = 0; i < eachRingPointCount.size(); i++)
		{
			pxs[i] = new double[eachRingPointCount[i]];
			pys[i] = new double[eachRingPointCount[i]];
			memset(pxs[i], 0x00, sizeof(double) * eachRingPointCount[i]);
			memset(pxs[i], 0x00, sizeof(double) * eachRingPointCount[i]);
		}

		switch (projectionType)
		{
		case 0:
		{
			if (normal.z > 0)
			{
				for (size_t i = 0; i < eachRingPointCount.size(); i++)
				{
					for (size_t j = 0; j < eachRingPointCount[i]; j++)
					{
						pxs[i][j] = xs[i][j];
						pys[i][j] = ys[i][j];
					}
				}
			}
			else
			{
				for (size_t i = 0; i < eachRingPointCount.size(); i++)
				{
					for (size_t j = 0; j < eachRingPointCount[i]; j++)
					{
						pxs[i][j] = xs[i][j];
						pys[i][j] = -ys[i][j];
					}
				}
			}
		}
		break;
		case 1:
		{
			if (normal.x > 0)
			{
				for (size_t i = 0; i < eachRingPointCount.size(); i++)
				{
					for (size_t j = 0; j < eachRingPointCount[i]; j++)
					{
						pxs[i][j] = ys[i][j];
						pys[i][j] = zs[i][j];
					}
				}
			}
			else
			{
				for (size_t i = 0; i < eachRingPointCount.size(); i++)
				{
					for (size_t j = 0; j < eachRingPointCount[i]; j++)
					{
						pxs[i][j] = ys[i][j];
						pys[i][j] = -zs[i][j];
					}
				}
			}
		}
		break;
		case 2:
		{
			if (normal.y > 0)
			{
				for (size_t i = 0; i < eachRingPointCount.size(); i++)
				{
					for (size_t j = 0; j < eachRingPointCount[i]; j++)
					{
						pxs[i][j] = zs[i][j];
						pys[i][j] = xs[i][j];
					}
				}
			}
			else
			{
				for (size_t i = 0; i < eachRingPointCount.size(); i++)
				{
					for (size_t j = 0; j < eachRingPointCount[i]; j++)
					{
						pxs[i][j] = zs[i][j];
						pys[i][j] = -xs[i][j];
					}
				}
			}
		}
		break;
		}

		// 3. set normal of outter ring
		int outerRingNormal = 1;

		// 4. find wheter inner rings should be reversed with comparison to the normal of this polygon
		std::vector<bool> bReverseInnerRings;
		for (size_t i = 1; i < eachRingPointCount.size(); i++)
		{
			int innerRingNormal;

			find2DPlaneNormal(pxs[i], pys[i], eachRingPointCount[i], innerRingNormal);

			if (outerRingNormal == innerRingNormal)
				bReverseInnerRings.push_back(true);
			else
				bReverseInnerRings.push_back(false);
		}

		// 5. let's ear-cut

		// 5-1. find lower-left point of MBR from all inner rings
		double minx, miny;
		minx = pxs[1][0];
		miny = pys[1][0];
		for (size_t i = 1; i < eachRingPointCount.size(); i++)
		{
			for (size_t j = 0; j < eachRingPointCount[i]; j++)
			{
				if(minx > pxs[i][j]) 
					minx = pxs[i][j];
				if(miny > pys[i][j])
					miny = pys[i][j];
			}
		}
		
		// 5-2. sort inner rings by distance from this minimum bounding point
		std::vector<std::pair<size_t, size_t>> sortedRingsAndTheirLowerLeftPointList;
		std::vector<size_t> eachInnerRingPointCount;
		eachInnerRingPointCount.insert(eachInnerRingPointCount.begin(), eachRingPointCount.begin() + 1, eachRingPointCount.end());
		getSortedRingsByDistFromPointAndMarkedIndices(pxs + 1, pys + 1, eachInnerRingPointCount, minx, miny, sortedRingsAndTheirLowerLeftPointList, bDebug);

		// 5-3. initialize result container by filling it with outer ring points
		result.clear();
		for (size_t i = 0; i < eachRingPointCount[0]; i++)
			result.push_back(std::pair<size_t, size_t>(0, i));

		// 5-4. ear cut between outer ring and inner rings by turn on marked indices
		std::vector<size_t>eliminatedHoleIndices;
		std::map<size_t, size_t> failureStatus;
		size_t sortedHoleIndexToBeEliminated = 0;
		while (true)
		{
			// 1. check if this hole is eliminated already
			bool bEliminated = false;
			for (size_t i = 0; i < eliminatedHoleIndices.size(); i++)
			{
				if (sortedHoleIndexToBeEliminated == eliminatedHoleIndices[i])
				{
					bEliminated = true;
					break;
				}
			}

			if (bEliminated)
			{
				sortedHoleIndexToBeEliminated++;
				sortedHoleIndexToBeEliminated = sortedHoleIndexToBeEliminated%sortedRingsAndTheirLowerLeftPointList.size();
				continue;
			}

			// 2. eliminated the selected inner hole
			size_t targetHoleIndex = sortedRingsAndTheirLowerLeftPointList[sortedHoleIndexToBeEliminated].first + 1;
			size_t targetPointIndexOfTargetHole = sortedRingsAndTheirLowerLeftPointList[sortedHoleIndexToBeEliminated].second;
			bool bReverseThisInnerHole = bReverseInnerRings[targetHoleIndex - 1];

			bool bThisHoleEliminated = false;
			for (size_t i = 0; i < eachRingPointCount[targetHoleIndex]; i++)
			{
				if (earCutHoleOfPolygon(pxs, pys, eachRingPointCount, targetHoleIndex, (targetPointIndexOfTargetHole + i)% eachRingPointCount[targetHoleIndex], bReverseThisInnerHole, result))
				{
					eliminatedHoleIndices.push_back(sortedHoleIndexToBeEliminated);
					bThisHoleEliminated = true;
					break;
				}
			}

			if (bThisHoleEliminated)
			{
				if (eliminatedHoleIndices.size() == sortedRingsAndTheirLowerLeftPointList.size())
					break;
			}
			else
			{
				// check if this failure happens again in same condition
				if (failureStatus.find(sortedHoleIndexToBeEliminated) == failureStatus.end() ||
					failureStatus[sortedHoleIndexToBeEliminated] != eliminatedHoleIndices.size())
				{
					failureStatus[sortedHoleIndexToBeEliminated] = eliminatedHoleIndices.size();
				}
				else
				{
					// this means ear cut of this hole failed in same condition
					for (size_t i = 0; i < eachRingPointCount.size(); i++)
					{
						delete[] pxs[i];
						delete[] pys[i];
					}
					delete[] pxs;
					delete[] pys;

					return false;
				}
			}

			sortedHoleIndexToBeEliminated++;
			sortedHoleIndexToBeEliminated = sortedHoleIndexToBeEliminated%sortedRingsAndTheirLowerLeftPointList.size();
		}

		// 6. clear
		for (size_t i = 0; i < eachRingPointCount.size(); i++)
		{
			delete[] pxs[i];
			delete[] pys[i];
		}
		delete[] pxs;
		delete[] pys;

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
	///< 각 나라 언어에 맞는 wide string을 utf8로 변환
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

#ifdef _WIN32
	std::string StringUtility::convertMultibyteToUtf8(std::string& sourceString)
	{
		int neededLength = MultiByteToWideChar(CP_ACP, 0, sourceString.c_str(), (int)sourceString.size(), NULL, 0);
		wchar_t* receiver = new wchar_t[neededLength + 1];
		memset(receiver, 0x00, sizeof(wchar_t)*(neededLength + 1));
		MultiByteToWideChar(CP_ACP, 0, sourceString.c_str(), (int)sourceString.size(), receiver, neededLength);
		std::wstring wideString(receiver);
		delete[] receiver;
		return convertWideStringToUtf8(wideString);
	}
#endif

///< 해당 라이브러리(STB) 사용을 위해서 define과 include
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../util/stb_image_write.h"
	///< memory의 image를 파일로 저장
	void ImageUtility::writeMemoryImageToFile(unsigned char* buffer, int width, int height, const char* fullPath)
	{
		stbi_write_jpg(fullPath, width, height, 4, buffer, 0);
	}
}


