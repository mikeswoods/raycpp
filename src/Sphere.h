/******************************************************************************
 *
 * Sphere geometry definition
 *
 * @file Sphere.h
 * @author Michael Woods
 *
 ******************************************************************************/

#ifndef SPHERE_H
#define SPHERE_H

#include "Geometry.h"

/******************************************************************************/

class Sphere : public Geometry
{
	private:
		P center_;
		float radius_;
		BoundingSphere volume;
		AABB aabb;

		void buildVolume();
		void computeAABB();

	protected:
		virtual Intersection intersectImpl(const Ray &ray, std::shared_ptr<SceneContext> scene) const;
		virtual glm::vec3 sampleImpl() const;

	public:
		Sphere();
		Sphere(const P& center, float radius);
		virtual ~Sphere();

		virtual const BoundingVolume& getVolume() const;
		virtual const P& getCentroid() const;
		virtual const AABB& getAABB() const;
		virtual void buildGeometry();
		virtual void repr(std::ostream& s) const;
};

/******************************************************************************/

#endif
