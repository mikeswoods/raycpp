/******************************************************************************
 *
 * Cube geometry definition
 *
 * @file Cube.h
 * @author Michael Woods
 *
 ******************************************************************************/

#include <algorithm>
#include "Cube.h"

using namespace std;

/*****************************************************************************/

// Creates a unit cube centered at (0, 0, 0)
Cube::Cube() :
    Geometry(CUBE),
	p1(P(0.5f, 0.5f, -0.5f)), // BACK_TOP_RIGHT
	p2(P(-0.5f, -0.5f, 0.5f)) // FRONT_BOTTOM_LEFT

{
    this->buildGeometry();
	this->computeCentroid();
	this->buildVolume();
}

Cube::Cube(const Cube& other) :
	Geometry(CUBE),
	p1(other.p1),
	p2(other.p2),
	centroid(other.centroid),
	volume(other.volume)
{ }

Cube::~Cube() 
{

}

const BoundingVolume& Cube::getVolume() const
{
	return this->volume;
}

void Cube::computeCentroid()
{
	this->centroid = P((this->p1.xyz + this->p2.xyz) / 2.0f);
}

void Cube::buildVolume()
{
	float dist1  = glm::distance(this->centroid.xyz, this->p1.xyz);
	float dist2  = glm::distance(this->centroid.xyz, this->p2.xyz);
	float radius = std::max(dist1, dist2);
	this->volume = BoundingSphere(this->centroid, radius + 0.2f);
	//this->volume = TrivialVolume();
}

const P& Cube::getCentroid() const
{
	return this->centroid;
}

void Cube::repr(std::ostream& s) const
{
	s << "Cube<p1=" << this->p1 
	  << ", p2="    << this->p2 
	  << ">";
}

void Cube::buildGeometry()
{
    vertices_.clear();
    normals_.clear();
    indices_.clear();

	glm::vec3 CENTER(0.0f, 0.0f, 0.0f);
	glm::vec3 BACK_TOP_LEFT(-0.5f, 0.5f, -0.5f);     // 0
	glm::vec3 BACK_TOP_RIGHT(0.5f, 0.5f, -0.5f);     // 1
	glm::vec3 FRONT_TOP_LEFT(-0.5f, 0.5f, 0.5f);     // 2
	glm::vec3 FRONT_TOP_RIGHT(0.5f, 0.5f, 0.5f);     // 3
	glm::vec3 BACK_BOTTOM_LEFT(-0.5f, -0.5f, -0.5f); // 4
	glm::vec3 BACK_BOTTOM_RIGHT(0.5f, -0.5f, -0.5f); // 5
	glm::vec3 FRONT_BOTTOM_LEFT(-0.5f, -0.5f, 0.5f); // 6
	glm::vec3 FRONT_BOTTOM_RIGHT(0.5f, -0.5f, 0.5f); // 7

	// Construct vertices:
	vertices_.push_back(BACK_TOP_LEFT);
	vertices_.push_back(BACK_TOP_RIGHT);
	vertices_.push_back(FRONT_TOP_LEFT);
	vertices_.push_back(FRONT_TOP_RIGHT);
	vertices_.push_back(BACK_BOTTOM_LEFT);
	vertices_.push_back(BACK_BOTTOM_RIGHT);
	vertices_.push_back(FRONT_BOTTOM_LEFT);
	vertices_.push_back(FRONT_BOTTOM_RIGHT);

	// Construct normals
	normals_.push_back(glm::normalize(BACK_TOP_LEFT - CENTER));
	normals_.push_back(glm::normalize(BACK_TOP_RIGHT - CENTER));
	normals_.push_back(glm::normalize(FRONT_TOP_LEFT - CENTER));
	normals_.push_back(glm::normalize(FRONT_TOP_RIGHT - CENTER));
	normals_.push_back(glm::normalize(BACK_BOTTOM_LEFT - CENTER));
	normals_.push_back(glm::normalize(BACK_BOTTOM_RIGHT - CENTER));
	normals_.push_back(glm::normalize(FRONT_BOTTOM_LEFT - CENTER));
	normals_.push_back(glm::normalize(FRONT_BOTTOM_RIGHT - CENTER));

	// Construct indices:
	// TOP
	indices_.push_back(0); indices_.push_back(1); indices_.push_back(2);
	indices_.push_back(1); indices_.push_back(2); indices_.push_back(3);
	// LEFT
	indices_.push_back(0); indices_.push_back(2); indices_.push_back(6);
	indices_.push_back(0); indices_.push_back(4); indices_.push_back(6);
	// RIGHT
	indices_.push_back(1); indices_.push_back(3); indices_.push_back(7);
	indices_.push_back(1); indices_.push_back(5); indices_.push_back(7);
	// BOTTOM
	indices_.push_back(4); indices_.push_back(5); indices_.push_back(6);
	indices_.push_back(5); indices_.push_back(6); indices_.push_back(7);
	// BACK
	indices_.push_back(0); indices_.push_back(1); indices_.push_back(5);
	indices_.push_back(0); indices_.push_back(4); indices_.push_back(5);
	// FRONT
	indices_.push_back(2); indices_.push_back(3); indices_.push_back(7);
	indices_.push_back(2); indices_.push_back(6); indices_.push_back(7);
}

