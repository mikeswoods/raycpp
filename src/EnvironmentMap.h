/******************************************************************************
 *
 * A simple spherical environment map model
 *
 * @file EnvironmentMap.h
 * @author Michael Woods
 *
 *****************************************************************************/


#ifndef ENVIRONMENT_MAP_H
#define ENVIRONMENT_MAP_H

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include "Sphere.h"
#include "Cube.h"
#include "Color.h"
#include "SurfaceMap.h"

/*****************************************************************************/

class EnvironmentMap : public TextureMap
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
		Sphere* sphere;
		Cube* cube;

		MappingType stringToType(const std::string& name) const;

	public:
		EnvironmentMap(const std::string& filename, const std::string& mapType, float radius = 1.0e3f);

		MappingType getMappingType() const { return this->mapType; }
		Color getColorFromRay(const Ray& ray) const;
};

/*****************************************************************************/

#endif
