/******************************************************************************
 *
 * Various math and text processing utility functions
 *
 * @file Utils.h
 * @author Michael Woods
 *
 ******************************************************************************/

#ifndef UTILS_H
#define UTILS_H

#define WHITESPACE_CHARS " \t\r\n"
#include <sstream>
#include <iostream>
#include <string>
#include <vector>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#if defined(_WIN32) || defined(_WIN64)
    #include <direct.h>
    #include <windows.h>
    #define getcwd _getcwd
    #define chdir _chdir
    #define DirSep "\\"
#else
    #include <unistd.h>
    #include <errno.h>
    #define DirSep "/"
#endif

#include "Ray.h"

namespace Utils 
{
	// Constants ///////////////////////////////////////////////////////////////

	const float EPSILON  = 1.0e-4f;

    // Numeric functions ///////////////////////////////////////////////////////

    float almostEqual(float a, float b, float epsilon);

    // Clamp a value to the range [lo,hi]
    float clamp(float n, float lo, float hi);

    // Clamp a value to the unit range [0,1]
    float unitClamp(float n);

    // Converts a number (n) and range [lo,hi] such that lo < n <= hi, to a
    // new number (n') and range [lo',hi'] pwhile preserving n's proportional
    // value to lo and hi
    float reRange(float value, float a0, float a1, float b0, float b1);

    // Same as reRange, but rescales to the range [0,1]
    float unitRange(float value, float lo, float hi);

    // Linear interpolation
    float lerp(float v1, float v2, float t);

    // Trilinear interpolation
    float trilerp(float xd, float yd, float zd
                 ,float v000, float v001
                 ,float v010, float v011
                 ,float v100, float v101
                 ,float v110, float v111);

	// Generate a random float in the range [0,1]
	float unitRand();

	// Generate a random float in the range [lo,hi]
	float randInRange(float lo, float hi);

    // Vector functions ////////////////////////////////////////////////////////

    // Test if two vectors are orthogonal
    bool orthogonal(glm::vec3 v1, glm::vec3 v2);

    // Test if two vectors are parallel
    bool parallel(glm::vec3 v1, glm::vec3 v2);

    // Fixes degenerate "up" vector cases
    glm::vec3 fixUpVector(const glm::vec3& viewDir, const glm::vec3& up);

	// Returns the smallest of two values >= zero
	bool leastGreaterThanZero(float x, float y, float& smallest);

    // Geometry functions //////////////////////////////////////////////////////

    // Tests if the ray defined by origin and dir intersects the plane specified
    // by a surface normal and distance
    float hitsPlane(const glm::vec3& origin
                   ,const glm::vec3& dir
                   ,const glm::vec3& center
                   ,const glm::vec3& normal);

    // Text functions //////////////////////////////////////////////////////////

    // Return the current working directory
    std::string cwd(const std::string& relFile = "");

    // Returns the absolute realpath of the given relative path
    std::string realPath(const std::string& path);

    // Returns the basename of the given path
    std::string baseName(const std::string& path);

    // Resolves a file path relative to another path
    std::string resolvePath(const std::string& path, const std::string& relative);

    // Trims leading and trailing whitespace from a string 
    std::string trim(const std::string& s);

	// Convert s to upper case
	std::string uppercase(std::string s);

	// Convert s to upper case
	std::string lowercase(std::string s);

    // Convert the input i to a string
    std::string S(int i);
    std::string S(float i);
    std::string S(std::vector<std::string> i);

    // Split a string into pieces given a delimiter
    std::vector<std::string> split(std::string str, std::string delim);

	// Attempt to parse an int, returning a default value in the event of failure
	template<typename T> T parseNumber(std::string str, T def)
	{
		std::istringstream ss(str);
		T extract;
		ss >> extract;
		return ss.bad() ? def : extract;
	}

	// Read a text file into a string
	std::string textFileRead(const char* filename);
    std::string textFileRead(const std::string& filename);
}

#endif
