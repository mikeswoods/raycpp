#define _USE_MATH_DEFINES
#include <cassert>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <limits>
#include <list>
#include <tuple>
#include "glm/glm.hpp"
#include "Voxel.h"
#include "Utils.h"

using namespace std;

/*******************************************************************************
 * Voxel
 ******************************************************************************/

Voxel::Voxel(float _density) :
	density(_density)
{ 
    fill(this->light, this->light + MAX_LIGHTS, -1.0f);
};

ostream& operator<<(ostream &s, const Voxel &v)
{
	return s << "{ light = "   << v.light 
			 << ", density = " << v.density << " }";
}

/*******************************************************************************
 * Voxel buffer
 ******************************************************************************/

VoxelBuffer::VoxelBuffer(int x, int y, int z, Geometry const * geometry) :
	_x(x),
	_y(y),
	_z(z),
	buffer(unique_ptr<Voxel[]>(new Voxel[x * y * z])),
	aabb(geometry->getAABB())
{
	this->voxelWidth  = static_cast<float>(this->aabb.width() / x);
	this->voxelHeight = static_cast<float>(this->aabb.height() / y);
	this->voxelDepth  = static_cast<float>(this->aabb.depth() / z);
}

VoxelBuffer::~VoxelBuffer()
{

}

Voxel* VoxelBuffer::operator() (int i, int j, int k) const 
{
	return &this->buffer[sub2ind(i,j,k)];
}

Voxel* VoxelBuffer::operator[](int i) const
{
	return &this->buffer[i];
}

void VoxelBuffer::set(int i, const Voxel& v)
{
	this->buffer[i] = v;
}

void VoxelBuffer::set(int i, int j, int k, const Voxel& v)
{
	this->buffer[sub2ind(i,j,k)] = v;
}

/*
bool VoxelBuffer::intersects(const Ray& ray, const RenderContext& ctx, Hit& hit)
{
	P entered, exited;

	if (!this->bounds.isHit(ray, entered, exited)) {
		return false;
	}

	// Basic color accumulation:
	RayMarch rm       = rayMarch(ctx, *this, entered, exited);
	hit.color         = rm.color;
	hit.transmittance = rm.transmittance;

	return true;
}
*/

/** 
 * Sets center to the center point of the voxel the point p falls within. 
 * If this method returns false, point o does not fall within a voxel
 */
bool VoxelBuffer::center(const P& p, P& center) const
{   
	int i,j,k;

	if (!this->positionToIndex(p, i, j, k)) {
		return false;
	}

	float dx  = (this->aabb.width()) / static_cast<float>(this->_x);
	float dy  = (this->aabb.height()) / static_cast<float>(this->_y);
	float dz  = (this->aabb.depth()) / static_cast<float>(this->_z);
	float dx2 = 0.5f * dx; 
	float dy2 = 0.5f * dy;
	float dz2 = 0.5f * dz; 

	center = P(x(get<0>(this->aabb.vertices())) + dx2 + (dx * static_cast<float>(i))
			  ,y(get<0>(this->aabb.vertices())) + dy2 + (dy * static_cast<float>(j))
			  ,z(get<0>(this->aabb.vertices())) + dz2 + (dz * static_cast<float>(k)));

	return true;
}

/**
 * Returns the (i,j,k) coordinates of the voxel point p falls within. If this
 * method returns false, point p is not within a voxel
 */
