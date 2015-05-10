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
#include <memory>
#include <utility> 
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#define cimg_display 0
#include <CImg.h>
#include "Config.h"
#include "Camera.h"
#include "Ray.h"
#include "SceneContext.h"

/******************************************************************************/

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
		{ 

		}

		TraceOptions(const TraceOptions& opts) :
			samplesPerLight(opts.samplesPerLight),
			samplesPerPixel(opts.samplesPerPixel),
			enablePixelDebug(opts.enablePixelDebug),
			xDebugPixel(opts.xDebugPixel),
			yDebugPixel(opts.yDebugPixel)
		{ 

		}

		friend std::ostream& operator<<(std::ostream& s, const TraceOptions& options);
};

/**
 * A representation of the current raytracing state/context state during
 * rendering
 */
class TraceContext
{
	public:
		// Scene context
		std::shared_ptr<SceneContext> scene;

		// Current ray
		Ray ray;

		// Current transformation matrix
		glm::mat4 T;

		// Current closest intersection
		Intersection closestIsect;

		TraceContext(std::shared_ptr<SceneContext> _scene) :
			scene(_scene),
			T(glm::mat4()),
			closestIsect(Intersection::miss())
		{ 

		}

		TraceContext(std::shared_ptr<SceneContext> _scene
			        ,const Ray& _ray) :
			scene(_scene),
			ray(_ray),
			T(glm::mat4()),
			closestIsect(Intersection::miss())
		{ 

		}

		TraceContext(std::shared_ptr<SceneContext> _scene
			        ,const Ray& _ray
			        ,const glm::mat4& _T) :
			scene(_scene),
			ray(_ray),
			T(_T),
			closestIsect(Intersection::miss())
		{ 

		}

		TraceContext(std::shared_ptr<SceneContext> _scene
			        ,const Ray& _ray
			        ,const glm::mat4& _T
			        ,const Intersection& _closestIsect) :
			scene(_scene),
			ray(_ray),
			T(_T),
			closestIsect(_closestIsect)
		{ 

		}

		TraceContext(const TraceContext& other) :
			scene(other.scene),
			ray(other.ray),
			T(other.T),
			closestIsect(other.closestIsect)
		{

		}
};

/******************************************************************************/

/**
 * Raytracing code 
 */
void initRaytrace(Camera&, std::shared_ptr<SceneContext> scene);

void rayTrace(std::shared_ptr<cimg_library::CImg<unsigned char>>
	         ,const Camera&
	         ,std::shared_ptr<SceneContext>
	         ,std::shared_ptr<TraceOptions>);

/******************************************************************************/

#endif


