/******************************************************************************
 *
 * This file defines a world state, which as the name implies is a type that
 * contains quanities, flags, and other pieces of data used to define the 
 * current state of the rendering world at a given point in time
 *
 * @file GLWorldState.h
 * @author Michael Woods
 *
 ******************************************************************************/

#define _USE_MATH_DEFINES
#include <memory>
#include <cmath>
#include "GLWorldState.h"

using namespace std;

/******************************************************************************/

const glm::vec3 GLWorldState::DEFAULT_LIGHT_POSITION = glm::vec3(0.0f, 9.0f, 0.0f);

/******************************************************************************/

GLWorldState::GLWorldState(const Graph& _graph) :
	iterator(unique_ptr<Graph::iterator>(new Graph::pre_iterator(_graph, true))),
	flagRotateScene(false),
	flagCycleLightHue(false),
	polyModeIndex(0),
	previewLight(PointLight(DEFAULT_LIGHT_POSITION, Color::WHITE)),
	globalLightHue(0.0f)
{ 

}

GLWorldState::~GLWorldState() 
{ 

}

// Node traversal operations //////////////////////////////////////////////////

GraphNode* GLWorldState::gotoRoot()
{
	return this->iterator->reset();
}

GraphNode* GLWorldState::getCurrentNode()
{
	return this->iterator->current();
}

GraphNode* GLWorldState::getNextNode()
{
	return this->iterator->next();
}

// Operations /////////////////////////////////////////////////////////////////

void GLWorldState::toggleRotateScene()
{
	this->flagRotateScene = !this->flagRotateScene; 
}

bool GLWorldState::doRotateScene()
{
	return this->flagRotateScene;
}

void GLWorldState::highlightNextNode()
{
	GLGeometry* currentInstance = this->getCurrentNode()->getInstance();
	GLGeometry* nextInstance    = this->getNextNode()->getInstance();

	if (currentInstance != nullptr) {
		currentInstance->unHighlightObject();
	}

	if (nextInstance != nullptr) {
		nextInstance->highlightObject();
	}
}

// Switch between polygon drawing modes:
void GLWorldState::switchPolygonMode()
{
	GLenum useMode = GL_FILL;
	switch (++this->polyModeIndex % 3) {
		case 0:
			useMode = GL_FILL;
			break;
		case 1:
			useMode = GL_LINE;
			break;
		case 2:
			useMode = GL_POINT;
			break;
	}

	for (auto i = this->graph.begin(); !i.done(); i++) {
		GLGeometry* instance = (*i)->getInstance();
		if (instance != nullptr) {
			instance->setPolyMode(useMode);
		}
	}
}

// used in GLWorldState::deleteSelectedNode()
static void _deleteNode(GraphNode* node, Graph* graph)
{
	GLGeometry* instance = node->getInstance();
	node->detachFromParent();

	if (node != graph->getRoot()) {
		if (instance != nullptr) {
			delete instance;
		}
		delete node;
	} else { // Special handling for root node:
		graph->setRoot(nullptr);
		if (instance != nullptr) {
			delete instance;
		}
	}
}

// Delete the selected node
bool GLWorldState::deleteSelectedNode()
{
	postWalk(this->getCurrentNode(), _deleteNode, &this->graph);

	if (graph.getRoot() != nullptr) {
		this->gotoRoot();
		return false;
	} else {
		return true;
	}
}

// Translate //////////////////////////////////////////////////////////////////

void GLWorldState::translateSelectedXPos()
{
	GraphNode* node = this->getCurrentNode();
	#ifdef DEBUG
	clog << "translate +X<before>: " << *node << endl;
	#endif
	node->translateXBy(TRANSLATE_BY_UNIT);
	#ifdef DEBUG
	clog << "translate +X<after>: " << *node << endl;
	#endif
}

