/******************************************************************************
 *
 * Sphere geometry definition
 *
 * @file Sphere.h
 * @author Michael Woods
 ******************************************************************************/

#ifndef SPHERE_H
#define SPHERE_H

#include "Geometry.h"

////////////////////////////////////////////////////////////////////////////////

class Sphere : public Geometry
{
	private:
		P center_;
		float radius_;
		BoundingSphere volume;

	protected:
		virtual Intersection intersectImpl(const Ray &ray, const glm::mat4& T) const;
		virtual glm::vec3 sampleImpl() const;

	public:
		Sphere();
		Sphere(const P& center, float radius);
		virtual ~Sphere();

		virtual const BoundingVolume& getVolume() const;
		virtual const P& getCentroid() const;
		virtual void buildGeometry();
		virtual void repr(std::ostream& s) const;
};

////////////////////////////////////////////////////////////////////////////////

#endif
