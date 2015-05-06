#define _USE_MATH_DEFINES
#include "Sampling.h"
#include <algorithm>
#include <cstdlib>
#include <cmath>

/******************************************************************************/

glm::vec3 Sampling::getCosineWeightedDirection(const glm::vec3& normal) 
{
	// Pick 2 random numbers in the range (0, 1)
	float xi1 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	float xi2 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

	float up = sqrt(xi1); 			// cos(theta)
	float over = sqrt(1 - up * up); // sin(theta)
	float around = xi2 * 2.0f * static_cast<float>(M_PI);

    // Find a direction that is not the normal based off of whether or not the normal's components 
    // are all equal to sqrt(1/3) or whether or not at least one component is less than sqrt(1/3).
	const float SQRT_OF_ONE_THIRD = sqrt(1.0f / 3.0f);
	glm::vec3 directionNotNormal;

	if (abs(normal.x) < SQRT_OF_ONE_THIRD) {
		directionNotNormal = glm::vec3(1.f, 0.f, 0.f);
	} else if (abs(normal.y) < SQRT_OF_ONE_THIRD) {
		directionNotNormal = glm::vec3(0.f, 1.f, 0.f);
	} else {
		directionNotNormal = glm::vec3(0.f, 0.f, 1.f);
	}

	//Use not-normal direction to generate two perpendicular directions
	glm::vec3 perpendicularDirection1 = glm::normalize(glm::cross(normal, directionNotNormal));
	glm::vec3 perpendicularDirection2 = glm::normalize(glm::cross(normal, perpendicularDirection1));

	float X = static_cast<float>(cos(around)) * over;
	float Y = static_cast<float>(sin(around)) * over;

	return (up * normal) + (X * perpendicularDirection1) + (Y * perpendicularDirection2);
}

/******************************************************************************/
