/******************************************************************************
 *
 * This file defines the core of the raytracer implementation
 *
 * @file Raytrace.h
 * @author Michael Woods
 *
 *****************************************************************************/

#include <algorithm>
#include <ctime>
#include <chrono>
#include <cassert>
#include <omp.h>
#include <cstdlib>
#include <iostream>
#include <utility>
#include <stack>
#include "Raytrace.h"
#include "Intersection.h"
#include "EnvironmentMap.h"
#include "AreaLight.h"

using namespace std;

/******************************************************************************
 *
 * Macro definitions
 *
 *****************************************************************************/

#ifndef __FUNCTION_NAME__
    #ifdef WIN32   //WINDOWS
        #define __FUNCTION_NAME__   __FUNCTION__  
    #else          //*NIX
        #define __FUNCTION_NAME__   __func__ 
    #endif
#endif

// Enable pixel-level debugging:
//#define ENABLE_PIXEL_DEBUG 1

// Maximum raytrace recursion depth:
#define MAX_DEPTH 5

/******************************************************************************
 *
 * Foward declarations
 *
 *****************************************************************************/

static Color trace(TraceOptions& options
	              ,const Ray& ray
		          ,const Graph& graph
				  ,EnvironmentMap const * envMap
		          ,const list<Light*>& lights
		          ,int depth
				  ,bool isDebugPixel);

/******************************************************************************
 *
 * Output operations
 *
 *****************************************************************************/

std::ostream& operator<<(std::ostream& s, const Intersection& isect)
{
	s << "Intersection {"                      << endl
	  << "  t="           << isect.t           << endl
	  << "  node="        << isect.node        << endl
	  << "  inside="      << isect.inside      << endl
	  << "  hitWorld="    << isect.hitWorld    << endl
	  << "  hitLocal="    << isect.hitLocal    << endl
	  << "  normal="      << isect.normal      << endl
	  << "  normalLocal=" << isect.normalLocal << endl
	  << "}"                                   << endl;
	return s;
}

std::ostream& operator<<(std::ostream& s, const TraceContext& ctx)
{
	s << "TraceContext {"                      << endl
	  << "  ray="          << ctx.ray          << endl
	  << "  closestIsect=" << ctx.closestIsect << endl
	  << "}"                                   << endl;
	return s;
}

std::ostream& operator<<(std::ostream& s, const TraceOptions& options)
{
	s << "TraceOptions {"      << endl
	  << "  samplesPerLight="  << options.samplesPerLight << endl
	  << "  samplesPerPixel="  << options.samplesPerPixel << endl
	  << "  enablePixelDebug=" << (options.enablePixelDebug ? "yes" : "no") << endl;
	if (options.enablePixelDebug) {
		s << "  debugPixel=(" << options.xDebugPixel << "," << options.yDebugPixel << ")" 
		  << endl;
	}
	s << "}" << endl;
	return s;
}

/******************************************************************************
 *
 * Debug a given pixel
 *
 *****************************************************************************/

static std::ostream& debugPixel(std::ostream& os, string funcName, int depth)
{
	for (int i=0; i<depth; i++) {
		os << "    ";
	}
	os << funcName << "<" << depth << ">: ";
	return os;
}

static std::ostream& debugPixel(string funcName, int depth, const Color& output)
{
	return debugPixel(cerr, funcName, depth) << output << endl;
}

static std::ostream& debugPixel(string funcName, int depth, const glm::vec3& output)
{
	return debugPixel(cerr, funcName, depth) << output << endl;
}

static std::ostream& debugPixel(string funcName, int depth, float output)
{
	return debugPixel(cerr, funcName, depth) << output << endl;
}

static std::ostream& debugPixel(string funcName, int depth, int output)
{
	return debugPixel(cerr, funcName, depth) << output << endl;
}

static std::ostream& debugPixel(string funcName, int depth, string output)
{
	return debugPixel(cerr, funcName, depth) << output << endl;
}

/******************************************************************************
 *
 * Initializes the raytracer camera
 *
 *****************************************************************************/

