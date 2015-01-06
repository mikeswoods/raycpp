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

ostream& operator<<(ostream &s, const Voxel &v)
{
	return s << "{ light = "   << v.light 
			 << ", density = " << v.density << " }";
}

/*******************************************************************************
 * Voxel buffer
 ******************************************************************************/

VoxelBuffer::VoxelBuffer(int xDim, int yDim, int zDim, Geometry const * geometry)
{
	this->xDim   = xDim;
	this->yDim   = yDim;
	this->zDim   = zDim;
	this->buffer = new Voxel[this->xDim * this->yDim * this->zDim];

	//this->vWidth  = (float)this->bounds.width() / this->xDim;
	//this->vHeight = (float)this->bounds.height() / this->yDim;
	//this->vDepth  = (float)this->bounds.depth() / this->zDim;

	this->vWidth  = 1.0f;
	this->vHeight = 1.0f;
	this->vDepth  = 1.0f;
}

VoxelBuffer::~VoxelBuffer()
{
	delete this->buffer;
	this->buffer = nullptr;
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
	/*
	P p1 = this->bounds.getP1();
	P p2 = this->bounds.getP2();

	float dx  = (x(p2) - x(p1)) / static_cast<float>(this->xDim);
	float dy  = (y(p2) - y(p1)) / static_cast<float>(this->yDim);
	float dz  = (z(p2) - z(p1)) / static_cast<float>(this->zDim);
	float dx2 = 0.5f * dx; 
	float dy2 = 0.5f * dy;
	float dz2 = 0.5f * dz; 

	center = P(x(p1) + dx2 + (dx * static_cast<float>(i))
			  ,y(p1) + dy2 + (dy * static_cast<float>(j))
			  ,z(p1) + dz2 + (dz * static_cast<float>(k)));
	*/
	return true;
}

/**
 * Sets center to the center point of the voxel the point p falls within.
 * If this method returns false, the voxel at (i,j,k) does not exist
 */
bool VoxelBuffer::center(int i, int j, int k, P& center) const
{
	if (!this->valid(i, j, k)) {
		return false;
	}
	/*
	P p1 = this->bounds.getP1();
	P p2 = this->bounds.getP2();

	float dx  = (x(p2) - x(p1)) / static_cast<float>(this->xDim);
	float dy  = (y(p2) - y(p1)) / static_cast<float>(this->yDim);
	float dz  = (z(p2) - z(p1)) / static_cast<float>(this->zDim);
	float dx2 = 0.5f * dx; 
	float dy2 = 0.5f * dy;
	float dz2 = 0.5f * dz; 

	center = P(x(p1) + dx2 + (dx * static_cast<float>(i))
			  ,y(p1) + dy2 + (dy * static_cast<float>(j))
			  ,z(p1) + dz2 + (dz * static_cast<float>(k)));
	*/
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
	/*
	P p1 = this->bounds.getP1();
	P p2 = this->bounds.getP2();

	dx = Utils::unitRange(x(p), x(p1), x(p2));
	dy = Utils::unitRange(y(p), y(p1), y(p2));
	dz = Utils::unitRange(z(p), z(p1), z(p2));

	float threshold = 1.0e-6f;

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

	float xLoc = dx * ((static_cast<float>(this->xDim)) - this->getVoxelWidth());
	float yLoc = dy * ((static_cast<float>(this->yDim)) - this->getVoxelHeight());
	float zLoc = dz * ((static_cast<float>(this->zDim)) - this->getVoxelDepth());
	xCell = static_cast<int>(floor(xLoc));
	yCell = static_cast<int>(floor(yLoc));
	zCell = static_cast<int>(floor(zLoc));

	if (!this->valid(xCell, yCell, zCell)) {
		return false;
	}

	i = xCell;
	j = yCell;
	k = zCell;
	*/
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
	/*
	P p1 = this->bounds.getP1();
	P p2 = this->bounds.getP2();

	dx = Utils::unitRange(x(p), x(p1), x(p2));
	dy = Utils::unitRange(y(p), y(p1), y(p2));
	dz = Utils::unitRange(z(p), z(p1), z(p2));

	float xLoc    = dx * ((float)this->xDim - 1);
	float xWeight = xLoc - floor(xLoc); 
	x1 = static_cast<int>(floor(xLoc));
	x2 = static_cast<int>(ceil(xLoc));

	float yLoc = dy * ((float)this->yDim - 1);
	float yWeight = yLoc - floor(yLoc); 
	y1 = static_cast<int>(floor(yLoc));
	y2 = static_cast<int>(ceil(yLoc));    

	float zLoc = dz * ((float)this->zDim - 1);
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
	*/
	return 1.0f;
}

/**
 * Convert a 3D index to a linear index
 */
int VoxelBuffer::sub2ind(int i, int j, int k) const
{
	return i + (j * this->xDim) + k * (this->xDim * this->yDim);
}

/**
 * Convert a linear index to a 3D index
 */
void VoxelBuffer::ind2sub(int w, int& i, int& j, int& k) const
{
	i = w % this->xDim;
	j = (w / this->xDim) % this->yDim;
	k = w / (this->yDim * this->xDim); 
}

/**
 * Tests if an index is valid
 */
bool VoxelBuffer::valid(int i, int j, int k) const
{
	return (i >= 0 && i < this->xDim) &&
		   (j >= 0 && j < this->yDim) &&
		   (k >= 0 && k < this->zDim);
}

ostream& operator<<(ostream &s, const VoxelBuffer &vb)
{
	s << "VoxelBuffer[" << vb.xDim << "]"   <<
					"[" << vb.yDim << "]"   <<
					"[" << vb.zDim << "] {" << endl;
	int q = 0, w = 0;

	for (int k=0; k<vb.zDim; k++) {
		for (int j=0; j<vb.yDim; j++) {
			for (int i=0; i<vb.xDim; i++) {

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

			int stepsToLight = steps(stepSize, offset, center, light->getPosition(), LX, LN);

			if (voxel->light[k] < 0.0f) {
				voxel->light[k] = Q(vb, kappa, stepSize, stepsToLight, LX, LN);
			}

			accumColor += light->getColor() * 
						  Color::WHITE * 
						  attenuation * 
						  T * 
						  voxel->light[k];
		}
	}

	return RayPath(accumColor, T);
}
