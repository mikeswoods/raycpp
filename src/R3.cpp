#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <limits>
#include "R3.h"

////////////////////////////////////////////////////////////////////////////////
// Vector type in R3 space
////////////////////////////////////////////////////////////////////////////////

const P P::AT_INFINITY = P(std::numeric_limits<float>::infinity()
                          ,std::numeric_limits<float>::infinity()
                          ,std::numeric_limits<float>::infinity());

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
std::ostream& operator<<(std::ostream &s, const V &v)
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

////////////////////////////////////////////////////////////////////////////////
// Point type in R3 space
////////////////////////////////////////////////////////////////////////////////

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

// P copy constructor
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

// Get a string representation of a point:
std::ostream& operator<<(std::ostream &s, const P &p)
{
    return s << "[" << x(p) << "," << y(p) << "," << z(p) << "]";
}

// Compute the distance between this point and another
float dist(const P &p1, const P &p2)
{
    return glm::distance(p1.xyz, p2.xyz);
}

// Computes the location of a point relative to a given origin position
P relative(const P& p, const P& origin)
{
    return P(0.0f, 0.0f, 0.0f) + (origin - p);
}

// Step along calculation, returning the number of steps as well as
// setting the initial position point X and step vector N
int traverse(float stepSize, float offset, const P& start, const P& end, P& X, V& N)
{
    V D = glm::normalize(end - start);
    N = D * stepSize;
    X = start + (D * offset);
    return (int)ceil(dist(start, end) / stepSize);
}

bool operator==(const P& p, const P& q)
{
	return p.xyz == q.xyz;
}

bool operator!=(const P& p, const P& q)
{
	return p.xyz != q.xyz;
}

// Add a point and a vector yields a point:
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

// Subtracting a point and a point yields a vector:
V operator-(const P &p1, const P &p2)
{
    return V(x(p1) - x(p2), y(p1) - y(p2), z(p1) - z(p2));
}

P operator-(const P& p, float mu)
{
    return P(x(p) - mu, y(p) - mu, z(p) - mu);   
}

////////////////////////////////////////////////////////////////////////////////

glm::vec3 transform(glm::mat4 T, glm::vec4 pointOrVector)
{
	glm::vec4 result = T * pointOrVector;
	// First see if we have a "point" or a vector: if 
	if (pointOrVector.w != 0.0f) { // Points
		float w = result.w == 0.0f ? 1.0f : result.w;
		return glm::vec3(result.x / w, result.y / w, result.z / w);
	} else { // Vector
		return glm::vec3(result);
	}
}

////////////////////////////////////////////////////////////////////////////////