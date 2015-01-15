/******************************************************************************
 *
 * This file defines OpenGL-soecfic utility functions
 *
 * @file GLUtils.h
 * @author Michael Woods
 *
 ******************************************************************************/

#ifndef GLUTILS_H
#define GLUTILS_H

#include <ostream>
#include <glew/glew.h>

/******************************************************************************/

namespace GLUtils {

	// Print the general GLSL error log
	void printErrorLog();

	// Print the status of the GLSL linker
	void printLinkInfoLog(GLint prog);

	// Print the status of the GLSL shader
	void printShaderInfoLog(GLint shader);
}

/******************************************************************************/

#endif
