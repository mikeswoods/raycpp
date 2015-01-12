/*******************************************************************************
 *
 * This file defines the core of the raytracer implementation
 *
 * @file Raytrace.h
 * @author Michael Woods
 *
 ******************************************************************************/

#define GLM_FORCE_RADIANS
#include <algorithm>
#include <ctime>
#include <chrono>
#include <cassert>
#include <omp.h>
#include <cstdlib>
#include <iostream>
#include <stack>
#include "Image.h"
#include "Raytrace.h"
#include "Intersection.h"
#include "EnvironmentMap.h"
#include "AreaLight.h"

using namespace std;
using namespace glm;
using namespace cimg_library;

/*******************************************************************************
 *
 * Macro definitions
 *
 ******************************************************************************/

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

/*******************************************************************************
 *
 * Foward declarations
 *
 ******************************************************************************/

static Color trace(const Ray&, shared_ptr<SceneContext>, shared_ptr<TraceOptions>, int depth, bool isDebugPixel);

/*******************************************************************************
 *
 * Output operations
 *
 ******************************************************************************/

ostream& operator<<(ostream& s, const Intersection& isect)
{
    s << "Intersection {"                 << endl
      << "  t="         << isect.t        << endl
      << "  node="      << isect.node     << endl
      << "  inside="    << isect.inside   << endl
      << "  hitWorld="  << isect.hitWorld << endl
      << "  hitLocal="  << isect.hitLocal << endl
      << "  normal="    << isect.normal   << endl
      << "}"                              << endl;
    return s;
}

ostream& operator<<(ostream& s, const TraceContext& ctx)
{
    s << "TraceContext {"                      << endl
      << "  ray="          << ctx.ray          << endl
      << "  closestIsect=" << ctx.closestIsect << endl
      << "}"                                   << endl;
    return s;
}

ostream& operator<<(ostream& s, const TraceOptions& opts)
{
	s << "[samplesPerLight: " << opts.samplesPerLight <<
		 ", samplesPerPixel: " << opts.samplesPerPixel << 
		 ", enablePixelDebug: " << (opts.enablePixelDebug ? "yes" : "no") <<
		 "]" << endl;
    return s;
}

/*******************************************************************************
 *
 * Debug a given pixel
 *
 ******************************************************************************/

static ostream& debugPixel(ostream& os, string funcName, int depth)
{
    for (int i=0; i<depth; i++) {
        os << "    ";
    }
    os << funcName << "<" << depth << ">: ";
    return os;
}

static ostream& debugPixel(string funcName, int depth, const Color& output)
{
    return debugPixel(cerr, funcName, depth) << output << endl;
}

static ostream& debugPixel(string funcName, int depth, const vec3& output)
{
    return debugPixel(cerr, funcName, depth) << output << endl;
}

static ostream& debugPixel(string funcName, int depth, float output)
{
    return debugPixel(cerr, funcName, depth) << output << endl;
}

static ostream& debugPixel(string funcName, int depth, int output)
{
    return debugPixel(cerr, funcName, depth) << output << endl;
}

static ostream& debugPixel(string funcName, int depth, string output)
{
    return debugPixel(cerr, funcName, depth) << output << endl;
}

/*******************************************************************************
 *
 * Initializes the raytracer camera
 *
 ******************************************************************************/

void initRaytrace(Camera& camera, shared_ptr<SceneContext> scene)
{
    camera.setPosition(scene->getEyePosition());
    camera.setViewDir(scene->getViewDir());
    camera.setUp(scene->getUpDir());
    // Divide this by 2 to match the FOV produced by perspective(). 
    camera.setFOV(scene->getFOVAngle() / 2.0f);
    camera.setAspectRatio(scene->getAspectRatio());
}

/******************************************************************************/

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
static TraceContext intersectNode(shared_ptr<GraphNode> node, TraceContext ctx)
{
    shared_ptr<Geometry> geometry = node->getGeometry();

    auto nextT = applyTransform(node, ctx.T);
    auto isect = geometry 
        ? geometry->intersect(nextT, ctx.ray, ctx.scene) 
        : Intersection::miss();

    if (isect.isHit()) {
        isect.node = node;
    }

    return TraceContext(ctx.scene, ctx.ray, nextT, isect);
}

/**
 * Returns the TraceContext instance with the closest (smallest >= 0) t value.
 * Used by closestIntersection
 */
