/*******************************************************************************
 *
 * This file defines a type for representing triangular mesh gemoetric object
 * as well as operations over such objects
 *
 * @file Mesh.h
 * @author Michael Woods
 *
 ******************************************************************************/

#include <cassert>
#include <stdexcept>
#include <algorithm>
#include <cmath>
#include <iterator>
#include <numeric>
#include <limits>
#include "Mesh.h"

using namespace std;

/******************************************************************************/

ostream& operator<<(ostream& s, const VNIndex& vnIndex)
{
	s << "{ " << get<0>(vnIndex)
	  << ", " << get<1>(vnIndex)
	  << ", " << get<2>(vnIndex)
	  << "}";
	return s;
}

ostream& operator<<(ostream& os, const Face& face)
{
	return os << "{ v=<" << face.v[0] << "," << face.v[1] << "," << face.v[2] << ">; "
	          << "t=<" << face.t[0] << "," << face.t[1] << "," << face.t[2] << ">; "
			  << "n=<" << face.n[0] << "," << face.n[1] << "," << face.n[2] << "> }";
}

/******************************************************************************/

Mesh::Mesh(const vector<glm::vec3>& vertices
	      ,const vector<glm::vec3>& normals
		  ,const std::vector<glm::vec3>& textureUV
		  ,const vector<Face>& _faces) :
    Geometry(MESH)
{ 
	copy(vertices.begin(), vertices.end(), back_inserter(this->vertices_));
	copy(normals.begin(), normals.end(), back_inserter(this->normals_));
	copy(textureUV.begin(), textureUV.end(), back_inserter(this->textureUV));
	copy(_faces.begin(), _faces.end(), back_inserter(this->faces));

	this->buildGeometry();
	this->buildVolume();
	this->computeCentroid();
	this->computeAABB();

	//this->tree = nullptr;
	this->tree = new KDTree(this->triangles, new CycleAxisStrategy(), new MaxValuesPerLeaf(20));
}

Mesh::Mesh(const Mesh& other) :
    Geometry(MESH),
	centroid(other.centroid),
	volume(other.volume),
	tree(other.tree),
	faces(other.faces),
	triangles(other.triangles)
{ 

}

Mesh::~Mesh() 
{ 
	if (this->tree) {
		delete this->tree;
	}
	
	this->tree = nullptr;
}

void Mesh::repr(std::ostream& s) const
{
	s << "Mesh {"      << endl;
	s << "  vertices<" << (this->vertices_.size()) << "> = {" << endl;
	for (auto i=this->vertices_.begin(); i != this->vertices_.end(); i++) {
		s << "    " << *i << endl;
	}
	s << "  }" << endl;
	s << "  normals<" << (this->normals_.size()) << "> = {" << endl;
	for (auto i=this->normals_.begin(); i != this->normals_.end(); i++) {
		s << "    " << *i << endl;
	}
	s << "  faces<" << (this->faces.size()) << "> = {" << endl;
	for (auto i=this->faces.begin(); i != this->faces.end(); i++) {
		s << "    " << *i << endl;
	}
	s << "  }" << endl;
	s << "}" << endl;
}

void Mesh::computeCentroid()
{
	this->centroid = mean(this->vertices_);
}

void Mesh::computeAABB()
{
	this->aabb = AABB();

	for (auto i=this->triangles.begin(); i != this->triangles.end(); i++) {
		this->aabb += i->getAABB();
	}
}

const P& Mesh::getCentroid() const
{
	return this->centroid;
}

const AABB& Mesh::getAABB() const
{
    return this->aabb;
}

void Mesh::buildVolume()
{
	// Start the radius at 0 and update it when a new maximum is encountered:
	float radius = 0.0f;

	// Iterate and sum over each vertex to compute the centroid of the mesh:
	glm::vec3 center = accumulate(this->vertices_.begin(), this->vertices_.end(), glm::vec3());
	center /= static_cast<float>(this->getVertexCount());

	// Then, for every vertex, compute the distance to the center, updating radius to contain
	// the current maximum distance:
	for (auto i=this->vertices_.begin(); i != this->vertices_.end(); i++) {

		float dist = glm::distance(center, *i);
		if (dist > radius) {
			radius = dist;
		}
	}

	this->volume = TrivialVolume();
}

