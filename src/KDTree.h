#ifndef KDTREE_H
#define KDTREE_H

#include <ctime>
#include <iostream>
#include <vector>
#include <utility>
#include "Ray.h"
#include "Tri.h"
#include "AABB.h"

/******************************************************************************
 *
 * K dimensional tree implementation for spatial indexing an intersection
 * test acceleration
 *
 * @file KDTree.h
 * @author Michael Woods
 *****************************************************************************/

// Safety measure to prevent the stack + heap from blowing up
#define DEEPEST_DEPTH_ALLOWED 45

// Forward declarations
class NodeChild;

///////////////////////////////////////////////////////////////////////////////
// Define various splitting strategies
///////////////////////////////////////////////////////////////////////////////

/**
 * Abstract splitting strategy base class
 */
class SplitStrategy
{
	public:
		SplitStrategy() { }
		virtual ~SplitStrategy() { }

		// Returns the name of this splitting strategy
		virtual std::string getName() const = 0;

		// Returns the axis to split on
		virtual int nextAxis(const std::vector<Tri>& _data) = 0;

		// Divide the splitter state into two new instances for
		// the left and right subtrees to recurse on
		virtual std::pair<SplitStrategy*, SplitStrategy*> divide() const = 0;
};

/**
 * CycleAxisStrategy -- Basic axis-cycling strategy: 0->1,1->2,2->0
 */
class CycleAxisStrategy : public SplitStrategy
{
	protected:
		int axis;

	public:
		CycleAxisStrategy(int _axis = 0);
		CycleAxisStrategy(const CycleAxisStrategy& other);
		virtual ~CycleAxisStrategy() { }

		virtual std::string getName() const;

		virtual int nextAxis(const std::vector<Tri>& _data);
		
		virtual std::pair<SplitStrategy*, SplitStrategy*> divide() const;
};

/**
 * RandomAxisStrategy -- choose an axis at random
 */
class RandomAxisStrategy : public SplitStrategy
{
	public:
		RandomAxisStrategy() { }
		RandomAxisStrategy(const RandomAxisStrategy& other);
		virtual ~RandomAxisStrategy() { }

		virtual std::string getName() const;

		virtual int nextAxis(const std::vector<Tri>& _data);
		
		virtual std::pair<SplitStrategy*, SplitStrategy*> divide() const;
};

/**
 * SurfaceAreaStrategy -- Chooses the axis with the least surface area cost
 */
class SurfaceAreaStrategy : public SplitStrategy
{
	public:
		SurfaceAreaStrategy() { }
		SurfaceAreaStrategy(const SurfaceAreaStrategy& other);
		virtual ~SurfaceAreaStrategy() { }

		virtual std::string getName() const;

		virtual int nextAxis(const std::vector<Tri>& _data);
		
		virtual std::pair<SplitStrategy*, SplitStrategy*> divide() const;
};

///////////////////////////////////////////////////////////////////////////////
// Define various leaf storage strategies
///////////////////////////////////////////////////////////////////////////////

/**
 * Basic storage strategy
 */
class StorageStrategy
{
	public:

		virtual ~StorageStrategy() { }

		// Test if the tree building step should stop for a given subtree
		// based on the provided parameters: tree depth and value count
		virtual bool done(int depth, int count) const = 0 ;
};

/**
 * Keep building until a maximum depth is reached:
 */
class MaxTreeDepth : public StorageStrategy
{
	protected:
		int maxDepth;

	public:
		MaxTreeDepth(int _maxDepth) : maxDepth(_maxDepth) { }
		virtual ~MaxTreeDepth() { }

		virtual bool done(int depth, int count) const
		{
			return depth >= this->maxDepth || depth >= DEEPEST_DEPTH_ALLOWED;
		}
};

/**
 * Keep building until there are fewer than k children per leaf
 */
class MaxValuesPerLeaf : public StorageStrategy
{
	protected:
		int maxCount;

	public:
		MaxValuesPerLeaf(int _maxCount) : maxCount(_maxCount) { }
		virtual ~MaxValuesPerLeaf() { }

		virtual bool done(int depth, int count) const
		{
			return count <= this->maxCount || depth >= DEEPEST_DEPTH_ALLOWED;
		}
};

///////////////////////////////////////////////////////////////////////////////

/**
 * Tree leaves contain the actual data to be looked up
 */
class Leaf
{
	protected:
		std::vector<Tri> data;
		AABB aabb;
		int depth;

	public:
		Leaf(const std::vector<Tri>& _data, const AABB& extent, int depth);
		Leaf(const Leaf& other);

		const std::vector<Tri>& getData() const { return this->data; }
		const AABB& getAABB() const             { return this->aabb; }
		int getCount() const                    { return this->data.size(); }
		int getDepth() const                    { return this->depth; }

