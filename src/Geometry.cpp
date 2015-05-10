/*******************************************************************************
 *
 * This file defines the class used to represent abstract geometric
 * objects in the rendering system, both for raytracer-based rendering as well
 * as OpenGL rendering
 *
 * @file Geometry.h
 * @author Michael Woods
 *
 ******************************************************************************/

#include <sstream>
#include "Geometry.h"
#include "Graph.h"
#include "Utils.h"
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

/******************************************************************************/

using namespace std;
using namespace glm;
using namespace Utils;

/******************************************************************************/

ostream& operator<<(ostream& s, const Geometry& geometry) 
{
    geometry.repr(s);
    return s;
}

/******************************************************************************/

Geometry::Geometry(Type _type) :
    type(_type)
{ 

}

Geometry::~Geometry()
{
    vertices_.clear();
    normals_.clear();
    indices_.clear();
}

/**
 * Generates vec3 color instances for every vertex  based on the supplied 
 * Color instance
 */
vector<vec3> Geometry::getColors(const Color& color) const
{
	return vector<vec3>(this->getVertexCount()
							,vec3(color.fR(), color.fG(), color.fB()));
}

Intersection Geometry::intersect(const mat4 &T
                                ,const Ray& rayWorld
                                ,shared_ptr<SceneContext> scene) const
{
	Ray rayNormal = Ray(rayWorld.orig, normalize(rayWorld.dir));
	mat4 invT     = inverse(T);

    // Transform the ray into OBJECT-LOCAL-space, for intersection calculation.
	// (Remember that position = vec4(vec3, 1) while direction = vec4(vec3, 0).)
	Ray rayLocal(transform(invT, vec4(rayNormal.orig, 1.0f))
		        ,transform(invT, vec4(rayNormal.dir, 0.0f)));

	// Test the bounding volume first:
	if (!this->getVolume().intersects(rayLocal)) {
		return Intersection::miss();
	}

    // Compute the intersection in LOCAL-space.
    Intersection isect = this->intersectImpl(rayLocal, scene);

    if (isect.isHit()) {

        // Transform the local-space intersection BACK into world-space.
        // (Note that, as long as you didn't re-normalize the ray direction
        // earlier, `t` doesn't need to change.)
        const vec3 normalLocal = isect.normal;

        // Inverse-transpose-transform the normal to get it back from 
		// local-space to world-space. (If you were transforming a position, 
		// you would just use the unmodified transform T.)
		//
        // http://www.arcsynthesis.org/gltut/Illumination/Tut09%20Normal%20Transformation.html
        isect.normal = normalize(transform(transpose(invT), vec4(normalLocal, 0.0f)));

		// Compute the hit position in world space:
		isect.hitWorld = rayNormal.project(isect.t);

		// Compute the hit position in local space:
		isect.hitLocal = transform(invT, vec4(isect.hitWorld, 1.0f));

		// Make sure the intersection surface normal always points toward (
        // not away from) the incident ray's origin. Note: this should only
        // be done for instances in which the correctNormal flag on the
        // on the Intersection object is true
		if (dot(isect.normal, rayWorld.dir) > 0.0f) {
            
            if (isect.correctNormal || !rayWorld.isPrimaryRay()) {
                isect.normal = -isect.normal;
            }
			
            isect.inside = true;
		}

        #ifdef DEBUG
		assert(abs(length(isect.normal) - 1.0f) <= 1.0e-6f);
        #endif
    }

    // The final output intersection data is in WORLD-space.
    return isect;
}

// Returns a sample point from the surface of the object in WORLD-space
vec3 Geometry::sample(const mat4& T) const
{
	return transform(T, vec4(this->sampleImpl(), 1.0f));
}

/******************************************************************************/
