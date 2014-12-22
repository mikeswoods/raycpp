/******************************************************************************
 *
 * Various bounding volume class and interface definition(s)
 *
 * @file BoundingVolume.h
 * @author Michael Woods
 *
 ******************************************************************************/

#ifndef BOUNDING_VOLUME_H
#define BOUNDING_VOLUME_H

#include "R3.h"
#include "Ray.h"

/*******************************************************************************
 * Abstract bounding volume type
 ******************************************************************************/

class BoundingVolume
{
	public:

		BoundingVolume() { }

		// Intersects the bounding volume
		virtual bool intersects(const Ray& ray) const = 0;
};


/*******************************************************************************
 * Trivial volume that is always intersected
 ******************************************************************************/

class TrivialVolume : public BoundingVolume
{
	public:

		TrivialVolume() : BoundingVolume() { }

		// Intersects the bounding volume
		virtual bool intersects(const Ray& ray) const { return true; }
};

/*******************************************************************************
 * Bounding sphere volume parameterized by a center and radius
 ******************************************************************************/

class BoundingSphere : public BoundingVolume
{
	protected:
		P center;
		float radius;

	public:
		BoundingSphere();
		BoundingSphere(const glm::vec3& _center, float _radius);
		BoundingSphere(const P& _center, float _radius);
		BoundingSphere(const BoundingSphere& other);

		virtual bool intersects(const Ray& ray) const;
};

/******************************************************************************/

#endif