		std::ostream& repr(std::ostream& s
			              ,void (*annotate)(std::ostream& s, Leaf const * leaf, void* data) = NULL
						  ,void* data = NULL) const;
};

std::ostream& operator<<(std::ostream& s, const Leaf& leaf);

/**
 * Nodes contain two children: a right and left. Each child can either be
 * a terminal Leaf or another non-terminal Node. additionally, Node instances
 * contain bookkeeping data
 */
class Node
{
	protected:
		NodeChild const * left;
		NodeChild const * right;
		AABB aabb;
		int depth;
		int axis; // Axis ordinal [0-2]: 0 = X, 1 = Y, 2 = Z

	public:
		Node(NodeChild const * left, NodeChild const * right, const AABB& aabb, int depth, int axis);
		Node(const Node& other);

		NodeChild const * getLeftChild() const  { return this->left; }
		NodeChild const * getRightChild() const { return this->right; }
		const AABB& getAABB() const             { return this->aabb; }
		int getDepth() const                    { return this->depth; }
		int getAxis() const                     { return this->axis; }

		std::ostream& repr(std::ostream& s
			              ,void (*annotateNode)(std::ostream& s, Node const * node, void* data) = NULL
						  ,void (*annotateLeaf)(std::ostream& s, Leaf const * leaf, void* data) = NULL
						  ,void* data = NULL) const;
};

std::ostream& operator<<(std::ostream& s, const Node& node);

/**
 * A simple tagged union type for representing a child in KD-tree. 
 * This used to more-or-less (mostly less) approximate an algebraic
 * data type KDTree, if it were defined in Haskell:
 *
 *   data KDTree = Node KDTree KDTree | Leaf [Tri]
 */
class NodeChild
{
	public:
		// Used for identifying the type in a tree node tagged union datatype
		enum NodeType 
		{
			 LEAF // Terminal
			,NODE // Non-terminal
		};

	protected:
		// Used to disambiguate the union data below:
		NodeType type;

		// Child data
		union {
			Leaf const * leaf;
			Node const * node;
		} data;

	public:
		NodeChild(Leaf const * leaf) :
			 type(LEAF)
		{
			this->data.leaf = leaf;
		}

		NodeChild(Node const * node) :
			 type(NODE)
		{
			this->data.node = node;
		}

		NodeType getType() const { return this->type; }

		// Are we at a leaf node?
		bool isLeaf() const { return this->type == LEAF; }

		// Return the child as a leaf:
		Leaf const * asLeaf() const { return this->data.leaf; }

		// Are we at a junction i.e. a node with children? 
		bool isNode() const { return this->type == NODE; }

		// Return the child as a node:
		Node const * asNode() const { return this->data.node; }

		std::ostream& repr(std::ostream& s
						  ,void (*annotateNode)(std::ostream& s, Node const * node, void* data) = NULL
	                      ,void (*annotateLeaf)(std::ostream& s, Leaf const * Leaf, void* data) = NULL
						  ,void* data = NULL) const;
};

std::ostream& operator<<(std::ostream& s, const NodeChild& child);

/**
 * The tree container class itself. In addition to the tree root, it
 * contains summary information about the tree itself
 */
class KDTree
{
	private:
		// Build time in milliseconds:
		int msBuildTime;

		// Recursive helper to count the number of primitives per leaf node:
		int countInNode(NodeChild const * root) const ;

		// Recursive helper function for KDTree::intersects
		bool intersectWalk(const Ray& ray, NodeChild const * root, std::vector<Tri>& tris) const;

	protected:
		// Root of the KD-tree
		NodeChild const * root;

	public:
		KDTree(const std::vector<Tri>& data, SplitStrategy* splitStrategy, StorageStrategy* storageStrategy);

		// Count the number of primitives/triangles indexed in the tree
		int count() const { return this->countInNode(this->root); }

		// Tests if the given ray intersects the KD-tree, returning any triangles
		// that are potentially intersected, collecting the resulting triangle
		// instances into the supplied vector
		bool intersects(const Ray& ray, std::vector<Tri>& tris);

		// Get the build time in milliseconds
		int getBuildTime() const { return this->msBuildTime; }

		std::ostream& repr(std::ostream& s
	                      ,void (*annotateTree)(std::ostream& s, KDTree const * tree, void* data) = NULL
						  ,void (*annotateNode)(std::ostream& s, Node const * node, void* data) = NULL
						  ,void (*annotateLeaf)(std::ostream& s, Leaf const * leaf, void* data) = NULL
						  ,void* data = NULL) const;

		// Summarization function
		friend void generateSummary(KDTree const * tree
			                       ,const std::string& name
								   ,std::ostream& out);
};

std::ostream& operator<<(std::ostream& s, const KDTree& tree);

///////////////////////////////////////////////////////////////////////////////

void debugIntersectAll(std::ostream& s, KDTree const * tree, const Ray& ray);

///////////////////////////////////////////////////////////////////////////////

#endif
