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

/******************************************************************************/

using namespace std;
using namespace glm;

/******************************************************************************/

AreaLight::AreaLight(shared_ptr<GraphNode> _node, mat4 _T) :
	Light(AREA_LIGHT),
	node(_node),
	T(_T)
{ 
	shared_ptr<Geometry> geometry = this->node->getGeometry();

	assert(!!geometry);

	this->centroidWorld = vec3(Utils::transform(this->T, vec4(geometry->getCentroid(), 1.0f)));
}

AreaLight::AreaLight(const AreaLight& other) :
	Light(AREA_LIGHT),
	centroidWorld(other.centroidWorld),
	node(other.node),
	T(other.T)
{ 
	
}

void AreaLight::repr(ostream& s) const
{
	s << "AreaLight { }";
}

vec3 AreaLight::fromCenter(const vec3& from) const
{
	shared_ptr<Geometry> geometry = this->node->getGeometry();

	assert(!!geometry);

	// Necessary b/c we have to transform the centroid in object space to world space:
	return this->centroidWorld - from;
}

vec3 AreaLight::fromSampledPoint(const vec3& from) const
{
	shared_ptr<Geometry> geometry = this->node->getGeometry();

	assert(!!geometry);

	return geometry->sample(this->T) - from;
}

vec3 AreaLight::fromSampledPoint(const vec3& from, float& cosineAngle) const
{
	shared_ptr<Geometry> geometry  = this->node->getGeometry();

	assert(!!geometry);

	vec3 samplePoint = geometry->sample(this->T);
	vec3 centroid    = geometry->getCentroid();
	vec3 L           = samplePoint - from;
	vec3 D           = samplePoint - centroid;
	cosineAngle      = dot(normalize(L), normalize(D));

	return L;
}

Color AreaLight::getColor(const vec3& from) const
{
	shared_ptr<Material> mat = this->node->getMaterial();

	assert(!!mat);

	return mat->getDiffuseColor();
}

bool AreaLight::isLightSourceNode(shared_ptr<GraphNode> testNode) const
{
	return testNode == this->node;
}

/******************************************************************************/
