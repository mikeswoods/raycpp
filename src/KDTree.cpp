/*******************************************************************************
 *
 * K dimensional tree implementation for spatial indexing an intersection
 * test acceleration
 *
 * @file KDTree.h
 * @author Michael Woods
 *
 ******************************************************************************/

#define _USE_MATH_DEFINES
#include <cmath>
#include <cassert>
#include <cstring>
#include <memory>
#include <algorithm>
#include <iterator>
#include <utility>
#include <limits>
#include <queue>
#include "KDTree.h"
#include "Utils.h"

/******************************************************************************/

using namespace std;

/******************************************************************************/

static AABB findExtent(const std::vector<Tri>& triangles);

static NodeChild const * build(const std::vector<Tri>& triangles
	                          ,int currentDepth
			 	              ,unique_ptr<SplitStrategy> splitHalf
					          ,StorageStrategy* storageStrategy);

/*******************************************************************************
 * CycleAxisStrategy -- Basic axis-cycling strategy: 0->1,1->2,2->0
 ******************************************************************************/

int CycleAxisStrategy::nextAxis(const std::vector<Tri>& _data)
{
	// Ignore data and cycle the axis
	int currentAxis = this->axis;
	this->axis = (this->axis + 1)  % 3; // For 3 dimensions
	return currentAxis;
}
		
Split CycleAxisStrategy::divide() const
{
	return Split(unique_ptr<SplitStrategy>(new CycleAxisStrategy(*this))
		        ,unique_ptr<SplitStrategy>(new CycleAxisStrategy(*this)));
}

/*******************************************************************************
 * RandomAxisStrategy -- choose an axis at random
 ******************************************************************************/

int RandomAxisStrategy::nextAxis(const std::vector<Tri>& data)
{
	return static_cast<int>(Utils::randInRange(0, 2));
}

Split RandomAxisStrategy::divide() const
{
	return Split(unique_ptr<SplitStrategy>(new RandomAxisStrategy(*this))
		        ,unique_ptr<SplitStrategy>(new RandomAxisStrategy(*this)));
}

/*******************************************************************************
 * SurfaceAreaStrategy -- Chooses the axis with the least surface area cost
 * Adapted from http://www.flipcode.com/archives/Raytracing_Topics_Techniques-Part_7_Kd-Trees_and_More_Speed.shtml
 ******************************************************************************/

int SurfaceAreaStrategy::nextAxis(const std::vector<Tri>& data)
{
	AABB totalExtent = findExtent(data);
	glm::vec3 center = totalExtent.centroid();
	float cost[3]    = { 0.0f, 0.0f, 0.0f };
	float inf        = numeric_limits<float>::infinity();

	// For each axis partition the points according to the current
	// axis and compute the size of each resulting bounding box
	for (int axis=0; axis<3; axis++) {

		float splitValue = center[axis];

		float xMinL = inf, xMinR  = inf;
		float xMaxL = -inf, xMaxR = -inf;
		float yMinL = inf, yMinR  = inf;
		float yMaxL = -inf, yMaxR = -inf;
		float zMinL = inf, zMinR  = inf;
		float zMaxL = -inf, zMaxR = -inf;
		int countL = 0, countR = 0;

		for (auto i=data.begin(); i != data.end(); i++) {
			
			Tri T               = *i;
			glm::vec3 triCenter = T.getAABB().centroid();

			// Partition the points based on the current axis and update the extrema
			// of the bounding box formed on each side of the partition:
			if (triCenter[axis] >= splitValue) {
				xMinR = min(xMinR, T.getXMinima());
				yMinR = min(yMinR, T.getYMinima());
				zMinR = min(zMinR, T.getZMinima());
				xMaxR = max(xMaxR, T.getXMaxima());
				yMaxR = max(yMaxR, T.getYMaxima());
				zMaxR = max(zMaxR, T.getZMaxima());
				countR++;
			} else {
				xMinL = min(xMinL, T.getXMinima());
				yMinL = min(yMinL, T.getYMinima());
				zMinL = min(zMinL, T.getZMinima());
				xMaxL = max(xMaxL, T.getXMaxima());
				yMaxL = max(yMaxL, T.getYMaxima());
				zMaxL = max(zMaxL, T.getZMaxima());
				countL++;
			}
		}

		AABB left  = AABB(glm::vec3(xMinL, yMinL, zMinL), glm::vec3(xMaxL, yMaxL, zMaxL));
		AABB right = AABB(glm::vec3(xMinR, yMinR, zMinR), glm::vec3(xMaxR, yMaxR, zMaxR));

		// Finally, compute the cost, defined as:
		// cost = #triangles(left) * area(left) + #triangles(right) * area(right)

		cost[axis] = (left.area() * static_cast<float>(countL)) + 
				     (right.area() * static_cast<float>(countR));
	}

	// Finally, examine the computed cost per axis and return the minimum:
	float minCost = inf;
	int minAxis   = -1;

	for (int axis=0; axis<3; axis++) {
		//cout << "axis = " << axis << ", cost = " << cost[axis] << endl;
		if (cost[axis] < minCost) {
			minCost = cost[axis];
			minAxis = axis;
		}
	}

	//assert (minAxis >= 0 && minAxis <= 2);
	// Finally, if there's no good choice, just pick 0
	if (minAxis == -1) {
		minAxis = 0;
	}

	return minAxis;
}
		