void initRaytrace(const Configuration& config, WorldState& state, Camera& camera)
{
	P position = P(config.EYEP[0],config.EYEP[1],config.EYEP[2]);
	V viewDir  = V(config.VDIR[0],config.VDIR[1],config.VDIR[2]);
	V upVec    = Utils::fixUpVector(V(config.VDIR[0],config.VDIR[1],config.VDIR[2])
		                           ,V(config.UVEC[0],config.UVEC[1],config.UVEC[2]));

	// Camera
	camera.setPosition(position);
	camera.setViewDir(viewDir);
	camera.setUp(upVec);

	// Divide this by 2 to match the FOV produced by glm::perspective(). 
	// See Piazza post @374 for details.
	camera.setFOV(config.FOVY / 2.0f);

	// Resolution
	camera.setAspectRatio(static_cast<float>(config.RESO[0]) / static_cast<float>(config.RESO[1]));

	// Environment map:
	state.setEnvironmentMap(config.getEnvironmentMap());
}

/*****************************************************************************/

/**
 * Fold function used in closestIntersection() below
 */
static bool isCloser(const TraceContext& test, TraceContext& other)
{
	return test.closestIsect.isCloser(other.closestIsect);
}

/**
 * A fold visitor function used in closestIntersection() below
 */
static TraceContext intersectNode(GraphNode* node, TraceContext ctx)
{
	Geometry const * geometry = node->getGeometry();

	glm::mat4 nextT    = applyTransform(node, ctx.T);
	Intersection isect = geometry != NULL ? geometry->intersect(nextT, ctx.ray) : Intersection::miss();

	if (isect.isHit()) {
		isect.node = node;
	}

	return TraceContext(ctx.ray, nextT, isect);
}

/**
 * Returns the TraceContext instance with the closest (smallest >= 0) t value.
 * Used by closestIntersection
 */
static TraceContext findClosestContextNode(TraceContext currentCtx, TraceContext lastCtx)
{
	return isCloser(currentCtx, lastCtx) ? currentCtx : lastCtx;
}

/******************************************************************************
 *
 * Given a ray, this function computes the closest intersect in a scene graph
 *
 *****************************************************************************/

static TraceContext closestIntersection(const Ray& ray
	                                   ,const Graph& graph
									   ,bool& hit)
{
	glm::mat4 I           = glm::mat4();
	TraceContext initCtx  = TraceContext(ray, I); // Identity transform
	TraceContext finalCtx = fold(graph, intersectNode, findClosestContextNode, initCtx);

	hit = finalCtx.closestIsect.isHit();

	return finalCtx;
}

/******************************************************************************
 *
 * Faster method to determine if something is hit. As soon as an intersection
 * occurs, the loop exits and returns the first intersection
 *
 *****************************************************************************/

static bool fastTestInShadow(const Ray& ray, const Graph& graph, GraphNode* ignore, float withinDist)
{
	// Initial transformation matrix is the identity matrix:
	glm::mat4 I = glm::mat4();

	stack<pair<GraphNode*,glm::mat4>> S;

	S.push(make_pair(graph.getRoot(), I));

	while (!S.empty()) {

		pair<GraphNode*,glm::mat4> nodeAndT = S.top();
		S.pop();

		GraphNode* node = nodeAndT.first;
		glm::mat4 T     = nodeAndT.second;

		Geometry const * geometry = node->getGeometry();
		glm::mat4 nextT = applyTransform(node, T);

		if (node != ignore && geometry != NULL) {

			Intersection isect = geometry->intersect(nextT, ray);
			isect.node = node;

			if (isect.isHit() && !node->isAreaLight() && isect.t < withinDist) {

				return true; // We're done
			}
		}

		std::list<GraphNode*> children = node->getChildren();

		// Otherwise, go through the children:
		for (std::list<GraphNode*>::const_iterator i=children.begin(); i != children.end(); i++) {
			S.push(make_pair(*i, nextT));
		}
	}

	return false;
}

/******************************************************************************
 *
 * Tests if the given point is in the shadow of another object
 *
 *****************************************************************************/

