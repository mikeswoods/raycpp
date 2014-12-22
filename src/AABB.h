/*****************************************************************************
 *
 * Simple axis-aligned bounding box (AABB) implementation
 *
 * @file Raytrace.h
 * @author Michael Woods
 *
 *****************************************************************************/

#ifndef AABB_H
#define AABB_H

#include "R3.h"
#include "Ray.h"

/*****************************************************************************/

class AABB
{
	protected:
		P v1, v2; // Extrema defining the AABB
		P C;      // The centroid of the AABB

		// Computes the centroid of the AABB
		P computeCentroid() const;

	public:
		AABB();
		AABB(const P& v1, const P& v2);
		AABB(const AABB& other);

		// Returns the centroid of the AABB
		const P& centroid() const { return this->C; }

		// Computes the area of the AABB
		float area() const;

		// Tests if the given ray intersects the AABB
		bool intersected(const Ray& ray) const;

        friend std::ostream& operator<<(std::ostream& s, const AABB& aabb);		

		// AABBs can be added: The sum of two AABBs P and Q a new
		// AABB large enough to contain the AABB
        friend const AABB operator+(const AABB& p, const AABB& q);
};

/*****************************************************************************/

#endif
