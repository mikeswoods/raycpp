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
using namespace glm;

/*******************************************************************************
 * Numeric functions
 ******************************************************************************/

/**
 * Almost equal
 */
float Utils::almostEqual(float a, float b, float epsilon)
{
    // assume small positive epsilon
    assert(epsilon >= 0.0f && epsilon <= 1.0f);

    float diff  = abs(a - b);
    float maxab = std::max(abs(a), abs(b));

    // if the multiply won't underflow then use a multiply
    if (maxab >= 1.0f) {
        return diff <= (epsilon * maxab);
    } else if (maxab > 0.0f) {
        // multiply could underflow so use a divide if nonzero denominator
        // correctly returns false on divide overflow
        // (inf < = epsilon is false), since overflow means the
        // relative difference is large and they are therefore not close
        return diff / maxab <= epsilon;
    }

    // both a and b are zero
    return false;
}

/** 
 * Clamp values to a given range
 */
float Utils::clamp(float n, float lo, float hi)
{
    return std::min(std::max(n, lo), hi); 
}

/**
 * Clamp values to the range [0,1]
 */
float Utils::unitClamp(float n)
{
    return std::min(std::max(n, 0.0f), 1.0f); 
}

/** 
 * Convert one range to another while preserving proportionality
 */
float Utils::reRange(float value, float a0, float a1, float b0, float b1)
{
    // Adapted from http://stackoverflow.com/a/12413880
    return ((value - a0) / (a1 - a0)) * (b1 - b0) + b0;
}

/** 
 * Convert one range to [0,1] while preserving proportionality
 */
float Utils::unitRange(float value, float lo, float hi)
{
    return reRange(value, lo, hi, 0.0f, 1.0f);
}

/**
 * Linearly interpolate between two values
 */
float Utils::lerp(float v1, float v2, float t)
{
    return ((1.0f - t) * v1) + (t * v2);   
}

/**
 * Trilinear interpolation
 */
float Utils::trilerp(float xd, float yd, float zd
                    ,float v000, float v001
                    ,float v010, float v011
                    ,float v100, float v101
                    ,float v110, float v111)
{
    float c00 = lerp(v000, v100, xd);
    float c10 = lerp(v010, v110, xd);
    float c01 = lerp(v001, v101, xd);
    float c11 = lerp(v011, v111, xd);
    float c0  = lerp(c00, c10, yd);
    float c1  = lerp(c01, c11, yd);
    return lerp(c0, c1, zd);
}

/**
 * Generate a random float in the range [0,1]
 */
float Utils::unitRand()
{
	return static_cast<float>(rand()) / (static_cast<float>(RAND_MAX));
}

/**
 * Generate a random float in the range [lo,hi]
 */
float Utils::randInRange(float lo, float hi)
{
	return lo + static_cast<float>(rand()) /(static_cast<float>(RAND_MAX / (hi - lo)));
}

/**
 * Returns the smallest of two values >= zero
 */
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

/**
 * Test if two vectors are orthogonal
 */
bool Utils::orthogonal(vec3 v1, vec3 v2)
{
    return abs(dot(v1, v2) / (length(v1) * length(v2))) < static_cast<float>(EPSILON);
}

/**
 * Test if two vectors are parallel
 */
bool Utils::parallel(vec3 v1, vec3 v2)
{
    return abs(dot(v1, v2) / (length(v1) * length(v2))) > 1.0f - static_cast<float>(EPSILON);
}

/**
 * Fixes degenerate "up" vector cases
 */
vec3 Utils::fixUpVector(const vec3& viewDir, const vec3& up)
{
    if (Utils::parallel(viewDir, up)) {
        return vec3(up.x, up.y, up.z + static_cast<float>(EPSILON));
    }

    return up;
}

/*******************************************************************************
 * Geometry functions
 ******************************************************************************/

/**
 * Tests if the given ray intersects the specified plane
 */
float Utils::hitsPlane(const vec3& origin
                      ,const vec3& dir
                      ,const vec3& center
                      ,const vec3& normal)
{
    vec3 n = normalize(normal);
    vec3 d = normalize(dir);
    float k     = dot(n, d);

    if (abs(k) < EPSILON) {
        return -1.0f; // Failed
    }

    float t = -(dot(origin - center, n)) / k;

    return t >= 0.0f ? t : -1.0f;
}

/*******************************************************************************
 * Text functions
 ******************************************************************************/

/**
 * Return the current working directory. This solution was adapted from
 * the answer on Stackoverflow at http://stackoverflow.com/a/145309
 */
string Utils::cwd(const string& relFile)
{
    // Determine the current working directory. Any texture files will be
    // loaded relative to  it
    char buffer[FILENAME_MAX];

    if (!getcwd(buffer, sizeof(buffer))) {
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

/**
 * Returns the absolute realpath of the given relative path
 */
string Utils::realPath(const string& path)
{
    char buffer[FILENAME_MAX];

    #if defined(_WIN32) || defined(_WIN64)
    GetFullPathName(path.c_str(), sizeof(buffer), buffer, nullptr);
    #else
    realpath(path.c_str(), buffer);
    #endif

    return string(buffer);
}

/**
 * Returns the base component of a filename
 */
string Utils::baseName(const string& path)
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

/**
 * Resolves a file path relative to another path
 */
string Utils::resolvePath(const string& path, const string& relative)
{
    string current = cwd();
    chdir(relative.c_str());
    string p = realPath(path);
    chdir(current.c_str());
    return p;
}

/**
 * Adapted from http://www.toptip.ca/2010/03/trim-leading-or-trailing-white-spaces.html
 */
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

/**
 * Convert the given string to upper case
 */
string Utils::uppercase(string s)
{
	transform(s.begin(), s.end(), s.begin(), ::toupper);
	return s;
}

/**
 * Convert the given string to upper case
 */
string Utils::lowercase(string s)
{
	transform(s.begin(), s.end(), s.begin(), ::tolower);
	return s;
}

/**
 * Convert the input i to a string
 */
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

/** 
 * Courtesy of http://stackoverflow.com/a/16286297
 */
vector<string> Utils::split(string str, string delim)
{
    string s   = string(str);
    string token;
    vector<string> tokens;

    auto i = 0U;
    auto j = s.find(delim);
    while (i != j && j != string::npos)
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

/**
 * Read a text file into a string
 */
string Utils::textFileRead(const char* filename)
{
    // http://insanecoding.blogspot.com/2011/11/how-to-read-in-file-in-c.html
    ifstream in(filename, ios::in);
    if (!in) {
        cerr << "Error reading file: " << string(filename) << endl;
        throw (errno);
    }
    return string(istreambuf_iterator<char>(in), istreambuf_iterator<char>());
}

string Utils::textFileRead(const string& filename)
{
    return Utils::textFileRead(filename.c_str());
}

/******************************************************************************/