Split SurfaceAreaStrategy::divide() const
{
	return Split(unique_ptr<SplitStrategy>(new SurfaceAreaStrategy(*this))
		        ,unique_ptr<SplitStrategy>(new SurfaceAreaStrategy(*this)));
}

/******************************************************************************
 * KDTree :: Leaf
 *****************************************************************************/

Leaf::Leaf(const vector<Tri>& _data, const AABB& _aabb, int _depth) : 
	data(_data),
	aabb(_aabb),
	depth(_depth)
{ }

Leaf::Leaf(const Leaf& other) :
	data(other.data),
	aabb(other.aabb),
	depth(other.depth)
{ }

ostream& Leaf::repr(ostream& s
	               ,void (*annotate)(std::ostream& s, Leaf const * leaf, void* data)
				   ,void* data) const
{
	for (int i=0; i<this->depth; i++) {
		s << "  ";
	}

	s << "Leaf@" << this;
	if (annotate != nullptr) {
		s << "<";
		annotate(s, this, data);
		s << ">: ";
	}
	s << "{"
	  << " size="   << this->data.size()
	  << ", aabb="  << this->aabb
	  << ", depth=" << this->depth 
	  << " }";
	s << endl;

	/*
	// Print the actual leaf values:
	for (auto i=this->data.begin(); i != this->data.end(); i++) {
		for (int j=0; j<this->depth; j++) {
			s << "  ";
		}
		s << "  " << *i << endl;
	}
	*/

	return s;
}

ostream& operator<<(ostream& s, const Leaf& leaf)
{
	return leaf.repr(s);
}

/******************************************************************************
 * KDTree :: NodChild (internal)
 *****************************************************************************/

std::ostream& NodeChild::repr(std::ostream& s
							 ,void (*annotateNode)(std::ostream& s, Node const * node, void* data)
	                         ,void (*annotateLeaf)(std::ostream& s, Leaf const * Leaf, void* data)
							 ,void* data) const
{
	if (this->isLeaf()) {
		Leaf const * leaf = this->asLeaf();
		if (leaf == nullptr) {
			s << "*empty*";
		} else {
			leaf->repr(s, annotateLeaf, data);
		}
	} else {
		Node const * node = this->asNode();
		if (node == nullptr) {
			s << "*empty*";
		} else {
			node->repr(s, annotateNode, annotateLeaf, data);
		}
	}
	return s;
}

ostream& operator<<(ostream& s, const NodeChild& child)
{
	return child.repr(s);
}

/******************************************************************************
 * KDTree :: Node
 *****************************************************************************/

Node::Node(NodeChild const * _left
	      ,NodeChild const * _right
		  ,const AABB& _aabb
		  ,int _depth
		  ,int _axis) :
	left(_left),
	right(_right),
	aabb(_aabb),
	depth(_depth),
	axis(_axis)
{ }

Node::Node(const Node& other) :
	left(other.left),
	right(other.right),
	aabb(other.aabb),
	depth(other.depth),
	axis(other.axis)
{ }

ostream& Node::repr(ostream& s
	               ,void (*annotateNode)(std::ostream& s, Node const * node, void* data)
				   ,void (*annotateLeaf)(std::ostream& s, Leaf const * leaf, void* data)
				   ,void* data) const
{
	for (int i=0; i<this->depth; i++) {
		s << "  ";
	}

	s << "Node@" << this;
	if (annotateNode != nullptr) {
		s << "<";
		annotateNode(s, this, data);
		s << ">: ";
	}
	s << "{"
	  << " axis="   << this->axis
	  << ", aabb="  << this->aabb
	  << ", depth=" << this->depth 
	  << " }";
	s << endl;

	if (this->left != nullptr) {
		this->left->repr(s, annotateNode, annotateLeaf, data);
	}

	if (this->right != nullptr) {
		this->right->repr(s, annotateNode, annotateLeaf, data);
	}

	return s;
}

