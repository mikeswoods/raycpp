/*******************************************************************************
 *
 * This file defines a type for representing triangular mesh gemoetric object
 * as well as operations over such objects
 *
 * @file Mesh.h
 * @author Michael Woods
 *
 ******************************************************************************/

#ifndef MESH_H
#define MESH_H

#include <memory>
#include <tuple>
#include <vector>
#include "R3.h"
#include "Geometry.h"
#include "Tri.h"
#include "KDTree.h"
#include "Face.h"

/******************************************************************************/

typedef std::tuple<int,int,int> VNIndex;

/******************************************************************************/

class Mesh : public Geometry
{
	private:
		P centroid;
		TrivialVolume volume;
		AABB aabb;
		std::unique_ptr<KDTree> tree;
		bool withNormals;
		bool withTextures;

		void computeCentroid();
		void computeAABB();
		void buildVolume();

	protected:
		// All triangular faces that are part of the mesh
		std::vector<Face> faces;

		// Texture UV coordinates:
		std::vector<glm::vec3> textures;

		// Self-contained triangle data:
		std::vector<Tri> triangles;

		// Triangle vertex normal indices
		std::vector<VNIndex> vnIndex;

		virtual Intersection intersectImpl(const Ray &ray, std::shared_ptr<SceneContext> scene) const;
		virtual glm::vec3 sampleImpl() const;

	public:
		Mesh(const std::vector<glm::vec3>& vertices
			,const std::vector<glm::vec3>& normals
			,const std::vector<glm::vec3>& textures
			,const std::vector<Face>& faces);

		virtual ~Mesh();

		virtual const P& getCentroid() const;
		virtual const BoundingVolume& getVolume() const;
		virtual const AABB& getAABB() const;
		virtual void buildGeometry();
		virtual void repr(std::ostream& s) const;

		const std::vector<Tri>& getTriangles() { return this->triangles; }
};

#endif

/******************************************************************************/