bool VoxelBuffer::positionToIndex(const P& p, int& i, int& j, int& k) const
{
	float dx, dy, dz;
	int xCell, yCell, zCell;

	dx = Utils::unitRange(x(p)
		                 ,x(get<0>(this->aabb.vertices()))
		                 ,x(get<1>(this->aabb.vertices())));
	dy = Utils::unitRange(y(p)
		                 ,y(get<0>(this->aabb.vertices()))
		                 ,y(get<1>(this->aabb.vertices())));
	dz = Utils::unitRange(z(p)
		                 ,z(get<0>(this->aabb.vertices()))
		                 ,z(get<1>(this->aabb.vertices())));

	float threshold = Utils::EPSILON;

	// Needed for rounding errors: if the number is smaller than the threshold,
	// it just becomes 0.0f
	if (fabs(dx) < threshold) {
		dx = 0.0f;
	}
	if (fabs(dy) < threshold) {
		dy = 0.0f;
	}
	if (fabs(dz) < threshold) {
		dz = 0.0f;
	}

	float xLoc = dx * ((static_cast<float>(this->_x)) - this->getVoxelWidth());
	float yLoc = dy * ((static_cast<float>(this->_y)) - this->getVoxelHeight());
	float zLoc = dz * ((static_cast<float>(this->_z)) - this->getVoxelDepth());
	xCell = static_cast<int>(floor(xLoc));
	yCell = static_cast<int>(floor(yLoc));
	zCell = static_cast<int>(floor(zLoc));

	if (!this->valid(xCell, yCell, zCell)) {
		return false;
	}

	i = xCell;
	j = yCell;
	k = zCell;

	return true;
}

 Voxel* VoxelBuffer::positionToVoxel(const P& p) const
 {
	int i,j,k;
	return positionToIndex(p,i,j,k) ? (*this)(i,j,k) : nullptr;
 }

/**
 * Gets the trilinearly interpolated density for the given position
 */
float VoxelBuffer::getInterpolatedDensity(const P& p) const
{
	float dx, dy, dz;
	int x1, x2, y1, y2, z1, z2;

	dx = Utils::unitRange(x(p)
		                 ,x(get<0>(this->aabb.vertices()))
		                 ,x(get<1>(this->aabb.vertices())));
	dy = Utils::unitRange(y(p)
		                 ,y(get<0>(this->aabb.vertices()))
		                 ,y(get<1>(this->aabb.vertices())));
	dz = Utils::unitRange(z(p)
		                 ,z(get<0>(this->aabb.vertices()))
		                 ,z(get<1>(this->aabb.vertices())));

	float xLoc    = dx * ((float)this->_x - 1);
	float xWeight = xLoc - floor(xLoc); 
	x1 = static_cast<int>(floor(xLoc));
	x2 = static_cast<int>(ceil(xLoc));

	float yLoc = dy * ((float)this->_y - 1);
	float yWeight = yLoc - floor(yLoc); 
	y1 = static_cast<int>(floor(yLoc));
	y2 = static_cast<int>(ceil(yLoc));    

	float zLoc = dz * ((float)this->_z - 1);
	float zWeight = zLoc - floor(zLoc); 
	z1 = static_cast<int>(floor(zLoc));
	z2 = static_cast<int>(ceil(zLoc));

	// All index combinations:
	// x1,y1,z1
	// x1,y1,z2
	// x1,y2,z1
	// x1,y2,z2
	// x2,y1,z1
	// x2,y1,z2
	// x2,y2,z1
	// x2,y2,z2

	float x1y1z1D = 0.0f;
	float x1y1z2D = 0.0f;
	float x1y2z1D = 0.0f;
	float x1y2z2D = 0.0f;
	float x2y1z1D = 0.0f;
	float x2y1z2D = 0.0f;
	float x2y2z1D = 0.0f;
	float x2y2z2D = 0.0f;

	if (this->valid(x1,y1,z1)) {
		x1y1z1D = (*this)(x1,y1,z1)->density;
	}
	if (this->valid(x1,y1,z2)) {
		x1y1z2D = (*this)(x1,y1,z2)->density;
	}
	if (this->valid(x1,y2,z1)) {
		x1y2z1D = (*this)(x1,y2,z1)->density;
	}
	if (this->valid(x1,y2,z2)) {
		x1y2z2D = (*this)(x1,y2,z2)->density;
	}
	if (this->valid(x2,y1,z1)) {
		x2y1z1D = (*this)(x2,y1,z1)->density;
	}
	if (this->valid(x2,y1,z2)) {
		x2y1z2D = (*this)(x2,y1,z2)->density;
	}
	if (this->valid(x2,y2,z1)) {
		x2y2z1D = (*this)(x2,y2,z1)->density;
	}
	if (this->valid(x2,y2,z2)) {
		x2y2z2D = (*this)(x2,y2,z2)->density;
	}

	return Utils::trilerp(xWeight, yWeight, zWeight, x1y1z1D, x1y1z2D, x1y2z1D,
						  x1y2z2D, x2y1z1D, x2y1z2D, x2y2z1D, x2y2z2D) / 3.0f;

	return 1.0f;
}