ostream& operator<<(ostream& s, const Node& node)
{
	return node.repr(s);
}

/******************************************************************************
 * KDTree 
 *****************************************************************************/

KDTree::KDTree(const vector<Tri>& data
	          ,SplitStrategy* splitStrategy
	          ,StorageStrategy* storageStrategy)
{ 
	// Start timing
	clock_t start = clock(); 

	// Build the tree
	this->root = build(data
		              ,0
		              ,unique_ptr<SplitStrategy>(splitStrategy)
		              ,storageStrategy);

	// End timing
	this->msBuildTime = (clock() - start) * 1000 / CLOCKS_PER_SEC;

	cout << (this->msBuildTime / 1000) << "." << (this->msBuildTime % 1000) << endl;
}

std::ostream& KDTree::repr(std::ostream& s
	                      ,void (*annotateTree)(std::ostream& s, KDTree const * tree, void* data)
						  ,void (*annotateNode)(std::ostream& s, Node const * node, void* data)
						  ,void (*annotateLeaf)(std::ostream& s, Leaf const * leaf, void* data)
						  ,void* data) const
{
	s << "KDTree@" << this;
	if (annotateTree != nullptr) {
		s << "<";
		annotateTree(s, this, data);
		s << ">: ";
	}
	s << endl;

	if (this->root != nullptr) {
		s << endl;
		this->root->repr(s, annotateNode, annotateLeaf, data);
	} else {
		s << " *empty*";
	}
	s << endl;
	return s;
}

ostream& operator<<(ostream& s, const KDTree& tree)
{
	return tree.repr(s);
}

int KDTree::countInNode(NodeChild const * root) const
{
	if (root == nullptr) {
		return 0;
	}

	if (root->isLeaf()) {
		return root->asLeaf()->getCount();
	} else {
		Node const * node = root->asNode();
		return countInNode(node->getLeftChild()) + countInNode(node->getRightChild());
	}
}

/**
 * Recursive helper function for KDTree::intersects
 */
bool KDTree::intersectWalk(const Ray& ray, NodeChild const * root, vector<Tri>& tris) const
{
	queue<NodeChild const *> Q;
	bool hit = false;

	Q.push(root);

	while (!Q.empty()) {

		NodeChild const * head = Q.front();
		Q.pop();

		if (head == nullptr) {
			continue;
		}

		if (head->isLeaf()) {

			Leaf const * leaf = head->asLeaf();
			hit = true;
			copy(leaf->getData().begin(), leaf->getData().end(), back_inserter(tris));

		} else {

			Node const * node = head->asNode();

			if (node->getAABB().intersected(ray)) {

				NodeChild const * left  = node->getLeftChild();
				NodeChild const * right = node->getRightChild();
				hit = true;

				if (left != nullptr) {
					Q.push(left);
				}

				if (right != nullptr) {
					Q.push(right);
				}
			}
		}
	}

	return hit;
}

/**
 * Tests if the given ray intersects the KD-tree, returning any triangles
 * that are potentially intersected, collecting the resulting triangle
 * instances into the supplied vector
 */
bool KDTree::intersects(const Ray& ray, vector<Tri>& tris)
{
	return intersectWalk(ray, this->root, tris);
}

/**
 * Given a list of triangles, this function computes the largest AABB 
 * needed to contain all of the triangles 
 */
