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

#include <tuple>
#include "R3.h"
#include "Ray.h"

/*****************************************************************************/

class AABB
{
	private:
		// Returns the width (x) of the AABB
		float computeWidth() const;

		// Returns the height (y) of the AABB
		float computeHeight() const;

		// Returns the depth (z) of the AABB
		float computeDepth() const;

		// Computes the area of the AABB
		float computeArea() const;

	protected:
		std::tuple<P,P> Vs; // Vertices defining the AABB
		P C;                // The centroid of the AABB
		float _width, _height, _depth, _area;

	public:
		AABB();
		AABB(const P& v1, const P& v2);
		AABB(const AABB& other);

		// Get the pair of vertices comprising the AABB
		const std::tuple<P,P>& vertices() const { return this->Vs; }

		// Returns the centroid of the AABB
		const P& centroid() const { return this->C; }

		// Returns the width (x) of the AABB
		float width() const { return this->_width; };

		// Returns the height (y) of the AABB
		float height() const { return this->_height; };

		// Returns the depth (z) of the AABB
		float depth() const { return this->_depth; };

		// Computes the area of the AABB
		float area() const { return this->_area; }

		// Tests if the given ray intersects the AABB
		bool intersected(const Ray& ray) const;

        friend std::ostream& operator<<(std::ostream& s, const AABB& aabb);		

        AABB& operator+=(const AABB &other);

		// AABBs can be added: The sum of two AABBs P and Q a new
		// AABB large enough to contain the AABB
        friend const AABB operator+(const AABB& p, const AABB& q);
};

/*****************************************************************************/

#endif
