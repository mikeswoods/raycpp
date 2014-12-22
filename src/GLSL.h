/******************************************************************************
*
 * GLSL-specific macros, etc.
 *
 * @file GLSL.h
 * @author Michael Woods
 ******************************************************************************/

#ifndef GLSL_H
#define GLSL_H

/**
 * Courtesy of http://stackoverflow.com/a/13874526
 *
 * This preprocessor macro allows you to expand GLSL inline 
 * like so:
 *
 * const GLchar* vert = GLSL(120,
 *
 *   attribute vec2 position;
 *
 *   void main()
 *   {
 *       gl_Position = vec4( position, 0.0, 1.0 );
 *   }
 * );
 */
#define GLSL(version, body)  "#version " #version "\n" #body

#endif
