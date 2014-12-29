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

	this->tree = nullptr;
	//this->tree = new KDTree(this->triangles, new CycleAxisStrategy(), new MaxValuesPerLeaf(20));
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

	this->volume = TrivialVolume();
}

const BoundingVolume& Mesh::getVolume() const
{
	return this->volume;
}

void Mesh::buildGeometry()
{
    const int vc = this->getVertexCount();
    const int fc = this->faces.size();
    glm::vec3* normals = new glm::vec3[vc]; 

    // For each face, compute the face normal and add the normal to
    // each shared vertex normal
    for (auto i=0; i<fc; i++) {

        int j = this->faces[i].v[0] - 1;
        int k = this->faces[i].v[1] - 1;
        int l = this->faces[i].v[2] - 1;

        this->indices_.push_back(j);
        this->indices_.push_back(k);
        this->indices_.push_back(l);

        Tri T(i, this->vertices_[j], this->vertices_[k], this->vertices_[l]);

        this->triangles.push_back(T);
        this->vnIndex.push_back(VNIndex(j,k,l));

        V n = T.getNormal();
        normals[j] += n;
        normals[k] += n;
        normals[l] += n;
    }

    for (auto i=0; i<vc; i++) {
        this->normals_.push_back(glm::normalize(normals[i]));
    }

    assert(this->triangles.size() == this->vnIndex.size());

    delete [] normals;
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
		float d = tris[i].intersected(ray, W);
		if (d >= 0.0f && d < t) {
			t = d;
			//cout << "* (" << tris.size() << ") t=" << d << endl;
			k = i;
			found = true;
		}
	}

	if (!found) {
		t = -1.0f;
		return false;
	}

	//cout << "FINAL t=" << t << endl;

	return true;
}

Intersection Mesh::intersectImpl(const Ray &ray) const
{
	vector<Tri> collected;
	float t = -1.0f; // t distance
	int I   = -1;    // Index of closest triangle found in collected
	glm::vec3 W;     // barycentric weights

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

		// Get the index of the triangle as it appears in this->triangles
		//I = this->triangles[k].getMeshIndex(); 
		I = k;
	}

	assert(I >=0 && I <= this->triangles.size() && I <= this->vnIndex.size());

	// Normal at point-of-intersection
	//glm::vec3 N = this->triangles[I].getNormal();

	VNIndex VN  = this->vnIndex[I];
	
	cout << "T[" << I << "] : (" << get<0>(VN) << ", " << get<1>(VN) << ", " << get<2>(VN) << ")" << endl;


	glm::vec3 N = (this->triangles[get<0>(VN)].getNormal() * W[2]) + 
	              (this->triangles[get<1>(VN)].getNormal() * W[0]) + 
	              (this->triangles[get<2>(VN)].getNormal() * W[1]);

	return Intersection(t, N);
}

glm::vec3 Mesh::sampleImpl() const
{
	throw runtime_error("Mesh::sampleImpl() not implemented");
}

/*****************************************************************************/