static TraceContext findClosestContextNode(TraceContext currentCtx, TraceContext lastCtx)
{
    return isCloser(currentCtx, lastCtx) ? currentCtx : lastCtx;
}

/*******************************************************************************
 *
 * Given a ray, this function computes the closest intersect in a scene graph
 *
 ******************************************************************************/

static TraceContext closestIntersection(const Ray& ray
                                       ,shared_ptr<SceneContext> scene
                                       ,bool& hit)
{
    // Identity matrix as the initial transformation:
    auto I        = mat4(); 
    auto initCtx  = TraceContext(scene, ray, I);
    auto finalCtx = fold(scene->getSceneGraph()
                        ,intersectNode
                        ,findClosestContextNode
                        ,initCtx);

    hit = finalCtx.closestIsect.isHit();

    return finalCtx;
}

/*******************************************************************************
 *
 * Faster method to determine if something is hit. As soon as an intersection
 * occurs, the loop exits and returns the first intersection
 *
 ******************************************************************************/

static bool fastTestInShadow(const Ray& ray
                            ,shared_ptr<SceneContext> scene
                            ,shared_ptr<GraphNode> ignore
                            ,float withinDist)
{
    auto graph = scene->getSceneGraph();

    // Initial transformation matrix is the identity matrix:
    auto I = mat4();

    stack<pair<shared_ptr<GraphNode>, mat4>> S;

    S.push(make_pair(graph.getRoot(), I));

    while (!S.empty()) {

        pair<shared_ptr<GraphNode>, mat4> nodeAndT = S.top();
        S.pop();

        shared_ptr<GraphNode> node = nodeAndT.first;
        mat4 T     = nodeAndT.second;

        shared_ptr<Geometry> geometry = node->getGeometry();
        mat4 nextT = applyTransform(node, T);

        if (node != ignore && geometry) {

            Intersection isect = geometry->intersect(nextT, ray, scene);
            isect.node = node;

            if (isect.isHit() && !node->isAreaLight() && isect.t < withinDist) {

                return true; // We're done
            }
        }

        list<shared_ptr<GraphNode>> children = node->getChildren();

        // Otherwise, go through the children:
        for (auto i=children.begin(); i != children.end(); i++) {
            S.push(make_pair(*i, nextT));
        }
    }

    return false;
}

/*******************************************************************************
 *
 * Tests if the given point is in the shadow of another object
 *
 ******************************************************************************/

static bool isOccludedFromPosition(shared_ptr<SceneContext> scene
                                  ,shared_ptr<GraphNode> selfNode
                                  ,const P& hitAt
                                  ,shared_ptr<Light> light)
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

    Ray ray(hitAt, normalize(L), Utils::EPSILON, Ray::SHADOW);

    return fastTestInShadow(ray, scene, selfNode, length(L));
};

/*******************************************************************************
 *
 * Given a hit position, a light source, and a number of samples, this function
 * tests if the position is considered to be in a shadow, returning the shade
 * factor apply to the coloring at the position in the range [0,1].
 *
 * A shade factor of 0 indicates the object is fully occluded (in shadow), 
 * while a value of 1 indicates it is not shadowed at all.
 *
 ******************************************************************************/

static float shadow(shared_ptr<SceneContext> scene
                    ,shared_ptr<GraphNode> selfNode
                    ,const P& hitAt
                    ,shared_ptr<Light> light
                    ,int samples)
{
    // If the self node being tested is the light itself, bail immediately:
    if (light->isLightSourceNode(selfNode)) {
        return 1.0f;
    }

    // Point lights only need 1 sample, 
    if (light->getLightType() == Light::POINT_LIGHT) {
        return isOccludedFromPosition(scene, selfNode, hitAt, light) 
            ? 0.0f 
            : 1.0f;
    }

    float contribution = 1.0f / static_cast<float>(samples);
    float shadeFactor  = 1.0f;

    for (int i=0; i<samples; i++) {
        if (isOccludedFromPosition(scene, selfNode, hitAt, light)) {
            shadeFactor -= contribution;
        }
    }

    return shadeFactor;
};

/*******************************************************************************
 *
 * Compute the color contribution from the reflected ray
 *
 ******************************************************************************/

