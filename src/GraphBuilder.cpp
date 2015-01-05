/******************************************************************************
 *
 * This class defines the functionality needed to construct a scene graph
 * with individual scene graph nodes
 *
 * @file GraphBuilder.h
 * @author Michael Woods
 *
 ******************************************************************************/

#include <stdexcept>
#include "GraphBuilder.h"

using namespace std;

/******************************************************************************/

GraphBuilder::GraphBuilder() :
    root(nullptr)
{

}

GraphBuilder::GraphBuilder(const GraphBuilder& other) :
    root(other.root),
    nodeMap(other.nodeMap)
{

}

GraphBuilder::~GraphBuilder()
{
    this->nodeMap.clear();
}

GraphBuilder const * GraphBuilder::registerNode(GraphNode* node)
{
    this->nodeMap[node->getName()] = node;
    return this;
}

GraphNode const * GraphBuilder::getNode(const string& name) const
{
    return this->nodeMap.at(name);
}

bool GraphBuilder::nodeExists(const string& name) const
{
    return this->nodeMap.find(name) != this->nodeMap.end();
}

GraphBuilder const * GraphBuilder::linkNodes(const string& parentName, GraphNode* child)
{
    if (!this->nodeExists(parentName)) {
        throw runtime_error("linkNodes: Parent node does not exist: " + parentName);
    }

    this->linkNodes(this->nodeMap.at(parentName), child);
    return this;
}

GraphBuilder const * GraphBuilder::linkNodes(GraphNode* parent, GraphNode* child)
{
    if (parent == nullptr) {
        throw runtime_error("linkNodes: parent cannot be null");
    }
    if (child == nullptr) {
        throw runtime_error("linkNodes: child cannot be null");
    }

    child->setParent(parent);
    parent->addChild(child);

    return this;
}

GraphBuilder const * GraphBuilder::setRoot(GraphNode* root)
{
    this->root = root;
    return this;
}

/******************************************************************************/
