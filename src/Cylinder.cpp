/******************************************************************************
 *
 * Cylinder geometry definition
 *
 * @file Cylinder.h
 * @author Michael Woods
 *
 ******************************************************************************/

#define _USE_MATH_DEFINES
#include <stdexcept>
#include <cmath>
#include <algorithm>
#include <limits>
#include "Ray.h"
#include "Cylinder.h"

using namespace std;

/*****************************************************************************/

// Creates a unit cylinder centered at (0, 0, 0)
Cylinder::Cylinder() :
    Geometry(CYLINDER),
    center_(P(0.f, 0.f, 0.f)),
    radius_(0.5f),
    height_(1.0f)
{
    this->buildGeometry();
	this->buildVolume();
    this->computeAABB();
}

Cylinder::Cylinder(const Cylinder& other) :
    Geometry(CYLINDER),
	center_(other.center_),
	radius_(other.radius_),
	height_(other.height_),
	volume(other.volume)
{ 
	
}

Cylinder::~Cylinder() 
{

}

void Cylinder::repr(std::ostream& s) const
{
	s << "Cylinder<center=" << this->center_ 
	  << ", radius="        << this->radius_
	  << ", height="        << this->height_
	  << ">";
}

const P& Cylinder::getCentroid() const
{
	return this->center_;
}

const BoundingVolume& Cylinder::getVolume() const
{
	return this->volume;
}

const AABB& Cylinder::getAABB() const
{
    return this->aabb;
}

void Cylinder::buildVolume()
{
	this->volume = BoundingSphere(this->center_, (this->center_.xyz.y + this->height_) + Utils::EPSILON);
}

void Cylinder::computeAABB()
{
    this->aabb = AABB(P(x(this->center_) - this->radius_, y(this->center_) - this->height_, z(this->center_) - this->radius_)
                     ,P(x(this->center_) + this->radius_, y(this->center_) + this->height_, z(this->center_) + this->radius_));
}

void Cylinder::buildGeometry()
{
    vertices_.clear();
    normals_.clear();
    indices_.clear();

    unsigned short subdiv = 20;
    float dtheta = 2 * static_cast<float>(M_PI) / subdiv;

    glm::vec4 point_top(0.0f, 0.5f * height_, radius_, 1.0f),
        point_bottom (0.0f, -0.5f * height_, radius_, 1.0f);
    vector<glm::vec3> cap_top, cap_bottom;

    // top and bottom cap vertices
    for (int i = 0; i < subdiv + 1; ++i) {
        glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), i * dtheta, glm::vec3(0.f, 1.f, 0.f));
        glm::mat4 translate = glm::translate(glm::mat4(1.0f), center_.xyz);

        cap_top.push_back(glm::vec3(translate * rotate * point_top));
        cap_bottom.push_back(glm::vec3(translate * rotate * point_bottom));
    }

    //Create top cap.
    for ( int i = 0; i < subdiv - 2; i++) {
        vertices_.push_back(cap_top[0]);
        vertices_.push_back(cap_top[i + 1]);
        vertices_.push_back(cap_top[i + 2]);
    }
    //Create bottom cap.
    for (int i = 0; i < subdiv - 2; i++) {
        vertices_.push_back(cap_bottom[0]);
        vertices_.push_back(cap_bottom[i + 1]);
        vertices_.push_back(cap_bottom[i + 2]);
    }
    //Create barrel
    for (int i = 0; i < subdiv; i++) {
        //Right-side up triangle
        vertices_.push_back(cap_top[i]);
        vertices_.push_back(cap_bottom[i + 1]);
        vertices_.push_back(cap_bottom[i]);
        //Upside-down triangle
        vertices_.push_back(cap_top[i]);
        vertices_.push_back(cap_top[i + 1]);
        vertices_.push_back(cap_bottom[i + 1]);
    }

    // create normals
    glm::vec3 top_centerpoint(0.0f , 0.5f * height_ , 0.0f),
        bottom_centerpoint(0.0f, -0.5f * height_, 0.0f);
    glm::vec3 normal(0, 1, 0);

    // Create top cap.
    for (int i = 0; i < subdiv - 2; i++) {
        normals_.push_back(normal);
        normals_.push_back(normal);
        normals_.push_back(normal);
    }
    // Create bottom cap.
    for (int i = 0; i < subdiv - 2; i++) {
        normals_.push_back(-normal);
        normals_.push_back(-normal);
        normals_.push_back(-normal);
    }

    // Create barrel
    for (int i = 0; i < subdiv; i++) {
        //Right-side up triangle
        normals_.push_back(glm::normalize(cap_top[i] - top_centerpoint));
        normals_.push_back(glm::normalize(cap_bottom[i + 1] - bottom_centerpoint));
        normals_.push_back(glm::normalize(cap_bottom[i] - bottom_centerpoint));
        //Upside-down triangle
        normals_.push_back(glm::normalize(cap_top[i] - top_centerpoint));
        normals_.push_back(glm::normalize(cap_top[i + 1] - top_centerpoint));
        normals_.push_back(glm::normalize(cap_bottom[i + 1] - bottom_centerpoint));
    }

    for (unsigned int i = 0; i < vertices_.size(); ++i) {
        indices_.push_back(i);
    }
}

