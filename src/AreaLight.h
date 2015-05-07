/*******************************************************************************
 *
 * This file defines a simple area light implementation
 *
 * @file AreaLight.h
 * @author Michael Woods
 *
 ******************************************************************************/

#ifndef AREA_LIGHT_H
#define AREA_LIGHT_H

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <memory>
#include <iostream>
#include "Light.h"
#include "Graph.h"

/******************************************************************************/

class AreaLight : public Light
{
	protected:
		// The centroid of the light in world space
		glm::vec3 centroidWorld;

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

		virtual glm::vec3 fromCenter(const glm::vec3& from) const;

		virtual glm::vec3 fromSampledPoint(const glm::vec3& from) const;
		virtual glm::vec3 fromSampledPoint(const glm::vec3& from, float& cosineAngle) const;

		virtual Color getColor(const glm::vec3& from) const;

		virtual bool isLightSourceNode(std::shared_ptr<GraphNode> testNode) const;
};

/******************************************************************************/

#endif
