/******************************************************************************
 *
 * This file defines a type for representing triangular mesh gemoetric object
 * as well as operations over such objects
 *
 * @file Mesh.h
 * @author Michael Woods
 *
 *****************************************************************************/

#include <cassert>
#include <stdexcept>
#include <algorithm>
#include <cmath>
#include <iterator>
#include <numeric>
#include <limits>
#include "Mesh.h"

using namespace std;

/*****************************************************************************/

Mesh::Mesh(const vector<glm::vec3>& vertices
	      ,const vector<glm::vec3>& normals
		  ,const std::vector<glm::vec2>& textureUV
		  ,const vector<Face>& _faces) :
    Geometry(MESH)
{ 
	copy(vertices.begin(), vertices.end(), back_inserter(this->vertices_));

	this->normals_  = normals;

	copy(textureUV.begin(), textureUV.end(), back_inserter(this->textureUV));

	this->faces = _faces;

	this->buildGeometry();
	this->buildVolume();
	this->computeCentroid();

	//this->tree = nullptr;
	this->tree = new KDTree(this->triangles, new CycleAxisStrategy(), new MaxValuesPerLeaf(20));
	//this->tree = new KDTree(this->triangles, new RandomAxisStrategy(), new MaxValuesPerLeaf(10));
	//this->tree = new KDTree(this->triangles, new SurfaceAreaStrategy(), new MaxValuesPerLeaf(10));
}

Mesh::Mesh(const Mesh& other) :
    Geometry(MESH),
	centroid(other.centroid),
	volume(other.volume),
	tree(other.tree),
	faces(other.faces),
	triangles(other.triangles)
{ }

Mesh::~Mesh() 
{ 
	if (this->tree != nullptr) {
		delete this->tree;
	}
	
	this->tree = nullptr;
}

ostream& operator<<(ostream& os, const Face& face)
{
	return os << "{ v=<" << face.v[0] << "," << face.v[1] << "," << face.v[2] << ">; "
	          << "t=<" << face.t[0] << "," << face.t[1] << "," << face.t[2] << ">; "
			  << "n=<" << face.n[0] << "," << face.n[1] << "," << face.n[2] << "> }";
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
	float cx = 0.0f;
	float cy = 0.0f;
	float cz = 0.0f;
	float N  = static_cast<float>(this->triangles.size());

	for (auto i=this->triangles.begin(); i != this->triangles.end(); i++) {
		P c = i->getCentroid();
		cx += x(c);
		cy += y(c);
		cz += z(c);
	}

	this->centroid = P(cx / N, cy / N, cz / N);
}

const P& Mesh::getCentroid() const
{
	return this->centroid;
}

void Mesh::buildVolume()
{
	// Start the radius at 0 and update it when a new maximum is encountered:
	float radius = 0.0f;

	// Iterate and sum over each vertex to compute the centroid of the mesh:
	glm::vec3 center = accumulate(this->vertices_.begin(), this->vertices_.end(), glm::vec3(0.0f, 0.0f, 0.0f));
	center /= static_cast<float>(this->getVertexCount());

	// Then, for every vertex, compute the distance to the center, updating radius to contain
	// the current maximum distance:
	for (auto i=this->vertices_.begin(); i != this->vertices_.end(); i++) {

		float dist = glm::distance(center, *i);
		if (dist > radius) {
			radius = dist;
		}
	}

	//this->volume = BoundingSphere(center, radius + Utils::EPSILON);
	this->volume = TrivialVolume();
}

const BoundingVolume& Mesh::getVolume() const
{
	return this->volume;
}

void Mesh::buildGeometry()
{
	unsigned int N = this->faces.size();
	this->normals_ = vector<glm::vec3>(this->getVertexCount(), glm::vec3(0.0f, 0.0f, 0.0f));

	// For each face, compute the face normal and add the normal to
	// each shared vertex normal
	for (unsigned int i=0; i<N; i++) {

		int j = this->faces[i].v[0] - 1;
		int k = this->faces[i].v[1] - 1;
		int l = this->faces[i].v[2] - 1;

		this->indices_.push_back(j);
		this->indices_.push_back(k);
		this->indices_.push_back(l);

		Tri T(this->vertices_[j], this->vertices_[k], this->vertices_[l]);

		this->triangles.push_back(T);

		V n = T.getNormal();

		this->normals_[j] += n;
		this->normals_[k] += n;
		this->normals_[l] += n;
	}

	for (unsigned int i=0; i<N; i++) {

		int j = this->faces[i].v[0] - 1;
		int k = this->faces[i].v[1] - 1;
		int l = this->faces[i].v[2] - 1;

		this->normals_[j] = glm::normalize(this->normals_[j]);
		this->normals_[k] = glm::normalize(this->normals_[k]);
		this->normals_[l] = glm::normalize(this->normals_[l]);

		this->triangles[i].n1 = this->normals_[j];
		this->triangles[i].n2 = this->normals_[k];
		this->triangles[i].n3 = this->normals_[l];
	}
}

/**
 * Given a vector of triangles, this function returns the closest one with
 * a t value >= 0
 */
static bool closestTriangle(const Ray& ray, const vector<Tri>& tris, Tri& closestTri, float& t)
{
	int j    = -1;
	size_t N = tris.size();
	t        = numeric_limits<float>::infinity();

	for (unsigned int i=0; i<N; i++) {
		float z = tris[i].intersected(ray);
		if (z >= 0.0f) {
			if (z < t) {
				j = i;
				t = z;
			}
		}
	}

	if (j < 0) {
		return false;
	}

	closestTri = tris[j];

	return true;
}

Intersection Mesh::intersectImpl(const Ray &ray) const
{
	float t = -1.0f; // t distance
	Tri tri; // Closest triangle

	if (this->tree != nullptr) { // Yes

		// First, get the set of triangles we'll be working with
		// from the spatial KD-tree index:
		vector<Tri> collected;

		if (!this->tree->intersects(ray, collected)) {
			return Intersection::miss(); // No intersections: bail out
		}

		if (!closestTriangle(ray, collected, tri, t)) {
			return Intersection::miss();
		}

	} else { // No

		if (!closestTriangle(ray, this->triangles, tri, t)) {
			return Intersection::miss();
		}
	}

	glm::vec3 N = tri.getNormal();

	return Intersection(t, N);
}

glm::vec3 Mesh::sampleImpl() const
{
	throw runtime_error("Mesh::sampleImpl() not implemented");
}

/*****************************************************************************/
