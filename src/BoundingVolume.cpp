#define _USE_MATH_DEFINES
#include <iostream>
#include <algorithm>
#include <cmath>
#include "BoundingVolume.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////
// Bounding sphere
////////////////////////////////////////////////////////////////////////////////

BoundingSphere::BoundingSphere() :
	center(P(0.0f, 0.0f, 0.0f)),
	radius(0.5f)
{ }

BoundingSphere::BoundingSphere(const glm::vec3& _center, float _radius) :
	center(P(_center)),
	radius(_radius)
{ }

BoundingSphere::BoundingSphere(const P& _center, float _radius) :
	center(_center),
	radius(_radius)
{ }

BoundingSphere::BoundingSphere(const BoundingSphere& other) :
	center(other.center),
	radius(other.radius)
{ }

bool BoundingSphere::intersects(const Ray& ray) const
{
	glm::vec3 oc = ray.orig - this->center;
	float a = glm::dot(ray.dir, ray.dir);
	float b = 2.0f * glm::dot(ray.dir, ray.orig - this->center);
	float c = glm::dot(oc, oc) - (this->radius * this->radius);

	return ((b * b) - (4.0f * a * c)) >= 0.0f;
}

////////////////////////////////////////////////////////////////////////////////
