/******************************************************************************
 *
 * Various functions for sampling
 *
 * @file Sampling.h
 * @author Michael Woods
 ******************************************************************************/

#ifndef SAMPLING_H
#define SAMPLING_H

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

namespace Sampling {

	// Given a normal vector, find a cosine weighted random direction in a
	// hemisphere. Adapted from CIS 565
	glm::vec3 getCosineWeightedDirection(const glm::vec3& normal);
}

#endif