/**
 * Convert a 3D index to a linear index
 */
int VoxelBuffer::sub2ind(int i, int j, int k) const
{
	return i + (j * this->_x) + k * (this->_x * this->_y);
}

/**
 * Convert a linear index to a 3D index
 */
void VoxelBuffer::ind2sub(int w, int& i, int& j, int& k) const
{
	i = w % this->_x;
	j = (w / this->_x) % this->_y;
	k = w / (this->_y * this->_x); 
}

/**
 * Tests if an index is valid
 */
bool VoxelBuffer::valid(int i, int j, int k) const
{
	return (i >= 0 && i < this->_x) &&
		   (j >= 0 && j < this->_y) &&
		   (k >= 0 && k < this->_z);
}

ostream& operator<<(ostream &s, const VoxelBuffer &vb)
{
	s << "VoxelBuffer[" << vb._x << "]"   <<
					"[" << vb._y << "]"   <<
					"[" << vb._z << "] {" << endl;
	int q = 0, w = 0;

	for (int k=0; k<vb._z; k++) {
		for (int j=0; j<vb._y; j++) {
			for (int i=0; i<vb._x; i++) {

				int ii,jj,kk;
				w = vb.sub2ind(i,j,k);
				vb.ind2sub(w, ii, jj, kk);
				s << q++ << "\t[(" << i << "," << j << "," << k << ")" 
						 <<  " => (" << ii << "," << jj << "," << kk <<")"
						 << " => " << vb.buffer[w] 
				  << endl;
			}
		}
	}
	return s << "}";
}

/**
 * Transmittance function, Q
 */
static float Q(const VoxelBuffer& buffer, float kappa, float step, int iters
			  ,const P& X, const V& N)
{
	Voxel *voxel = buffer.positionToVoxel(X);

	// Done or outside of the volume
	if (voxel == nullptr || iters <= 0) {
		return 1.0f;
	}

	return exp(-kappa * step * voxel->density) * 
		   Q(buffer, kappa, step, iters - 1, X + N, N);
}

/**
 * Perform ray marching through the volume accumulating density and 
 * transmittance values
 */
RayPath rayMarch(const VoxelBuffer& vb
				,const P& start
				,const P& end
				,float stepSize
				,bool interpolate
				,const list<Light*>& lights
				,float (*densityFunction)(Voxel* voxel, const P& X, void* densityData)
				,void* densityData)
{
	float kappa      = 1.0f;
	float T          = 1.0f;
	Color accumColor = Color::BLACK;
	Voxel* voxel     = nullptr;

	P X;
	V N;
	int iterations = steps(stepSize, FLT_EPSILON, start, end, X, N);

	for (int i=0; i<iterations; i++, X += N) {

		if (!(voxel = vb.positionToVoxel(X))) {
			break;
		}

		// If the density function is provided, use it
		float density = densityFunction == nullptr 
			? voxel->density 
			: densityFunction(voxel, X, densityData);
		
		if (interpolate) {
			density = vb.getInterpolatedDensity(X);
		}

		float deltaT      = exp(-kappa * stepSize * density);
		float attenuation = (1.0f - deltaT) / kappa;

		T *= deltaT;

		P center;
		vb.center(X, center);

		P LX;
		V LN;
		float offset = (2.0f * stepSize) + FLT_EPSILON;

		// For every light in the scene:
		auto li = lights.begin();

		for (int k=0; li != lights.end(); li++, k++) {

			Light* light = *li;

			int stepsToLight = steps(stepSize, offset, center, light->fromSampledPoint(center), LX, LN);

			if (voxel->light[k] < 0.0f) {
				voxel->light[k] = Q(vb, kappa, stepSize, stepsToLight, LX, LN);
			}

			accumColor += light->getColor(center) * 
						  Color::WHITE * 
						  attenuation * 
						  T * 
						  voxel->light[k];
		}
	}

	return RayPath(accumColor, T);
}
