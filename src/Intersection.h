/******************************************************************************
 *
 * This file defines a basic type for representing ray-object intersections
 *
 * @file Intersection.h
 * @author Michael Woods
 ******************************************************************************/

#ifndef INTERSECTION_H
#define INTERSECTION_H
#include <iostream>
#define _USE_MATH_DEFINES
#include <cmath>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include "R3.h"

////////////////////////////////////////////////////////////////////////////////

// Have to forward declare this guy:
class GraphNode;

struct Intersection 
{
    // The parameter `t` along the ray which was used. 
	// (A negative value indicates no intersection.)
    float t;

	// The node of the scene graph that was intersected
	GraphNode* node;

    // The surface normal at the point of intersection. (Ignored if t < 0.)
    V normal;

	// If the intersection occurred inside an object, this flag will be set
	bool inside;

	// Hit position in world space. Only valid if t >= 0
	P hitWorld;

	// Hit position in local space. Only valid if t >= 0
	P hitLocal;

	// Normal vector in local/object space. Only valid if t >= 0
	V normalLocal;

	// Flag to indicate if a UV mappign vector is present
	bool hasUV;

	// UV mapping vector
	V uv;

	// Compare two intersections, returning the closet of the two
	static Intersection getClosest(const Intersection& current, const Intersection& last)
	{
		return current.isCloser(last) ? current : last;
	}

	// Miss constructor function
	static Intersection miss()
	{
		return Intersection();
	}

	Intersection() : 
		t(-1.0f),
		node(NULL),
		inside(false)
	{ 
		
	}

	Intersection(float _t, V _normal) : 
		t(_t), 
		node(NULL),
		normal(_normal),
		inside(false),
		uv(glm::vec3()),
		hasUV(false)
	{ 
		
	}

	Intersection(float _t, V _normal, V _uv) : 
		t(_t), 
		node(NULL),
		normal(_normal),
		inside(false),
		uv(_uv),
		hasUV(true)
	{ 
		
	}

	Intersection(const Intersection& other) : 
		t(other.t), 
		normal(other.normal),
		node(other.node),
		inside(other.inside),
		hitWorld(other.hitWorld),
		hitLocal(other.hitLocal),
		normalLocal(other.normalLocal),
		uv(other.uv),
		hasUV(other.hasUV)
	{ }

	// Is the intersection a miss?
	bool isMiss() const { return this->t < 0.0f; }

	// Is the intersection a hit?
	bool isHit()  const { return !this->isMiss(); }

	// Is the given intersection closer that this intersection?
	bool isCloser(const Intersection& other) const
	{
		if (this->isMiss()) {
			return false;
		} else if (other.isMiss()) {
			return true;
		} else {
			return this->t < other.t;
		}
	}
};

////////////////////////////////////////////////////////////////////////////////

#endif
