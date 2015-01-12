#ifndef AREA_LIGHT_H
#define AREA_LIGHT_H

/*******************************************************************************
 *
 * This file defines a simple area light implementation
 *
 * @file AreaLight.h
 * @author Michael Woods
 *
 ******************************************************************************/

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <memory>
#include <iostream>
#include "Light.h"
#include "R3.h"
#include "Graph.h"

/******************************************************************************/

class AreaLight : public Light
{
	protected:
		// The centroid of the light in world space
		P centroidWorld;

		// Graph node that acts as the light source:
		std::shared_ptr<GraphNode> node;

		// Transformation matrix associated with the node acting as the
		// light source
		glm::mat4 T;

	public:
		AreaLight(std::shared_ptr<GraphNode> node, glm::mat4 T);
		AreaLight(const AreaLight& other);

		virtual void repr(std::ostream& s) const;

		std::shared_ptr<GraphNode> getNode() { return this->node; }
		const glm::mat4& getT()              { return this->T; }

		virtual V fromCenter(const P& from) const;

		virtual V fromSampledPoint(const P& from) const;
		virtual V fromSampledPoint(const P& from, float& cosineAngle) const;

		virtual Color getColor(const P& from) const;

		virtual bool isLightSourceNode(std::shared_ptr<GraphNode> testNode) const;
};

/******************************************************************************/

#endif
