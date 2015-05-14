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

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <tuple>
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
		glm::vec3 v1, v2; // Vertices defining the AABB
		glm::vec3 C;      // The centroid of the AABB
		float _width, _height, _depth, _area;

	public:
		AABB();
		AABB(const glm::vec3& v1, const glm::vec3& v2);

		// Returns the centroid of the AABB
		const glm::vec3& centroid() const { return this->C; }

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
