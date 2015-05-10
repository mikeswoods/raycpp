/*******************************************************************************
 *
 * Triangle intersection test
 *
 * @file Triangle.h
 * @author Michael Woods
 *
 ******************************************************************************/

#ifndef TRIANGLE_H
#define TRIANGLE_H

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include "Geometry.h"
#include "AABB.h"

/******************************************************************************/

class Tri
{
	private:
		unsigned int meshIndex;
		glm::uvec3 indices;     // Vertex indices
		glm::vec3 vertices[3];  // Corner vertices
		glm::vec3 normal;       // precomputed normal
		AABB aabb;

		void buildAABB();
		void computeCentroid();
		void computeNormal();

	protected:
		float naiveIntersect(const Ray& ray, glm::vec3& W) const;
		float mollerTrumboreIntersect(const Ray& ray, glm::vec3& W) const;

	public:
		Tri();
		Tri(unsigned int meshIndex
		   ,glm::uvec3 _indices
		   ,glm::vec3 v1
		   ,glm::vec3 v2
		   ,glm::vec3 v3);
		Tri(const Tri& other);
		virtual ~Tri() { }

		unsigned int getMeshIndex() const   { return this->meshIndex; }
		glm::uvec3 getVertexIndices() const { return this->indices; }

		float getXMinima() const;
		float getYMinima() const;
		float getZMinima() const;
		float getXMaxima() const;
		float getYMaxima() const;
		float getZMaxima() const;

		glm::vec3 barycenter(const glm::vec3& p) const;

		glm::vec3 const * getVertices() const { return this->vertices; }

		glm::vec3 getNormal() const;
		const AABB& getAABB() { return this->aabb; }

		float intersected(const Ray& ray, glm::vec3& W) const;
};
 
/******************************************************************************/

#endif
