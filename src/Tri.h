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
		int meshIndex;
		P v[3];     // corner vertices
		V normal;   // precomputed normal
		AABB aabb;

		void buildAABB();
		void computeCentroid();
		void computeNormal();

	protected:
		float naiveIntersect(const Ray& ray, glm::vec3& W) const;
		float mollerTrumboreIntersect(const Ray& ray, glm::vec3& W) const;

	public:
		Tri();
		Tri(int meshIndex, const glm::vec3& _v1, const glm::vec3& _v2, const glm::vec3& _v3);
		Tri(const Tri& other);
		virtual ~Tri() { }

		int getMeshIndex() const { return this->meshIndex; }

		float getXMinima() const;
		float getYMinima() const;
		float getZMinima() const;
		float getXMaxima() const;
		float getYMaxima() const;
		float getZMaxima() const;

		glm::vec3 barycenter(const P& p) const;

		P const * getVertices() const { return this->v; }

		V getNormal() const;
		const AABB& getAABB() { return this->aabb; }

		float intersected(const Ray& ray, glm::vec3& W) const;
};
 
/******************************************************************************/

#endif
