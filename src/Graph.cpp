#include <algorithm>
#include <iterator>
#include <string>
#include "Graph.h"
#include "AreaLight.h"

using namespace std;

/*****************************************************************************/

GraphNode::GraphNode(const string& name)
{
	this->name     = name;
	this->parent   = nullptr;
	this->instance = nullptr;
	this->material = nullptr;
	this->geometry = nullptr;
	this->T        = glm::vec3(0.0f, 0.0f, 0.0f);
	this->R        = glm::vec3(0.0f, 0.0f, 0.0f);
	this->S        = glm::vec3(1.0f, 1.0f, 1.0f);
}

GraphNode::GraphNode(const GraphNode& other)
{
	this->name     = other.name;
	this->parent   = other.parent;
	this->instance = other.instance;
	this->material = other.material;
	this->geometry = other.geometry;
	this->T        = other.T;
	this->R        = other.R;
	this->S        = other.S;

	// Copy the children list:
	copy(other.children.begin(), other.children.end(), back_inserter(this->children));
}

GraphNode::~GraphNode() 
{

}

void GraphNode::detachChild(GraphNode* child)
{
	this->children.remove(child);
}

void GraphNode::detachFromParent()
{
	if (this->parent != nullptr) {
		this->parent->detachChild(this);
		//this->parent = nullptr;
	}
}

/**
 * Tests if the node is considered an area light:
 */
bool GraphNode::isAreaLight() const
{
	Material const * mat = this->getMaterial();

	if (mat == nullptr) {
		return false;
	}

	return mat->isEmissive();
}

ostream& operator<<(ostream& os, const GraphNode& node)
{
	os  << "Node { \"" << node.name << "\", material = ";
	if (node.material == nullptr) {
		os << "<null>";
	} else {
		os << *(node.material);
	}
	os << ", geometry = " << node.geometry << "}";
	return os;
}

ostream& operator<<(ostream& os, const glm::mat4 M)
{
	os << "[ ";
	for (int i=0; i<4; i++) {
		for (int j=0; j<4; j++) {
			os << M[i][j] << " ";
		}
		os << "\\";
	}
	os << " ]"; 
	return os;
}

// Based on the parameters of the given node, this function will apply 
// the given transformation matrix T, yielding a new transformation matrix T'

glm::mat4 applyTransform(GraphNode* node, glm::mat4 current)
{
	glm::vec3 T = node->getTranslate();
	glm::vec3 R = node->getRotate();
	glm::vec3 S = node->getScale();

	// Assume the "identity" matrix we are using is centered about 
	// node->getCenter() initially:
	glm::mat4 I  = glm::translate(glm::mat4(), toVec3(node->getCenter()));

	float xAngle = R[0];
	float yAngle = R[1];
	float zAngle = R[2];

	// Construct the rotation matrix about XYZ in that order
	glm::mat4 RM = glm::rotate(I, xAngle, glm::vec3(1.0f, 0.0f, 0.0f));
	RM           = glm::rotate(RM, yAngle, glm::vec3(0.0f, 1.0f, 0.0f));
	RM           = glm::rotate(RM, zAngle, glm::vec3(0.0f, 0.0f, 1.0f));

	// Multiply in reverse order to actuall apply in the desired order of
	// transformation: scale => rotate-X => rotate-Y => rotate-Z => translate
	return current * (glm::translate(I, T) * RM * glm::scale(I, S));
}

/*****************************************************************************/

Graph::Graph()
{ 
	this->root = nullptr;
}

Graph::Graph(GraphNode* root)
{ 
	this->root = root;
}

/*****************************************************************************/

/**
 * Accumulator function used to fold over the scene graph collecting
 * all graph nodes that are emissive by getAllAreaLights()
 */
static LightsAndMatrix collectAreaLight(GraphNode* node, LightsAndMatrix current)
{
	glm::mat4 nextT = applyTransform(node, current.second);

	// Found a node with an emissive material assigned to it:
	if (node->getMaterial() != nullptr && node->getMaterial()->isEmissive()) {

		list<Light*>* areaLights = current.first;

		areaLights->push_back(new AreaLight(node, nextT));
	}

	return make_pair(current.first, nextT);
}

/**
 * Dummy visit function used by getAllAreaLights()
 */
static LightsAndMatrix returnCurrent(LightsAndMatrix current, LightsAndMatrix last)
{
	return current;
}

void Graph::addAreaLights(std::list<Light*>* lights) const
{
	glm::mat4 I                          = glm::mat4();
	pair<list<Light*>*, glm::mat4> init  = make_pair(lights, I);
	//pair<list<Light*>*, glm::mat4> final = fold(*this, collectAreaLight, returnCurrent, init);
	fold(*this, collectAreaLight, returnCurrent, init);
}

/*****************************************************************************/

static ostream* walkAndPrint(GraphNode* node, ostream* os, int depth)
{
	for (int i=0; i<(2*depth); i++) {
		(*os) << "-";
	}
	(*os) << *node << endl;
	return os;
}

ostream& operator<<(ostream& os, const Graph& graph)
{
	walk(graph, walkAndPrint, &os);
	return os;
}

/*****************************************************************************/
