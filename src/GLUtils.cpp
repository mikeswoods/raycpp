#include <iostream>
#include <ostream>
#include <glew/glew.h>
#include <GL/glut.h>
#include "GLUtils.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////

void GLUtils::printErrorLog(ostream& os)
{
    GLenum error = glGetError();

    if (error != GL_NO_ERROR) {

        os << "OpenGL error " << error << ": ";

		const char *e =
            error == GL_INVALID_OPERATION ? "GL_INVALID_OPERATION" :
            error == GL_INVALID_ENUM      ? "GL_INVALID_ENUM" :
            error == GL_INVALID_VALUE     ? "GL_INVALID_VALUE" :
            error == GL_INVALID_INDEX     ? "GL_INVALID_INDEX" :
            "unknown";

        os << e << endl;

        // Throwing here allows us to use the debugger stack trace to track
        // down the error.
#ifndef __APPLE__
        // But don't do this on OS X. It might cause a premature crash.
        // http://lists.apple.com/archives/mac-opengl/2012/Jul/msg00038.html
        throw;
#endif
    }
}

void GLUtils::printLinkInfoLog(ostream& os, GLint prog)
{
    GLint linked;
    glGetProgramiv(prog, GL_LINK_STATUS, &linked);

    if (linked == GL_TRUE) {
        return;
    }

    os << "GLSL LINK ERROR" << std::endl;

    int infoLogLen = 0;
    int charsWritten = 0;
    GLchar *infoLog;

    glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &infoLogLen);

    if (infoLogLen > 0) {

        infoLog = new GLchar[infoLogLen];

        // error check for fail to allocate memory omitted
        glGetProgramInfoLog(prog, infoLogLen, &charsWritten, infoLog);

        os << "InfoLog:" << endl << infoLog << endl;

        delete[] infoLog;
    }

    // Throwing here allows us to use the debugger to track down the error.
    throw;
}

void GLUtils::printShaderInfoLog(ostream& os, GLint shader)
{
    GLint compiled;

    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

	if (compiled == GL_TRUE) {
        return;
    }

    os << "GLSL COMPILE ERROR" << std::endl;

    int infoLogLen = 0, charsWritten = 0;
    GLchar *infoLog;

    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLen);

    if (infoLogLen > 0) {
        
		infoLog = new GLchar[infoLogLen];

		// error check for fail to allocate memory omitted
        glGetShaderInfoLog(shader, infoLogLen, &charsWritten, infoLog);

        os << "InfoLog:" << endl << infoLog << endl;

        delete[] infoLog;
    }

    // Throwing here allows us to use the debugger to track down the error.
    throw;
}

///////////////////////////////////////////////////////////////////////////////