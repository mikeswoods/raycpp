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
#include <iterator>
#include "GraphBuilder.h"

/******************************************************************************/

using namespace std;

/******************************************************************************/

GraphBuilder::GraphBuilder() :
    root(nullptr)
{

}

GraphBuilder::GraphBuilder(const GraphBuilder& other) :
    root(other.root)
{
    this->nodeMap.insert(other.nodeMap.begin(), other.nodeMap.end());
}

GraphBuilder::~GraphBuilder()
{
    this->nodeMap.clear();
}

GraphBuilder const * GraphBuilder::registerNode(shared_ptr<GraphNode> node)
{
    this->nodeMap[node->getName()] = node;
    return this;
}

shared_ptr<GraphNode> GraphBuilder::getNode(const string& name) const
{
    return this->nodeMap.at(name);
}

bool GraphBuilder::nodeExists(const string& name) const
{
    return this->nodeMap.find(name) != this->nodeMap.end();
}

GraphBuilder const * GraphBuilder::linkNodes(const string& parentName, shared_ptr<GraphNode> child)
{
    if (!this->nodeExists(parentName)) {
        throw runtime_error("linkNodes: Parent node does not exist: " + parentName);
    }

    this->linkNodes(this->nodeMap.at(parentName), child);
    return this;
}

GraphBuilder const * GraphBuilder::linkNodes(shared_ptr<GraphNode> parent, shared_ptr<GraphNode> child)
{
    if (!parent) {
        throw runtime_error("linkNodes: parent cannot be null");
    }

    if (!child) {
        throw runtime_error("linkNodes: child cannot be null");
    }

    child->setParent(parent);
    parent->addChild(child);

    return this;
}

GraphBuilder const * GraphBuilder::setRoot(shared_ptr<GraphNode> root)
{
    this->root = root;
    return this;
}

Graph GraphBuilder::build() const
{
    return Graph(this->root);
}

/******************************************************************************/
