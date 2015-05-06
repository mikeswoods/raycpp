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
#include "Light.h"
#include "R3.h"
#include "Color.h"

/******************************************************************************/

class PointLight : public Light
{
	protected:
		// Light world position
		P position;

		// Light color
		Color color;

    public:
        PointLight();
		PointLight(const glm::vec3& position);
		PointLight(const P& position);
        PointLight(const P& position, const Color& color);
        PointLight(const PointLight &other);

		virtual void repr(std::ostream& s) const;

		const P& getPosition() const        { return this->position; }
		void setPosition(const P& position) { this->position = position; }

		void translateX(float amount) { this->position.xyz.x += amount; }
		void translateY(float amount) { this->position.xyz.y += amount; }
		void translateZ(float amount) { this->position.xyz.z += amount; }

		const Color& getColor() const     { return this->color; }
		void setColor(const Color& color) { this->color = color; }

		virtual V fromCenter(const P& from) const;

		virtual V fromSampledPoint(const P& from) const;
		virtual V fromSampledPoint(const P& from, float& cosineAngle) const;

		virtual Color getColor(const P& from) const;

		virtual bool isLightSourceNode(std::shared_ptr<GraphNode> testNode) const;
};

/******************************************************************************/

#endif
