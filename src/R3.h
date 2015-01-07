/*******************************************************************************
 *
 * This file defines operations over point and vector data types in R^3 space
 *
 * @file R3.h
 * @author Michael Woods
 *
 ******************************************************************************/

#ifndef R3_H
#define R3_H

#include <vector>
#include <iostream>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

/*******************************************************************************
 * Vector type in R3 space
 ******************************************************************************/

typedef glm::vec3 V;

float x(const V& v);
float y(const V& v);
float z(const V& v);

std::ostream& operator<<(std::ostream& s, const V& v);

V operator*(const V& v, float s);
V operator/(const V& v, float s);

/*******************************************************************************
 * Point type in R3 space
 ******************************************************************************/

class P
{
    public:
		// xyz position in R^3 space
        glm::vec3 xyz; 

        P();
		P(glm::vec3 xyz);
        P(float x, float y, float z);
        P(float xyz[3]);
        P(const P& other);

        float x() { return this->xyz.x; } 
        float y() { return this->xyz.y; } 
        float z() { return this->xyz.z; }

		void updateX(float amt) { this->xyz[0] += amt; }
		void updateY(float amt) { this->xyz[1] += amt; }
		void updateZ(float amt) { this->xyz[2] += amt; }

        friend std::ostream& operator<<(std::ostream& s, const P& p);

        P& operator+=(const V& v);
        P& operator-=(const V& v);
};

float x(const P& p);
float y(const P& p);
float z(const P& p);
glm::vec3 toVec3(const P& p);

bool operator==(const P& p, const P& q);
bool operator!=(const P& p, const P& q);

P operator+(const P& p, const V& v);
P operator+(const P& p, float mu);
P operator*(const P& p, float mu);
P operator+(const V& v, const P& p);
V operator-(const P& p1, const P& p2);
P operator-(const P& p, float mu);

/*******************************************************************************
 * Operations
 ******************************************************************************/

/**
 * Applies the affine transformation specified in the 4x4 matrix T
 * to the homogenous point/vector pointOrVector, yielding a position
 * or vector in R^3 space
 */
glm::vec3 transform(glm::mat4 T, glm::vec4 pointOrVector);

/** 
 * Step along calculation, returning the number of steps as well as
 * setting the initial position point X and step vector N
 */
int steps(float stepSize, float offset, const P& start, const P& end, P& X, V& N);
int steps(float stepSize, float offset, const P& start, const V& along, P& X, V& N);

/**
 * Computes the centroid point between the two given points
 */
glm::vec3 mean(const glm::vec3& p, const glm::vec3& q);
glm::vec3 mean(const std::vector<glm::vec3>& ps);
P mean(const P& p, const P& q);
P mean(const std::vector<P>& ps);

/**
 * Given two points, this function returns a new point consisting of 
 * the components with overall maximum values
 */
glm::vec3 maximum(const glm::vec3& p, const glm::vec3& q);
glm::vec3 maximum(const std::vector<glm::vec3>& ps);
P maximum(const P& p, const P& q);
P maximum(const std::vector<P>& ps);

/**
 * Given two points, this function returns a new point consisting of 
 * the components with overall minimum values
 */
glm::vec3 minimum(const glm::vec3& p, const glm::vec3& q);
glm::vec3 minimum(const std::vector<glm::vec3>& ps);
P minimum(const P& p, const P& q);
P minimum(const std::vector<P>& ps);

/******************************************************************************/

#endif
