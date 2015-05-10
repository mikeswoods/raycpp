/******************************************************************************
 *
 * Cylinder geometry definition
 *
 * @file Cylinder.h
 * @author Michael Woods
 *
 ******************************************************************************/

#ifndef CYLINDER_H
#define CYLINDER_H

#include "Geometry.h"

/******************************************************************************/

class Cylinder : public Geometry
{
	private:
		glm::vec3 center_;
		float radius_;
		float height_;
		BoundingSphere volume;
		AABB aabb;

		void buildVolume();
		void computeAABB();

	protected:
		virtual Intersection intersectImpl(const Ray &ray, std::shared_ptr<SceneContext> scene) const;
		virtual glm::vec3 sampleImpl() const;

	public:
		Cylinder();
		Cylinder(const Cylinder& other);
		virtual ~Cylinder();

		virtual const glm::vec3& getCentroid() const;
		virtual const BoundingVolume& getVolume() const;
		virtual const AABB& getAABB() const;
		virtual void buildGeometry();
		virtual void repr(std::ostream& s) const;
};

/******************************************************************************/

#endif