static AABB findExtent(const std::vector<Tri>& triangles)
{
	float xMin = numeric_limits<float>::infinity();
	float yMin = numeric_limits<float>::infinity();
	float zMin = numeric_limits<float>::infinity();
	float xMax = -numeric_limits<float>::infinity();
	float yMax = -numeric_limits<float>::infinity();
	float zMax = -numeric_limits<float>::infinity();
	float eps = Utils::EPSILON;

	for (auto i=triangles.begin(); i != triangles.end(); i++) {
		Tri T = *i;
		xMin  = min(xMin, T.getXMinima());
		yMin  = min(yMin, T.getYMinima());
		zMin  = min(zMin, T.getZMinima());
		xMax  = max(xMax, T.getXMaxima());
		yMax  = max(yMax, T.getYMaxima());
		zMax  = max(zMax, T.getZMaxima());
	}

	// Finally expand the borders slightly:
	if (xMin < 0.0f) {
		xMin -= eps;
	} else if (xMin > 0.0f)  {
		xMin += eps;
	}
	if (yMin < 0.0f) {
		yMin -= eps;
	} else if (yMin > 0.0f) {
		yMin += eps;
	}
	if (zMin < 0.0f) {
		zMin -= eps;
	} else if (zMin > 0.0f) {
		zMin += eps;
	}
	if (xMax < 0.0f) {
		xMax -= eps;
	} else if (xMax > 0.0f) {
		xMax += eps;
	}
	if (yMax < 0.0f) {
		yMax -= eps;
	} else if (yMax > 0.0f) {
		yMax += eps;
	}
	if (zMax < 0.0f) {
		zMax -= eps;
	} else if (zMax > 0.0f) {
		zMax += eps;
	}

	return AABB(glm::vec3(xMin, yMin, zMin), glm::vec3(xMax, yMax, zMax));
}

// Axis getter helpers:
static float getX(const glm::vec3& p) {return p.x; }
static float getY(const glm::vec3& p) {return p.y; }
static float getZ(const glm::vec3& p) {return p.z; }

/**
 * Given a list of triangles and a splitting strategy, this function returns
 * KD-tree node
 *
 * @param const std::vector<Tri>& triangles 
 *   The current set of triangles to index
 * @param int depth 
 *   The current depth in the tree the function is being called at
 * @param SplitStrategy splitStrategy 
 *   The strategy used to determine where to split on
 * @param StorageStrategy storageStrategy 
 *   The strategy used to determine where to split on
 * @returns Node
 */
NodeChild const * build(const std::vector<Tri>& triangles
	                   ,int currentDepth
				       ,unique_ptr<SplitStrategy> splitStrategy
					   ,StorageStrategy* storageStrategy)
{
	// Compute the AABB needed to contain all triangle corners
	AABB extent = findExtent(triangles);

	// Have we reached a situation in which we create a leaf?
	//if (currentDepth > maxDepth) {
	if (storageStrategy->done(currentDepth, triangles.size())) {

		// Only bother to create a Leaf instance if there's any actual data to store:
		if (triangles.size() > 0) {
			return new NodeChild(new Leaf(triangles, extent, currentDepth));
		}

		return nullptr;
	}

	glm::vec3 splitPoint = extent.centroid();

	// Now find the split axis: 0 = X, 1 = Y, 2 = Z
	int axis = splitStrategy->nextAxis(triangles);
	assert (axis >= 0 && axis <= 2);

	// Component getter:
	float (*axisValue)(const glm::vec3& p) = nullptr;

	switch (axis) {
		case 0:
			axisValue = getX;
			break;
		case 1:
			axisValue = getY;
			break;
		case 2:
			axisValue = getZ;
			break;
	}

	// Partition the triangles according to the scheme:
	std::vector<Tri> left, right;

	for (auto i=triangles.begin(); i != triangles.end(); i++) {

		Tri T              = *i;
		glm::vec3 centroid = T.getAABB().centroid();

		// Based on the split-axis. Compare the chosen axis-component of each triangle's
		// centroid against the split axis value. Those less than splitOnValue go in one
		// list, everything else goes in the other list. Afterward, we recurse on the
		// partitions

		if (axisValue(centroid) < axisValue(splitPoint)) {

			left.push_back(*i);

		} else if (axisValue(centroid) >= axisValue(splitPoint)) {

			right.push_back(*i);
		} 
	}

	Split S = splitStrategy->divide();

	NodeChild const * N =
		new NodeChild(new Node(left.size() > 0 
								? build(left, currentDepth + 1, move(get<0>(S)), storageStrategy) 
								: nullptr
			                  ,right.size() > 0 
			                  	? build(right, currentDepth + 1, move(get<1>(S)), storageStrategy) 
			                  	: nullptr
							  ,extent
							  ,currentDepth
					          ,axis));

	return N;
}

/*****************************************************************************/

void __intersectNode(std::ostream& s, Node const * node, void* data)
{
	Ray* ray = reinterpret_cast<Ray*>(data);
	if (ray != nullptr) {
		AABB aabb = node->getAABB();
		if (aabb.intersected(*ray)) {
			s << "N(*)@" << node;
		} else {
			s << "N( )@" << node;
		}
	}
}