static Color traceReflect(shared_ptr<SceneContext> scene
                         ,shared_ptr<TraceOptions> opts
                         ,const Intersection& isect
                         ,const V& I
                         ,const V& N
                         ,int depth
                         ,bool isDebugPixel = false)
{
    shared_ptr<Material> mat = isect.node->getMaterial();
    assert(!!mat);

    V R = reflect(I, N);

    #ifdef ENABLE_PIXEL_DEBUG
    if (opts->enablePixelDebug && isDebugPixel) {
        debugPixel(__FUNCTION_NAME__ "/debug:traceReflect", depth, R);
    }
    #endif

    Ray ray(isect.hitWorld, R, Utils::EPSILON, Ray::REFLECTION);

    return mat->getReflectColor() * 
           trace(ray, scene, opts, depth + 1, isDebugPixel);
}

/*******************************************************************************
 *
 * Compute the color contribution from the refracted ray
 *
 ******************************************************************************/

static Color traceRefract(shared_ptr<SceneContext> scene
                         ,shared_ptr<TraceOptions> opts
                         ,const Intersection& isect
                         ,const V& I
                         ,const V& N
                         ,float n
                         ,int depth
                         ,bool isDebugPixel)
{
    V R = refract(I, N, n);

    // Is R a zero vector? If so, reflect instead:
    if (R == vec3(0, 0, 0)) {

        return traceReflect(scene, opts, isect, I, N, depth, isDebugPixel);
    }

    #ifdef ENABLE_PIXEL_DEBUG
    if (opts->enablePixelDebug && isDebugPixel) {
        debugPixel(__FUNCTION_NAME__ "/debug:I=", depth, I);
        debugPixel(__FUNCTION_NAME__ "/debug:N=", depth, N);
        debugPixel(__FUNCTION_NAME__ "/debug:R=", depth, R);
        debugPixel(__FUNCTION_NAME__ "/debug:n=", depth, n);
    }
    #endif

    Ray ray(isect.hitWorld, R, Utils::EPSILON, Ray::REFRACTION);

    return trace(ray, scene, opts, depth + 1, isDebugPixel);
}

/*******************************************************************************
 *
 * Computes Schlick's approximation of the Fresnel term
 * http://en.wikipedia.org/wiki/Schlick%27s_approximation
 *
 ******************************************************************************/

float reflectCoeff(const V& lightDir, const V& viewDir, float n1, float n2)
{
    // Adapted in part from http://www.cs.utah.edu/~shirley/books/fcg2/rt.pdf
    V H        = normalize(lightDir + viewDir);
    float R0   = powf((n1 - n2) / (n2 + n1), 2.0f);
    float cosI = dot(viewDir, H);

    return R0 + ((1.0f - R0) * powf(std::max(0.0f, 1.0f - cosI), 5.0f));
}

/*******************************************************************************
 *
 * Applies diffuse + specular shading
 *
 ******************************************************************************/

static vec3 blinnPhongShade(shared_ptr<TraceOptions> opts
                                ,const Intersection& isect
                                ,const V& I
                                ,shared_ptr<Light> light
                                ,Color& ambient
                                ,Color& diffuse
                                ,Color& specular
                                ,bool isDebugPixel)
{
    // Coefficients
    float ka = 0.15f; // ambient
    float kd = 0.95f; // diffuse
    float ks = 1.0f;  // specular

    shared_ptr<Material> mat      = isect.node->getMaterial();
    shared_ptr<Geometry> geometry = isect.node->getGeometry();

    assert(mat && geometry);

    // Adjust the height of the normal vector by multiplying it with the
    // intensity value of the bump map
    V N = isect.normal;

    // Choose the UV mapping vector. If one is given in the intersection, use it,
    // otherwise use the local hit
    vec3 uvFromHit = normalize(isect.hitLocal.xyz);

    if (mat->hasBumpMap()) {

        V B = mat->getNormal(uvFromHit, geometry);

        #ifdef ENABLE_PIXEL_DEBUG
        if (opts->enablePixelDebug && isDebugPixel) {
            debugPixel(__FUNCTION_NAME__ "/debug:has-bump-map", -99, B);
        }
        #endif

        N = normalize(N + B);
    }

    // Get the color at the hit position:
    Color matColor = mat->getColor(uvFromHit, geometry);

    // Set the base ambient color component:
    ambient = (mat->getAmbientCoeff() < 0.0f ? ka : mat->getAmbientCoeff()) * matColor;

    // If the material is assigned to an object that acts as a light source
    // (as indicated by its "emissiveness", then there's no further work to do
    if (mat->isEmissive()) {

        diffuse  = matColor;
        specular = matColor;

        return N; // The surface normal
    }

    V L        = normalize(light->fromCenter(isect.hitWorld));
    V R        = reflect(L, N);
    Color lcol = light->getColor(isect.hitWorld);

    // Diffuse component:
    float cosineAngle = dot(L, N);

    #ifdef ENABLE_PIXEL_DEBUG
    if (opts->enablePixelDebug && isDebugPixel) {
        debugPixel(__FUNCTION_NAME__ "/debug:diffuse:cosine", -99, cosineAngle);
    }
    #endif

    float Id = std::max(0.0f, cosineAngle);
    diffuse += kd * Id * matColor * lcol;

    // Specular component:
    if (mat->getSpecularExponent() > 0.0f) {
        float Is = dot(I, R);
        if (Is > 0.0f) {
            specular += ks * powf(Is, mat->getSpecularExponent()) * lcol;
        }
    }

    return N; // The surface normal
}

