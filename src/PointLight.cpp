/*****************************************************************************
 *
 * This file defines a simple point light implementation
 *
 * @file PointLight.h
 * @author Michael Woods
 *
 *****************************************************************************/

#include <iostream>
#include <limits>
#include "PointLight.h"

/******************************************************************************/

using namespace std;

/******************************************************************************/

PointLight::PointLight() :
	Light(POINT_LIGHT),
	position(glm::vec3(0.0f, 0.0f, 0.0f)),
	color(Color::WHITE)
{ 
	
}

PointLight::PointLight(const glm::vec3& _position) :
	Light(POINT_LIGHT),
	position(_position),
	color(Color::WHITE)
{ 
	
}

PointLight::PointLight(const glm::vec3& _position, const Color& _color) :
	Light(POINT_LIGHT),
	position(_position),
	color(_color)
{ 
	
}

PointLight::PointLight(const PointLight &other) :
	Light(other.type),
	position(other.position),
	color(other.color)
{ 
	
}

void PointLight::repr(std::ostream& s) const
{
	s << "PointLight";
}

glm::vec3 PointLight::fromCenter(const glm::vec3& from) const
{
	return this->position - from;
}

glm::vec3 PointLight::fromSampledPoint(const glm::vec3& from) const
{
	return this->fromCenter(from);
}

glm::vec3 PointLight::fromSampledPoint(const glm::vec3& from, float& cosineAngle) const
{
	cosineAngle = numeric_limits<float>::infinity();
	return this->fromCenter(from);
}

Color PointLight::getColor(const glm::vec3& from) const
{
	return this->color;
}

bool PointLight::isLightSourceNode(shared_ptr<GraphNode> testNode) const
{
	// Always false, since no node is involved with point lights
	return false;
}

/******************************************************************************/