static bool isOccludedFromPosition(const Graph& graph
	                              ,GraphNode* const selfNode
					              ,const P& hitAt
					              ,Light * const light)
{
	float cosine = 0.0f;
	V L          = light->fromSampledPoint(hitAt, cosine);

	// if the cosine angle is less than zero, then the sample on the surface of
	// the light geometry is pointing away from the position to test for 
	// occlusion. As a result, it cannot occlude, so we can exit early:
	/*
	if (cosine < 0.0f) {
		return false;
	}
	*/

	Ray ray(hitAt, glm::normalize(L), Utils::EPSILON, Ray::SHADOW);

	return fastTestInShadow(ray, graph, selfNode, glm::length(L));

	/*
	bool hit = false;

	TraceContext ctx = closestIntersection(ray, graph, hit);

	// Was there a hit AND the node that was intersected was not the object 
	// being test for occlusion AND the distance from the test object is
	// less than or equal to the distance from intersected node to the 
	// light source?

	return hit && ctx.closestIsect.node != selfNode 
		       && !light->isLightSourceNode(ctx.closestIsect.node)
		       && ctx.closestIsect.t < glm::length(L);
	*/
};

/******************************************************************************
 *
 * Given a hit position, a light source, and a number of samples, this function
 * tests if the position is considered to be in a shadow, returning the shade
 * factor apply to the coloring at the position in the range [0,1].
 *
 * A shade factor of 0 indicates the object is fully occluded (in shadow), 
 * while a value of 1 indicates it is not shadowed at all.
 *
 *****************************************************************************/

static float shadow(const Graph& graph
	                ,GraphNode* const selfNode
					,const P& hitAt
					,Light * const light
					,int samples)
{
	// If the self node being tested is the light itself, bail immediately:
	if (light->isLightSourceNode(selfNode)) {
		return 1.0f;
	}

	// Point lights only need 1 sample, 
	if (light->getLightType() == Light::POINT_LIGHT) {
		return isOccludedFromPosition(graph, selfNode, hitAt, light) 
			? 0.0f 
			: 1.0f;
	}

	float contribution = 1.0f / static_cast<float>(samples);
	float shadeFactor  = 1.0f;

	for (int i=0; i<samples; i++) {
		if (isOccludedFromPosition(graph, selfNode, hitAt, light)) {
			shadeFactor -= contribution;
		}
	}

	return shadeFactor;
};

/******************************************************************************
 *
 * Compute the color contribution from the reflected ray
 *
 *****************************************************************************/

static Color traceReflect(TraceOptions& options
	                     ,const Graph& graph
						 ,EnvironmentMap const * envMap
	                     ,const Intersection& isect
	                     ,const V& I
						 ,const V& N
						 ,const list<Light*>& lights
						 ,int depth
						 ,bool isDebugPixel = false)
{
	Material const * mat = isect.node->getMaterial();
	assert(mat != NULL);

	V R = glm::reflect(I, N);

	#ifdef ENABLE_PIXEL_DEBUG
	if (options.enablePixelDebug && isDebugPixel) {
		debugPixel(__FUNCTION_NAME__ "/debug:traceReflect", depth, R);
	}
	#endif

	Ray ray(isect.hitWorld, R, Utils::EPSILON, Ray::REFLECTION);

	return mat->getReflectColor() * 
		   trace(options, ray, graph, envMap, lights, depth + 1, isDebugPixel);
}

/******************************************************************************
 *
 * Compute the color contribution from the refracted ray
 *
 *****************************************************************************/

