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
	Vs(tuple<P,P>(P(), P())),
	C(P()),
    _width(this->computeWidth()),
    _height(this->computeHeight()),
    _depth(this->computeDepth()),
    _area(this->computeArea())
{ 

}

AABB::AABB(const P& v1, const P& v2) :
    Vs(tuple<P,P>(v1, v2)),
    _width(this->computeWidth()),
    _height(this->computeHeight()),
    _depth(this->computeDepth()),
    _area(this->computeArea())
{
    this->C = mean(get<0>(this->Vs), get<1>(this->Vs));
}

AABB::AABB(const AABB& other) :
    Vs(other.Vs),
    C(other.C),
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

    float x1 = (x(get<0>(this->Vs)) - x(ray.orig)) / xd;
    float x2 = (x(get<1>(this->Vs)) - x(ray.orig)) / xd;
    float y1 = (y(get<0>(this->Vs)) - y(ray.orig)) / yd;
    float y2 = (y(get<1>(this->Vs)) - y(ray.orig)) / yd;
    float z1 = (z(get<0>(this->Vs)) - z(ray.orig)) / zd;
    float z2 = (z(get<1>(this->Vs)) - z(ray.orig)) / zd;

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
float AABB::computeWidth() const
{
    return max(x(get<0>(this->Vs)), x(get<1>(this->Vs))) - 
           min(x(get<0>(this->Vs)), x(get<1>(this->Vs))); 
}

/**
 * Returns the height (y) of the AABB
 */
float AABB::computeHeight() const
{
    return max(y(get<0>(this->Vs)), y(get<1>(this->Vs))) - 
           min(y(get<0>(this->Vs)), y(get<1>(this->Vs)));
}

/**
 * Returns the depth (z) of the AABB
 */
float AABB::computeDepth() const
{
    return max(z(get<0>(this->Vs)), z(get<1>(this->Vs))) -  
           min(z(get<0>(this->Vs)), z(get<1>(this->Vs)));
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
    AABB t = *this + other;
    get<0>(this->Vs) = get<0>(t.Vs);
    get<1>(this->Vs) = get<1>(t.Vs);
    return *this;
}

/******************************************************************************/

ostream& operator<<(ostream& s, const AABB& aabb)
{
	return s << "<[" 
             << get<0>(aabb.Vs) 
             << ", " 
             << get<0>(aabb.Vs) 
             << "]>";
}

const AABB operator+(const AABB& p, const AABB& q)
{
    return AABB(minimum(minimum(get<0>(p.Vs), get<1>(p.Vs))
                       ,minimum(get<0>(q.Vs), get<1>(q.Vs)))
               ,maximum(maximum(get<0>(p.Vs), get<1>(p.Vs))
                       ,maximum(get<0>(q.Vs), get<1>(q.Vs))));
}

/******************************************************************************/
