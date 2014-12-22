#include <stdexcept>
#include <algorithm>
#include "Tri.h"
#include "Utils.h"

using namespace std;

/*****************************************************************************/

Tri::Tri() :
	Geometry(TRI),
	v1(P(0.0f, 0.0f, 0.0f)),
	v2(P(0.0f, 0.0f, 0.0f)),
	v3(P(0.0f, 0.0f, 0.0f))
{ 
	
}

Tri::Tri(const P& _v1
	    ,const P& _v2
		,const P& _v3) :
    Geometry(TRI),
	v1(_v1),
	v2(_v2),
	v3(_v3)
{
	this->buildGeometry();
	this->computeCentroid();
	this->computeNormal();
	this->buildVolume();
	this->buildAABB();
}

Tri::Tri(const glm::vec3& _v1
	    ,const glm::vec3& _v2
		,const glm::vec3& _v3) :
    Geometry(TRI),
	v1(_v1),
	v2(_v2),
	v3(_v3)
{ 
	this->buildGeometry();
	this->computeCentroid();
	this->computeNormal();
	this->buildVolume();
	this->buildAABB();
}

Tri::Tri(const Tri& other) :
    Geometry(TRI),
	v1(other.v1),
	v2(other.v2),
	v3(other.v3),
	n1(other.n1),
	n2(other.n2),
	n3(other.n3),
	normal(other.normal),
	centroid(other.centroid),
	volume(other.volume),
	aabb(other.aabb)
{ 
	
}

void Tri::repr(ostream& s) const
{
	s << "Tri "
	  << "{v1="       << this->v1 
	  << ",v2="       << this->v2 
	  << ",v3="       << this->v3
	  << ",centroid=" << this->centroid
	  << "}";
}

float Tri::getXMinima() const
{
	return min(min(x(this->v1), x(this->v2)), x(this->v3));
}

float Tri::getYMinima() const
{
	return min(min(y(this->v1), y(this->v2)), y(this->v3));
}

float Tri::getZMinima() const
{
	return min(min(z(this->v1), z(this->v2)), z(this->v3));
}

float Tri::getXMaxima() const
{
	return max(max(x(this->v1), x(this->v2)), x(this->v3));
}

float Tri::getYMaxima() const
{
	return max(max(y(this->v1), y(this->v2)), y(this->v3));
}

float Tri::getZMaxima() const
{
	return max(max(z(this->v1), z(this->v2)), z(this->v3));
}

void Tri::computeCentroid()
{
	this->centroid = P((this->v1.xyz + this->v2.xyz + this->v3.xyz) / 3.0f);
}

void Tri::computeNormal()
{
	this->normal = glm::normalize(glm::cross(v3 - v1, v2 - v1));
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
	float dist1  = glm::distance(this->centroid.xyz, this->v1.xyz);
	float dist2  = glm::distance(this->centroid.xyz, this->v2.xyz);
	float dist3  = glm::distance(this->centroid.xyz, this->v3.xyz);
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
	float xMin = min(min(x(this->v1), x(this->v2)), x(this->v3));
	float xMax = max(max(x(this->v1), x(this->v2)), x(this->v3));
	float yMin = min(min(y(this->v1), y(this->v2)), y(this->v3));
	float yMax = max(max(y(this->v1), y(this->v2)), y(this->v3));
	float zMin = min(min(z(this->v1), z(this->v2)), z(this->v3));
	float zMax = max(max(z(this->v1), z(this->v2)), z(this->v3));

	this->aabb = AABB(P(xMin, yMin, zMin), P(xMax, yMax, zMax));
}

void Tri::buildGeometry()
{
	// Do nothing
}

bool Tri::barycenter(const glm::vec3& test, glm::vec3& weights)
{
	glm::vec3 e1 = this->v2 - this->v1;
	glm::vec3 e2 = this->v3 - this->v1;
	glm::vec3 e3 = test - this->v1;
	float d11    = glm::dot(e1, e1);
	float d12    = glm::dot(e1, e2);
	float d22    = glm::dot(e2, e2);
	float d31    = glm::dot(e3, e1);
	float d32    = glm::dot(e3, e2);
	float denom  = (d11 * d22) - (d12 * d12);

	weights[1] = ((d22 * d31) - (d12 * d32)) / denom;
	weights[2] = ((d11 * d32) - (d12 * d31)) / denom;
	weights[0] = 1.0f - weights[1] - weights[2];

	return weights[1] >= 0.0f && weights[1] >= 0.0f && weights[2] >= 0.0f;
}

/**
 * From http://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
 * 
 * Tests if the given ray intersected the triangle. If the returned value 
 * is less than zero, the ray does not intersect the triangle. Otherwise, the 
 * distance value t from the ray's origin on the direction vector to the point of
 * intersection is returned
 */
float Tri::intersected(const Ray& ray, bool normalizeDir) const
{
	glm::vec3 e1 = this->v2 - this->v1;
	glm::vec3 e2 = this->v3 - this->v1;
	glm::vec3 D  = normalizeDir ? glm::normalize(ray.dir) : ray.dir;
	glm::vec3 P  = glm::cross(D, e2);
	float det    = glm::dot(e1, P);
	float eps    = FLT_EPSILON;

	if (det > -eps && det < eps) {
		return -1.0f;
	}

	float invDet = 1.0f / det;
	glm::vec3 T  = ray.orig - this->v1;

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
		return t;
	}

	return -1.0f;
}

Intersection Tri::intersectImpl(const Ray &ray, const glm::mat4& T) const
{
	return Intersection(this->intersected(ray, false), this->getNormal());
}

glm::vec3 Tri::sampleImpl() const
{
	throw runtime_error("Tri::sampleImpl() not implemented");
}

/*****************************************************************************/
