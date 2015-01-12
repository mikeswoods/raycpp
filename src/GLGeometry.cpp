/******************************************************************************
 *
 * This class defines an instance of a geometric object
 *
 * @file GLInstance.h
 * @author Michael Woods
 *
 *****************************************************************************/

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "GLGeometry.h"
#include "GLWorldState.h"

using namespace std;

/*****************************************************************************/

GLGeometry::GLGeometry(shared_ptr<Geometry> _geometry) :
	geometry(_geometry),
	color(Color::WHITE),
    vao(0),
    vbo(0), vboIndex(0), 
	drawMode(GL_TRIANGLES),
	polyMode(GL_FILL)
{ 

}

GLGeometry::GLGeometry(shared_ptr<Geometry> _geometry, const Color& _color) :
	geometry(_geometry),
	color(_color),
	vao(0),
    vbo(0), vboIndex(0), 
	drawMode(GL_TRIANGLES),
	polyMode(GL_FILL)
{ 
	
}

GLGeometry::GLGeometry(const GLGeometry& other) :
	geometry(other.geometry),
	color(other.color),
	vao(other.vao),
    vbo(other.vbo), vboIndex(other.vboIndex),  
	drawMode(other.drawMode),
	polyMode(other.polyMode)
{ 
	
}

GLGeometry::~GLGeometry() 
{
	glDeleteBuffers(1, &this->vbo);
	glDeleteBuffers(1, &this->vboIndex);

	glDeleteVertexArrays(1, &this->vao);
}

void GLGeometry::upload(GLuint shaderProgram
	                   ,GLint locationPos
					   ,GLint locationNor
					   ,GLint locationCol)
{
	vector<glm::vec3> positions  = this->geometry->getVertices();
	vector<glm::vec3> normals    = this->geometry->getNormals();
	vector<glm::vec3> colors     = this->geometry->getColors(this->color);
	vector<unsigned int> indices = this->geometry->getIndices();
	size_t is                    = sizeof(unsigned int);
	unsigned int ic              = this->geometry->getIndexCount();

	////////////////////////////////////////////////////////////////////////////

	vector<VBOUnit> units;
	size_t n = positions.size();

	for (unsigned int i=0; i<n; i++) {
		VBOUnit unit = { positions[i], normals[i], colors[i] };
		units.push_back(unit);
	}

	////////////////////////////////////////////////////////////////////////////

	glUseProgram(shaderProgram);

	// Allocate and bind the VAO used to link all subsequent VBOs to:
	glGenVertexArrays(1, &this->vao);
    glBindVertexArray(this->vao);

    // VBO unit: position + normal + color
	glGenBuffers(1, &this->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VBOUnit) * units.size(), &units[0], GL_STATIC_DRAW);

	// Set up the attributes packed into VBOUnit
	// - Position:
    glEnableVertexAttribArray(locationPos);
    glVertexAttribPointer(locationPos, 3, GL_FLOAT, false, sizeof(VBOUnit), 0);
	// - Normals:
    glEnableVertexAttribArray(locationNor);
    glVertexAttribPointer(locationNor, 3, GL_FLOAT, false, sizeof(VBOUnit), (void*)(1 * sizeof(glm::vec3)));
	// - Colors:
    glEnableVertexAttribArray(locationCol);
    glVertexAttribPointer(locationCol, 3, GL_FLOAT, false, sizeof(VBOUnit), (void*)(2 * sizeof(glm::vec3)));

	// Indices:
	glGenBuffers(1, &this->vboIndex);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboIndex);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, ic * is, &indices[0], GL_STATIC_DRAW);

    glBindVertexArray(0);
}

void GLGeometry::draw(const GLWorldState& state
	                 ,GLuint shaderProgram
					 ,GLint unifModel
					 ,GLint unifModelInvT
					 ,glm::mat4 affine)
{
    glUseProgram(shaderProgram);

    // === Current affine transformation ======================================
    glUniformMatrix4fv(unifModel, 1, GL_FALSE, &affine[0][0]);

    const glm::mat4 modelInvTranspose = glm::inverse(glm::transpose(affine));
    glUniformMatrix4fv(unifModelInvT, 1, GL_FALSE, &modelInvTranspose[0][0]);

	glBindVertexArray(this->vao);
	glPolygonMode(GL_FRONT_AND_BACK, this->getPolyMode());
    glDrawElements(this->getDrawMode(), this->geometry->getIndexCount(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

/*****************************************************************************/
