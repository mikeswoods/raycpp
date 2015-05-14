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
#include <easylogging++.h>
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
    s << "Mesh<vertices=" << this->vertices_.size() << ">";
}

void Mesh::computeCentroid()
{
    this->centroid = glm::vec3();

    for (auto i = this->vertices_.begin(); i != this->vertices_.end(); i++) {
        this->centroid += *i;
    }

    this->centroid /= static_cast<float>(this->vertices_.size());
}

void Mesh::computeAABB()
{
    this->aabb = AABB();

    for (auto i = this->triangles.begin(); i != this->triangles.end(); i++) {
        this->aabb += i->getAABB();
    }
}

const glm::vec3& Mesh::getCentroid() const
{
    return this->centroid;
}

const AABB& Mesh::getAABB() const
{
    return this->aabb;
}

void Mesh::buildVolume()
{
    this->volume = TrivialVolume();
}

const BoundingVolume& Mesh::getVolume() const
{
    return this->volume;
}

void Mesh::buildGeometry()
{
    // Vertices

    assert(this->meshData->HasFaces() && this->meshData->HasNormals());

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
        if (face.mNumIndices != 3) {
            LOG(ERROR) << "face.mNumIndices = " << face.mNumIndices << endl;
            assert(face.mNumIndices == 3);
        }

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

MultiMesh::MultiMesh(std::vector<std::shared_ptr<Mesh>> _meshes) :
    Geometry(MESH),
    meshes(_meshes)
{ 
    this->buildGeometry();
    this->buildVolume();
    this->computeCentroid();
    this->computeAABB();
}

MultiMesh::~MultiMesh()
{

}

void MultiMesh::repr(std::ostream& s) const
{
    s << "MultiMesh<meshCount=" << this->meshes.size() << ">";
}

void MultiMesh::computeCentroid()
{
    this->centroid = glm::vec3();
    size_t n = this->meshes.size();

    for (auto i = this->meshes.begin(); i != this->meshes.end(); i++) {
        this->centroid += (*i)->getCentroid();
    }

    this->centroid /= static_cast<float>(n);
}

void MultiMesh::computeAABB()
{
    this->aabb = AABB();

    for (auto i = this->meshes.begin(); i != this->meshes.end(); i++) {
        this->aabb += (*i)->getAABB();
    }
}

const glm::vec3& MultiMesh::getCentroid() const
{
    return this->centroid;
}

const AABB& MultiMesh::getAABB() const
{
    return this->aabb;
}

void MultiMesh::buildVolume()
{
    this->volume = TrivialVolume();
}

const BoundingVolume& MultiMesh::getVolume() const
{
    return this->volume;
}

void MultiMesh::buildGeometry()
{

}

Intersection MultiMesh::intersectImpl(const Ray &ray, std::shared_ptr<SceneContext> scene) const
{
    for (auto i = this->meshes.begin(); i != this->meshes.end(); i++) {
        auto isect = (*i)->intersectImpl(ray, scene);
        if (isect.isHit()) {
            return isect;
        }
    }

    return Intersection::miss();
}

glm::vec3 MultiMesh::sampleImpl() const
{
    throw runtime_error("MultiMesh::sampleImpl() not implemented");
}

/******************************************************************************/