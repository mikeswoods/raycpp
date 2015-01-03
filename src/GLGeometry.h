/******************************************************************************
 *
 * This class defines an instance of a geometric object
 *
 * @file GLInstance.h
 * @author Michael Woods
 *
 *****************************************************************************/

#ifndef GL_GEOMETRY_H
#define GL_GEOMETRY_H

#include <glew/glew.h>
#include "Geometry.h"
#include "Color.h"

/*****************************************************************************/

class WorldState;

// Used to pack data into a VBO:

typedef struct VBOUnit {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 color;
} VBOUnit; 


class GLGeometry
{
	protected:
		// Geometry object definition
		Geometry const * geometry;

		// GL object instance color
		Color color;

		// VAO
		GLuint vao;

		// VBOs
		GLuint vbo, vboIndex;

		// Determines which drawing mode will be passed to glDrawElements()
		// when the geometry contained in this instance is rendered
		GLenum drawMode;

		// Determines which drawing mode will be passed to glPolyMode()
		// when the geometry contained in this instance is rendered
		GLenum polyMode;

		WorldState* state;

	public:
		GLGeometry(Geometry* geometry);
		GLGeometry(Geometry* geometry, const Color& color);
		GLGeometry(const GLGeometry& other);
		~GLGeometry();

		const Color& getColor() const { return this->color; }
		void setColor(const Color& color) { this->color = color; }

		// Sets the drawing mode passed to glDrawElements()
		GLenum getDrawMode() const { return this->drawMode; }
		void setDrawMode(GLenum drawMode) { this->drawMode = drawMode; }

		// Sets the polygon mode passed to glPolyMode
		GLenum getPolyMode() const        { return this->polyMode; }
		void setPolyMode(GLenum polyMode) { this->polyMode = polyMode; }

		void highlightObject() { this->setDrawMode(GL_LINES); }
		void unHighlightObject() { this->setDrawMode(GL_TRIANGLES); }

		// Get the underlying geometry of the instance
		Geometry const * getGeometry() { return this->geometry; }

		void upload(GLuint shaderProgram
			       ,GLint locationPos
				   ,GLint locationNor
				   ,GLint locationCol);

		void draw(WorldState* state
			     ,GLuint shaderProgram
				 ,GLint unifModel
				 ,GLint unifModelInvT
				 ,glm::mat4 affine);
};

/*****************************************************************************/

#endif
