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

Tri::Tri(unsigned int _meshIndex
	    ,glm::uvec3 _indices
	    ,glm::vec3 _v1
	    ,glm::vec3 _v2
	    ,glm::vec3 _v3) :
    meshIndex(_meshIndex)
{
	this->indices[0] = _indices[0];
	this->indices[1] = _indices[1];
	this->indices[2] = _indices[2];

	this->vertices[0] = _v1;
	this->vertices[1] = _v2;
	this->vertices[2] = _v3;

	this->computeNormal();
	this->buildAABB();
}

Tri::Tri(const Tri& other) :
    meshIndex(other.meshIndex),
    indices(other.indices),
	normal(other.normal),
	aabb(other.aabb)
{ 
	this->vertices[0] = other.vertices[0];
	this->vertices[1] = other.vertices[1];
	this->vertices[2] = other.vertices[2];
}

float Tri::getXMinima() const
{
	return std::min(std::min(this->vertices[0].x, this->vertices[1].x), this->vertices[2].x);
}

float Tri::getYMinima() const
{
	return std::min(std::min(this->vertices[0].y, this->vertices[1].y), this->vertices[2].y);
}

float Tri::getZMinima() const
{
	return std::min(std::min(this->vertices[0].z, this->vertices[1].z), this->vertices[2].z);
}

float Tri::getXMaxima() const
{
	return std::max(std::max(this->vertices[0].x, this->vertices[1].x), this->vertices[2].x);
}

float Tri::getYMaxima() const
{
	return std::max(std::max(this->vertices[0].y, this->vertices[1].y), this->vertices[2].y);
}

float Tri::getZMaxima() const
{
	return std::max(std::max(this->vertices[0].z, this->vertices[1].z), this->vertices[2].z);
}

void Tri::computeNormal()
{
	this->normal = glm::normalize(glm::cross(this->vertices[1] - this->vertices[0], this->vertices[2] - this->vertices[0]));
}

glm::vec3 Tri::getNormal() const
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
	float xMin = std::min(std::min(this->vertices[0].x, this->vertices[1].x), this->vertices[2].x);
	float xMax = std::max(std::max(this->vertices[0].x, this->vertices[1].x), this->vertices[2].x);
	float yMin = std::min(std::min(this->vertices[0].y, this->vertices[1].y), this->vertices[2].y);
	float yMax = std::max(std::max(this->vertices[0].y, this->vertices[1].y), this->vertices[2].y);
	float zMin = std::min(std::min(this->vertices[0].z, this->vertices[1].z), this->vertices[2].z);
	float zMax = std::max(std::max(this->vertices[0].z, this->vertices[1].z), this->vertices[2].z);

	this->aabb = AABB(glm::vec3(xMin, yMin, zMin), glm::vec3(xMax, yMax, zMax));
}

/**
 * Naive, but correct triangle intersection test
 */
float Tri::naiveIntersect(const Ray& ray, glm::vec3& W) const
{
	glm::vec3 e21 = this->vertices[1] - this->vertices[0];
	glm::vec3 e32 = this->vertices[2] - this->vertices[1];
	glm::vec3 e13 = this->vertices[0] - this->vertices[2];
	glm::vec3 e31 = this->vertices[2] - this->vertices[0];

	// First, test if the ray intersects the plane formed by the triangle:
	glm::vec3 k = glm::cross(e21, e31);
	glm::vec3 n = glm::normalize(k);
	float d     = glm::dot(n, this->vertices[0]);
	
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
	float t     = (d - (glm::dot(n, ray.orig))) / nd;
	glm::vec3 Q = ray.project(tInTri);

	glm::vec3 eQ1 = Q - this->vertices[0];
	glm::vec3 eQ2 = Q - this->vertices[1];
	glm::vec3 eQ3 = Q - this->vertices[2];

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
	glm::vec3 e1 = this->vertices[1] - this->vertices[0];
	glm::vec3 e2 = this->vertices[2] - this->vertices[0];
	glm::vec3 D  = ray.dir;
	glm::vec3 P  = glm::cross(D, e2);
	float det    = glm::dot(e1, P);
	float eps    = FLT_EPSILON;

	if (det > -eps && det < eps) {
		return -1.0f;
	}

	float invDet = 1.0f / det;
	glm::vec3 T  = ray.orig - this->vertices[0];

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

glm::vec3 Tri::barycenter(const glm::vec3& p) const
{
    glm::vec3 v0 = this->vertices[1] - this->vertices[0]; 
    glm::vec3 v1 = this->vertices[2] - this->vertices[0];
    glm::vec3 v2 = p - this->vertices[0];
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