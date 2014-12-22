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

/*****************************************************************************/

class Cylinder : public Geometry
{
	private:
		P center_;
		float radius_;
		float height_;
		BoundingSphere volume;

		void buildVolume();

	protected:
		virtual Intersection intersectImpl(const Ray &ray, const glm::mat4& T) const;
		virtual glm::vec3 sampleImpl() const;

	public:
		Cylinder();
		Cylinder(const Cylinder& other);
		virtual ~Cylinder();

		virtual const P& getCentroid() const;
		virtual const BoundingVolume& getVolume() const;
		virtual void buildGeometry();
		virtual void repr(std::ostream& s) const;
};

/*****************************************************************************/

#endif
