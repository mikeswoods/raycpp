#ifndef POINT_LIGHT_H
#define POINT_LIGHT_H

/*******************************************************************************
 *
 * This file defines a simple point light implementation
 *
 * @file PointLight.h
 * @author Michael Woods
 *
 ******************************************************************************/

#include <iostream>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include "Light.h"
#include "Color.h"

/******************************************************************************/

class PointLight : public Light
{
	protected:
		// Light world position
		glm::vec3 position;

		// Light color
		Color color;

    public:
        PointLight();
		PointLight(const glm::vec3& position);
        PointLight(const glm::vec3& position, const Color& color);
        PointLight(const PointLight &other);

		virtual void repr(std::ostream& s) const;

		const glm::vec3& getPosition() const        { return this->position; }
		void setPosition(const glm::vec3& position) { this->position = position; }

		void translateX(float amount) { this->position.x += amount; }
		void translateY(float amount) { this->position.y += amount; }
		void translateZ(float amount) { this->position.z += amount; }

		const Color& getColor() const     { return this->color; }
		void setColor(const Color& color) { this->color = color; }

		virtual glm::vec3 fromCenter(const glm::vec3& from) const;

		virtual glm::vec3 fromSampledPoint(const glm::vec3& from) const;
		virtual glm::vec3 fromSampledPoint(const glm::vec3& from, float& cosineAngle) const;

		virtual Color getColor(const glm::vec3& from) const;

		virtual bool isLightSourceNode(std::shared_ptr<GraphNode> testNode) const;
};

/******************************************************************************/

#endif
