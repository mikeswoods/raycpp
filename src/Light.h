/******************************************************************************
 *
 * This file defines a basic point-light type
 *
 * @file Light.h
 * @author Michael Woods
 *
 ******************************************************************************/

#ifndef LIGHT_H
#define LIGHT_H

/******************************************************************************
 *
 * This file the interface that all light objects must implement 
 *
 * @file Light.h
 * @author Michael Woods
 *
 ******************************************************************************/

#include <memory>
#include <iostream>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include "Color.h"

/******************************************************************************/

class GraphNode;

/*******************************************************************************
 * Abstract light class
 ******************************************************************************/

class Light
{
	public:
		/**
		 * Light type
		 */
		enum Type 
		{
			  POINT_LIGHT
			 ,AREA_LIGHT
		};

	protected:
		Type type;

    public:
        explicit Light(Type);

		/**
		 * Dump the representation of the light to the given stream
		 */
		virtual void repr(std::ostream& s) const = 0;

		Type getLightType() const { return this->type; };

		/**
		 * Returns the un-normalized direction vector from the given hit 
		 * point in world space to the center of the light source
		 */
		virtual glm::vec3 fromCenter(const glm::vec3& from) const = 0;

		/**
		 * Returns the un-normalized direction vector from the given hit 
		 * point in world space to a randomly sampled point on the surface 
		 * of the light source
		 */
		virtual glm::vec3 fromSampledPoint(const glm::vec3& from) const = 0;

		/**
		 * Like, fromSampledPoint(from), but returns the cosine angle from the 
		 * vector formed from the center of the light to a sampled point on 
		 * its surface and the computed light vector
		 */
		virtual glm::vec3 fromSampledPoint(const glm::vec3& from, float& cosineAngle) const = 0;

		/**
		 * Returns the color associated with the light relative to the given position
		 */
		virtual Color getColor(const glm::vec3& from) const = 0;

		/**
		 * Given a scene graph node, test if the given node is the same as that
		 * as the node that is acting as the light source
		 */
		virtual bool isLightSourceNode(std::shared_ptr<GraphNode> testNode) const = 0;

		friend std::ostream& operator<<(std::ostream& s, const Light& light);
};

/******************************************************************************/

#endif
