#include <algorithm>
#include <iterator>
#include <tuple>
#include <string>
#include "Graph.h"
#include "AreaLight.h"
#include "SceneContext.h"

/******************************************************************************/

using namespace std;
using namespace glm;

/******************************************************************************/

GraphNode::GraphNode(const string& _name) :
	name(_name),
	parent(nullptr),
	geometry(shared_ptr<Geometry>(nullptr)),
	instance(nullptr),
	material(nullptr),
	T(vec3(0.0f, 0.0f, 0.0f)),
	R(vec3(0.0f, 0.0f, 0.0f)),
	S(vec3(1.0f, 1.0f, 1.0f))
{

}

GraphNode::GraphNode(const GraphNode& other) :
	name(other.name),
	parent(other.parent),
	geometry(other.geometry),
	instance(other.instance),
	material(other.material),
	T(other.T),
	R(other.R),
	S(other.S)
{
	// Copy the children list:
	copy(other.children.begin(), other.children.end(), back_inserter(this->children));
}

GraphNode::~GraphNode() 
{

}

void GraphNode::detachChild(shared_ptr<GraphNode> child)
{
	this->children.remove(child);
}

void GraphNode::detachFromParent()
{
	if (this->parent) {
		this->parent->detachChild(shared_ptr<GraphNode>(this));
	}
}

/**
 * Tests if the node is considered an area light:
 */
bool GraphNode::isAreaLight() const
{
	shared_ptr<Material> mat = this->getMaterial();

	return !mat ? false : mat->isEmissive();
}

ostream& operator<<(ostream& os, const GraphNode& node)
{
	os  << "Node { \"" << node.name << "\", material = ";
	if (!node.material) {
		os << "<null>";
	} else {
		os << *(node.material);
	}
	os << ", geometry = " << node.geometry << "}";
	return os;
}

ostream& operator<<(ostream& os, const mat4 M)
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

mat4 applyTransform(shared_ptr<GraphNode> node, mat4 current)
{
	vec3 T = node->getTranslate();
	vec3 R = node->getRotate();
	vec3 S = node->getScale();

	// Assume the "identity" matrix we are using is centered about 
	// node->getCenter() initially:
	mat4 I  = translate(mat4(), node->getCenter());

	float xAngle = R[0];
	float yAngle = R[1];
	float zAngle = R[2];

	// Construct the rotation matrix about XYZ in that order
	mat4 RM = rotate(I, xAngle, vec3(1.0f, 0.0f, 0.0f));
	RM           = rotate(RM, yAngle, vec3(0.0f, 1.0f, 0.0f));
	RM           = rotate(RM, zAngle, vec3(0.0f, 0.0f, 1.0f));

	// Multiply in reverse order to actuall apply in the desired order of
	// transformation: scale => rotate-X => rotate-Y => rotate-Z => translate
	return current * (translate(I, T) * RM * scale(I, S));
}

/******************************************************************************/

Graph::Graph()
{ 

}

Graph::Graph(shared_ptr<GraphNode> root)
{ 
	this->root = root;
}

/******************************************************************************/

/**
 * Accumulator function used to fold over the scene graph collecting
 * all graph nodes that are emissive by getAllAreaLights()
 */
static pair<LIGHTS*, mat4> collectAreaLight(std::shared_ptr<GraphNode> node, pair<LIGHTS*, mat4> current)
{
	mat4 nextT = applyTransform(node, current.second);

	// Found a node with an emissive material assigned to it:
	if (node->getMaterial() && node->getMaterial()->isEmissive()) {

		auto areaLights = current.first;
		areaLights->push_back(make_shared<AreaLight>(node, nextT));
	}

	return make_pair(current.first, nextT);
}

/**
 * Dummy visit function used by getAllAreaLights()
 */
static pair<LIGHTS*, mat4> returnCurrent(pair<LIGHTS*, mat4> current, pair<LIGHTS*, mat4> last)
{
	return current;
}

/**
 * Gather all of the objects that are emissive, returning them in a list
 * as AreaLight instances
 */
unique_ptr<LIGHTS> Graph::areaLights() const
{
	auto lights = new list<shared_ptr<Light>>();
	mat4 I      = mat4();
	auto init   = make_pair(lights, I);

	fold(*this, collectAreaLight, returnCurrent, init);

	return unique_ptr<list<shared_ptr<Light>>>(lights);
}

/******************************************************************************/

static ostream* walkAndPrint(std::shared_ptr<GraphNode> node, ostream* os, int depth)
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

/******************************************************************************/