/*******************************************************************************
 *
 * Computes surface shading
 *
 ******************************************************************************/

static Color computeShading(const Ray& ray
                           ,shared_ptr<SceneContext> scene
                           ,shared_ptr<TraceOptions> opts
                           ,const Intersection& isect
                           ,int depth
                           ,bool isDebugPixel = false)
{
    auto selfNode = isect.node;
    assert(!!selfNode);

    shared_ptr<Material> mat = selfNode->getMaterial();
    assert(!!mat);

    /***************************************************************************
     * Compute Blinn-Phong shading
     **************************************************************************/

    Color ambient, diffuse, specular, reflected, refracted, volumetric;

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
    V I = normalize(ray.dir);
    V N = vec3();

    auto lights = scene->getLights();

    // For each light:
    for (auto l=lights->begin(); l != lights->end(); l++) {

        N += blinnPhongShade(opts, isect, I, *l, ambient, diffuse, specular, isDebugPixel);

        // Compute the Blinn-Phong diffuse and specular components for the current light
        // if not in the shadow:
        float amount = shadow(scene, isect.node, isect.hitWorld, *l, opts->samplesPerLight);
        //float amount = 1.0f;

        // Apply the shading factor to the diffuse + specular components
        diffuse  *= amount;
        specular *= amount;

        // For each light, compute the accumulated the Schlick approximation for  the Fresnel term:
        if (mat->isTransparent() && mat->isMirror()) {
            V L          = normalize((*l)->fromCenter(isect.hitWorld));
            fresnelTerm += reflectCoeff(L, I, n1, n2);
        }
    }

    /***************************************************************************
     * Reflection & refraction
     **************************************************************************/

    if (isect.density < 1.0f) {
        volumetric = traceRefract(scene, opts, isect, I, N, 1.0f, depth, isDebugPixel);
    }

    if (mat->isTransparent()) {

        refracted = traceRefract(scene, opts, isect, I, N, n, depth, isDebugPixel);

        #ifdef ENABLE_PIXEL_DEBUG
        if (opts->enablePixelDebug && isDebugPixel) {
            debugPixel(__FUNCTION_NAME__ "/debug:trace-refract", depth, refracted);
        }
        #endif
    }

    if (mat->isMirror()) {

        reflected = traceReflect(scene, opts, isect, I, N, depth, isDebugPixel);

        #ifdef ENABLE_PIXEL_DEBUG
        if (opts->enablePixelDebug && isDebugPixel) {
            debugPixel(__FUNCTION_NAME__ "/debug:trace-reflect", depth, reflected);
        }
        #endif
    }

    /***************************************************************************
     * Output
     **************************************************************************/
    if (mat->isTransparent() && mat->isMirror()) {

        #ifdef ENABLE_PIXEL_DEBUG
        if (opts->enablePixelDebug && isDebugPixel) {
            debugPixel(__FUNCTION_NAME__ "/debug:transparent+mirror", depth, refracted);
        }
        #endif

        fresnelTerm = Utils::unitClamp(fresnelTerm);

        return ((1.0f - fresnelTerm) * (refracted + specular) + fresnelTerm * (reflected + specular));
    }
    
    if (mat->isTransparent()) {

        #ifdef ENABLE_PIXEL_DEBUG
        if (opts->enablePixelDebug && isDebugPixel) {
            debugPixel(__FUNCTION_NAME__ "/debug:transparent-only", depth, refracted + specular);
        }
        #endif

        return refracted + specular;
    }
    
    if (mat->isMirror()) {

        #ifdef ENABLE_PIXEL_DEBUG
        if (opts->enablePixelDebug && isDebugPixel) {
            debugPixel(__FUNCTION_NAME__ "/debug:mirror-only", depth, reflected + specular);
        }
        #endif

        return reflected + specular;
    }
    
    // Otherwise, assume diffuse only

    #ifdef ENABLE_PIXEL_DEBUG
    if (opts->enablePixelDebug && isDebugPixel) {
        debugPixel(__FUNCTION_NAME__ "/debug:ambient", depth, ambient);
        debugPixel(__FUNCTION_NAME__ "/debug:diffuse", depth, diffuse);
        debugPixel(__FUNCTION_NAME__ "/debug:specular", depth, specular);
        debugPixel(__FUNCTION_NAME__ "/debug:ambient+diffuse+specular", depth, ambient + diffuse + specular);
    }
    #endif

    return ambient + diffuse + specular;
    //return ((ambient + diffuse + specular) * isect.density) + (volumetric * (1.0f - isect.density));
}

