/******************************************************************************
 *
 * This file defines a type for representing triangular mesh gemoetric object
 * as well as operations over such objects
 *
 * @file Mesh.h
 * @author Michael Woods
 *
 *****************************************************************************/

#ifndef MESH_H
#define MESH_H

#include <vector>
#include "R3.h"
#include "Geometry.h"
#include "Tri.h"
#include "KDTree.h"
#include "Face.h"

/*****************************************************************************/

class Mesh : public Geometry
{
	private:
		P centroid;
		//BoundingSphere volume;
		TrivialVolume volume;
		KDTree* tree;

		void computeCentroid();
		void buildVolume();

	protected:
		// All triangular faces that are part of the mesh
		std::vector<Face> faces;

		// Texture UV coordinates:
		std::vector<glm::vec2> textureUV;

		// Self-contained triangle data:
		std::vector<Tri> triangles;

		virtual Intersection intersectImpl(const Ray &ray, const glm::mat4& T) const;
		virtual glm::vec3 sampleImpl() const;

	public:
		Mesh(const std::vector<glm::vec3>& vertices
			,const std::vector<glm::vec3>& normals
			,const std::vector<glm::vec2>& textureUV
			,const std::vector<Face>& faces);

		Mesh(const Mesh& other);

		virtual ~Mesh();

		virtual const P& getCentroid() const;
		virtual const BoundingVolume& getVolume() const;
		virtual void buildGeometry();
		virtual void repr(std::ostream& s) const;

		const std::vector<Tri>& getTriangles() { return this->triangles; }
};

#endif

/*****************************************************************************/
