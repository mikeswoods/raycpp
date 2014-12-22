/******************************************************************************
 *
 * A simple spherical environment map model
 *
 * @file EnvironmentMap.h
 * @author Michael Woods
 *
 *****************************************************************************/

#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include "EnvironmentMap.h"

using namespace std;

/*****************************************************************************/

EnvironmentMap::EnvironmentMap(const std::string& filename
	                          ,const std::string& mapType
							  ,float radius) :
	TextureMap(filename),
	mapType(SPHERE)
{
	this->mapType = this->stringToType(mapType);
	this->sphere  = new Sphere();
	this->cube    = new Cube();
	this->T       = glm::scale(glm::mat4(1.0f), glm::vec3(radius));
}

EnvironmentMap::MappingType EnvironmentMap::stringToType(const std::string& name) const
{
	// Convert SHAPE to a mapping type: sphere by default
	EnvironmentMap::MappingType mapType = EnvironmentMap::SPHERE;

	if (name == "CUBE") {
		mapType = EnvironmentMap::CUBE;
	} else if (name == "SPHERE") {
		mapType = EnvironmentMap::SPHERE;
	} else if (name == "WILD1") {
		mapType = EnvironmentMap::WILD1;
	} else if (name == "WILD2") {
		mapType = EnvironmentMap::WILD2;
	}

	return mapType;
}

Color EnvironmentMap::getColorFromRay(const Ray& ray) const
{
	Intersection isect;

	switch (this->mapType) {
		case SPHERE:
			{
				isect = this->sphere->intersect(this->T, ray);
			}
			break;
		case CUBE:
		default:
			{
				isect = this->cube->intersect(this->T, ray);
			}
			break;
	}

	if (isect.isHit()) {

		glm::vec3 hit = glm::normalize(ray.project(isect.t).xyz);
		Color out;

		switch (this->mapType) {
			case WILD1:
				{
					out = Color(abs(hit.x), abs(hit.y), abs(hit.z));
				}
				break;
			case WILD2:
				{
					out = Color(max(0.0f, hit.x), max(0.0f, hit.y), max(0.0f, hit.z));
				}
				break;
			case SPHERE:
				{
					glm::vec2 uv = SurfaceMap::mapToSphere(hit);
					out          = this->getColor(uv[0], uv[1]);
				}
				break;
			case CUBE:
			default:
				{
					glm::vec2 uv = SurfaceMap::mapToCube(hit);
					out          = this->getColor(uv[0], uv[1]);
				}
				break;
		}

		return out;
	}

	return Color::BLACK;
};

/*****************************************************************************/
