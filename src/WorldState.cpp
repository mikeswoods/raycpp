#define _USE_MATH_DEFINES
#include <cmath>
#include "WorldState.h"

////////////////////////////////////////////////////////////////////////////////

const glm::vec3 WorldState::DEFAULT_LIGHT_POSITION = glm::vec3(0.0f, 9.0f, 0.0f);

WorldState::WorldState(const Configuration& _config) :
	config(_config)
{ 
	this->flagCycleLightHue = false;
	this->flagRotateScene   = false;
	this->polyModeIndex     = 0;
	this->envMap            = nullptr;
	this->iterator          = new Graph::pre_iterator(this->config.getSceneGraph(), true);
	this->previewLight      = PointLight(DEFAULT_LIGHT_POSITION, Color::WHITE);
	this->globalLightHue    = 0.0f;
}

WorldState::~WorldState() 
{ 
	delete this->iterator;
}

// Node traversal operations //////////////////////////////////////////////////

GraphNode* WorldState::gotoRoot()
{
	return this->iterator->reset();
}

GraphNode* WorldState::getCurrentNode()
{
	return this->iterator->current();
}

GraphNode* WorldState::getNextNode()
{
	return this->iterator->next();
}

// Operations /////////////////////////////////////////////////////////////////

// Rotation operations:
void WorldState::toggleRotateScene()
{
	this->flagRotateScene = !this->flagRotateScene; 
}

bool WorldState::doRotateScene()
{
	return this->flagRotateScene;
}

// Select the active node:
void WorldState::highlightNextNode()
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
void WorldState::switchPolygonMode()
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

	for (auto i = this->config.getSceneGraph().begin(); !i.done(); i++) {
		GLGeometry* instance = (*i)->getInstance();
		if (instance != nullptr) {
			instance->setPolyMode(useMode);
		}
	}
}

// used in WorldState::deleteSelectedNode()
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
bool WorldState::deleteSelectedNode()
{
	Graph graph = this->config.getSceneGraph();

	postWalk(this->getCurrentNode()
		    ,_deleteNode
			,&graph);
	
	if (graph.getRoot() != nullptr) {
		this->gotoRoot();
		return false;
	} else {
		return true;
	}
}

// Translate //////////////////////////////////////////////////////////////////

void WorldState::translateSelectedXPos()
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

void WorldState::translateSelectedXNeg()
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

void WorldState::translateSelectedYPos()
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

void WorldState::translateSelectedYNeg()
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

void WorldState::translateSelectedZPos()
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

void WorldState::translateSelectedZNeg()
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

void WorldState::rotateSelectedPosX()
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

void WorldState::rotateSelectedNegX()
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

void WorldState::rotateSelectedPosY()
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

void WorldState::rotateSelectedNegY()
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

void WorldState::rotateSelectedPosZ()
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

void WorldState::rotateSelectedNegZ()
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

void WorldState::scaleIncreaseSelectedX()
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

void WorldState::scaleDecreaseSelectedX()
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

void WorldState::scaleIncreaseSelectedY()
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

void WorldState::scaleDecreaseSelectedY()
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

void WorldState::scaleIncreaseSelectedZ()
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

void WorldState::scaleDecreaseSelectedZ()
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

void WorldState::toggleLightHueChange()
{
	this->flagCycleLightHue = !this->flagCycleLightHue;
}

bool WorldState::doLightHueChange()
{
	return this->flagCycleLightHue;
}

void WorldState::translateLightPosX()
{
	previewLight.translateX(TRANSLATE_BY_UNIT);
}

void WorldState::translateLightNegX()
{
	previewLight.translateX(-TRANSLATE_BY_UNIT);
}

void WorldState::translateLightPosY()
{
	previewLight.translateY(TRANSLATE_BY_UNIT);
}

void WorldState::translateLightNegY()
{
	previewLight.translateY(-TRANSLATE_BY_UNIT);
}

void WorldState::translateLightPosZ()
{
	previewLight.translateZ(TRANSLATE_BY_UNIT);
}

void WorldState::translateLightNegZ()
{
	previewLight.translateZ(-TRANSLATE_BY_UNIT);
}

void WorldState::shiftGlobalLightHue()
{
	PointLight previewLight = this->getPreviewLight();
	this->globalLightHue    = fmod(globalLightHue + HUE_UNIT, 360.0f);
	previewLight.setColor(Color::fromHSV(this->globalLightHue, 0.75f, 1.0f));
}

///////////////////////////////////////////////////////////////////////////////
