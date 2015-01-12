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

/******************************************************************************/

class Cube : public Geometry
{
	protected:
		P p1, p2, centroid;

	private:
		BoundingSphere volume;
		AABB aabb;
		void buildVolume();
		void computeCentroid();
		void computeAABB();

	protected:
		virtual Intersection intersectImpl(const Ray &ray, std::shared_ptr<SceneContext> scene) const;
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

/******************************************************************************/

#endif
