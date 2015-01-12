/******************************************************************************
 *
 * A general camera class definition. This camera is used primary for 
 * raytracer-based rendering.
 *
 * @file Camera.h
 * @author Michael Woods
 *
 ******************************************************************************/

#ifndef CAMERA_H
#define CAMERA_H

#include <iostream>
#include "R3.h"
#include "Ray.h"

/******************************************************************************/

class Camera
{
    private:
        void calibrateViewPlane();

    protected:
        P position;
        V viewDir, up;
        V u, v, w;     // Basis vectors
        float fov, aspectRatio, phi, theta;

        V viewPlaneX, viewPlaneY;
        P midpoint;

    public:

		// Computes the width and height dimensions of the given pixel:
		static void pixelDimensions(int resoX, int resoY, float& pw, float& ph);

        Camera() {};
        Camera(const P &position, const V &viewDir, const V &up, float fov, float aspectRatio);
        Camera(const P &position, const V &viewDir, float fov, float aspectRatio);
        Camera(const P &position, const P &lookAt, float fov, float aspectRatio);
        Camera(const Camera &other);

        void setPosition(const P& position);
		const P& getPosition() const { return this->position; }
        
		void setViewDir(const V& viewDir);
		const V& getViewDir() const { return this->viewDir; }

		const V& getUp() const { return this->up; }
		void setUp(const V& up);

		void setFOV(float fov);
		float getFOV() const { return this->fov; }
        
		void setAspectRatio(float aspectRatio);
		float getAspectRatio() const { return this->aspectRatio; }

        P ndc2World(float x, float y) const;
        P screen2World(float x, float y, float resoX, float resoY) const;

        Ray spawnRay(float x, float y) const;
        Ray spawnRay(float x, float y, float resoX, float resoY) const;

        friend std::ostream& operator<<(std::ostream& s, const Camera& c);
};

#endif

/******************************************************************************/
