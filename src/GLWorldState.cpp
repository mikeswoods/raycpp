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

/******************************************************************************/

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

shared_ptr<GraphNode> GLWorldState::gotoRoot()
{
	return this->iterator->reset();
}

shared_ptr<GraphNode> GLWorldState::getCurrentNode()
{
	return this->iterator->current();
}

shared_ptr<GraphNode> GLWorldState::getNextNode()
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
	shared_ptr<GLGeometry> current = this->getCurrentNode()->getInstance();

	if (current) {
		current->unHighlightObject();
	}

	shared_ptr<GLGeometry> next = this->getNextNode()->getInstance();

	if (next) {
		next->highlightObject();
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
		
		shared_ptr<GLGeometry> instance = (*i)->getInstance();
		if (instance) {
			instance->setPolyMode(useMode);
		}
	}
}

// used in GLWorldState::deleteSelectedNode()
static void _deleteNode(shared_ptr<GraphNode> node, Graph* graph)
{
	shared_ptr<GLGeometry> instance = node->getInstance();
	node->detachFromParent();
}

// Delete the selected node
bool GLWorldState::deleteSelectedNode()
{
	postWalk(this->getCurrentNode(), _deleteNode, &this->graph);

	if (graph.getRoot()) {
		this->gotoRoot();
		return false;
	} else {
		return true;
	}
}

// Translate //////////////////////////////////////////////////////////////////

void GLWorldState::translateSelectedXPos()
{
	auto node = this->getCurrentNode();
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
	auto node = this->getCurrentNode();
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
	auto node = this->getCurrentNode();
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
	auto node = this->getCurrentNode();
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
	auto node = this->getCurrentNode();
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
	auto node = this->getCurrentNode();
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
	auto node = this->getCurrentNode();
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
	auto node = this->getCurrentNode();
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
	auto node = this->getCurrentNode();
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
	auto node = this->getCurrentNode();
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
	auto node = this->getCurrentNode();
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
	auto node = this->getCurrentNode();
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
	auto node = this->getCurrentNode();
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
	auto node = this->getCurrentNode();
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
	auto node = this->getCurrentNode();
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
	auto node = this->getCurrentNode();
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
	auto node = this->getCurrentNode();
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
	auto node = this->getCurrentNode();
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

/******************************************************************************/