void GLWorldState::translateSelectedXNeg()
{
	GraphNode* node = this->getCurrentNode();
	#ifdef DEBUG
	clog << "translate -X<before>: " << *node << endl;
	#endif
	node->translateXBy(-TRANSLATE_BY_UNIT);
	#ifdef DEBUG
	clog << "translate -X<after>: " << *node << endl;
	#endif
}

void GLWorldState::translateSelectedYPos()
{
	GraphNode* node = this->getCurrentNode();
	#ifdef DEBUG
	clog << "translate +Y<before>: " << *node << endl;
	#endif
	node->translateYBy(TRANSLATE_BY_UNIT);
	#ifdef DEBUG
	clog << "translate +Y<after>: " << *node << endl;
	#endif
}

void GLWorldState::translateSelectedYNeg()
{
	GraphNode* node = this->getCurrentNode();
	#ifdef DEBUG
	clog << "translate -Y<before>: " << *node << endl;
	#endif
	node->translateYBy(-TRANSLATE_BY_UNIT);
	#ifdef DEBUG
	clog << "translate -Y<after>: " << *node << endl;
	#endif
}

void GLWorldState::translateSelectedZPos()
{
	GraphNode* node = this->getCurrentNode();
	#ifdef DEBUG
	clog << "translate +Z<before>: " << *node << endl;
	#endif
	node->translateZBy(TRANSLATE_BY_UNIT);
	#ifdef DEBUG
	clog << "translate +Z<after>: " << *node << endl;
	#endif
}

void GLWorldState::translateSelectedZNeg()
{
	GraphNode* node = this->getCurrentNode();
	#ifdef DEBUG
	clog << "translate -Z<before>: " << *node << endl;
	#endif
	node->translateZBy(-TRANSLATE_BY_UNIT);
	#ifdef DEBUG
	clog << "translate -Z<after>: " << *node << endl;
	#endif
}

// Rotate /////////////////////////////////////////////////////////////////////

void GLWorldState::rotateSelectedPosX()
{
	GraphNode* node = this->getCurrentNode();
	#ifdef DEBUG
	clog << "rotate +X<before>: " << *node << endl;
	#endif
	node->rotateXBy(ROTATE_BY_UNIT);
	#ifdef DEBUG
	clog << "rotate +X<after>: " << *node << endl;
	#endif
}

void GLWorldState::rotateSelectedNegX()
{
	GraphNode* node = this->getCurrentNode();
	#ifdef DEBUG
	clog << "rotate -X<before>: " << *node << endl;
	#endif
	node->rotateXBy(-ROTATE_BY_UNIT);
	#ifdef DEBUG
	clog << "rotate -X<after>: " << *node << endl;
	#endif
}

void GLWorldState::rotateSelectedPosY()
{
	GraphNode* node = this->getCurrentNode();
	#ifdef DEBUG
	clog << "rotate +Y<before>: " << *node << endl;
	#endif
	node->rotateYBy(ROTATE_BY_UNIT);
	#ifdef DEBUG
	clog << "rotate +Y<after>: " << *node << endl;
	#endif
}

void GLWorldState::rotateSelectedNegY()
{
	GraphNode* node = this->getCurrentNode();
	#ifdef DEBUG
	clog << "rotate -Y<before>: " << *node << endl;
	#endif
	node->rotateYBy(-ROTATE_BY_UNIT);
	#ifdef DEBUG
	clog << "rotate -Y<after>: " << *node << endl;
	#endif
}

void GLWorldState::rotateSelectedPosZ()
{
	GraphNode* node = this->getCurrentNode();
	#ifdef DEBUG
	clog << "rotate +Z<before>: " << *node << endl;
	#endif
	node->rotateZBy(ROTATE_BY_UNIT);
	#ifdef DEBUG
	clog << "rotate +Z<after>: " << *node << endl;
	#endif
}

