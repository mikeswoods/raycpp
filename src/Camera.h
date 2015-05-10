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
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include "Ray.h"

/******************************************************************************/

class Camera
{
    private:
        void calibrateViewPlane();

    protected:
        glm::vec3 position;
        glm::vec3 viewDir, up;
        glm::vec3 u, v, w;     // Basis vectors
        float fov, aspectRatio, phi, theta;

        glm::vec3 viewPlaneX, viewPlaneY;
        glm::vec3 midpoint;

    public:

		// Computes the width and height dimensions of the given pixel:
		static void pixelDimensions(int resoX, int resoY, float& pw, float& ph);

        Camera() {};
        Camera(const glm::vec3& position, const glm::vec3& viewDir, const glm::vec3 &up, float fov, float aspectRatio);
        Camera(const glm::vec3& position, const glm::vec3& lookAt, float fov, float aspectRatio);
        Camera(const Camera &other);

        void setPosition(const glm::vec3& position);
		const glm::vec3& getPosition() const { return this->position; }
        
		void setViewDir(const glm::vec3& viewDir);
		const glm::vec3& getViewDir() const { return this->viewDir; }

		const glm::vec3& getUp() const { return this->up; }
		void setUp(const glm::vec3& up);

		void setFOV(float fov);
		float getFOV() const { return this->fov; }
        
		void setAspectRatio(float aspectRatio);
		float getAspectRatio() const { return this->aspectRatio; }

        glm::vec3 ndc2World(float x, float y) const;
        glm::vec3 screen2World(float x, float y, float resoX, float resoY) const;

        Ray spawnRay(float x, float y) const;
        Ray spawnRay(float x, float y, float resoX, float resoY) const;

        friend std::ostream& operator<<(std::ostream& s, const Camera& c);
};

#endif

/******************************************************************************/
