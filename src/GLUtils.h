/******************************************************************************
 *
 * This file defines OpenGL-soecfic utility functions
 *
 * @file GLUtils.h
 * @author Michael Woods
 ******************************************************************************/

#ifndef GLUTILS_H
#define GLUTILS_H

#include <ostream>
#include <glew/glew.h>
#include <GL/glut.h>

///////////////////////////////////////////////////////////////////////////////

namespace GLUtils {

	// Print the general GLSL error log
	void printErrorLog(std::ostream& os);

	// Print the status of the GLSL linker
	void printLinkInfoLog(std::ostream& os, GLint prog);

	// Print the status of the GLSL shader
	void printShaderInfoLog(std::ostream& os, GLint shader);
};

///////////////////////////////////////////////////////////////////////////////

#endif
