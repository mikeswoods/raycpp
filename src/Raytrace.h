/*******************************************************************************
 *
 * This file defines the core of the raytracer implementation
 *
 * @file Raytrace.h
 * @author Michael Woods
 *
 ******************************************************************************/

#ifndef RAYTRACE_H
#define RAYTRACE_H

#include <iostream>
 
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <EasyBMP/EasyBMP.h>

#include "Config.h"
#include "Camera.h"
#include "Ray.h"
#include "WorldState.h"

/******************************************************************************/

/**
 * NDC coordinate pair datatype
 */
struct NDCCoord
{
	float x,y;
};

/** 
 * Options specifying various parameters during raytracing like sampling, 
 * debugging, etc.
 */
class TraceOptions
{
	public:

		static const unsigned int SAMPLES_PER_LIGHT_DEFAULT = 4;
		static const unsigned int SAMPLES_PER_PIXEL_DEFAULT = 1;

		// Number of samples to take for soft shadows
		int samplesPerLight;

		// Number of samples to take per pixel
		int samplesPerPixel;

		// Debug pixel flag
		bool enablePixelDebug;

		// Debug pixel-x
		int xDebugPixel;

		// Debug pixel-y
		int yDebugPixel;

		TraceOptions() :
			samplesPerLight(SAMPLES_PER_LIGHT_DEFAULT),
			samplesPerPixel(SAMPLES_PER_PIXEL_DEFAULT),
			enablePixelDebug(false),
			xDebugPixel(-1),
			yDebugPixel(-1)
		{ }

		TraceOptions(const TraceOptions& opts) :
			samplesPerLight(opts.samplesPerLight),
			samplesPerPixel(opts.samplesPerPixel),
			enablePixelDebug(opts.enablePixelDebug),
			xDebugPixel(opts.xDebugPixel),
			yDebugPixel(opts.yDebugPixel)
		{ }

		friend std::ostream& operator<<(std::ostream& s, const TraceOptions& options);
};

/**
 * A representation of the current raytracing state/context state during
 * rendering
 */
struct TraceContext
{
	// Current ray
	Ray ray;

	// Current transformation matrix
	glm::mat4 T;

	// Current closest intersection
	Intersection closestIsect;

	TraceContext() :
		T(glm::mat4()),
		closestIsect(Intersection::miss())
	{ }

	TraceContext(const Ray& _ray) :
		ray(_ray),
		T(glm::mat4()),
		closestIsect(Intersection::miss())
	{ }

	TraceContext(const Ray& _ray, const glm::mat4& _T) :
		ray(_ray),
		T(_T),
		closestIsect(Intersection::miss())
	{ }

	TraceContext(const Ray& _ray, const glm::mat4& _T, const Intersection& _closestIsect) :
		ray(_ray),
		T(_T),
		closestIsect(_closestIsect)
	{ }

	TraceContext(const TraceContext& other) :
		ray(other.ray),
		T(other.T),
		closestIsect(other.closestIsect)
	{ }
};

/**
 * Raytracing code 
 */
void initRaytrace(const Configuration& config, WorldState& state, Camera& camera);

void rayTrace(BMP& output
	         ,const WorldState& state
	         ,const Camera& camera
			 ,const Graph& graph
			 ,int resoX
			 ,int resoY
			 ,TraceOptions& options);

#endif
