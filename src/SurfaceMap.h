/******************************************************************************
 *
 * A surface map class that can map bitmap data as both color and 
 * normal information
 *
 * @file SurfaceMap.h
 * @author Michael Woods
 *
 ******************************************************************************/

#ifndef SURFACE_MAP_H
#define SURFACE_MAP_H

#include <iostream>
#include <memory>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <EasyBMP/EasyBMP.h>
#include "Color.h"
#include "R3.h"
#include "Geometry.h"

/*******************************************************************************
 * Abstract surface map type
 ******************************************************************************/
class SurfaceMap
{
	public:
		enum MapType 
		{
			 TEXTURE_MAP
			,BUMP_MAP
		};

		// Maps the given position P to a spherical UV position
		static glm::vec2 mapToSphere(const glm::vec3& d);

		// Maps the given position P to a cubic UV position
		static glm::vec2 mapToCube(const glm::vec3& d);

	protected:
		MapType type;
		std::unique_ptr<BMP> bitmap;
		std::string filename;
		int iWidth, iHeight;
		float fWidth, fHeight;

		inline void convert1DTo2D(int i, int&x, int&y) const
		{
			x = i / this->iHeight;
			y = i % this->iHeight;
		}

		inline int convert2DTo1D(int i, int j) const
		{
			return (i * this->iHeight) + j;
		}

		void loadBitmap(const std::string& filename);
		virtual void uvToXY(float u, float v, int& x, int& y) const;

	public:
		SurfaceMap(const std::string& filename, MapType type);
		virtual ~SurfaceMap();

		// Given u and v coordinates, this function computes 4 weights
		// for bilinear filtering
		glm::vec4 getBilinearWeights(float u, float v, glm::vec2& P1, glm::vec2& P2, glm::vec2& P3, glm::vec2& P4) const;

		const std::string& getFileName() const { return this->filename; }

		MapType getType() const   { return this->type; }
		bool isTextureMap() const { return this->type == TEXTURE_MAP; }
		bool isBumpMap() const    { return this->type == BUMP_MAP; }

		friend std::ostream& operator<<(std::ostream& s, const SurfaceMap& m);
};

/*******************************************************************************
 * Texture map
 ******************************************************************************/
class TextureMap : public SurfaceMap
{
	public:
		TextureMap(const std::string& filename);
		virtual ~TextureMap();

		// Get the color at the given cartesian (i,j) or (u,v) position
		virtual Color getColor(int i, int j) const;
		virtual Color getColor(float u, float v) const;
};

/*******************************************************************************
 * Bump map
 ******************************************************************************/
class BumpMap : public SurfaceMap
{
	private:
		unsigned int N;
		float* bU; // Bump map derivative in the X direction
		float* bV; // Bump map derivative in the Y direction

	protected:
		void initHeightMap();
		void computeDerivatives();

	public:
		BumpMap(const std::string& filename);
		virtual ~BumpMap();

		// Get the intensity at the given cartesian (i,j) or (u,v) position
		virtual float getIntensity(int i, int j) const;
		virtual float getIntensity(float u, float v) const;

		// Get the bump "depth" at the given cartesian (i,j) or (u,v) position
		glm::vec3 getNormal(float u, float v) const;
};

/******************************************************************************/

#endif
