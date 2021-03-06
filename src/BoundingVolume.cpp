/******************************************************************************
 *
 * Various bounding volume class and interface definition(s)
 *
 * @file BoundingVolume.h
 * @author Michael Woods
 *
 ******************************************************************************/

#define _USE_MATH_DEFINES
#include <iostream>
#include <algorithm>
#include <cmath>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include "BoundingVolume.h"

/******************************************************************************/

using namespace std;
using namespace glm;

/*******************************************************************************
 * Bounding sphere
 ******************************************************************************/

BoundingSphere::BoundingSphere() :
	center(vec3(0.0f, 0.0f, 0.0f)),
	radius(0.5f)
{ 

}

BoundingSphere::BoundingSphere(const vec3& _center, float _radius) :
	center(vec3(_center)),
	radius(_radius)
{ 

}

BoundingSphere::BoundingSphere(const BoundingSphere& other) :
	center(other.center),
	radius(other.radius)
{ 

}

bool BoundingSphere::intersects(const Ray& ray) const
{
	vec3 oc = ray.orig - this->center;
	float a = dot(ray.dir, ray.dir);
	float b = 2.0f * dot(ray.dir, ray.orig - this->center);
	float c = dot(oc, oc) - (this->radius * this->radius);

	return ((b * b) - (4.0f * a * c)) >= 0.0f;
}

/******************************************************************************/
