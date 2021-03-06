/******************************************************************************
 *
 * A general camera class definition. This camera is used primary for 
 * raytracer-based rendering.
 *
 * @file Camera.h
 * @author Michael Woods
 *
 ******************************************************************************/

#define _USE_MATH_DEFINES
#include <cmath>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include "Ray.h"
#include "Camera.h"
#include "Utils.h"

using namespace std;

/******************************************************************************/

Camera::Camera(const glm::vec3 &position
              ,const glm::vec3 &viewDir
              ,const glm::vec3 &up
              ,float fov
              ,float aspectRatio)
{
    this->position    = position;
    this->viewDir     = viewDir;
    this->up          = glm::normalize(up);
    this->fov         = fov;
    this->phi         = 0.0f;
    this->theta       = 0.0f;
    this->aspectRatio = aspectRatio;

    this->calibrateViewPlane();
}

Camera::Camera(const glm::vec3& position
              ,const glm::vec3& lookAt
              ,float fov
              ,float aspectRatio)
{
    this->position    = position;
    this->viewDir     = lookAt - position;
    this->up          = glm::vec3(0.0f, 1.0f, 0.0f);
    this->fov         = fov;
    this->phi         = 0.0f;
    this->theta       = 0.0f;
    this->aspectRatio = aspectRatio;

    this->calibrateViewPlane();
}

Camera::Camera(const Camera &other)
{
    this->position   = other.position;
    this->viewDir    = other.viewDir;
    this->up         = other.up;
    this->fov        = other.fov;
    this->phi        = other.phi;
    this->theta      = other.theta;
    this->viewPlaneX = other.viewPlaneX;
    this->viewPlaneY = other.viewPlaneY;
    this->midpoint   = other.midpoint;
}

/**
 * Update the camera's position
 */
void Camera::setPosition(const glm::vec3& position)
{
    this->position = position;

    this->calibrateViewPlane();
}

/**
 * Update the camera's view direction
 */
void Camera::setViewDir(const glm::vec3& viewDir)
{
    this->viewDir = viewDir;

    this->calibrateViewPlane();
}

/**
 * Update the camera's "up" vector
 */
void Camera::setUp(const glm::vec3& up)
{
    this->up = up;

    this->calibrateViewPlane();
}

/** 
 * Updates the FOV Y value of the camera
 */
void Camera::setFOV(float fov)
{
    this->fov = fov;

    this->calibrateViewPlane();
}

/** 
 * Updates the camera's aspect ratio
 */
void Camera::setAspectRatio(float aspectRatio)
{
    this->aspectRatio = aspectRatio;

    this->calibrateViewPlane();
}

/** 
 * Convert an (u,v) coordinate in the range {[0,1],[0,1]} to a position 
 * in R3 space
 */
glm::vec3 Camera::ndc2World(float x, float y) const 
{
    // This is needed since (0,0) corresponds to the bottom-left corner.
    // This inverts (0,0) such that it is in the upper-left corner, since y
    // is int he range [0,1].
    float yPrime = 1.0f - y;
    return this->midpoint + (this->viewPlaneX * ((2.0f * x) - 1.0f))
                          + (this->viewPlaneY * ((2.0f * yPrime) - 1.0f));
}

/**
 * Convert an x,y coorindate in the range {[0,screenWidth),[0,screenHeight)} 
 * to a position in R3 space
 */
glm::vec3 Camera::screen2World(float x, float y, float resoX, float resoY) const
{
    return ndc2World(static_cast<float>(x) / static_cast<float>(resoX)
		            ,static_cast<float>(y) / static_cast<float>(resoY));
}

/** 
 * Spawns a ray from the camera's current position (eye) along the 
 * vector formed from the eye to the world point in NDC space given
 * by (x,y)
 */
Ray Camera::spawnRay(float x, float y) const
{
    return Ray(this->position, ndc2World(x,y) - this->position);
}

/**
 * Spawns a ray from the camera's current position (eye) along the 
 * vector formed from the eye to the world point in screen space given 
 * by (x,y) and (w,h)
 */
Ray Camera::spawnRay(float x, float y, float resoX, float resoY) const
{
    return Ray(this->position, screen2World(x, y, resoX, resoY) - this->position);
}

/**
 * Computes the width and height dimensions of the given pixel, setting 
 * width and height with the computed values
 */
void Camera::pixelDimensions(int resoX, int resoY, float& pw, float& ph)
{
    pw = 1.0f / static_cast<float>(resoX);
    ph = 1.0f / static_cast<float>(resoY);
}

/******************************************************************************/

ostream& operator<<(ostream& s, const Camera& c)
{
    return s                                << 
        "Camera {"                          << endl <<
        "  position    = <" << c.position.x << ", " << c.position.y << ", " << c.position.z << "> " << endl << 
        "  viewDir     = <" << c.viewDir.x << ", " << c.viewDir.y << ", " << c.viewDir.z << "> " << endl << 
        "  fov         = " << c.fov         << endl <<
        "  theta       = " << c.theta       << endl <<
        "  phi         = " << c.phi         << endl <<
        "  aspectRatio = " << c.aspectRatio << endl <<
        "  midpoint    = [" << c.midpoint.x << ", " << c.midpoint.y << ", " << c.midpoint.z << "]" << endl << 
        "}";
}

void Camera::calibrateViewPlane()
{
    // Basis vectors: 
    //   u = up 
    //   v = right
    //   w = forward
    this->w        = this->viewDir;
    this->u        = glm::cross(this->w,this->up);
    this->v        = glm::cross(this->u, this->w);
    this->midpoint = this->position + this->viewDir;

    float Ubar    = glm::length(this->u);
    float Vbar    = glm::length(this->v);
    float Wbar    = glm::length(this->w);

    this->phi   = tanf(this->fov * static_cast<float>(M_PI) / 180.0f);
    this->theta = this->phi * this->aspectRatio;
    
    this->viewPlaneX = (this->u * Wbar * this->theta) / Ubar;    
    this->viewPlaneY = (this->v * Wbar * this->phi) / Vbar;
}

/******************************************************************************/