static Color traceRefract(TraceOptions& options
	                     ,const Graph& graph
						 ,EnvironmentMap const * envMap
	                     ,const Intersection& isect
	                     ,const V& I
						 ,const V& N
						 ,float n
						 ,const list<Light*>& lights
						 ,int depth
						 ,bool isDebugPixel)
{
	V R = glm::refract(I, N, n);

	// Is R a zero vector? If so, reflect instead:
	if (R == glm::vec3(0, 0, 0)) {

		return traceReflect(options
	                       ,graph
						   ,envMap
	                       ,isect
	                       ,I
						   ,N
						   ,lights
						   ,depth
						   ,isDebugPixel);
	}

	#ifdef ENABLE_PIXEL_DEBUG
	if (options.enablePixelDebug && isDebugPixel) {
		debugPixel(__FUNCTION_NAME__ "/debug:I=", depth, I);
		debugPixel(__FUNCTION_NAME__ "/debug:N=", depth, N);
		debugPixel(__FUNCTION_NAME__ "/debug:R=", depth, R);
		debugPixel(__FUNCTION_NAME__ "/debug:n=", depth, n);
	}
	#endif

	Ray ray(isect.hitWorld, R, Utils::EPSILON, Ray::REFRACTION);

	return trace(options, ray, graph, envMap, lights, depth + 1, isDebugPixel);
}

/******************************************************************************
 *
 * Computes Schlick's approximation of the Fresnel term
 * http://en.wikipedia.org/wiki/Schlick%27s_approximation
 *
 *****************************************************************************/

float reflectCoeff(const V& lightDir, const V& viewDir, float n1, float n2)
{
	// Adapted in part from http://www.cs.utah.edu/~shirley/books/fcg2/rt.pdf
	V H        = glm::normalize(lightDir + viewDir);
	float R0   = powf((n1 - n2) / (n2 + n1), 2.0f);
	float cosI = glm::dot(viewDir, H);

	return R0 + ((1.0f - R0) * powf(max(0.0f, 1.0f - cosI), 5.0f));
}

/******************************************************************************
 *
 * Applies diffuse + specular shading
 *
 *****************************************************************************/

glm::vec3 blinnPhongShade(TraceOptions& options
	                     ,const Intersection& isect
			   			 ,const V& I
						 ,Light const * light
						 ,Color& ambient
						 ,Color& diffuse
						 ,Color& specular
						 ,bool isDebugPixel)
{
	// Coefficients
	float ka = 0.15f; // ambient
	float kd = 0.95f; // diffuse
	float ks = 1.0f;  // specular

	Material const * mat      = isect.node->getMaterial();
	Geometry const * geometry = isect.node->getGeometry();

	assert(mat != NULL && geometry != NULL);

	// Adjust the height of the normal vector by multiplying it with the
	// intensity value of the bump map
	V N = isect.normal;

	// Choose the UV mapping vector. If one is given in the intersection, use it,
	// otherwise use the local hit
	glm::vec3 uv = isect.hasUV ? isect.uv : glm::normalize(isect.hitLocal.xyz);

	if (mat->hasBumpMap()) {

		V B = mat->getNormal(uv, geometry);

		#ifdef ENABLE_PIXEL_DEBUG
		if (options.enablePixelDebug && isDebugPixel) {
			debugPixel(__FUNCTION_NAME__ "/debug:has-bump-map", -99, B);
		}
		#endif

		N = glm::normalize(N + B);
	}

	// Get the color at the hit position:
	Color matColor = mat->getColor(uv, geometry);

	// Set the base ambient color component:
	ambient = mat->getAmbientCoeff() * matColor;

	// If the material is assigned to an object that acts as a light source
	// (as indicated by its "emissiveness", then there's no further work to do
	if (mat->isEmissive()) {

		diffuse  = matColor;
		specular = matColor;

		return N; // The surface normal
	}

	V L        = glm::normalize(light->fromCenter(isect.hitWorld));
	V R        = glm::reflect(L, N);
	Color lcol = light->getColor(isect.hitWorld);

	// === Diffuse component ==================================================

	float cosineAngle = glm::dot(L, N);

	#ifdef ENABLE_PIXEL_DEBUG
	if (options.enablePixelDebug && isDebugPixel) {
		debugPixel(__FUNCTION_NAME__ "/debug:diffuse:cosine", -99, cosineAngle);
	}
	#endif

	//float Id = max(0.0f, cosineAngle);
	float Id = cosineAngle;
	diffuse += kd * Id * matColor * lcol;

	// === Specular component =================================================
	if (mat->getSpecularExponent() > 0.0f) {
		float Is = glm::dot(I, R);
		if (Is > 0.0f) {
			specular += ks * powf(Is, mat->getSpecularExponent()) * lcol;
		}
	}

	return N; // The surface normal
}

