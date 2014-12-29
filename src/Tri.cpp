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
	Geometry(TRI),
	meshIndex(-1)
{ 
	
}

Tri::Tri(int _meshIndex, const P& v1, const P& v2, const P& v3) :
    Geometry(TRI),
    meshIndex(_meshIndex)
{
	this->v[0] = v1;
	this->v[1] = v2;
	this->v[2] = v3;

	this->buildGeometry();
	this->computeCentroid();
	this->computeNormal();
	this->buildVolume();
	this->buildAABB();
}

Tri::Tri(int _meshIndex, const glm::vec3& _v1, const glm::vec3& _v2, const glm::vec3& _v3) :
    Geometry(TRI),
    meshIndex(_meshIndex)
{ 
	this->v[0] = P(_v1);
	this->v[1] = P(_v2);
	this->v[2] = P(_v3);

	this->buildGeometry();
	this->computeCentroid();
	this->computeNormal();
	this->buildVolume();
	this->buildAABB();
}

Tri::Tri(const Tri& other) :
    Geometry(TRI),
    meshIndex(other.meshIndex),
	normal(other.normal),
	centroid(other.centroid),
	volume(other.volume),
	aabb(other.aabb)
{ 
	this->v[0] = other.v[0];
	this->v[1] = other.v[1];
	this->v[2] = other.v[2];
}

void Tri::repr(ostream& s) const
{
	s << "Tri<" << this->meshIndex << ">"
	  << "["<< this->v[0] <<","<< this->v[1] << this->v[2] << "]"
	  << ",centroid=" << this->centroid
	  << "}";
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

bool Tri::hasVertex(const P& u) const
{
	for (auto i=0; i <3; i++) {
		if (u == this->v[i]) {
			return true;
		}
	}
	return false;
}

bool Tri::hasVertex(const glm::vec3& v) const
{
	return this->hasVertex(P(v));
}

void Tri::computeCentroid()
{
	this->centroid = P((this->v[0].xyz + this->v[1].xyz + this->v[2].xyz) / 3.0f);
}

void Tri::computeNormal()
{
	this->normal = glm::normalize(glm::cross(this->v[1] - this->v[0], this->v[2] - this->v[0]));
}

V Tri::getNormal() const
{
	return this->normal;
}

const P& Tri::getCentroid() const
{
	return this->centroid;
}

/**
 * Builds the bounding volume used by this object for faster intersection
 * testing
 */
void Tri::buildVolume()
{
	float dist1  = glm::distance(this->centroid.xyz, this->v[0].xyz);
	float dist2  = glm::distance(this->centroid.xyz, this->v[1].xyz);
	float dist3  = glm::distance(this->centroid.xyz, this->v[2].xyz);
	float radius = max(dist1, max(dist2, dist3));

	this->volume = BoundingSphere(this->centroid, radius + Utils::EPSILON);
}

const BoundingVolume& Tri::getVolume() const
{
	return this->volume;
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

void Tri::buildGeometry()
{
	// Do nothing
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

		assert(abs(1.0f - (W[0] + W[1] + W[2])) < 1.0e-6f);

		return t;
	}

	return -1.0f;
}

/**
 * Moller-Trumbore intersection test
 */
float Tri::mollerTrumboreIntersect(const Ray& ray, glm::vec3& W) const
{
	W = glm::vec3(0.0f,0.0f,0.0f);

	glm::vec3 e1 = this->v[1] - this->v[0];
	glm::vec3 e2 = this->v[2] - this->v[0];
	glm::vec3 D  = glm::normalize(ray.dir);
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

	if (t > eps) {
		W = glm::vec3(u, v, 1.0f - (u + v));
		return t;
	}

	return -1.0f;
}

float Tri::intersected(const Ray& ray, glm::vec3& W) const
{
	return this->naiveIntersect(ray, W);
	//return this->mollerTrumboreIntersect(ray, W);
}

Intersection Tri::intersectImpl(const Ray &ray) const
{
	glm::vec3 W;
	return Intersection(this->intersected(ray, W), this->getNormal());
}

glm::vec3 Tri::sampleImpl() const
{
	throw runtime_error("Tri::sampleImpl() not implemented");
}

/******************************************************************************/