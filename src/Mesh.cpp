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

/******************************************************************************/

using namespace std;

/******************************************************************************/

Mesh::Mesh(shared_ptr<aiMesh> _meshData) :
    Geometry(MESH),
    meshData(_meshData),
    tree(unique_ptr<KDTree>(nullptr))
{ 
    this->buildGeometry();
    this->buildVolume();
    this->computeCentroid();
    this->computeAABB();

    this->tree = unique_ptr<KDTree>(new KDTree(this->triangles, new CycleAxisStrategy(), new MaxValuesPerLeaf(20)));
}

Mesh::~Mesh() 
{ 

}

void Mesh::repr(std::ostream& s) const
{
    /*
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
    */
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
    // Vertices

    assert(this->meshData->mVertices && this->meshData->mNormals);

    for (unsigned int i = 0; i < this->meshData->mNumVertices; i++) {

        auto vertex = this->meshData->mVertices[i];

        this->vertices_.push_back(glm::vec3(vertex.x, vertex.y, vertex.z));
    }

    // Normals

    for (unsigned int i = 0; i < this->meshData->mNumVertices; i++) {

        auto normal = this->meshData->mNormals[i];

        this->normals_.push_back(glm::vec3(normal.x, normal.y, normal.z));
    }

    // Indices

    for (unsigned int i = 0; i < this->meshData->mNumFaces; i++) {

        auto face = this->meshData->mFaces[i];
        assert(face.mNumIndices == 3);     

        unsigned int u = face.mIndices[0];
        unsigned int v = face.mIndices[1];
        unsigned int w = face.mIndices[2];

        this->indices_.push_back(u);
        this->indices_.push_back(v);
        this->indices_.push_back(w);

        // and generate a triangle from the face: 

        Tri t(i, glm::uvec3(u, v, w), this->vertices_[u], this->vertices_[v], this->vertices_[w]);

        this->triangles.push_back(t);
    }
}

/**
 * Given a vector of triangles, this function returns the closest one with
 * a t value >= 0
 */
static bool closestTriangle(const Ray& ray
                           , const vector<Tri>& tris
                           ,int& k
                           ,float& t
                           ,glm::vec3& W)
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

    //assert(this->vertices_.size() == this->normals_.size());

    glm::uvec3 indices = this->triangles[I].getVertexIndices();

    // Interpolate the normal at the point-of-intersection:

    glm::vec3 N  = (W[0] * this->normals_[indices[0]]) 
                 + (W[1] * this->normals_[indices[1]]) 
                 + (W[2] * this->normals_[indices[2]]);

    Intersection isect(t, glm::normalize(N));

    isect.correctNormal = false;

    return isect;
}

glm::vec3 Mesh::sampleImpl() const
{
    throw runtime_error("Mesh::sampleImpl() not implemented");
}

/******************************************************************************/
