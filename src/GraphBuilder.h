/******************************************************************************
 *
 * This class defines the functionality needed to construct a scene graph
 * with individual scene graph nodes
 *
 * @file GraphBuilder.h
 * @author Michael Woods
 *
 ******************************************************************************/

#ifndef GRAPH_BUILDER_H
#define GRAPH_BUILDER_H

#include <map>
#include <memory>
#include "Graph.h"

/******************************************************************************/

 class GraphBuilder
{
    protected:
        std::shared_ptr<GraphNode> root;
        Graph graph;
        std::map<std::string, std::shared_ptr<GraphNode>> nodeMap;

    public:
        GraphBuilder();
        GraphBuilder(const GraphBuilder& other);
        ~GraphBuilder();

        bool nodeExists(const std::string& name) const;
        GraphBuilder const * registerNode(std::shared_ptr<GraphNode> node);
        std::shared_ptr<GraphNode> getNode(const std::string& name) const;
        GraphBuilder const * linkNodes(const std::string& parentName, std::shared_ptr<GraphNode> child);
        GraphBuilder const * linkNodes(std::shared_ptr<GraphNode> parent, std::shared_ptr<GraphNode> child);
        GraphBuilder const * setRoot(std::shared_ptr<GraphNode> root);
        Graph build() const;
};

/******************************************************************************/

#endif