void GLWorldState::rotateSelectedNegZ()
{
	GraphNode* node = this->getCurrentNode();
	#ifdef DEBUG
	clog << "rotate -Z<before>: " << *node << endl;
	#endif
	node->rotateZBy(-ROTATE_BY_UNIT);
	#ifdef DEBUG
	clog << "rotate -Z<after>: " << *node << endl;
	#endif
}

// Scale //////////////////////////////////////////////////////////////////////

void GLWorldState::scaleIncreaseSelectedX()
{
	GraphNode* node = this->getCurrentNode();
	#ifdef DEBUG
	clog << "scale +X<before>: " << *node << endl;
	#endif
	node->scaleXBy(SCALE_BY_UNIT);
	#ifdef DEBUG
	clog << "scale +X<after>: " << *node << endl;
	#endif
}

void GLWorldState::scaleDecreaseSelectedX()
{
	GraphNode* node = this->getCurrentNode();
	#ifdef DEBUG
	clog << "scale -X<before>: " << *node << endl;
	#endif
	node->scaleXBy(-SCALE_BY_UNIT);
	#ifdef DEBUG
	clog << "scale -X<after>: " << *node << endl;
	#endif
}

void GLWorldState::scaleIncreaseSelectedY()
{
	GraphNode* node = this->getCurrentNode();
	#ifdef DEBUG
	clog << "scale +Y<before>: " << *node << endl;
	#endif
	node->scaleYBy(SCALE_BY_UNIT);
	#ifdef DEBUG
	clog << "scale +Y<after>: " << *node << endl;
	#endif
}

void GLWorldState::scaleDecreaseSelectedY()
{
	GraphNode* node = this->getCurrentNode();
	#ifdef DEBUG
	clog << "scale -Y<before>: " << *node << endl;
	#endif
	node->scaleYBy(-SCALE_BY_UNIT);
	#ifdef DEBUG
	clog << "scale -Y<after>: " << *node << endl;
	#endif
}

void GLWorldState::scaleIncreaseSelectedZ()
{
	GraphNode* node = this->getCurrentNode();
	#ifdef DEBUG
	clog << "scale +Z<before>: " << *node << endl;
	#endif
	node->scaleZBy(SCALE_BY_UNIT);
	#ifdef DEBUG
	clog << "scale +Z<after>: " << *node << endl;
	#endif
}

void GLWorldState::scaleDecreaseSelectedZ()
{
	GraphNode* node = this->getCurrentNode();
	#ifdef DEBUG
	clog << "scale -Z<before>: " << *node << endl;
	#endif
	node->scaleZBy(-SCALE_BY_UNIT);
	#ifdef DEBUG
	clog << "scale -Z<after>: " << *node << endl;
	#endif
}

// Light //////////////////////////////////////////////////////////////////////

void GLWorldState::toggleLightHueChange()
{
	this->flagCycleLightHue = !this->flagCycleLightHue;
}

bool GLWorldState::doLightHueChange()
{
	return this->flagCycleLightHue;
}

void GLWorldState::translateLightPosX()
{
	previewLight.translateX(TRANSLATE_BY_UNIT);
}

void GLWorldState::translateLightNegX()
{
	previewLight.translateX(-TRANSLATE_BY_UNIT);
}

void GLWorldState::translateLightPosY()
{
	previewLight.translateY(TRANSLATE_BY_UNIT);
}

void GLWorldState::translateLightNegY()
{
	previewLight.translateY(-TRANSLATE_BY_UNIT);
}

void GLWorldState::translateLightPosZ()
{
	previewLight.translateZ(TRANSLATE_BY_UNIT);
}

void GLWorldState::translateLightNegZ()
{
	previewLight.translateZ(-TRANSLATE_BY_UNIT);
}

void GLWorldState::shiftGlobalLightHue()
{
	PointLight previewLight = this->getPreviewLight();
	this->globalLightHue    = fmod(globalLightHue + HUE_UNIT, 360.0f);
	previewLight.setColor(Color::fromHSV(this->globalLightHue, 0.75f, 1.0f));
}
