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

#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <vector>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Color.h"
#include "Intersection.h"
#include "Ray.h"
#include "BoundingVolume.h"

/******************************************************************************/

class Geometry
{
	private:
		TrivialVolume volume;

	public:
		// Type that specifies the geometric primitive type:
		enum Type 
		{
			 CUBE
			,SPHERE
			,CYLINDER
			,MESH
		};

	protected:
		Type type;

		// OpenGL vertex buffer data
		std::vector<glm::vec3> vertices_;

		// OpenGL normal buffer data
		std::vector<glm::vec3> normals_;

		// OpenGl index buffer data
		std::vector<unsigned int> indices_;

		// Compute an intersection with an OBJECT-LOCAL-space ray.
		virtual Intersection intersectImpl(const Ray &ray) const = 0;

		// Sample a point on the object's surface in OBJECT-LOCAL-space ray.
		virtual glm::vec3 sampleImpl() const = 0;

	public:
		// Enums for the types of geometry that your scene graph is required to contain.
		// Feel free to add more.
		explicit Geometry(Type);

		virtual ~Geometry();

		// Dump the representation of the geometry to the given stream
		virtual void repr(std::ostream& s) const = 0;

		// Function for building vertex data, i.e. vertices, normals, and indices.
		// Implemented in Sphere and Cylinder.
		virtual void buildGeometry() = 0;

		// Return the bounding volume used to contain this geometric object
		virtual const BoundingVolume& getVolume() const { return this->volume; }

		// Returns the centroid of the geometric object
		virtual const P& getCentroid() const = 0;

		// Generates vec3 color instances for every vertex 
		// based on the supplied Color instance
		std::vector<glm::vec3> getColors(const Color& color) const;

		// Getters
		const std::vector<glm::vec3>& getVertices() const   { return this->vertices_; };
		const std::vector<glm::vec3>& getNormals() const    { return this->normals_; };
		const std::vector<unsigned int>& getIndices() const { return this->indices_; };
		
		unsigned int getVertexCount() const { return this->vertices_.size(); };
		unsigned int getIndexCount() const  { return this->indices_.size(); };
		
		Type getGeometryType() const { return this->type; };

		// Compute an intersection with a WORLD-space ray.
		Intersection intersect(const glm::mat4& T, const Ray& rayWorld) const;

		// Returns a sample point from the surface of the object in WORLD-space
		glm::vec3 sample(const glm::mat4& T) const;

		friend std::ostream& operator<<(std::ostream& s, const Geometry& geometry);
};

/******************************************************************************/

#endif
