/******************************************************************************
 *
 * A surface map class that can map bitmap data as both color and 
 * normal intensity (bump) information
 *
 * @file SurfaceMap.h
 * @author Michael Woods
 *
 *****************************************************************************/

#define _USE_MATH_DEFINES
#include <algorithm>
#include <cmath>
#include <cstring>
#include <stdexcept>
#include "SurfaceMap.h"
#include "Utils.h"

using namespace std;
using namespace glm;
using namespace cimg_library;

#define USE_BILINEAR_FILTERING 1

/******************************************************************************/

/**
 *  Static method that maps the given position P to a spherical UV position
 *
 * Code was adapted from the sphere example on the Wikwpedia article "UV Mapping"
 * at http://en.wikipedia.org/wiki/UV_mapping
 */
vec2 SurfaceMap::mapToSphere(const vec3& d)
{
	float u = 0.5f + ((atan(z(d), x(d)) / (2.0f * static_cast<float>(M_PI))));
	float v = 0.5f - (asin(y(d)) / static_cast<float>(M_PI));
	return vec2(u, v);
}

/**
 * Static method that maps the given position P to a cubic  UV position
 */
vec2 SurfaceMap::mapToCube(const vec3& d)
{
	float X = abs(d.x);
	float Y = abs(d.y);
	float Z = abs(d.z);
	float M = std::max(X, std::max(Y, Z));

	if (X == M) { // X is max
		float u = Utils::unitRange(d.z, -1.0f, 1.0f);
		float v = Utils::unitRange(d.x, -1.0f, 1.0f);
		return  vec2(u, v);
	} else if (Y == M) { // Y is max
		float u = Utils::unitRange(d.x, -1.0f, 1.0f);
		float v = Utils::unitRange(d.z, -1.0f, 1.0f);
		return  vec2(u, v);
	} else { // Z is max
		float u = Utils::unitRange(d.x, 1.0f, -1.0f);
		float v = Utils::unitRange(d.y, 1.0f, -1.0f);
		return  vec2(u, v);
	}
}

/******************************************************************************/

SurfaceMap::SurfaceMap(const string& _filename, MapType _type) :
	type(_type),
	filename(_filename),
	iWidth(0),
	iHeight(0),
	fWidth(0.0f),
	fHeight(0.0f)
{
	this->loadBitmap(_filename);
}

SurfaceMap::~SurfaceMap()
{

}

void SurfaceMap::loadBitmap(const string& filename)
{
	if (!this->image.load(filename.c_str())) {
		throw runtime_error("SurfaceMap: Could not read image from file");
	}

	this->iWidth  = this->image.width();
	this->iHeight = this->image.height();
	this->fWidth  = static_cast<float>(this->iWidth);
	this->fHeight = static_cast<float>(this->iHeight);
}

/**
 * Converts a UV coordinate, where both u and v are the range [0,1]
 * to a pixel coordinate (x,y) where x is in the range [0,ImageWidth-1] 
 * and y is in the range [0,ImageHeight-1];
 */
void SurfaceMap::uvToXY(float u, float v, int& x, int& y) const
{
	assert(u >= 0.0f && u <= 1.0f && v >= 0.0f && v <= 1.0f);

	x = static_cast<int>(floor(u * this->fWidth));
	y = static_cast<int>(floor(v * this->fHeight));
}

/**
 * Given u and v coordinates, this function computes four weight values for 
 * four (x,y) coordinates to intgerpolate using bilinear filtering
 */
vec4 SurfaceMap::getBilinearWeights(float u
	                                    ,float v
										,vec2& P1
										,vec2& P2
										,vec2& P3
										,vec2& P4) const
{
	assert(u >= 0.0f && u <= 1.0f && v >= 0.0f && v <= 1.0f);

	float U = u * this->fWidth;
	float V = v * this->fHeight;

	// Find the fractional parts of u and v:
	float S = U - floorf(U);
	float T = U - floorf(U);

	// Weights 1-4 corresponding to positions <P1,P2,P3,P4>
	vec4 W = vec4((1.0f - S) * (1.0f - T), S * (1.0f - T), (1.0f - S) * T, S *  T);

	float WW = this->fWidth - 1.0f;
	float HH = this->fHeight - 1.0f;
	float UF = static_cast<float>(floor(U));
	float UC = static_cast<float>(ceil(U));
	float VF = static_cast<float>(floor(V));
	float VC = static_cast<float>(ceil(V));

	P1 = vec2(static_cast<int>(Utils::clamp(UF, 0.0f, WW)), static_cast<int>(Utils::clamp(VF, 0.0f, HH)));
	P2 = vec2(static_cast<int>(Utils::clamp(UC, 0.0f, WW)), static_cast<int>(Utils::clamp(VF, 0.0f, HH)));
	P3 = vec2(static_cast<int>(Utils::clamp(UF, 0.0f, WW)), static_cast<int>(Utils::clamp(VC, 0.0f, HH)));
	P4 = vec2(static_cast<int>(Utils::clamp(UC, 0.0f, WW)), static_cast<int>(Utils::clamp(VC, 0.0f, HH)));

	return W;
}

/******************************************************************************/

TextureMap::TextureMap(const string& filename) :
	SurfaceMap(filename, TEXTURE_MAP)
{

}

