/*******************************************************************************
 *
 * Volumetric object definition
 *
 * @file Volume.h
 * @author Michael Woods
 *
 ******************************************************************************/

#ifndef VOLUME_H
#define VOLUME_H

#include "Geometry.h"
#include "Voxel.h"

/******************************************************************************/

class Volume : public Geometry
{
    private:
        Geometry const * geometry;
        VoxelBuffer buffer;

    protected:
        virtual Intersection intersectImpl(const Ray &ray, std::shared_ptr<SceneContext> scene) const;
        virtual glm::vec3 sampleImpl() const;

    public:
        Volume(Geometry const * geometry, int x = 100, int y = 100, int z = 100);
        virtual ~Volume();

        Geometry const * getGeometry() { return this->geometry; }

        virtual const BoundingVolume& getVolume() const;
        virtual const P& getCentroid() const;
        virtual const AABB& getAABB() const;
        virtual void buildGeometry();
        virtual void repr(std::ostream& s) const;
};

/******************************************************************************/

#endif
