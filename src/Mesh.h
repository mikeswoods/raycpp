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

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <memory>
#include <tuple>
#include <vector>
#include "R3.h"
#include "Geometry.h"
#include "Tri.h"
#include "KDTree.h"

/******************************************************************************/

class Mesh : public Geometry
{
	private:
		P centroid;
		TrivialVolume volume;
		AABB aabb;
		std::shared_ptr<aiMesh> meshData;
		std::unique_ptr<KDTree> tree;

		void computeCentroid();
		void computeAABB();
		void buildVolume();

	protected:
		// Self-contained triangle data:
		std::vector<Tri> triangles;

		virtual Intersection intersectImpl(const Ray &ray, std::shared_ptr<SceneContext> scene) const;
		virtual glm::vec3 sampleImpl() const;

	public:
		Mesh(std::shared_ptr<aiMesh> meshData);

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
