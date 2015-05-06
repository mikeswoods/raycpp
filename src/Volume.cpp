#include <stdexcept>
#include <cstdlib>
#include "Volume.h"
#include "SceneContext.h"

/******************************************************************************/

using namespace std;

/******************************************************************************/

Volume::Volume(Geometry const * _geometry, int x, int y, int z) : 
    Geometry(VOLUME),
    geometry(_geometry),
    buffer(VoxelBuffer(x, y, z, _geometry->getAABB()))
{
    for (auto i=0; i<x; i++) {
        for (auto j=0; j<y; j++) {
            for (auto k=0; k<z; k++) {
                this->buffer.set(i,j,k, Voxel(0.04f));
            }
        }
    }
}

Volume::~Volume()
{

}

Intersection Volume::intersectImpl(const Ray &ray, shared_ptr<SceneContext> scene) const
{
    Intersection isect = this->geometry->intersectImpl(ray, scene);

    if (isect.isHit()) {

        float density = rayMarch(this->buffer
                                ,ray.project(isect.t)
                                ,glm::normalize(ray.dir)
                                ,0.005
                                ,false
                                ,scene->getLights());

        return Intersection(isect.t, density, isect.normal);
    }

    return Intersection::miss();
}

glm::vec3 Volume::sampleImpl() const
{
    throw runtime_error("Not supported");
}

const BoundingVolume& Volume::getVolume() const
{
    return this->geometry->getVolume();
}

const P& Volume::getCentroid() const
{
    return this->geometry->getCentroid();
}

const AABB& Volume::getAABB() const
{
    return this->geometry->getAABB();
}

void Volume::buildGeometry()
{
    // Do nothing since buildGeometry() was already called for the geometry
    // object contained in this class instance
}

void Volume::repr(std::ostream& s) const
{
    s << "Volume<...>";
}

/******************************************************************************/