TextureMap::~TextureMap()
{

}

/**
 * Given a pixel (i,j) coordinate, this returns an RGB color value
 */
Color TextureMap::getColor(int i, int j) const
{
	ImagePixel p = pixel(this->image, i, j);
	return Color(RED(p), GREEN(p), BLUE(p));
}

/**
 * Given a (u,v) coordinate, this returns an RGB color value
 */
Color TextureMap::getColor(float u, float v) const
{
	#ifdef USE_BILINEAR_FILTERING

	vec2 P1, P2, P3, P4;
	vec4 W = this->getBilinearWeights(u, v, P1, P2, P3, P4);

	ImagePixel pixels[4] = {
		pixel(this->image, static_cast<int>(P1.x), static_cast<int>(P1.y)),
		pixel(this->image, static_cast<int>(P2.x), static_cast<int>(P2.y)),
		pixel(this->image, static_cast<int>(P3.x), static_cast<int>(P3.y)),
		pixel(this->image, static_cast<int>(P4.x), static_cast<int>(P4.y))
	};

	return (W[0] * Color(RED(pixels[0]), GREEN(pixels[0]), BLUE(pixels[0]))) + 
		   (W[1] * Color(RED(pixels[1]), GREEN(pixels[1]), BLUE(pixels[1]))) +
		   (W[2] * Color(RED(pixels[2]), GREEN(pixels[2]), BLUE(pixels[2]))) +
		   (W[3] * Color(RED(pixels[3]), GREEN(pixels[3]), BLUE(pixels[3])));
	#else

	int x = 0, y = 0;
	uvToXY(u, v, x, y);
	ImagePixel p = pixel(this->image->GetPixel(x, y);
	return Color(RED(p), GREEN(p), BLUE(p));
	#endif
}

/******************************************************************************/

BumpMap::BumpMap(const string& filename) :
	SurfaceMap(filename, BUMP_MAP),
	N(0),
	bU(nullptr),
	bV(nullptr)
{
	this->initHeightMap();
	this->computeDerivatives();
}

BumpMap::~BumpMap()
{
	delete [] this->bU;
	delete [] this->bV;
}

void BumpMap::initHeightMap()
{
	this->N = this->iWidth * this->iHeight;

	// The derivative in the X & Y directions:
	this->bU = new float[N];
	this->bV = new float[N];

	// Zero everything out initially:
	memset(&this->bU[0], 0, sizeof(this->bU[0]) * N);
	memset(&this->bV[0], 0, sizeof(this->bV[0]) * N);
}

/**
 * Computes the derivatives in X and Y based on the technique
 * outlined in the lecture notes pg 911-13
 */
void BumpMap::computeDerivatives()
{
	unsigned int k = 0;

	for (int i=0; i<this->iWidth; i++) {
		for (int j=0; j<this->iHeight; j++) {

			k = this->convert2DTo1D(i, j);
			assert(k < this->N);

			float bIJ   = this->getIntensity(i, j);
			this->bU[k] = this->getIntensity(i+1 < this->iWidth ? i+1 : i, j) - bIJ;
			this->bV[k] = this->getIntensity(i, j+1 < this->iHeight ? j+1 : j) - bIJ;
		}
	}
}

/**
 * Given a pixel (i,j) coordinate, this returns an intensity value in the range [0,1]
 */
float BumpMap::getIntensity(int i, int j) const
{
	ImagePixel p = pixel(this->image, i, j);
	return Color(RED(p), GREEN(p), BLUE(p)).luminosity();
}

/**
 * Given a (u,v) coordinate, this returns an intensity value in the range [0,1]
 */
float BumpMap::getIntensity(float u, float v) const
{
	int i = 0, j = 0;
	uvToXY(u, v, i, j);
	ImagePixel p = pixel(this->image, i, j);
	return Color(RED(p), GREEN(p), BLUE(p)).luminosity();
}

/**
 * Get the bump "depth" at the given cartesian (i,j) or (u,v) position
 */
vec3 BumpMap::getNormal(float u, float v) const
{
	// Adapted from http://web.cse.ohio-state.edu/~hwshen/781/Site/Slides_files/bump.pdf
	// and lecture notes
	assert(u >= 0.0f && u <= 1.0f && v >= 0.0f && v <= 1.0f);

	int x = 0, y = 0;
	uvToXY(u, v, x, y);

	// Get the partial derivates bU and bV
	unsigned int k = this->convert2DTo1D(x, y);
	assert(k < this->N);

	float bu = this->bU[k];
	float bv = this->bV[k];
	vec3 pu  = vec3(u, v, bu);
	vec3 pv  = vec3(u, v, bv);
	vec3 n   = cross(pu, pv);
	float Ln = std::max(FLT_EPSILON, length(n));

	return n + (bu * (cross(n, pv) / Ln)) + (bv * (cross(n, pu) / Ln));
}

/******************************************************************************/

ostream& operator<<(ostream& s, const SurfaceMap& map)
{
	if (map.isTextureMap()) {
		s << "TextureMap { \"" << map.filename << "\" }";
	} else {
		s << "BumpMap { \"" << map.filename << "\" }";
	}
	return s;
}

/******************************************************************************/
