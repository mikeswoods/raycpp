/******************************************************************************
 *
 * Various math and text processing utility functions
 *
 * @file Utils.h
 * @author Michael Woods
 *
 ******************************************************************************/

#define _USE_MATH_DEFINES
#include <cmath>
#include <algorithm>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <cstring>
#include <cstdlib>
#include "Utils.h"

using namespace std;

/*******************************************************************************
 * Numeric functions
 ******************************************************************************/

// Clamp values to a given range
float Utils::clamp(float n, float lo, float hi)
{
    return min(max(n,lo),hi); 
}

// Clamp values to the range [0,1]
float Utils::unitClamp(float n)
{
    return min(max(n, 0.0f), 1.0f); 
}

// Convert one range to another while preserving proportionality
float Utils::reRange(float value, float a0, float a1, float b0, float b1)
{
    // Adapted from http://stackoverflow.com/a/12413880
    return ((value - a0) / (a1 - a0)) * (b1 - b0) + b0;
}

// Convert one range to [0,1] while preserving proportionality
float Utils::unitRange(float value, float lo, float hi)
{
    return reRange(value, lo, hi, 0.0f, 1.0f);
}

// Linearly interpolate between two values
float Utils::lerp(float v1, float v2, float t)
{
    return ((1.0f - t) * v1) + (t * v2);   
}

// Generate a random float in the range [0,1]
float Utils::unitRand()
{
	return static_cast<float>(rand()) / (static_cast<float>(RAND_MAX));
}

// Generate a random float in the range [lo,hi]
float Utils::randInRange(float lo, float hi)
{
	return lo + static_cast<float>(rand()) /(static_cast<float>(RAND_MAX / (hi - lo)));
}

// Returns the smallest of two values >= zero
bool Utils::leastGreaterThanZero(float x, float y, float& smallest)
{
	if (x > 0.0f && y > 0.0f) {
		smallest = x < y ? x : y;
		return true;
	} else if (x > 0.0f && y < 0.0f) {
		smallest = x;
		return true;
	} else if (x < 0.0f && y > 0.0f) {
		smallest = y;
		return true;
	}
	// Leave smallest as-is
	return false;
}

/*******************************************************************************
 * Vector functions
 ******************************************************************************/

// Test if two vectors are orthogonal
bool Utils::orthogonal(glm::vec3 v1, glm::vec3 v2)
{
    return abs(glm::dot(v1, v2) / (glm::length(v1) * glm::length(v2))) < static_cast<float>(EPSILON);
}

// Test if two vectors are parallel
bool Utils::parallel(glm::vec3 v1, glm::vec3 v2)
{
    return abs(glm::dot(v1, v2) / (glm::length(v1) * glm::length(v2))) > 1.0f - static_cast<float>(EPSILON);
}

// Fixes degenerate "up" vector cases
glm::vec3 Utils::fixUpVector(const glm::vec3& viewDir, const glm::vec3& up)
{
    if (Utils::parallel(viewDir, up)) {
        return glm::vec3(up.x, up.y, up.z + static_cast<float>(EPSILON));
    }

    return up;
}

/*******************************************************************************
 * Geometry functions
 ******************************************************************************/

// Tests if the given ray intersects the specified plane
float Utils::hitsPlane(const glm::vec3& origin
                      ,const glm::vec3& dir
                      ,const glm::vec3& center
                      ,const glm::vec3& normal)
{
    glm::vec3 n = glm::normalize(normal);
    glm::vec3 d = glm::normalize(dir);
    float k     = glm::dot(n, d);

    if (std::abs(k) < EPSILON) {
        return -1.0f; // Failed
    }

    float t = -(glm::dot(origin - center, n)) / k;

    return t >= 0.0f ? t : -1.0f;
}

/*******************************************************************************
 * Text functions
 ******************************************************************************/