void __intersectLeaf(std::ostream& s, Leaf const * leaf, void* data)
{
	Ray* ray = reinterpret_cast<Ray*>(data);
	if (ray != nullptr) {
		AABB aabb = leaf->getAABB();
		if (aabb.intersected(*ray)) {
			s << "L(*)@" << leaf;
		} else {
			s << "L( )@" << leaf;
		}
	}
}

void debugIntersectAll(ostream& s, KDTree const * tree, const Ray& ray)
{
	s << "RAY: " << ray << endl << endl;
	tree->repr(s, nullptr, __intersectNode, __intersectLeaf, (void*)&ray);
}

/**
 * Generates a summary of various statistics 
 */
void generateSummary(KDTree const * tree
	                ,const string& name = "<untitled>"
	                ,ostream& out = std::cout)
{
	#define N 100

	// Track max subtree depth + overflow counter
	int maxSubtreeDepth = 0, subtreeDepthOverflow = 0;
	float avgSubtreeDepth = 0.0f;

	// Subtree depth histogram
	int subtreeDepth[N];
	memset(&subtreeDepth[0], 0, sizeof(int) * N);

	// Track max leaf value count + overflow counter
	int totalLeafValueCount = 0, maxLeafValueCount = 0, leafValueCountOverflow = 0;
	float avgLeafValueCount = 0.0f;

	// Subtree leaf value count
	int subtreeValueCount[N];
	memset(&subtreeValueCount[0], 0, sizeof(int) * N);

	// Leaf + node count:
	int leafCount = 0, nodeCount = 0;

	///////////////////////////////////////////////////////////////////////////
	
	queue<NodeChild const *> Q;

	Q.push(tree->root);

	while (!Q.empty()) {

		NodeChild const * head = Q.front();
		Q.pop();

		if (head == nullptr) {
			continue;
		}

		if (head->isLeaf()) {
			
			// Collect statistics at the leaf level:
			Leaf const * leaf = head->asLeaf();

			int depth = leaf->getDepth();
			int count = leaf->getCount();

			avgSubtreeDepth += static_cast<float>(depth);
			avgLeafValueCount += static_cast<float>(count);

			totalLeafValueCount += count;
			leafCount++;

			// Update the maximum subtree depth:
			if (depth > maxSubtreeDepth) {
				maxSubtreeDepth = depth;
			}

			// Add the current depth to the histogram:
			if (depth < N) {
				subtreeDepth[depth]++;
			} else {
				subtreeDepthOverflow++;
			}

			// Update the maximum subtree value count:
			if (count > maxLeafValueCount) {
				maxLeafValueCount = count;
			}

			// Add the current leaf value count to the histogram:
			if (count < N) {
				subtreeValueCount[count]++;
			} else {
				leafValueCountOverflow++;
			}

		} else {

			// Update the node count:
			nodeCount++;

			Node const * node       = head->asNode();
			NodeChild const * left  = node->getLeftChild();
			NodeChild const * right = node->getRightChild();

			if (left) {
				Q.push(left);
			}

			if (right) {
				Q.push(right);
			}
		}
	}

	avgSubtreeDepth /= static_cast<float>(leafCount);
	avgLeafValueCount /= static_cast<float>(leafCount);

	// Display out the summary:
	int buildTime = tree->getBuildTime();

	out << "------------------------------------------------------------" << endl;
	out << "KDTree statistics [" << name << "]" << endl;
	out << "------------------------------------------------------------" << endl;
	out << "- Build time: " << (buildTime / 1000) << "." << (buildTime % 1000) << "s" << endl;
	out << "- Total number of triangles in tree: " << totalLeafValueCount << endl;
	out << "- Number of leaves: " << leafCount << endl;
	out << "- Number of nodes: " << nodeCount << endl;
	out << "- Average subtree depth: " << ceil(avgSubtreeDepth) << endl;
	out << "- Maximum subtree depth: " << maxSubtreeDepth << endl;
	out << "- Subtree depth histogram: " << endl;
	for (int i=0; i<N; i++) {
		if (subtreeDepth[i] > 0) {
			out << "-- " << i << ": " << subtreeDepth[i] << endl;
		}
	}
	out << "- Average number of triangles per leaf: " << ceil(avgLeafValueCount) << endl;
	out << "- Maximum number of triangles in a leaf: " << maxLeafValueCount << endl;
	out << "- Leaf triangle count histogram: " << endl;
	for (int i=0; i<N; i++) {
		if (subtreeValueCount[i] > 0) {
			out << "-- " << i << ": " << subtreeValueCount[i] << endl;
		}
	}
	out << endl;
}

/******************************************************************************/
