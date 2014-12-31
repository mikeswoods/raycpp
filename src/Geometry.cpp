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
#include "R3.h"

using namespace std;

/******************************************************************************/

ostream& operator<<(ostream& s, const Geometry& geometry) 
{
    geometry.repr(s);
    return s;
}

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

// Generates vec3 color instances for every vertex 
// based on the supplied Color instance
vector<glm::vec3> Geometry::getColors(const Color& color) const
{
	return vector<glm::vec3>(this->getVertexCount()
							,glm::vec3(color.fR(), color.fG(), color.fB()));
}

Intersection Geometry::intersect(const glm::mat4 &T, const Ray& rayWorld) const
{
	Ray rayNormal  = Ray(rayWorld.orig, glm::normalize(rayWorld.dir));
	glm::mat4 invT = glm::inverse(T);

    // Transform the ray into OBJECT-LOCAL-space, for intersection calculation.
	// (Remember that position = vec4(vec3, 1) while direction = vec4(vec3, 0).)
	Ray rayLocal(transform(invT, glm::vec4(rayNormal.orig, 1.0f))
		        ,transform(invT, glm::vec4(rayNormal.dir, 0.0f)));

	// Test the bounding volume first:
	if (!this->getVolume().intersects(rayLocal)) {
		return Intersection::miss();
	}

    // Compute the intersection in LOCAL-space.
    Intersection isect = this->intersectImpl(rayLocal);

    if (isect.isHit()) {

        // Transform the local-space intersection BACK into world-space.
        // (Note that, as long as you didn't re-normalize the ray direction
        // earlier, `t` doesn't need to change.)
        const glm::vec3 normalLocal = isect.normal;

        // Inverse-transpose-transform the normal to get it back from 
		// local-space to world-space. (If you were transforming a position, 
		// you would just use the unmodified transform T.)
		//
        // http://www.arcsynthesis.org/gltut/Illumination/Tut09%20Normal%20Transformation.html
        isect.normal = glm::normalize(transform(glm::transpose(invT), glm::vec4(normalLocal, 0.0f)));

		// Compute the hit position in world space:
		isect.hitWorld = rayNormal.project(isect.t);

		// Compute the hit position in local space:
		isect.hitLocal = transform(invT, glm::vec4(isect.hitWorld.xyz, 1.0f));

		//  Make sure the intersection surface normal always points toward (not away from) the 
		// incident ray's origin. See Piazza post @354

		if (glm::dot(isect.normal, rayWorld.dir) > 0.0f) {

            // Only for non-mesh objects:
            if (isect.node != nullptr  && 
                isect.node->getGeometry() != nullptr && 
                isect.node->getGeometry()->getGeometryType() != Geometry::MESH) 
            {
                isect.normal = -isect.normal; // Flip the normal:    
            }
			
			isect.inside = true;
		}

        #ifdef DEBUG
		assert(abs(glm::length(isect.normal) - 1.0f) <= 1.0e-6f);
        #endif
    }

    // The final output intersection data is in WORLD-space.
    return isect;
}

// Returns a sample point from the surface of the object in WORLD-space
glm::vec3 Geometry::sample(const glm::mat4& T) const
{
	return transform(T, glm::vec4(this->sampleImpl(), 1.0f));
}

/******************************************************************************/
