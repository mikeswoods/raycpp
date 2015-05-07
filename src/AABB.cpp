/*******************************************************************************
 *
 * Simple axis-aligned bounding box (AABB) implementation
 *
 * @file Raytrace.h
 * @author Michael Woods
 *
 ******************************************************************************/

#define _USE_MATH_DEFINES
#include <cmath>
#include <algorithm>
#include <iostream>
#include "AABB.h"
#include "Utils.h"

/******************************************************************************/

AABB::AABB() :
	v1(glm::vec3()),
    v2(glm::vec3()),
	C(glm::vec3()),
    _width(this->computeWidth()),
    _height(this->computeHeight()),
    _depth(this->computeDepth()),
    _area(this->computeArea())
{ 

}

AABB::AABB(const glm::vec3& _v1, const glm::vec3& _v2) :
    v1(_v1),
    v2(_v2),
    _width(this->computeWidth()),
    _height(this->computeHeight()),
    _depth(this->computeDepth()),
    _area(this->computeArea())
{
    this->C = (this->v1 + this->v2) * 0.5f;
}

AABB::AABB(const AABB& other) :
    v1(other.v1),
    v2(other.v2),
    _width(other._width),
    _height(other._height),
    _depth(other._depth),
    _area(other._area)
{

}

/**
 * Tests if the given ray intersects the AABB
 */
bool AABB::intersected(const Ray& ray) const
{
	float xd  = ray.dir.x;
    float yd  = ray.dir.y;
    float zd  = ray.dir.z;
	float eps = Utils::EPSILON;

    if (xd == 0.0f) {
        xd = eps;
    }
    if (yd == 0.0f) {
        yd = eps;
    }
    if (zd == 0.0f) {
        zd = eps;
    }

    float x1 = (this->v1.x - ray.orig.x) / xd;
    float x2 = (this->v2.x - ray.orig.x) / xd;
    float y1 = (this->v1.y - ray.orig.y) / yd;
    float y2 = (this->v2.y - ray.orig.y) / yd;
    float z1 = (this->v1.z - ray.orig.z) / zd;
    float z2 = (this->v2.z - ray.orig.z) / zd;

    if (x1 > x2) {
		std::swap(x1, x2);
    }
    if (y1 > y2) {
		std::swap(y1, y2);
    }
    if (z1 > z2) {
		std::swap(z1, z2);   
    }

    float tNear = std::max(x1, std::max(y1, z1));
    float tFar  = std::min(x2, std::min(y2, z2));

    if (tNear > tFar || tFar < 0.0f) {
        return false;
    }

	return true;
}

/**
 * Returns the width (x) of the AABB
 */
float AABB::computeWidth() const
{
    return std::max(this->v1.x, this->v2.x) - std::min(this->v1.x, this->v2.x);
}

/**
 * Returns the height (y) of the AABB
 */
float AABB::computeHeight() const
{
    return std::max(this->v1.y, this->v2.y) - std::min(this->v1.y, this->v2.y);
}

/**
 * Returns the depth (z) of the AABB
 */
float AABB::computeDepth() const
{
    return std::max(this->v1.z, this->v2.z) - std::min(this->v1.z, this->v2.z);
}

/**
 * Computes the area of the AABB
 */
float AABB::computeArea() const
{
	return this->width() * this->height() * this->depth();
}

AABB& AABB::operator+=(const AABB &other)
{
    AABB t   = *this + other;
    this->v1 = t.v1;
    this->v2 = t.v2;
    return *this;
}

/******************************************************************************/

std::ostream& operator<<(std::ostream& s, const AABB& aabb)
{
	//return s << "<["<< aabb.v1 << ", "<< aabb.v2 << "]>";
}

const AABB operator+(const AABB& p, const AABB& q)
{
    return AABB(glm::min(glm::min(p.v1, p.v2) ,glm::min(q.v1, q.v2))
               ,glm::max(glm::max(p.v1, p.v2) ,glm::max(q.v1, q.v2)));
}

/******************************************************************************/
