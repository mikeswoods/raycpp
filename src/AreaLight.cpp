/*****************************************************************************
 *
 * This file defines a simple area light implementation
 *
 * @file AreaLight.h
 * @author Michael Woods
 *
 *****************************************************************************/

#include <cassert>
#include <iostream>
#include "AreaLight.h"

using namespace std;

/*****************************************************************************/

AreaLight::AreaLight(GraphNode const * _node, glm::mat4 _T) :
	Light(AREA_LIGHT),
	node(_node),
	T(_T)
{ 
	Geometry const * geometry = this->node->getGeometry();

	assert(geometry != NULL);

	this->centroidWorld = P(transform(this->T, glm::vec4(geometry->getCentroid().xyz, 1.0f)));
}

AreaLight::AreaLight(const AreaLight& other) :
	Light(AREA_LIGHT),
	node(other.node),
	T(other.T),
	centroidWorld(other.centroidWorld)
{ 
	
}

void AreaLight::repr(std::ostream& s) const
{
	s << "AreaLight { }";
}

V AreaLight::fromCenter(const P& from) const
{
	Geometry const * geometry = this->node->getGeometry();

	assert(geometry != NULL);

	// Necessary b/c we have to transform the centroid in object space to world space:
	return this->centroidWorld.xyz - from.xyz;
}

V AreaLight::fromSampledPoint(const P& from) const
{
	Geometry const * geometry = this->node->getGeometry();

	assert(geometry != NULL);

	return geometry->sample(this->T) - from;
}

V AreaLight::fromSampledPoint(const P& from, float& cosineAngle) const
{
	Geometry const * geometry = this->node->getGeometry();

	assert(geometry != NULL);

	P samplePoint = geometry->sample(this->T);
	P centroid    = geometry->getCentroid();
	V L           = samplePoint - from;
	V D           = samplePoint - centroid;

	cosineAngle = glm::dot(glm::normalize(L), glm::normalize(D));

	return L;
}

Color AreaLight::getColor(const P& from) const
{
	Material* mat = this->node->getMaterial();
	assert(mat != NULL);

	return mat->getDiffuseColor();
}

bool AreaLight::isLightSourceNode(GraphNode const * testNode) const
{
	return testNode == this->node;
}

/*****************************************************************************/
