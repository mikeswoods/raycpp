/*******************************************************************************
 *
 * A simple spherical environment map model
 *
 * @file EnvironmentMap.h
 * @author Michael Woods
 *
 ******************************************************************************/

#ifndef ENVIRONMENT_MAP_H
#define ENVIRONMENT_MAP_H

#include <memory>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include "Sphere.h"
#include "Cube.h"
#include "Color.h"
#include "SurfaceMap.h"

/******************************************************************************/

class SceneContext;

/*******************************************************************************
 * Abstract environment map type
 ******************************************************************************/

class EnvironmentMap
{
	public:
		enum MappingType 
		{
			 SPHERE
			,CUBE
			,WILD1
			,WILD2
		};

	protected:
		MappingType mapType;
		glm::mat4 T;
		Sphere sphere;
		Cube cube;

		MappingType stringToType(const std::string& name) const;

	public:
		EnvironmentMap(const std::string& mapType, float radius = 1.0e3f);
		EnvironmentMap(const EnvironmentMap& other);

		MappingType getMappingType() const { return this->mapType; }

		virtual Color getColor(float u, float v) const = 0;
		Color getColor(const Ray& ray, std::shared_ptr<SceneContext> scene) const;
};

/*******************************************************************************
 * Simple color-based environment map type
 ******************************************************************************/

class ColorEnvironmentMap : public EnvironmentMap
{
	protected:
		Color color;

	public:
		ColorEnvironmentMap(const Color& color);

		virtual Color getColor(float u, float v) const;

		const Color& getColor() const { return this->color; }
};

/*******************************************************************************
 * Texture environment map type
 ******************************************************************************/

class TextureEnvironmentMap : public EnvironmentMap, public TextureMap
{
	public:
		TextureEnvironmentMap(const std::string& filename, const std::string& mapType);
		TextureEnvironmentMap(const std::string& filename, const std::string& mapType, float radius);

		virtual Color getColor(float u, float v) const;
};

/******************************************************************************/

#endif