/******************************************************************************
 *
 * Computes surface shading
 *
 *****************************************************************************/

static Color computeShading(TraceOptions& options
	                       ,const Intersection& isect
	                       ,const Graph& graph
						   ,EnvironmentMap const * envMap
						   ,const Ray& ray
						   ,const list<Light*>& lights
						   ,int depth
						   ,bool isDebugPixel = false)
{
	GraphNode* const selfNode = isect.node;
	assert(selfNode != NULL);

	Material const * mat = selfNode->getMaterial();
	assert(mat != NULL);

	/**************************************************************************
	 * Compute Blinn-Phong shading
	 *************************************************************************/

	float R  = 0.0f; // Fresnel reflection coefficient

	Color ambient, diffuse, specular, reflected, refracted;

	// Index of refraction coefficients:
	float n1, n2;

	if (isect.inside) {
		n1 = mat->getIndexOfRefraction();
		n2 = 1.0f;
	} else {
		n1 = 1.0f;
		n2 = mat->getIndexOfRefraction();
	}

	// Schlick Fresnel term approximation:	
	float fresnelTerm = 0.0f;
	float n           = n1 / n2;

	// Incident ray:
	V I = glm::normalize(ray.dir);
	V N = glm::vec3();

	// For each light:
	for (list<Light*>::const_iterator i=lights.begin(); i != lights.end(); i++) {

		N += blinnPhongShade(options, isect, I, *i, ambient, diffuse, specular, isDebugPixel);

		// Compute the Blinn-Phong diffuse and specular components for the current light
		// if not in the shadow:
		float amount = shadow(graph, isect.node, isect.hitWorld, *i, options.samplesPerLight);

		// Apply the shading factor to the diffuse + specular components
		diffuse  *= amount;
		specular *= amount;

		// For each light, compute the accumulated the Schlick approximation for  the Fresnel term:
		if (mat->isTransparent() && mat->isMirror()) {
			V L          = glm::normalize((*i)->fromCenter(isect.hitWorld));
			fresnelTerm += reflectCoeff(L, I, n1, n2);
		}
	}

	/**************************************************************************
	 * Reflection & refraction
	 *************************************************************************/
	if (mat->isTransparent()) {

		refracted = traceRefract(options, graph, envMap, isect, I, N, n, lights, depth, isDebugPixel);
	
		#ifdef ENABLE_PIXEL_DEBUG
		if (options.enablePixelDebug && isDebugPixel) {
			debugPixel(__FUNCTION_NAME__ "/debug:trace-refract", depth, refracted);
		}
		#endif
	}

	if (mat->isMirror()) {

		reflected = traceReflect(options, graph, envMap, isect, I, N, lights, depth, isDebugPixel);

		#ifdef ENABLE_PIXEL_DEBUG
		if (options.enablePixelDebug && isDebugPixel) {
			debugPixel(__FUNCTION_NAME__ "/debug:trace-reflect", depth, reflected);
		}
		#endif
	}

	/**************************************************************************
	 * Output
	 *************************************************************************/
	if (mat->isTransparent() && mat->isMirror()) {

		#ifdef ENABLE_PIXEL_DEBUG
		if (options.enablePixelDebug && isDebugPixel) {
			debugPixel(__FUNCTION_NAME__ "/debug:transparent+mirror", depth, refracted);
		}
		#endif

		fresnelTerm = Utils::unitClamp(fresnelTerm);

		return ((1.0f - fresnelTerm) * (refracted + specular) + fresnelTerm * (reflected + specular));
	}
	
	if (mat->isTransparent()) {

		#ifdef ENABLE_PIXEL_DEBUG
		if (options.enablePixelDebug && isDebugPixel) {
			debugPixel(__FUNCTION_NAME__ "/debug:transparent-only", depth, refracted + specular);
		}
		#endif

		return refracted + specular;
	}
	
	if (mat->isMirror()) {

		#ifdef ENABLE_PIXEL_DEBUG
		if (options.enablePixelDebug && isDebugPixel) {
			debugPixel(__FUNCTION_NAME__ "/debug:mirror-only", depth, reflected + specular);
		}
		#endif

		return reflected + specular;
	}
	
	// Otherwise, assume diffuse only

	#ifdef ENABLE_PIXEL_DEBUG
	if (options.enablePixelDebug && isDebugPixel) {
		debugPixel(__FUNCTION_NAME__ "/debug:ambient", depth, ambient);
		debugPixel(__FUNCTION_NAME__ "/debug:diffuse", depth, diffuse);
		debugPixel(__FUNCTION_NAME__ "/debug:specular", depth, specular);
		debugPixel(__FUNCTION_NAME__ "/debug:ambient+diffuse+specular", depth, ambient + diffuse + specular);
	}
	#endif

	return ambient + diffuse + specular;
}