Intersection Cylinder::intersectImpl(const Ray &ray, shared_ptr<SceneContext> scene) const
{
	float inf   = numeric_limits<float>::infinity();
    float r2    = this->radius_ * this->radius_;
	glm::vec3 E = ray.orig - this->center_;

	float xo = x(ray.orig);
    float yo = y(ray.orig);
    float zo = z(ray.orig);
    float xd = x(ray.dir);
    float yd = y(ray.dir);
    float zd = z(ray.dir);
    float A  = (xd * xd) + (zd * zd);
    float B  = (2.0f * xo * xd) + (2.0f * zo * zd);
    float C  = (xo * xo) + (zo * zo) - r2;
    float D  = (B * B) - (4.0f * A * C);

	// No roots = no hit
    if (D < 0.0) {
		return Intersection::miss();
	}

	glm::vec3 normal = glm::vec3(1.0f, 0.0, 1.0f);

	float halfH = this->height_ * 0.5f;
	float t     = -1.0f;

    float D2 = sqrt(D);
	float t0 = A == 0.0f ? ((-B + D2)) : (-B + D2) / (2.0f * A);
	float t1 = A == 0.0f ? ((-B - D2)) : (-B - D2) / (2.0f * A);

	if (t0 > t1) {
		std::swap(t0, t1);
	}

	float tSide   = -1.0f;
	float tBottom = -1.0f;
	float tTop    = -1.0f;

	// Test for intersections: sides
	float y0  = yo + (t0 * yd);
	float y1  = yo + (t1 * yd);
	float yLo = this->center_.xyz.y - halfH;
	float yHi = this->center_.xyz.y + halfH;
	float side1 = inf, side2 = inf;

	if (y0 >= yLo && y0 <= yHi) {
		side1 = t0;
	}
	if (y1 >= yLo && y1 <= yHi) {
		side2 = t1;
	}

	Utils::leastGreaterThanZero(side1, side2, tSide);
	if (tSide == inf) {
		tSide = -1.0f;
	}

	// Test for intersections: top
	if (   (y0 <= yHi && y1 >= yHi) 
		|| (y1 <= yHi && y0 >= yHi) 
		|| (y0 == y1)) 
	{
		tTop = (-y(E) + halfH) / yd;
	}

	// Test for intersections:  bottom:
	if (   (y0 <= yLo && y1 >= yLo) 
		|| (y1 <= yLo && y0 >= yLo) 
		|| (y0 == y1))
	{
		tBottom = (-y(E) - halfH) / yd;
	}

	// Bad results when yd is zero:
	if (abs(tTop) == inf) {
		tTop = -1.0f;
	}
	if (abs(tBottom) == inf) {
		tBottom = -1.0f;
	}

	if (!Utils::leastGreaterThanZero(tSide, tTop, t)) {
		t = tBottom <= 0.0f ? -1.0f : tBottom; // Its either tBottom or failure:
	} else {
		Utils::leastGreaterThanZero(t, tBottom, t);
	}

	if (t < 0.0f) {
		return Intersection::miss();
	}

	if (t == tTop) {
		normal = glm::vec3(0.0f, 1.0f, 0.0f);
	} else if (t == tBottom) {
		normal = glm::vec3(0.0f, -1.0f, 0.0f);
	} else {
		P p = ray.project(t);
		normal = glm::vec3(x(p) / this->radius_, 0.0f, z(p) / this->radius_);
	}

	return Intersection(t, normal);
}

glm::vec3 Cylinder::sampleImpl() const
{
	throw runtime_error("Cylinder::sampleImpl() not implemented");
}

/*****************************************************************************/
