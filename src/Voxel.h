#ifndef VOXEL_H
#define VOXEL_H

#include <algorithm>
#include <iostream>
#include <list>
#include "Color.h"
#include "Geometry.h"
#include "Ray.h"
#include "Light.h"

#define MAX_LIGHTS 5

/*******************************************************************************
 * Voxel buffer element
 ******************************************************************************/

typedef struct Voxel
{
    float light[MAX_LIGHTS];
    float density;

    Voxel(float density = 0.0f)
    { 
        this->density = density;
        std::fill(this->light, this->light + MAX_LIGHTS, -1.0f);
    };

} Voxel;

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
        int xDim, yDim, zDim;
        float vWidth, vHeight, vDepth;
        Voxel* buffer;
        AABB aabb;

    public:
        VoxelBuffer(int x, int y, int z, Geometry const * geometry);
        ~VoxelBuffer();

        int getX() const { return this->xDim; }
        int getY() const { return this->yDim; }
        int getZ() const { return this->zDim; }

        float getVoxelWidth()  const { return this->vWidth; }
        float getVoxelHeight() const { return this->vHeight; }
        float getVoxelDepth()  const { return this->vDepth; }

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

////////////////////////////////////////////////////////////////////////////////

typedef struct RayPath {

    Color color;
    float transmittance;

    RayPath() : 
        transmittance(0.0f)
    { };
    RayPath(Color _color, float _transmittance) : 
        color(_color), 
        transmittance(_transmittance)
    { };

} RayPath;

RayPath rayMarch(const VoxelBuffer& vb
                ,const P& start
                ,const P& end
                ,float step
                ,bool interpolate
                ,const std::list<Light*>& lights
                ,float (*densityFunction)(Voxel* voxel, const P& X, void* densityData)
                ,void* densityData);

/******************************************************************************/

#endif