/******************************************************************************
 *
 * Traces a ray for the given pixel (i,j), returning the color
 *
 *****************************************************************************/

static Color trace(TraceOptions& options
	              ,const Ray& ray
		          ,const Graph& graph
				  ,EnvironmentMap const * envMap
		          ,const list<Light*>& lights
		          ,int depth
				  ,bool isDebugPixel)
{
	if (depth > MAX_DEPTH) {

		#ifdef ENABLE_PIXEL_DEBUG
		if (options.enablePixelDebug && isDebugPixel) {
			debugPixel(__FUNCTION_NAME__ "/debug:MAX-DEPTH", depth, Color::DEBUG);
		}
		#endif

		return envMap != NULL ? envMap->getColorFromRay(ray) : Color::BLACK;
	}

	bool hit         = false;
	TraceContext ctx = closestIntersection(ray, graph, hit);

	#ifdef ENABLE_PIXEL_DEBUG
	Color output = Color::DEBUG;
	#else
	Color output = Color::BLACK;
	#endif

	if (hit) {
		output = computeShading(options
			                   ,ctx.closestIsect
			                   ,graph
							   ,envMap
							   ,ray
							   ,lights
							   ,depth
							   ,isDebugPixel);

		#ifdef ENABLE_PIXEL_DEBUG
		if (options.enablePixelDebug && isDebugPixel) {
			debugPixel(__FUNCTION_NAME__ "/debug:trace:post-hit", depth, output);
		}
		#endif
	
	} else {

		// Hit against the environment map (if it is given):
		if (envMap != NULL) {
			output = envMap->getColorFromRay(ray);
		}

		// Otherwise return black
	}

	return output;
}

/******************************************************************************
 *
 * Samples a pixel with N rays, producing an averaged Color value
 *
 *****************************************************************************/

static Color samplePixel(TraceOptions& options
	                    ,const Camera& camera
			            ,const Graph& graph
						,EnvironmentMap const * envMap
				        ,const list<Light*>& lights
	                    ,float pixelW
	                    ,float pixelH
						,float screenW
						,float screenH
		 		        ,int i
				        ,int j
						,bool isDebugPixel = false)
{
	// Sample by an N by N grid:
	int N  = options.samplesPerPixel;
	int N2 = N * N;
	Color C;

	// Find the offsets:
	float X  = pixelW * static_cast<float>(i);
	float Y  = pixelH * static_cast<float>(j);
	float dx = pixelW / static_cast<float>(N2);
	float dy = pixelH / static_cast<float>(N2);

	// Otherwise, average red, green, and blue components:
	float avgR = C.fR();
	float avgG = C.fG();
	float avgB = C.fB();

	float u, v, xNDC, yNDC;
	float K = static_cast<float>(N2);

	// For each sub-pixel sampling point (N * N):
	for (int k=0; k<N2; k++) {

		// Find the sampling point:
		u = static_cast<float>(k / N2);
		v = static_cast<float>(k % N2);

		// Find the jittered (x,y) sampling point coordinate in NDC space:
		xNDC = X + (u * dx) + (Utils::unitRand() * 0.9f * dx);
		yNDC = Y + (v * dy) + (Utils::unitRand() * 0.9f * dy);

		// Now, shoot a ray per sub-pixel sampling point:
		C = trace(options, camera.spawnRay(xNDC, yNDC), graph, envMap, lights, 0, false);

		// Average the colors component-by-component to get around
		// the saturation limit imposed by clamping:
		avgR += C.fR();
		avgG += C.fG();
		avgB += C.fB();
	}

	return Color(avgR / K, avgG / K, avgB / K);
}


