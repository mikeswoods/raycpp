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
#include "Graph.h"

/******************************************************************************/

 class GraphBuilder
{
    protected:
        GraphNode* root;
        Graph graph;
        std::map<std::string,GraphNode*> nodeMap;

    public:
        GraphBuilder();
        GraphBuilder(const GraphBuilder& other);
        ~GraphBuilder();

        bool nodeExists(const std::string& name) const;
        GraphBuilder const * registerNode(GraphNode* node);
        GraphNode const * getNode(const std::string& name) const;
        GraphBuilder const * linkNodes(const std::string& parentName, GraphNode* child);
        GraphBuilder const * linkNodes(GraphNode* parent, GraphNode* child);
        GraphBuilder const * setRoot(GraphNode* root);
};

/******************************************************************************/

#endif
