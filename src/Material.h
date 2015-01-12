/******************************************************************************
 *
 * This class defines the qualities of an object's surface material
 *
 * @file Material.h
 * @author Michael Woods
 *
 ******************************************************************************/

#ifndef MATERIAL_H
#define MATERIAL_H

#include <memory>
#include <iostream>
#include "Color.h"
#include "SurfaceMap.h"

/******************************************************************************/

class Material
{
	protected:
		// Name of the material
		std::string name;

		// Diffuse color 
		Color diff;

		// Reflective/specular color
		Color refl;

		// Specular exponent
		float expo;

		// Index of refraction
		float ior;

		// Mirror surface flag
		bool mirr;

		// Transparent surface flag
		bool tran;

		// Emissive surface flag
		bool emit;

		// Ambient coefficient
		float ambient;

		// Texture map
		std::shared_ptr<TextureMap> textureMap;

		// Bump map
		std::shared_ptr<BumpMap> bumpMap;

	public:

		static const float DEFAULT_AMBIENT_COEFF;

		Material();
		Material(std::string name
	            ,Color diff
				,Color refl
				,float expo
				,float ior
				,bool mirr
				,bool tran
				,bool emit
				,float ambient
				,std::shared_ptr<TextureMap> textureMap = std::shared_ptr<TextureMap>(nullptr)
				,std::shared_ptr<BumpMap> bumpMap = std::shared_ptr<BumpMap>(nullptr));
		Material(const Material& other);

        friend std::ostream& operator<<(std::ostream& os, const Material& m);

		const std::string& getName() const                { return this->name; }
		const Color& getDiffuseColor() const              { return this->diff; }
		const Color& getReflectColor() const              { return this->refl; }
		float getSpecularExponent() const                 { return this->expo; }
		float getIndexOfRefraction() const                { return this->ior; }
		bool isMirror() const                             { return this->mirr; }
		bool isTransparent() const                        { return this->tran; }
		bool isEmissive() const                           { return this->emit; }
		float getAmbientCoeff() const                     { return this->ambient; } 
		std::shared_ptr<TextureMap> getTextureMap() const { return this->textureMap; } 
		bool hasTextureMap() const                        { return this->textureMap != nullptr; }
		std::shared_ptr<BumpMap> getBumpMap() const       { return this->bumpMap; } 
		bool hasBumpMap() const                           { return this->bumpMap != nullptr; }

		// "Smart" color function will return the correct color based on
		// assigned material attributes:
		Color getColor() const;

		// Color function that takes a position in R^3 and a geometric object
		// and maps a color based on the given information
		Color getColor(const glm::vec3& d, std::shared_ptr<Geometry> geometry) const;

		// Given a position in R^3 and a geometric object, this function returns
		// the normal intensity at the given position
		float getIntensity(const glm::vec3& d, std::shared_ptr<Geometry> geometry) const;

		// Given a position in R^3 and a geometric object, this function returns
		// the surface bump normal the given position
		glm::vec3 getNormal(const glm::vec3& d, std::shared_ptr<Geometry> geometry) const;
};

/******************************************************************************/

#endif