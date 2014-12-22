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

using namespace std;

/*****************************************************************************/

PointLight::PointLight() :
	Light(POINT_LIGHT),
	position(glm::vec3(0.0f, 0.0f, 0.0f)),
	color(Color::WHITE)
{ 
	
}

PointLight::PointLight(const glm::vec3& _position) :
	Light(POINT_LIGHT),
	position(P(_position)),
	color(Color::WHITE)
{ 
	
}

PointLight::PointLight(const P& _position) :
	Light(POINT_LIGHT),
	position(_position),
	color(Color::WHITE)
{ 
	
}

PointLight::PointLight(const P& _position, const Color& _color) :
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
	s << "PointLight { " 
	  << "  position = " << this->position
      << ", color = "    << this->color
	  << " }";
}

V PointLight::fromCenter(const P& from) const
{
	return this->position.xyz - from;
}

V PointLight::fromSampledPoint(const P& from) const
{
	return this->fromCenter(from);
}

V PointLight::fromSampledPoint(const P& from, float& cosineAngle) const
{
	cosineAngle = numeric_limits<float>::infinity();
	return this->fromCenter(from);
}

Color PointLight::getColor(const P& from) const
{
	return this->color;
}

bool PointLight::isLightSourceNode(GraphNode const * testNode) const
{
	// Always false, since no node is involved with point lights
	return false;
}

/*****************************************************************************/
