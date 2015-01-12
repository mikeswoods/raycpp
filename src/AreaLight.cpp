/******************************************************************************
 *
 * This file defines a simple area light implementation
 *
 * @file AreaLight.h
 * @author Michael Woods
 *
 ******************************************************************************/

#include <cassert>
#include <iostream>
#include <memory>
#include "AreaLight.h"

using namespace std;

/******************************************************************************/

AreaLight::AreaLight(GraphNode const * _node, glm::mat4 _T) :
	Light(AREA_LIGHT),
	node(_node),
	T(_T)
{ 
	shared_ptr<Geometry> geometry = this->node->getGeometry();

	assert(!!geometry);

	this->centroidWorld = P(transform(this->T, glm::vec4(geometry->getCentroid().xyz, 1.0f)));
}

AreaLight::AreaLight(const AreaLight& other) :
	Light(AREA_LIGHT),
	centroidWorld(other.centroidWorld),
	node(other.node),
	T(other.T)
{ 
	
}

void AreaLight::repr(std::ostream& s) const
{
	s << "AreaLight { }";
}

V AreaLight::fromCenter(const P& from) const
{
	shared_ptr<Geometry> geometry = this->node->getGeometry();

	assert(!!geometry);

	// Necessary b/c we have to transform the centroid in object space to world space:
	return this->centroidWorld.xyz - from.xyz;
}

V AreaLight::fromSampledPoint(const P& from) const
{
	shared_ptr<Geometry> geometry = this->node->getGeometry();

	assert(!!geometry);

	return geometry->sample(this->T) - from;
}

V AreaLight::fromSampledPoint(const P& from, float& cosineAngle) const
{
	shared_ptr<Geometry> geometry  = this->node->getGeometry();

	assert(!!geometry);

	P samplePoint = geometry->sample(this->T);
	P centroid    = geometry->getCentroid();
	V L           = samplePoint - from;
	V D           = samplePoint - centroid;
	cosineAngle   = glm::dot(glm::normalize(L), glm::normalize(D));

	return L;
}

Color AreaLight::getColor(const P& from) const
{
	shared_ptr<Material> mat = this->node->getMaterial();

	assert(!!mat);

	return mat->getDiffuseColor();
}

bool AreaLight::isLightSourceNode(GraphNode const * testNode) const
{
	return testNode == this->node;
}

/******************************************************************************/
