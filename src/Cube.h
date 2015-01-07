/******************************************************************************
 *
 * Cube geometry definition
 *
 * @file Cube.h
 * @author Michael Woods
 *
 ******************************************************************************/

#ifndef CUBE_H
#define CUBE_H

#include "Geometry.h"

/*****************************************************************************/

class Cube : public Geometry
{
	private:
		BoundingSphere volume;
		AABB aabb;
		void buildVolume();
		void computeCentroid();
		void computeAABB();

	protected:
		P p1, p2, centroid;
		virtual Intersection intersectImpl(const Ray &ray) const;
		virtual glm::vec3 sampleImpl() const;

	public:
		Cube();
		Cube(const Cube& other);
		virtual ~Cube();

		virtual const BoundingVolume& getVolume() const;
		virtual const P& getCentroid() const;
		virtual const AABB& getAABB() const;
		virtual void buildGeometry();
		virtual void repr(std::ostream& s) const;
};

/*****************************************************************************/

#endif
