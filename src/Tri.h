/******************************************************************************
 *
 * Triangle intersection test
 *
 * @file Triangle.h
 * @author Michael Woods
 *****************************************************************************/

#ifndef TRIANGLE_H
#define TRIANGLE_H

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include "Geometry.h"
#include "AABB.h"

///////////////////////////////////////////////////////////////////////////////

class Tri : public Geometry
{
	public:
		V n1, n2, n3;

	private:
		P v1, v2, v3, centroid;
		V normal;
		BoundingSphere volume;
		AABB aabb;

		void buildVolume();
		void buildAABB();
		void computeCentroid();
		void computeNormal();

	protected:
		virtual Intersection intersectImpl(const Ray &ray, const glm::mat4& T) const;
		virtual glm::vec3 sampleImpl() const;

	public:
		Tri();
		Tri(const P& _v1, const P& _v2, const P& _v3);
		Tri(const glm::vec3& _v1, const glm::vec3& _v2, const glm::vec3& _v3);
		Tri(const Tri& other);
		virtual ~Tri() { }

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
		bool getBarycenter(const P& p, glm::vec3& W);
		virtual const BoundingVolume& getVolume() const;

		bool barycenter(const glm::vec3& test, glm::vec3& weights);
		float intersected(const Ray& ray, bool normalizeDir) const;

		virtual void buildGeometry();
		virtual void repr(std::ostream& s) const;
};
 


///////////////////////////////////////////////////////////////////////////////

#endif