/******************************************************************************
 *
 * Detects the edges in the given bitmap using the Sobel operator, producing 
 * an edge intensity map. This map is used to selectively determine where to 
 * apply antialiasing
 *
 ******************************************************************************/

float luminosity(const RGBApixel& pixel)
{
	return (0.212655f * (static_cast<float>(pixel.Red) / 255.0f)) + 
	       (0.715158f * (static_cast<float>(pixel.Green) / 255.0f)) + 
	       (0.072187f * (static_cast<float>(pixel.Blue) / 255.0f));
}

// Returns the average edge intensity
float detectEdges(const BMP& input, int w, int h, float* E)
{	
	// Sobel filter in X:
	float Gx[3][3] = {{-1.0f, 0.0f, 1.0f}
	                 ,{-2.0f, 0.0f, 2.0f}
	                 ,{-1.0f, 0.0f, 1.0f}};
	// Sobel filter in Y:
	float Gy[3][3] = {{-1.0f, -2.0f, -1.0f}
	                 ,{ 0.0f,  0.0f,  0.0f}
	                 ,{ 1.0f,  2.0f,  1.0f}};

	float avgIntensity = 0.0f;

	#pragma omp parallel for
	for (int i=1; i<(w-1); i++) {
		for (int j=1; j<(h-1); j++) {

			// Gradient values in the X and Y directions at (i,j)
			float X = 0.0f;
			float Y = 0.0f;

			for (int u=0; u<3; u++) {
				for (int v=0; v<3; v++) {

					int ii = i + (u - 1);
					int jj = j + (v - 1);
					assert(ii >= 0 && ii < w);
					assert(jj >= 0 && jj < h);

					float L = luminosity(input.GetPixel(ii, jj));
					X      += (L * Gx[u][v]);
					Y      += (L * Gy[u][v]);
				}
			}

			// Compute the gradient magnitude and clamp to the range [0,1]:
			float magnitude = min(max(0.0f, sqrt(powf(X, 2.0f) + powf(Y, 2.0f))), 1.0f); 
			E[(i * h) + j]  = magnitude;

			avgIntensity += magnitude;
		}
	}

	avgIntensity /= static_cast<float>(w * h);
	return avgIntensity;
}

/**
 * Converts a Color instance to an EasyBMP RGBApixel value
 */
static inline RGBApixel colorToRGBAPixel(const Color& color)
{
	RGBApixel pixel;
	pixel.Red   = color.iR();
	pixel.Green = color.iG();
	pixel.Blue  = color.iB();
	return pixel;
}

/******************************************************************************
 *
 * Raytraces the entire scene
 *
 ******************************************************************************/

