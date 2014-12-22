/******************************************************************************
 *
 * This file defines the structure of the renderer's general object scene 
 * graph representation, as well as operations defined to manipulate the 
 * scene graph
 *
 * @file Graph.h
 * @author Michael Woods
 *
 ******************************************************************************/

#ifndef GRAPH_H
#define GRAPH_H

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <list>
#include <stack>
#include <queue>
#include "R3.h"
#include "Color.h"
#include "Material.h"
#include "Geometry.h"
#include "GLGeometry.h"
#include "Light.h"

/*****************************************************************************/

class GraphNode
{
	protected:
		// Node name
		std::string name;

		// Pointer to parent, if any. If NULL, node is the root of the graph
		GraphNode* parent;

		// The geometric object definition itself
		Geometry const * geometry;

		// GL wrapped instance of a geometric primitive
		GLGeometry* instance;

		// All child nodes of this node in the scene graph
		std::list<GraphNode*> children;

		// <x,y,z> translation values
		glm::vec3 T;

		// <x,y,z> rotation values
		glm::vec3 R;

		// <x,y,z> scaling values
		glm::vec3 S;

		// [x,y,z] center point
		P center;

		// Surface material
		Material* material;

	public:
		GraphNode(const std::string& name);
		GraphNode(const GraphNode& other);
		~GraphNode();

		bool isRoot() { return this->getParent() == NULL; }

		const std::string& getName() const    { return this->name; }
		void setName(const std::string& name) { this->name = name; }

		GraphNode* getParent() const      { return this->parent; }
		void setParent(GraphNode* parent) { this->parent = parent; }

		const std::list<GraphNode*>& getChildren() const { return this->children; }
		void addChild(GraphNode* child)                  { this->children.push_back(child); }

		void detachChild(GraphNode* child);
		void detachFromParent();

		bool isAreaLight() const;

		Geometry const * getGeometry() const        { return this->geometry; }
		void setGeometry(Geometry const * geometry) { this->geometry = geometry; }

		GLGeometry* getInstance() const        { return this->instance; }
		void setInstance(GLGeometry* instance) { this->instance = instance; }

		const glm::vec3& getTranslate() const       { return this->T; }
		void setTranslate(const glm::vec3& T)       { this->T = T; }
		void translateBy(float x, float y, float z) { this->T = glm::vec3(x, y, z); }
		void translateBy(float amt)                 { this->T += glm::vec3(amt, amt, amt); }
		void translateXBy(float amt)                { this->T[0] += amt; }
		void translateYBy(float amt)                { this->T[1] += amt; }
		void translateZBy(float amt)                { this->T[2] += amt; }

		const glm::vec3& getRotate() const       { return this->R; }
		void setRotate(const glm::vec3& R)       { this->R = R; }
		void rotateBy(float x, float y, float z) { this->R += glm::vec3(x, y, z); }
		void rotateBy(float amt)                 { this->R += glm::vec3(amt, amt, amt); }
		void rotateXBy(float amt)                { this->R[0] += amt; }
		void rotateYBy(float amt)                { this->R[1] += amt; }
		void rotateZBy(float amt)                { this->R[2] += amt; }

		const glm::vec3& getScale() const       { return this->S; }
		void setScale(const glm::vec3& S)       { this->S = S; }
		void scaleBy(float x, float y, float z) { this->S += glm::vec3(x, y, z); }
		void scaleBy(float amt)                 { this->S += glm::vec3(amt, amt, amt); }
		void scaleXBy(float amt)                { this->S[0] += amt; }
		void scaleYBy(float amt)                { this->S[1] += amt; }
		void scaleZBy(float amt)                { this->S[2] += amt; }

		const P& getCenter() const      { return this->center; }
		void setCenter(const P& center) { this->center = center; }

		Material* getMaterial() const        { return this->material; }
		void setMaterial(Material* material) { this->material = material; }

		friend std::ostream& operator<<(std::ostream& os, const GraphNode& node);
};

std::ostream& operator<<(std::ostream& os, const glm::mat4 M);

///////////////////////////////////////////////////////////////////////////////

// Based on the parameters of the given node, this function will apply 
// the given transformation matrix T, yielding a new transformation matrix T'

glm::mat4 applyTransform(GraphNode* node, glm::mat4 current);

///////////////////////////////////////////////////////////////////////////////

/**
 * Shorthand for data returned from collectAreaLight and returnCurrent
 */
typedef std::pair<std::list<Light*>*, glm::mat4> LightsAndMatrix;

class Graph
{
	protected:
		// Root of the graph to traverse
		GraphNode* root;

	public:

		// Abstract tree iterator implementation //////////////////////////////
		// Adapted from http://www.dreamincode.net/forums/topic/58468-making-your-own-iterators/
		class iterator 
		{
			protected:
				// Cyclic iteration mode flag
				bool cyclic;

				// Starting node of the traversal
				GraphNode* start;

				// Interal node stack
				std::stack<GraphNode*> st;

			public:
				iterator(const Graph& graph, bool _cyclic = false) : 
					cyclic(_cyclic),
					start(graph.getRoot())
				{ 
					this->reset();
				}

				iterator(GraphNode* _start, bool _cyclic = false) : 
					cyclic(_cyclic),
					start(_start)
				{ 
					this->reset();
				}

				iterator(const iterator& other) : 
					cyclic(other.cyclic),
					start(other.start),
					st(other.st)
				{ }

				virtual ~iterator() { }

				// Tests if the iterator has reached the end
				bool done() { return this->cyclic ? false : this->st.empty(); }

