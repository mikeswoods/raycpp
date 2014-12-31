/*******************************************************************************
 *
 * Triangle intersection test
 *
 * @file Triangle.h
 * @author Michael Woods
 *
 ******************************************************************************/

#include <stdexcept>
#include <cassert>
#include <cmath>
#include <algorithm>
#include "Tri.h"
#include "Utils.h"

using namespace std;

/******************************************************************************/

Tri::Tri() :
	meshIndex(-1)
{ 
	
}

Tri::Tri(int _meshIndex, const glm::vec3& _v1, const glm::vec3& _v2, const glm::vec3& _v3) :
    meshIndex(_meshIndex)
{ 
	this->v[0] = P(_v1);
	this->v[1] = P(_v2);
	this->v[2] = P(_v3);

	this->computeNormal();
	this->buildAABB();
}

Tri::Tri(const Tri& other) :
    meshIndex(other.meshIndex),
	normal(other.normal),
	aabb(other.aabb)
{ 
	this->v[0] = other.v[0];
	this->v[1] = other.v[1];
	this->v[2] = other.v[2];
}

float Tri::getXMinima() const
{
	return min(min(x(this->v[0]), x(this->v[1])), x(this->v[2]));
}

float Tri::getYMinima() const
{
	return min(min(y(this->v[0]), y(this->v[1])), y(this->v[2]));
}

float Tri::getZMinima() const
{
	return min(min(z(this->v[0]), z(this->v[1])), z(this->v[2]));
}

float Tri::getXMaxima() const
{
	return max(max(x(this->v[0]), x(this->v[1])), x(this->v[2]));
}

float Tri::getYMaxima() const
{
	return max(max(y(this->v[0]), y(this->v[1])), y(this->v[2]));
}

float Tri::getZMaxima() const
{
	return max(max(z(this->v[0]), z(this->v[1])), z(this->v[2]));
}

void Tri::computeNormal()
{
	this->normal = glm::normalize(glm::cross(this->v[1] - this->v[0], this->v[2] - this->v[0]));
}

V Tri::getNormal() const
{
	return this->normal;
}

/**
 * Builds the AABB of this object
 */
void Tri::buildAABB()
{
	// Find the min and max in X, Y, and Z to use as the two
	// extrema of the AABB:
	float xMin = min(min(x(this->v[0]), x(this->v[1])), x(this->v[2]));
	float xMax = max(max(x(this->v[0]), x(this->v[1])), x(this->v[2]));
	float yMin = min(min(y(this->v[0]), y(this->v[1])), y(this->v[2]));
	float yMax = max(max(y(this->v[0]), y(this->v[1])), y(this->v[2]));
	float zMin = min(min(z(this->v[0]), z(this->v[1])), z(this->v[2]));
	float zMax = max(max(z(this->v[0]), z(this->v[1])), z(this->v[2]));

	this->aabb = AABB(P(xMin, yMin, zMin), P(xMax, yMax, zMax));
}

/**
 * Naive, but correct triangle intersection test
 */
float Tri::naiveIntersect(const Ray& ray, glm::vec3& W) const
{
	glm::vec3 e21 = this->v[1] - this->v[0];
	glm::vec3 e32 = this->v[2] - this->v[1];
	glm::vec3 e13 = this->v[0] - this->v[2];
	glm::vec3 e31 = this->v[2] - this->v[0];

	// First, test if the ray intersects the plane formed by the triangle:
	glm::vec3 k = glm::cross(e21, e31);
	glm::vec3 n = glm::normalize(k);
	float d     = glm::dot(n, this->v[0].xyz);
	
	// Find the normal of the plane, i.e. the cross product of the two
	// vectors that are sides of the triangle A-B and B-C:
	float ndNorm  = glm::dot(n, glm::normalize(ray.dir));

	// Degenerate case: the ray is parallel to the surface triangle
	if (ndNorm == 0) {
		return -1.0f;
	}

	float nd = glm::dot(n, ray.dir);
	
	// Find the point of intersection on the triangle, Q
	float tInTri = (d - (glm::dot(n, ray.orig))) / ndNorm;
	
	// and the actual position from the unnormalized ray. This will make dealing
	// with transformations easier later:
	float t  = (d - (glm::dot(n, ray.orig))) / nd;
	P Q      = ray.project(tInTri);

	glm::vec3 eQ1 = Q - this->v[0];
	glm::vec3 eQ2 = Q - this->v[1];
	glm::vec3 eQ3 = Q - this->v[2];

	// Perform the "in-triangle" test against each vertex:
	float c1 = glm::dot(glm::cross(e21, eQ1), n);
	float c2 = glm::dot(glm::cross(e32, eQ2), n);
	float c3 = glm::dot(glm::cross(e13, eQ3), n);

	if (c1 >= 0.0f && c2 >= 0.0f && c3 >= 0.0f) {

		float nk = glm::dot(n, k);

		// Find the barycentric coordinates:
		W[0] = glm::dot(glm::cross(e32, eQ2), n) / nk; // alpha
		W[1] = glm::dot(glm::cross(e13, eQ3), n) / nk; // beta
		W[2] = glm::dot(glm::cross(e21, eQ1), n) / nk; // gamma

		#ifdef DEBUG
		assert(abs(1.0f - (W[0] + W[1] + W[2])) < 1.0e-6f);
		#endif

		return t;
	}

	return -1.0f;
}

/**
 * Moller-Trumbore intersection test
 */
float Tri::mollerTrumboreIntersect(const Ray& ray, glm::vec3& W) const
{
	glm::vec3 e1 = this->v[1] - this->v[0];
	glm::vec3 e2 = this->v[2] - this->v[0];
	glm::vec3 D  = ray.dir;
	glm::vec3 P  = glm::cross(D, e2);
	float det    = glm::dot(e1, P);
	float eps    = FLT_EPSILON;

	if (det > -eps && det < eps) {
		return -1.0f;
	}

	float invDet = 1.0f / det;
	glm::vec3 T  = ray.orig - this->v[0];

	float u = glm::dot(T, P) * invDet;
	if (u < 0.0f || u > 1.0f) {
		return -1.0f;
	}

	glm::vec3 Q  = glm::cross(T, e1);
	float v      = glm::dot(D, Q) * invDet;

	if (v < 0.0f || (u + v) > 1.0f) {
		return -1.0f;
	}

	float t = glm::dot(e2, Q) * invDet;
	W = glm::vec3(1.0f - u - v, u, v);

	#ifdef DEBUG
	assert(abs(1.0f - (W[0] + W[1] + W[2])) < 1.0e-6f);
	#endif

	if (t > eps) {
		return t;
	}

	return -1.0f;
}

glm::vec3 Tri::barycenter(const P& p) const
{
    glm::vec3 v0 = this->v[1] - this->v[0]; 
    glm::vec3 v1 = this->v[2] - this->v[0];
    glm::vec3 v2 = p - this->v[0];
    float d00 = glm::dot(v0, v0);
    float d01 = glm::dot(v0, v1);
    float d11 = glm::dot(v1, v1);
    float d20 = glm::dot(v2, v0);
    float d21 = glm::dot(v2, v1);
    float denom = d00 * d11 - d01 * d01;

    glm::vec3 W;
    W[1] = (d11 * d20 - d01 * d21) / denom;
    W[2] = (d00 * d21 - d01 * d20) / denom;
    W[0] = 1.0f - W[1] - W[2];

    return W;
}

float Tri::intersected(const Ray& ray, glm::vec3& W) const
{
	//return this->naiveIntersect(ray, W);
	return this->mollerTrumboreIntersect(ray, W);
}

/******************************************************************************/