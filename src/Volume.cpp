#include <stdexcept>
#include "Volume.h"
#include "SceneContext.h"

using namespace std;

/******************************************************************************/

Volume::Volume(Geometry const * _geometry, int x, int y, int z) : 
    Geometry(VOLUME),
    geometry(_geometry),
    buffer(VoxelBuffer(x, y, z, _geometry->getAABB()))
{

}

Volume::~Volume()
{

}

Intersection Volume::intersectImpl(const Ray &ray, shared_ptr<SceneContext> scene) const
{
    Intersection isect = this->geometry->intersectImpl(ray, scene);

    if (isect.isHit()) {
        rayMarch(this->buffer
                ,ray.project(isect.t)
                ,glm::normalize(ray.dir)
                ,0.005
                ,false
                ,scene->getLights());
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
