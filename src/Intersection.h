/******************************************************************************
 *
 * This file defines a basic type for representing ray-object intersections
 *
 * @file Intersection.h
 * @author Michael Woods
 *
 ******************************************************************************/

#ifndef INTERSECTION_H
#define INTERSECTION_H
#include <iostream>
#define _USE_MATH_DEFINES
#include <cmath>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include "R3.h"

/******************************************************************************/

// Have to forward declare this guy:
class GraphNode;

class Intersection 
{
	public:
	    // The parameter `t` along the ray which was used. 
		// (A negative value indicates no intersection.)
	    float t;

	    // Density of the object at the point of intersection. Usually this is
	    // 1 for objects, [0,1] for volumes 
	    float density;

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

		// Flag to indicate if the normal's direction should be corrected 
		// (i.e flipped) if it is pointing away from the ray's origin
		bool correctNormal;

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

		// Miss constructor by default:
		Intersection();
		Intersection(float _t, V _normal);
		Intersection(float _t, float _density, V _normal);
		//Intersection(const Intersection& other) ;

		// Is the intersection a miss?
		bool isMiss() const;

		// Is the intersection a hit?
		bool isHit()  const;

		// Is the given intersection closer that this intersection?
		bool isCloser(const Intersection& other) const;
};

/******************************************************************************/

#endif
