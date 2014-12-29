/******************************************************************************
 *
 * Triangle intersection test
 *
 * @file Triangle.h
 * @author Michael Woods
 *
 *****************************************************************************/

#ifndef TRIANGLE_H
#define TRIANGLE_H

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include "Geometry.h"
#include "AABB.h"

/******************************************************************************/

class Tri : public Geometry
{
	private:
		int meshIndex;
		P v1, v2, v3, centroid;
		V normal;
		BoundingSphere volume;
		AABB aabb;

		void buildVolume();
		void buildAABB();
		void computeCentroid();
		void computeNormal();

		float naiveIntersect(const Ray& ray, glm::vec3& W) const;
		float mollerTrumboreIntersect(const Ray& ray, glm::vec3& W) const;

	protected:
		virtual Intersection intersectImpl(const Ray &ray) const;
		virtual glm::vec3 sampleImpl() const;

	public:
		Tri();
		Tri(int meshIndex, const P& _v1, const P& _v2, const P& _v3);
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

		const P& getV1() { return this->v1; }
		const P& getV2() { return this->v2; }
		const P& getV3() { return this->v3; }

		virtual const P& getCentroid() const;
		V getNormal() const;
		const AABB& getAABB() { return this->aabb; }
		virtual const BoundingVolume& getVolume() const;

		float intersected(const Ray& ray, glm::vec3& W) const;

		virtual void buildGeometry();
		virtual void repr(std::ostream& s) const;
};
 
/******************************************************************************/

#endif
