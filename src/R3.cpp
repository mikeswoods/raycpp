/*******************************************************************************
 *
 * This file defines operations over point and vector data types in R^3 space
 *
 * @file R3.h
 * @author Michael Woods
 *
 ******************************************************************************/

#define _USE_MATH_DEFINES
#include <cmath>
#include <algorithm>
#include <iostream>
#include <limits>
#include "R3.h"

using namespace std;

float x(const V& v) 
{
    return v.x;
}

float y(const V& v) 
{
    return v.y;
}

float z(const V& v) 
{
    return v.z;
}

// Get a string representation of a vector:
ostream& operator<<(ostream &s, const V &v)
{
    return s << "<" << v.x << "," << v.y << "," << v.z << ">";
}

// Multiply a vector by a scalar yields a vector:
V operator*(const V &v, float s)
{
    return V(x(v) * s, y(v) * s, z(v) * s);
}

V operator/(const V &v, float s)
{
    return V(x(v) / s, y(v) / s, z(v) / s);
}

P::P()
{
    this->xyz = glm::vec3(0.0f, 0.0f, 0.0f);
}

P::P(glm::vec3 xyz)
{
	this->xyz = xyz;
}

P::P(float x, float y, float z)
{
    this->xyz = glm::vec3(x, y, z);
}

P::P(float xyz[3])
{
    this->xyz = glm::vec3(xyz[0], xyz[1], xyz[2]);
}

P::P(const P &other)
{
    this->xyz = other.xyz;
}

float x(const P& p)
{
    return p.xyz.x;
}

float y(const P& p)
{
    return p.xyz.y;
}

float z(const P& p)
{
    return p.xyz.z;
}

glm::vec3 toVec3(const P& p)
{
	return glm::vec3(p.xyz);
}

/**
 * Get a string representation of a point
 */
ostream& operator<<(ostream &s, const P &p)
{
    return s << "[" << x(p) << "," << y(p) << "," << z(p) << "]";
}

bool operator==(const P& p, const P& q)
{
	return p.xyz == q.xyz;
}

bool operator!=(const P& p, const P& q)
{
	return p.xyz != q.xyz;
}

P operator+(const P &p, const V &v)
{
    return P(x(p) + x(v), y(p) + y(v), z(p) + z(v));
}

P operator+(const P& p, float mu)
{
    return P(x(p) + mu, y(p) + mu, z(p) + mu);   
}

P operator*(const P& p, float mu)
{
    return P(x(p) * mu, y(p) * mu, z(p) * mu);   
}

P& P::operator+=(const V &v)
{
    this->xyz += v;
    return *this;
}

P& P::operator-=(const V &v)
{
    this->xyz -= v;
    return *this;
}

/**
 * Subtracting a point and a point yields a vector
 */
V operator-(const P &p1, const P &p2)
{
    return V(x(p1) - x(p2), y(p1) - y(p2), z(p1) - z(p2));
}

P operator-(const P& p, float mu)
{
    return P(x(p) - mu, y(p) - mu, z(p) - mu);   
}

/**
 * Homogenous coordinate transformation on a point or vector. 
 *
 * If the fourth/"w" component of V is 0, V is interpreted as a vector, 
 * otherwise, it is interpreted as a position.
 */
glm::vec3 transform(glm::mat4 T, glm::vec4 V)
{
	glm::vec4 result = T * V;
	if (V.w != 0.0f) { // Points
		float w = result.w == 0.0f ? 1.0f : result.w;
		return glm::vec3(result.x / w, result.y / w, result.z / w);
	} else { // Vector
		return glm::vec3(result);
	}
}

/** 
 * Step along calculation, returning the number of steps as well as
 * setting the initial position point X and step vector N
 */
int steps(float stepSize, float offset, const P& start, const P& end, P& X, V& N)
{
    V D = glm::normalize(end - start);
    N = D * stepSize;
    X = start + (D * offset);
    return static_cast<int>(ceil(glm::distance(start.xyz, end.xyz) / stepSize));
}

int steps(float stepSize, float offset, const P& start, const V& along, P& X, V& N)
{
    return steps(stepSize, offset, start, start + along, X, N);
}

/**
 * Computes the centroid point between the two given points
 */
glm::vec3 mean(const glm::vec3& p, const glm::vec3& q)
{
    return glm::vec3((p.x + q.x) / 2.0f, (p.y + q.y) / 2.0f, (p.z + q.z) / 2.0f);
}

glm::vec3 mean(const std::vector<glm::vec3>& ps)
{
    glm::vec3 T;
    size_t N = ps.size();
    for (auto i=ps.begin(); i != ps.end(); i++) {
        T += *i;
    }
    return glm::vec3(T.x / N, T.y / N, T.z / N);
}

P mean(const P& p, const P& q)
{
    return P(mean(p.xyz, q.xyz));
}

P mean(const std::vector<P>& ps)
{
    glm::vec3 T;
    size_t N = ps.size();
    for (auto i=ps.begin(); i != ps.end(); i++) {
        T += i->xyz;
    }
    return P(T.x / N, T.y / N, T.z / N);
}

/**
 * Given two points, this function returns a new point consisting of 
 * the components with overall maximum values
 */
glm::vec3 maximum(const glm::vec3& p, const glm::vec3& q)
{
    return glm::vec3(max(p.x, q.x), max(p.y, q.y), max(p.z, q.z));
}

glm::vec3 maximum(const std::vector<glm::vec3>& ps)
{
    float tx, ty, tz;
    tx = ty = tz = -numeric_limits<float>::infinity();
    for (auto i=ps.begin(); i != ps.end(); i++) {
        tx = max(tx, i->x);
        ty = max(ty, i->y);
        tz = max(tz, i->z);
    }
    return glm::vec3(tx, ty, tz);
}

P maximum(const P& p, const P& q)
{
    return P(maximum(p.xyz, q.xyz));
}

P maximum(const std::vector<P>& ps)
{
    float tx, ty, tz;
    tx = ty = tz = -numeric_limits<float>::infinity();
    for (auto i=ps.begin(); i != ps.end(); i++) {
        tx = max(tx, x(*i));
        ty = max(ty, y(*i));
        tz = max(tz, z(*i));
    }
    return P(tx, ty, tz);
}

/**
 * Given two points, this function returns a new point consisting of 
 * the components with overall minimum values
 */
glm::vec3 minimum(const glm::vec3& p, const glm::vec3& q)
{
    return glm::vec3(min(p.x, q.x), min(p.y, q.y), min(p.z, q.z));
}

glm::vec3 minimum(const std::vector<glm::vec3>& ps)
{
    float tx, ty, tz;
    tx = ty = tz = -numeric_limits<float>::infinity();
    for (auto i=ps.begin(); i != ps.end(); i++) {
        tx = max(tx, i->x);
        ty = max(ty, i->y);
        tz = max(tz, i->z);
    }
    return glm::vec3(tx, ty, tz);
}

P minimum(const P& p, const P& q)
{
    return P(minimum(p.xyz, q.xyz));
}

P minimum(const std::vector<P>& ps)
{
    float tx, ty, tz;
    tx = ty = tz = -numeric_limits<float>::infinity();
    for (auto i=ps.begin(); i != ps.end(); i++) {
        tx = max(tx, x(*i));
        ty = max(ty, y(*i));
        tz = max(tz, z(*i));
    }
    return P(tx, ty, tz);
}

