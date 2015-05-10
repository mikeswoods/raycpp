/******************************************************************************
 *
 * Sphere geometry definition
 *
 * @file Sphere.h
 * @author Michael Woods
 *
 ******************************************************************************/

#define _USE_MATH_DEFINES
#include <algorithm>
#include <cmath>
#include "Sphere.h"
#include "Utils.h"

/******************************************************************************/

using namespace std;

/******************************************************************************/

// Creates a unit sphere centered about the world origin
Sphere::Sphere() :
    Geometry(SPHERE),
    center_(glm::vec3(0.f, 0.f, 0.f)),
    radius_(1.0f)
{
    this->buildGeometry();
	this->buildVolume();
    this->computeAABB();
}

Sphere::Sphere(const glm::vec3& _center, float _radius) :
    Geometry(SPHERE),
    center_(_center),
    radius_(_radius)
{

}

Sphere::~Sphere() 
{

}

void Sphere::repr(ostream& s) const
{
    s << "Sphere";
}

void Sphere::buildVolume()
{
    this->volume = BoundingSphere(this->center_, this->radius_ + Utils::EPSILON);
}

void Sphere::computeAABB()
{
    this->aabb = AABB(this->center_ - this->radius_, this->center_ + this->radius_);
}

const glm::vec3& Sphere::getCentroid() const
{
	return this->center_;
}

const BoundingVolume& Sphere::getVolume() const
{
	return this->volume;
}

const AABB& Sphere::getAABB() const
{
    return this->aabb;
}

void Sphere::buildGeometry()
{
    vertices_.clear();
    normals_.clear();
    indices_.clear();

    // Find vertex positions for the sphere.
    unsigned int subdiv_axis = 16;      // vertical slices
    unsigned int subdiv_height = 16;        // horizontal slices
    float dphi = static_cast<float>(M_PI) / subdiv_height;
    float dtheta = 2.0f * static_cast<float>(M_PI) / subdiv_axis;
    float epsilon = 0.0001f;
    glm::vec3 color (0.6f, 0.6f, 0.6f);

    // North pole
    glm::vec3 point (0.0f, 1.0f, 0.0f);
    normals_.push_back(point);
    // scale by radius_ and translate by center_
    vertices_.push_back(center_ + radius_ * point);

    for (float phi = dphi; phi < static_cast<float>(M_PI); phi += dphi) {
        for (float theta = dtheta; theta <= 2.0f * static_cast<float>(M_PI) + epsilon; theta += dtheta) {
            float sin_phi = sin(phi);

            point[0] = sin_phi * sin(theta);
            point[1] = cos(phi);
            point[2] = sin_phi * cos(theta);

            normals_.push_back(point);
            vertices_.push_back(center_ + radius_ * point);
        }
    }
    // South pole
    point = glm::vec3(0.0f, -1.0f, 0.0f);
    normals_.push_back(point);
    vertices_.push_back(center_ + radius_ * point);

    // fill in index array.
    // top cap
    for (unsigned int i = 0; i < subdiv_axis - 1; ++i) {
        indices_.push_back(0);
        indices_.push_back(i + 1);
        indices_.push_back(i + 2);
    }
    indices_.push_back(0);
    indices_.push_back(subdiv_axis);
    indices_.push_back(1);

    // middle subdivs
    unsigned int index = 1;
    for (unsigned int i = 0; i < subdiv_height - 2; i++) {
        for (unsigned int j = 0; j < subdiv_axis - 1; j++) {
            // first triangle
            indices_.push_back(index);
            indices_.push_back(index + subdiv_axis);
            indices_.push_back(index + subdiv_axis + 1);

            // second triangle
            indices_.push_back(index);
            indices_.push_back(index + subdiv_axis + 1);
            indices_.push_back(index + 1);

            index++;
        }
        // reuse vertices from start and end point of subdiv_axis slice
        indices_.push_back(index);
        indices_.push_back(index + subdiv_axis);
        indices_.push_back(index + 1);

        indices_.push_back(index);
        indices_.push_back(index + 1);
        indices_.push_back(index + 1 - subdiv_axis);

        index++;
    }

    // end cap
    unsigned int bottom = (subdiv_height - 1) * subdiv_axis + 1;
    unsigned int offset = bottom - subdiv_axis;
    for (unsigned int i = 0; i < subdiv_axis - 1 ; ++i) {
        indices_.push_back(bottom);
        indices_.push_back(i + offset);
        indices_.push_back(i + offset + 1);
    }
    indices_.push_back(bottom);
    indices_.push_back(bottom - 1);
    indices_.push_back(offset);
}

Intersection Sphere::intersectImpl(const Ray &ray, shared_ptr<SceneContext> scene) const
{
	// Page 266 in the notes
	glm::vec3 oc = ray.orig - this->center_;
	float a = glm::dot(ray.dir, ray.dir);
	float b = 2.0f * glm::dot(ray.dir, ray.orig - this->center_);
	float c = glm::dot(oc, oc) - (this->radius_ * this->radius_);
	float D = (b * b) - (4.0f * a * c);

	// Miss
	if (D < 0.0f) {
		return Intersection::miss();
	}

	float q = (b < 0.0f ? -b + sqrt(D) : -b - sqrt(D)) / 2.0f;
	float m = q / a; 
	float n = c / q;
	float u = min(m, n);
	float v = max(m, n);
	float t = u < 0.0f ? v : u;

	glm::vec3 p = ray.orig + (ray.dir * t);
	glm::vec3 N = glm::normalize(p - this->center_);

	return Intersection(t, N);
}

glm::vec3 Sphere::sampleImpl() const
{
	// generate u, v, in the range (0, 1)
	float u = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	float v = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

	float theta = 2.0f * static_cast<float>(M_PI) * u;
	float phi = acos(2.0f * v - 1.0f);

	// find x, y, z coordinates assuming unit sphere in object space
	glm::vec3 point;
	point[0] = sin(phi) * cos(theta);
	point[1] = sin(phi) * sin(theta);
	point[2] = cos(phi);

	// TODO: transform point to world space
	return point;
}

////////////////////////////////////////////////////////////////////////////////
