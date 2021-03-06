/******************************************************************************
 *
 * A simple spherical environment map model
 *
 * @file EnvironmentMap.h
 * @author Michael Woods
 *
 ******************************************************************************/

#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include "EnvironmentMap.h"

/******************************************************************************/

using namespace std;

/*******************************************************************************
 * Abstract environment map type
 ******************************************************************************/

EnvironmentMap::EnvironmentMap(const std::string& mapType, float radius)
{
	this->mapType = this->stringToType(mapType);
	this->T       = glm::scale(glm::mat4(), glm::vec3(radius));
}

EnvironmentMap::EnvironmentMap(const EnvironmentMap& other) :
	mapType(other.mapType),
	T(other.T),
	sphere(other.sphere),
	cube(other.cube)
{

}

EnvironmentMap::MappingType EnvironmentMap::stringToType(const std::string& name) const
{
	// Convert SHAPE to a mapping type: sphere by default
	EnvironmentMap::MappingType mapType = EnvironmentMap::SPHERE;

	string _name = Utils::uppercase(name);

	if (_name == "CUBE") {

		mapType = EnvironmentMap::CUBE;

	} else if (_name == "SPHERE") {

		mapType = EnvironmentMap::SPHERE;

	} else if (_name == "WILD1") {

		mapType = EnvironmentMap::WILD1;

	} else if (_name == "WILD2") {

		mapType = EnvironmentMap::WILD2;
	}

	return mapType;
}

Color EnvironmentMap::getColor(const Ray& ray, shared_ptr<SceneContext> scene) const
{
	Intersection isect;

	switch (this->mapType) {
		case SPHERE:
			{
				isect = this->sphere.intersect(this->T, ray, scene);
			}
			break;
		case CUBE:
		default:
			{
				isect = this->cube.intersect(this->T, ray, scene);
			}
			break;
	}

	if (isect.isHit()) {

		glm::vec3 hit = glm::normalize(ray.project(isect.t));
		Color out;

		switch (this->mapType) {
			case WILD1:
				{
					out = Color(fabs(hit.x), fabs(hit.y), fabs(hit.z));
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
}

/*******************************************************************************
 * Simple color-based environment map type
 ******************************************************************************/

ColorEnvironmentMap::ColorEnvironmentMap(const Color& _color) :
	EnvironmentMap("SPHERE"),
	color(_color)
{

}

Color ColorEnvironmentMap::getColor(float u, float v) const
{
	return this->color;
}

/*******************************************************************************
 * Texture environment map type
 ******************************************************************************/

TextureEnvironmentMap::TextureEnvironmentMap(const std::string& filename
	                                        ,const std::string& mapType) :
	EnvironmentMap(mapType),
	TextureMap(filename)
{

}

TextureEnvironmentMap::TextureEnvironmentMap(const std::string& filename
	                                        ,const std::string& mapType
							                ,float radius) :
	EnvironmentMap(mapType, radius),
	TextureMap(filename)
{

}

Color TextureEnvironmentMap::getColor(float u, float v) const
{
	return TextureMap::getColor(u, v);
}

/******************************************************************************/
