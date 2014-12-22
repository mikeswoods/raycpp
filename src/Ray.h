/******************************************************************************
 *
 * This file defines a basic, parametric ray type used for computing
 * object intersections
 *
 * @file Ray.h
 * @author Michael Woods
 ******************************************************************************/

#ifndef RAY_H
#define RAY_H

#include <iostream>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
 
#include "R3.h"

////////////////////////////////////////////////////////////////////////////////
// Parametric ray definition
////////////////////////////////////////////////////////////////////////////////

struct Ray
{
	enum RayType 
	{ 
		 PRIMARY
		,REFLECTION
		,REFRACTION
		,SHADOW
	};

	// Ray origin
	glm::vec3 orig;

	// Ray direction (may not be normalized)
	V dir;

	// Ray type
	RayType type;

	inline Ray() { }

	Ray(glm::vec3 o, glm::vec3 d) : 
		orig(o), 
		dir(d),
		type(PRIMARY)
	{ }

	Ray(glm::vec3 o, glm::vec3 d, float epsilon) : 
		orig(o), 
		dir(d),
		type(PRIMARY)
	{ 
		this->nudge(epsilon);
	}

	Ray(glm::vec3 o, glm::vec3 d, float epsilon, RayType _type) : 
		orig(o), 
		dir(d),
		type(_type)
	{ 
		this->nudge(epsilon);
	}

	Ray(const P& o, const V& d) :
		orig(o.xyz),
		dir(d),
		type(PRIMARY)
	{ }

	Ray(const P& o, const V& d, float epsilon) :
		orig(o.xyz),
		dir(d),
		type(PRIMARY)
	{ 
		this->nudge(epsilon);	
	}

	Ray(const P& o, const V& d, float epsilon, RayType _type) :
		orig(o.xyz),
		dir(d),
		type(_type)
	{ 
		this->nudge(epsilon);	
	}

	Ray(const Ray& other) : 
		orig(other.orig),
		dir(other.dir),
		type(other.type)
	{ }

	// Normalizes the ray and returns a new instance
	Ray normalize() const;

	// Project a magnitude along the ray yielding a position
	P project(float t) const;

	// "Nudges" the position of the ray along the direction by epsilon
	void nudge(float epsilon);

	// Return the type of the ray
	RayType getType() const { return this->type; }

	// Tests if the ray is a primaryn ray originating from the camera
	bool isPrimaryRay()    const { return this->type == PRIMARY; }

	// Tests if the ray is a reflection ray
	bool isReflectionRay() const { return this->type == REFLECTION; }

	// Tests if the ray is a reflection ray
	bool isRefractionRay() const { return this->type == REFRACTION; }

	// Tests if the ray is a shadow ray
	bool isShadowRay()     const { return this->type == SHADOW; }

	friend std::ostream& operator<<(std::ostream& s, const Ray& ray);
};

////////////////////////////////////////////////////////////////////////////////

#endif