/*******************************************************************************
 *
 * Traces a ray for the given pixel (i,j), returning the color
 *
 ******************************************************************************/

static Color trace(const Ray& ray
                  ,shared_ptr<SceneContext> scene
                  ,shared_ptr<TraceOptions> opts
                  ,int depth
                  ,bool isDebugPixel)
{
    auto envMap = scene->getEnvironmentMap();

    if (depth > MAX_DEPTH) {

        #ifdef ENABLE_PIXEL_DEBUG
        if (opts->enablePixelDebug && isDebugPixel) {
            debugPixel(__FUNCTION_NAME__ "/debug:MAX-DEPTH", depth, Color::DEBUG);
        }
        #endif

        return envMap->getColor(ray, scene);
        //return Color::DEBUG;
    }

    bool hit         = false;
    TraceContext ctx = closestIntersection(ray, scene, hit);

    #ifdef ENABLE_PIXEL_DEBUG
    Color output = Color::DEBUG;
    #else
    Color output = Color::BLACK;
    #endif

    if (hit) {

        output = computeShading(ray, scene, opts, ctx.closestIsect, depth, isDebugPixel);

        #ifdef ENABLE_PIXEL_DEBUG
        if (opts->enablePixelDebug && isDebugPixel) {
            debugPixel(__FUNCTION_NAME__ "/debug:trace:post-hit", depth, output);
        }
        #endif
    
    } else {

        output = envMap->getColor(ray, scene);
        //output = Color::DEBUG;
    }

    return output;
}

/*******************************************************************************
 *
 * Samples a pixel with N rays, producing an averaged Color value
 *
 ******************************************************************************/

static Color samplePixel(const Camera& camera
                        ,shared_ptr<SceneContext> scene
                        ,shared_ptr<TraceOptions> opts
                        ,float pixelW
                        ,float pixelH
                        ,float screenW
                        ,float screenH
                        ,int i
                        ,int j
                        ,bool isDebugPixel = false)
{
    // Sample by an N by N grid:
    int N  = opts->samplesPerPixel;
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
        C = trace(camera.spawnRay(xNDC, yNDC), scene, opts, 0, false);

        // Average the colors component-by-component to get around
        // the saturation limit imposed by clamping:
        avgR += C.fR();
        avgG += C.fG();
        avgB += C.fB();
    }

    return Color(avgR / K, avgG / K, avgB / K);
}

/*******************************************************************************
 *
 * Raytraces the entire scene
 *
 ******************************************************************************/