// Return the current working directory. This solution was adapted from
// the answer on Stackoverflow at http://stackoverflow.com/a/145309
string Utils::cwd(const std::string& relFile)
{
    // Determine the current working directory. Any texture files will be
    // loaded relative to  it
    char buffer[FILENAME_MAX];

    if (!GetCurrentDir(buffer, sizeof(buffer))) {
        throw runtime_error("Couldn't get current working directory!");
    }

    string workingDir = string(buffer);

    if (relFile == "") {
        return workingDir;
    } else {
        ostringstream is;
        is << workingDir << DirSep << relFile;
        return is.str();
    }
}

// Returns the absolute realpath of the given relative path
std::string Utils::realPath(const std::string& relPath)
{
    char buffer[FILENAME_MAX];

    #if defined(_WIN32) || defined(_WIN64)
    GetFullPathName(relPath.c_str(), sizeof(buffer), buffer, nullptr);
    #else
    realpath(relPath.c_str(), buffer);
    #endif

    return string(buffer);
}

// Returns the base component of a filename
std::string Utils::baseName(const std::string& path)
{
    size_t pos = path.find_last_of(DirSep);
    if (pos == string::npos) {
        return path;
    }
    // See if there's another. If so, then it's a base case root directory
    if (path.find_last_of(DirSep, pos) == string::npos) {
        return path;
    } else {
        return path.substr(0, pos);
    }
}

// Adapted from http://www.toptip.ca/2010/03/trim-leading-or-trailing-white-spaces.html
string Utils::trim(const string& s)
{
    string sCopy(s);

    size_t p = sCopy.find_first_not_of(WHITESPACE_CHARS);
    sCopy.erase(0, p);

    p = sCopy.find_last_not_of(WHITESPACE_CHARS);
    if (string::npos != p) {
        sCopy.erase(p+1);
    }

    return sCopy;
}

// Convert s to upper case
string Utils::uppercase(string s)
{
	transform(s.begin(), s.end(), s.begin(), ::toupper);
	return s;
}

// Convert s to upper case
string Utils::lowercase(string s)
{
	transform(s.begin(), s.end(), s.begin(), ::tolower);
	return s;
}

// Convert the input i to a string
string Utils::S(int i)
{
    ostringstream out;
    out << i;
    return out.str();
}

string Utils::S(float i)
{
    ostringstream out;
    out << i;
    return out.str();
}

string Utils::S(vector<string> i)
{
    if (i.size() == 0) {
        return "";
    }

    ostringstream out;

    out << "[ ";
    for (auto j=i.begin(); j != i.end(); j++) {
        out << *j;
        out << " ";
    }
    out << "]";
    
    return out.str();
}

// Courtesy of http://stackoverflow.com/a/16286297
vector<string> Utils::split(string str, string delim)
{
    string s   = string(str);
    string token;
    vector<string> tokens;

    auto i = 0U;
    auto j = s.find(delim);
    while (i != j && j != std::string::npos)
    {
        token = s.substr(i, j - i);

        if (token.length() > 0) {
            tokens.push_back(token);
        }
        i = j + delim.length();
        j = s.find(delim, i);
    }

    tokens.push_back(s.substr(i));

    return tokens;
}

// Parses a numeric pair specifier string like "123,45" into its component 
// floats and sets the values of x and y. If this this function returns true, 
// then the string was successfully parsed, otherwise false is returned 
// and x and y are not updated
bool Utils::parseTuple(string str, float& x, float& y)
{
	vector<string> parts = split(str, string(","));
	if (parts.size() < 2) {
		return false;
	}
	bool good1 = false;
	bool good2 = false;

	istringstream ss(parts[0] + " " + parts[1]);
	ss >> x;
	good1 = !ss.fail();
	ss >> y;
	good2 = !ss.fail();

	return good1 && good2;
}

// Read a text file into a string
std::string Utils::textFileRead(const char* filename)
{
    // http://insanecoding.blogspot.com/2011/11/how-to-read-in-file-in-c.html
    std::ifstream in(filename, std::ios::in);
    if (!in) {
        std::cerr << "Error reading file: " << string(filename) << std::endl;
        throw (errno);
    }
    return std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
}

std::string Utils::textFileRead(const string& filename)
{
    return Utils::textFileRead(filename.c_str());
}

/******************************************************************************/
