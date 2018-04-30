#pragma once

#include "Vertex.h"

namespace gaia3d
{
	class Triangle
	{
	public:
		Triangle();

		~Triangle();

	protected:
		size_t vertexIndex[3];

		Vertex* vertex[3];

		Point3D normal;

	public:
		void setVertices(Vertex* vertex0, Vertex* vertex1, Vertex* vertex2) {vertex[0] = vertex0; vertex[1]= vertex1; vertex[2] = vertex2;}

		Vertex** getVertices() {return vertex;}

		void setVertexIndices(size_t id0, size_t id1, size_t id2) {vertexIndex[0] = id0; vertexIndex[1] = id1; vertexIndex[2] = id2;}

		size_t* getVertexIndices() {return vertexIndex;}

		Point3D* getNormal() {return &normal;}

		void setNormal(double x, double y, double z) {this->normal.set(x, y, z);}

		void alignVertexNormalsToPlaneNormal();

	};
}