void rayTrace(shared_ptr<Image> output
             ,const Camera& C
             ,shared_ptr<SceneContext> scene
             ,shared_ptr<TraceOptions> opts)
{
    Graph G   = scene->getSceneGraph();
    vec2 reso = scene->getResolution();
    int X     = reso.x;
    int Y     = reso.y;

    unsigned int line = 0;
    chrono::time_point<chrono::system_clock> start, end;
    chrono::duration<double> elapsed_sec_1, elapsed_sec_2;

    // Get all of the point lights, etc. defined in the scene configuration:
    auto lights = scene->getLights();

    // Collect all objects that constitute emissive objects and merge them
    // with the existing light list:
    auto areaLights = G.areaLights();
    for (auto l=areaLights->begin(); l != areaLights->end(); l++) {
        lights->push_back(*l);
    }

    // Compute the width and height of a single pixel
    float pixW = 0.0f;
    float pixH = 1.0f;
    Camera::pixelDimensions(X, Y, pixW, pixH);

    // Environment map:
    auto envMap = scene->getEnvironmentMap();

    // If the given environment map is null, just use a simple color:
    if (!envMap) {
        auto blank = shared_ptr<EnvironmentMap>(make_shared<ColorEnvironmentMap>(Color::BLACK));
        scene->setEnvironmentMap(blank);
    }

    float fX = static_cast<float>(X);
    float fY = static_cast<float>(Y);
    
    // Dump the trace opts:
    cout << "> Rendering with configuration: " << endl 
         << endl 
         << *opts 
         << endl;

    start = chrono::system_clock::now();

    #ifdef ENABLE_OPENMP
    #pragma omp parallel
    #endif
    {
        #ifdef ENABLE_OPENMP
        #pragma omp for schedule(static)
        #endif
        for (int i=0; i<X; i++) {
            for (int j=0; j<Y; j++) {

                #ifdef ENABLE_PIXEL_DEBUG
                bool hitDebugPixel =    opts->enablePixelDebug 
                                     && opts->xDebugPixel == i 
                                     && opts->yDebugPixel == j;
                #else
                bool hitDebugPixel = false;
                #endif

                // Shoot a single ray through the center of each pixel
                float xNDC = static_cast<float>(i) / fX;
                float yNDC = static_cast<float>(j) / fY;

                Color c = trace(C.spawnRay(xNDC, yNDC), scene, opts, 0, hitDebugPixel);

                #ifdef ENABLE_PIXEL_DEBUG
                // If we hit the debug pixel: break out, since there's nothing more to do
                if (opts->enablePixelDebug && hitDebugPixel) {

                    debugPixel(__FUNCTION_NAME__ ":done", 0, c);
                    exit(EXIT_FAILURE);
                }
                #endif

                (*output)(i, j, 0, 0) = c.iR(); // Set red channel
                (*output)(i, j, 0, 1) = c.iG(); // Set green channel
                (*output)(i, j, 0, 2) = c.iB(); // Set blue channel
            }

            ++line;

            clog << "(PASS-1) " << ((static_cast<float>(line) / fY) * 100.0f) << "%\r";
        }
    }

    elapsed_sec_1 = chrono::system_clock::now() - start;

    cout << endl 
         << endl 
         << "> Rendering elapsed time: "
         << elapsed_sec_1.count() << "s" 
         << endl 
         << endl;

    // Adaptively antialias:
    if (opts->samplesPerPixel > 1) {

        line = 0;

        // Detect the edges of the image:
        float avgIntensity = 0.0f;
        auto edgeMap       = edges(*output, X, Y, avgIntensity);

        cout << "> Adaptively supersampling with " << opts->samplesPerPixel << " x " << opts->samplesPerPixel 
             << " samples per pixel" 
             << endl << endl;

        start = chrono::system_clock::now();

        #ifdef ENABLE_OPENMP
        #pragma omp parallel
        #endif
        {
            #ifdef ENABLE_OPENMP
            #pragma omp for schedule(static)
            #endif
            for (int i=0; i<X; i++) {
                for (int j=0; j<Y; j++) {

                    // Linearize the index:
                    int k = (i * Y) + j;

                    // For any pixel at (i,j) that has an edge intensity greater than
                    // the average value, run antialiasing:
                    if (edgeMap[k] > avgIntensity) {

                        Color c = samplePixel(C, scene, opts, pixW, pixH, fX, fY, i ,j);

                        // Overwrite the value previously stored at (i,j) with the 
                        // supersampled color value:
                        (*output)(i, j, 0, 0) = c.iR(); // Set red channel
                        (*output)(i, j, 0, 1) = c.iG(); // Set green channel
                        (*output)(i, j, 0, 2) = c.iB(); // Set blue channel
                    }
                }

                ++line;

                clog << "(PASS-2) " << ((static_cast<float>(line) / fY) * 100.0f) << "%\r";
            }
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
