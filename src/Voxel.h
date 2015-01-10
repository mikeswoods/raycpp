#ifndef VOXEL_H
#define VOXEL_H

#include <algorithm>
#include <iostream>
#include <list>
#include <memory>
#include "Color.h"
#include "AABB.h"
#include "Ray.h"
#include "Light.h"

#define MAX_LIGHTS 5

/*******************************************************************************
 * Voxel buffer element
 ******************************************************************************/

class Voxel
{
    public:
        float light[MAX_LIGHTS];
        float density;

        Voxel(float density = 0.0f);
};

/*******************************************************************************
 * Voxel element
 ******************************************************************************/

class VoxelBuffer
{
    private:
        int sub2ind(int i, int j, int k) const;
        void ind2sub(int w, int& i, int& j, int& k) const;
        bool valid(int i, int j, int k) const;

    protected:
        int _x, _y, _z;
        float voxelWidth, voxelHeight, voxelDepth;
        std::unique_ptr<Voxel[]> buffer;
        AABB aabb;

    public:
        VoxelBuffer(int x, int y, int z, const AABB& aabb);
        VoxelBuffer(const VoxelBuffer& other);
        ~VoxelBuffer();

        float getVoxelWidth()  const { return this->voxelWidth; }
        float getVoxelHeight() const { return this->voxelHeight; }
        float getVoxelDepth()  const { return this->voxelDepth; }

        bool center(const P& p, P& center) const;
        bool positionToIndex(const P& p, int& i, int& j, int& k) const;
        Voxel* positionToVoxel(const P& p) const;
        float getInterpolatedDensity(const P& p) const;

        // Indexing and assignment operations
        Voxel* operator()(int i, int j, int k) const;
        Voxel* operator[](int i) const;

        void set(int i, const Voxel& v);
        void set(int i, int j, int k, const Voxel& v);

        friend std::ostream& operator<<(std::ostream &s, const VoxelBuffer& b);
};

/*******************************************************************************
 * Type defining the result of a ray march through a volume
 ******************************************************************************/
class RayPath 
{
    public:
        Color color;
        float transmittance;

        RayPath() : 
            transmittance(0.0f)
        { 

        };
        RayPath(Color _color, float _transmittance) : 
            color(_color), 
            transmittance(_transmittance)
        { 

        };
};

RayPath rayMarch(const VoxelBuffer& buffer
                ,const P& start
                ,const V& dir
                ,float step
                ,bool interpolate
                ,std::shared_ptr<std::list<std::shared_ptr<Light>>> lights
                ,float (*densityFunction)(Voxel* voxel, const P& X) = nullptr);

/******************************************************************************/

#endif