Intersection Cube::intersectImpl(const Ray &ray) const
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

    float x1 = (x(this->p1) - x(ray.orig)) / xd;
    float x2 = (x(this->p2) - x(ray.orig)) / xd;
    float y1 = (y(this->p1) - y(ray.orig)) / yd;
    float y2 = (y(this->p2) - y(ray.orig)) / yd;
    float z1 = (z(this->p1) - z(ray.orig)) / zd;
    float z2 = (z(this->p2) - z(ray.orig)) / zd;

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
        return Intersection::miss();
    }

	float t, nx, ny, nz;
	if (tNear < 0.0f) {
		t  = tFar;
		nx = x2; 
		ny = y2;
		nz = z2;
	} else {
		t = tNear;
		nx = x1;
		ny = y1;
		nz = z1;
	}

	// Now, compute the normal:
	glm::vec3 normal;
	if (abs(nx - t) < eps) {
		normal = glm::vec3(nx < 0.0f ? -1.0f : 1.0, 0.0f, 0.0f);
	} else if (abs(ny - t) < eps) {
		normal = glm::vec3(0.0f, ny < 0.0f ? -1.0f : 1.0f, 1.0f);
	} else {
		normal = glm::vec3(0.0f, 0.0f, nz < 0.0f ? -1.0f : 1.0f);
	}

	return Intersection(t, normal);
}

glm::vec3 Cube::sampleImpl() const
{
	// TODO: get the dimensions of the transformed cube in world space
	glm::vec3 dim (0, 0, 0);

	// Get surface area of the cube
	float side1 = dim[0] * dim[1];		// x-y
	float side2 = dim[1] * dim[2];		// y-z
	float side3 = dim[0] * dim[2];		// x-z
	float totalArea = 2.0f * (side1 + side2 + side3);	

	// pick random face weighted by surface area
	float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	// pick 2 random components for the point in the range (-0.5, 0.5)
	float c1 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX) - 0.5f;
	float c2 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX) - 0.5f;

	glm::vec3 point;
	if (r < side1 / totalArea) {				
		// x-y front
		point = glm::vec3(c1, c2, 0.5f);
	} else if (r < (side1 * 2) / totalArea) {
		// x-y back
		point = glm::vec3(c1, c2, -0.5f);
	} else if (r < (side1 * 2 + side2) / totalArea) {
		// y-z front
		point = glm::vec3(0.5f, c1, c2);
	} else if (r < (side1 * 2 + side2 * 2) / totalArea) {
		// y-z back
		point = glm::vec3(-0.5f, c1, c2);
	} else if (r < (side1 * 2 + side2 * 2 + side3) / totalArea) {
		// x-z front 
		point = glm::vec3(c1, 0.5f, c2);
	} else {
		// x-z back
		point = glm::vec3(c1, -0.5f, c2);
	}

	// TODO: transform point to world space
	return point;
}

////////////////////////////////////////////////////////////////////////////////