void rayTrace(BMP& output
	         ,const WorldState& state
	         ,const Camera& C
			 ,const Graph& G // Scene graph
			 ,int X          // X resolution
			 ,int Y          // Y resolution
			 ,TraceOptions& options)
{
	unsigned int line = 0;
	chrono::time_point<chrono::system_clock> start, end;
	chrono::duration<double> elapsed_sec_1, elapsed_sec_2;

	// Get all of the point lights, etc. defined in the scene configuration:
	list<Light*> lights = state.getLights();

	// Collect all objects that constitute emissive objects and merge them
	// with the existing light list:
	G.addAreaLights(&lights);

	// Function timing code adapted http://stackoverflow.com/a/459704
	clock_t pass1Start = clock();

	// Compute the width and height of a single pixel
	float pixW = 0.0f;
	float pixH = 1.0f;
	Camera::pixelDimensions(X, Y, pixW, pixH);

	// Use an edge map to adaptively find where to perform supersampling:
	float* edgeMap = NULL;
	
	// Only initialize the edge map if options.samplesPerPixel > 1

	if (options.samplesPerPixel > 1) {
		edgeMap = new float[X * Y];
		memset(&edgeMap[0], 0, sizeof(edgeMap[0]) * X * Y);
	}

	// Environment map:
	EnvironmentMap const * em = state.getEnvironmentMap();

	float fX = static_cast<float>(X);
	float fY = static_cast<float>(Y);
	
	// Dump the trace options:
	cout << "> Rendering with configuration: " << endl 
		 << endl << options << endl;

	start = chrono::system_clock::now();

	#pragma omp parallel
	{
		#pragma omp for schedule(static)
		for (int i=0; i<X; i++) {

			for (int j=0; j<Y; j++) {

				#ifdef ENABLE_PIXEL_DEBUG
				bool hitDebugPixel =    options.enablePixelDebug 
					                 && options.xDebugPixel == i 
									 && options.yDebugPixel == j;
				#else
				bool hitDebugPixel = false;
				#endif

				// Shoot a single ray through the center of each pixel
				float xNDC = static_cast<float>(i) / fX;
				float yNDC = static_cast<float>(j) / fY;

				Color c = trace(options, C.spawnRay(xNDC, yNDC), G, em, lights, 0, hitDebugPixel);

				#ifdef ENABLE_PIXEL_DEBUG
				// If we hit the debug pixel: break out, since there's nothing more to do
				if (options.enablePixelDebug && hitDebugPixel) {
					debugPixel(__FUNCTION_NAME__ ":done", 0, c);
					// It's still abnormal termination:
					exit(EXIT_FAILURE);
				}
				#endif

				output.SetPixel(i, j, colorToRGBAPixel(c));
			}

			++line;

			cout << "(PASS-1) " << ((static_cast<float>(line) / fY) * 100.0f) << "%\r";
		}
	}

	elapsed_sec_1 = chrono::system_clock::now() - start;

	cout << endl 
	     << endl 
		 << "> Rendering elapsed time: "
		 << elapsed_sec_1.count() << "s" 
		 << endl 
		 << endl;

	/// Adaptively antialias /////////////////////////////////////////////////

	if (options.samplesPerPixel > 1) {

		line = 0;

		// Detect the edges of the image:
		float avgIntensity = detectEdges(output, X, Y, edgeMap);
		int N = options.samplesPerPixel;

		cout << "> Adaptively supersampling with " << N << " x " << N 
		     << " samples per pixel" 
			 << endl << endl;

		start = chrono::system_clock::now();

		#pragma omp parallel
		{
			#pragma omp for schedule(static)
			for (int i=0; i<X; i++) {
				for (int j=0; j<Y; j++) {

					// Linearize the index:
					int k = (i * Y) + j;

					// For any pixel at (i,j) that has an edge intensity greater than
					// the average value, run antialiasing:
					if (edgeMap[k] > avgIntensity) {

						Color c = samplePixel(options, C, G, em, lights, pixW, pixH, fX, fY, i ,j);

						// Overwrite the value previously stored at (i,j) with the 
						// supersampled color value:
						output.SetPixel(i, j, colorToRGBAPixel(c));
					}
				}

				++line;

				cout << "(PASS-2) " << ((static_cast<float>(line) / fY) * 100.0f) << "%\r";
			}
		}

		if (edgeMap != NULL) {
			delete [] edgeMap;
			edgeMap = NULL;
		}

		elapsed_sec_2 = chrono::system_clock::now() - start;

		cout << endl 
		     << endl 
			 << "> Supersampling elapsed time: " << elapsed_sec_2.count() << "s"  
			 << endl
			 << "> Total elapsed time: " << (elapsed_sec_1 + elapsed_sec_2).count() << "s"  
			 << endl
			 << endl;
	}
}

/******************************************************************************/