const BoundingVolume& Mesh::getVolume() const
{
	return this->volume;
}

void Mesh::buildGeometry()
{
    const int VC = this->getVertexCount();
    const int FC = this->faces.size();

    // For each face, compute the face normal and add the normal to
    // each shared vertex normal
    for (auto i=0; i<VC; i++) {
    	this->normals_.push_back(glm::vec3());
    }

    for (auto i=0; i<FC; i++) {

        int u = this->faces[i].v[0] - 1;
        int v = this->faces[i].v[1] - 1;
        int w = this->faces[i].v[2] - 1;

        this->indices_.push_back(u);
        this->indices_.push_back(v);
        this->indices_.push_back(w);

        Tri TRI(i, this->vertices_[u], this->vertices_[v], this->vertices_[w]);

        this->triangles.push_back(TRI);
        this->vnIndex.push_back(VNIndex(u, v, w));

        glm::vec3 N = TRI.getNormal();

        this->normals_[u] += N;
        this->normals_[v] += N;
        this->normals_[w] += N;
    }
 
    for (auto i=0; i<VC; i++) {
    	this->normals_[i] = glm::normalize(this->normals_[i]);
    }
}

/**
 * Given a vector of triangles, this function returns the closest one with
 * a t value >= 0
 */
static bool closestTriangle(const Ray& ray, const vector<Tri>& tris
	                       ,int& k, float& t, glm::vec3& W)
{
	t = numeric_limits<float>::infinity();
	k = -1;
	bool found = false;
	
	for (auto i=0; i<static_cast<int>(tris.size()); i++) {
		float t_i = tris[i].intersected(ray, W);
		if (t_i >= 0.0f && t_i < t) {
			t = t_i;
			k = i;
			found = true;
		}
	}

	if (!found) {
		t = -1.0f;
		return false;
	}

	// Finally, recompute barycentric weights for the correct triangle at k:
	t = tris[k].intersected(ray, W);

	return true;
}

Intersection Mesh::intersectImpl(const Ray &ray, shared_ptr<SceneContext> scene) const
{
	vector<Tri> collected;
	float t  = -1.0f; // t distance
	size_t I = 0;     // Index of closest triangle found in collected
	glm::vec3 W;      // barycentric weights

	if (this->tree != nullptr) { // Yes

		// First, get the set of triangles we'll be working with
		// from the spatial KD-tree index:

		if (!this->tree->intersects(ray, collected)) {
			return Intersection::miss(); // No intersections: bail out
		}

		int k = -1;
		if (!closestTriangle(ray, collected, k, t, W)) {
			return Intersection::miss();
		}

		// Get the index of the triangle as it appears in this->triangles
		I = collected[k].getMeshIndex(); 

	} else { // No

		int k = -1;
		if (!closestTriangle(ray, this->triangles, k, t, W)) {
			return Intersection::miss();
		}

		I = k;
	}

	assert(I < this->triangles.size()
		   && this->vertices_.size() == this->normals_.size() 
		   && this->triangles.size() == this->vnIndex.size());

	VNIndex VN  = this->vnIndex[I];

	// Normal at point-of-intersection
	//glm::vec3 N = this->triangles[I].getNormal();
	glm::vec3 N  = (W[0] * this->normals_[get<0>(VN)]) 
	             + (W[1] * this->normals_[get<1>(VN)]) 
	             + (W[2] * this->normals_[get<2>(VN)]);

	Intersection isect(t, glm::normalize(N));

	isect.correctNormal = false;

	return isect;
}

glm::vec3 Mesh::sampleImpl() const
{
	throw runtime_error("Mesh::sampleImpl() not implemented");
}

/******************************************************************************/