				// Returns if the iterator is cyclic
				bool isCyclic() { return this->cyclic; }

				// Resets the iterator to the starting node
				GraphNode* reset()
				{
					this->st = std::stack<GraphNode*>();

					if (this->start != NULL) {
						this->st.push(this->start);
					}

					return this->start;
				}
				
				// If in cyclic mode, tests if the iterator should be reset, and
				// resets as needed
				void testAndReset()
				{
					if (this->cyclic && this->st.empty() && this->start != NULL) {
						this->reset();
					}
				}

				// Returns the current node in the traversal
				GraphNode* current()
				{
					this->testAndReset();

					return this->st.empty() ? NULL : this->st.top();
				}

				// Advances the iterator, returning true if elements
				// remain in the traversal
				virtual GraphNode* next() = 0;
				
				// Same as current()
				GraphNode* operator*() { return this->current(); }

				// Iterator equality
				bool operator==(iterator& other)
				{
					return this->current() == other.current();
				}

				// Iterator inequality
				bool operator!=(iterator& other)
				{
					return !this->operator==(other);
				}

				// Advance the iterator
				iterator& operator++() 
				{ 
					this->next();
					return *this;
				}

				// Advance the iterator
                iterator& operator++(int unused)
				{ 
					this->next();
					return *this;
				}
		};

		// Pre-order traversal iterator:
		class pre_iterator : public iterator
		{
			public:
				pre_iterator(GraphNode* start, bool cyclic = false) : 
					iterator(start, cyclic)
				{ }

				pre_iterator(const Graph& graph, bool _cyclic = false) : 
					iterator(graph, _cyclic)
				{ }

				virtual GraphNode* next()
				{
					this->testAndReset();

					if (this->st.empty()) {
						return NULL;
					}

					std::list<GraphNode*> children = this->st.top()->getChildren();
					this->st.pop();

					for (std::list<GraphNode*>::reverse_iterator i=children.rbegin()
						; i != children.rend()
						; i++)
					{
						this->st.push(*i);
					}

					this->testAndReset();

					return this->st.empty() ? NULL : this->st.top();
				}
		};

		Graph();
		Graph(GraphNode* root);

		GraphNode* getRoot() const    { return this->root; }
		void setRoot(GraphNode* root) { this->root = root; }

		// Collect all of the area lights in the scene graph
		void addAreaLights(std::list<Light*>* lights) const;

		pre_iterator begin() const { return pre_iterator(this->getRoot(), false); }

		friend std::ostream& operator<<(std::ostream& os, const Graph& graph);
};

/////////////////////////////////////////////////////////////////////////////////
// Scene graph tree traversal
/////////////////////////////////////////////////////////////////////////////////

/**
 * Perform a pre-order traversal over a scene graph w/o accumulation
 */
template<typename T> 
void walk(const Graph& graph
	     ,T (*visit)(GraphNode* node,  T current, int depth)
		 ,T initial
		 ,int depth = 0)
{
	 walk(graph.getRoot(), visit, initial, depth);
}

/**
 * Perform a pre-order traversal over a scene graph w/o accumulation
 */
template<typename T>
void walk(GraphNode* root // The graph root
	     ,T (*visit)(GraphNode* node,  T current, int depth)
		 ,T initial       // The initial value to begin accumulating from
		 ,int depth = 0)  // The initial depth
{
	T next = visit(root, initial, depth);

	std::list<GraphNode*> children = root->getChildren();

	for (std::list<GraphNode*>::const_iterator i=children.begin(); i != children.end(); i++) {
		walk(*i, visit, next, depth+1);
	}
}


/**
 * Perform a pre-order traversal over a scene graph with accumulation
 */
template<typename T>
T fold(const Graph& graph                      // The graph to traverse
	  ,T (*visit)(GraphNode* node,  T current) // Visit function: Takes a graph node and the current value, producing a new value
      ,T (*accum)(T current, T total)          // Accumulate function: Takes a current value and a sum value and returns the new sum value
	  ,T initial)                              // The initial value to begin accumulating from
{
	return fold(graph.getRoot(), visit, accum, initial);
}


/**
 * Perform a pre-order traversal over a scene graph with accumulation
 */
template<typename T>
T fold(GraphNode* root                         // The graph root
	  ,T (*visit)(GraphNode* node,  T current) // Visit function: Takes a graph node and the current value, producing a new value
      ,T (*accum)(T current, T total)          // Accumulate function: Takes a current value and a sum value and returns the new sum value
	  ,T initial)                              // The initial value to begin accumulating from
{
	T next  = visit(root, initial);
	T total = accum(next, initial);

	std::list<GraphNode*> children = root->getChildren();

	for (std::list<GraphNode*>::const_iterator i=children.begin(); i != children.end(); i++) {
		total = accum(fold(*i, visit, accum, next), total);
	}

	return total;
}

// Perform a post-order traversal over a scene graph
template<typename T>
void postWalk(const Graph& graph, void (*visit)(GraphNode* node,  T current), T context)
{
	 postWalk(graph.getRoot(), visit, context);
}

// Perform a post-order traversal over a scene graph
template<typename T>
void postWalk(GraphNode* root, void (*visit)(GraphNode* node,  T context), T context)
{
	std::list<GraphNode*> children = root->getChildren();

	for (std::list<GraphNode*>::const_iterator i=children.begin(); i != children.end(); i++) {
		postWalk(*i, visit, context);
	}

	visit(root, context);
}

/*****************************************************************************/

#endif
