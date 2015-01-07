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
#include "R3.h"
#include "AABB.h"
#include "Utils.h"

using namespace std;

/******************************************************************************/

AABB::AABB() :
	v1(P(0, 0, 0)),
	v2(P(0, 0, 0)),
	C(P(0, 0, 0))
{ 
	
}

AABB::AABB(const P& _v1, const P& _v2) :
	v1(_v1),
	v2(_v2)
{ 
	this->C = this->computeCentroid();		
}

AABB::AABB(const AABB& other) :
	v1(other.v1),
	v2(other.v2)	
{ 
	this->C = this->computeCentroid();	
}

/**
 * Internal method: computes the centroid (averaged center point) of the AABB
 */
P AABB::computeCentroid() const
{
	return P((x(this->v1) + x(this->v2)) / 2.0f
		    ,(y(this->v1) + y(this->v2)) / 2.0f
			,(z(this->v1) + z(this->v2)) / 2.0f);
}

/**
 * Tests if the given ray intersects the AABB
 */
bool AABB::intersected(const Ray& ray) const
{
	float xd  = x(ray.dir);
    float yd  = y(ray.dir);
    float zd  = z(ray.dir);
	float eps = Utils::EPSILON;

    if (xd == 0) {
        xd = eps;
    }
    if (yd == 0) {
        yd = eps;
    }
    if (zd == 0) {
        zd = eps;
    }

    float x1 = (x(this->v1) - x(ray.orig)) / xd;
    float x2 = (x(this->v2) - x(ray.orig)) / xd;
    float y1 = (y(this->v1) - y(ray.orig)) / yd;
    float y2 = (y(this->v2) - y(ray.orig)) / yd;
    float z1 = (z(this->v1) - z(ray.orig)) / zd;
    float z2 = (z(this->v2) - z(ray.orig)) / zd;

    if (x1 > x2) {
		swap(x1,x2);
    }
    if (y1 > y2) {
		swap(y1,y2);
    }
    if (z1 > z2) {
		swap(z1,z2);   
    }

    float tNear = max(x1, max(y1, z1));
    float tFar  = min(x2, min(y2, z2));

    if (tNear > tFar || tFar < 0) {
        return false;
    }

	return true;
}

/**
 * Returns the width (x) of the AABB
 */
float AABB::width() const
{
    return max(x(this->v1), x(this->v2)) - min(x(this->v1), x(this->v2)); 
}

/**
 * Returns the height (y) of the AABB
 */
float AABB::height() const
{
    return max(y(this->v1), y(this->v2)) - min(y(this->v1), y(this->v2));
}

/**
 * Returns the depth (z) of the AABB
 */
float AABB::depth() const
{
    return max(z(this->v1), z(this->v2)) -  min(z(this->v1), z(this->v2));
}

/**
 * Computes the area of the AABB
 */
float AABB::area() const
{
	return this->width() * this->height() * this->depth();
}

AABB& AABB::operator+=(const AABB &other)
{
    AABB t = *this + other;
    this->v1 = t.v1;
    this->v2 = t.v2;
    return *this;
}

/******************************************************************************/

ostream& operator<<(ostream& s, const AABB& aabb)
{
	return s << "<[" << aabb.v1 << ", " << aabb.v2 << "]>";
}

const AABB operator+(const AABB& p, const AABB& q)
{
    return AABB(minimum(minimum(p.v1, p.v2), minimum(q.v1, q.v2))
               ,maximum(maximum(p.v1, p.v2), maximum(q.v1, q.v2)));
}

/******************************************************************************/
