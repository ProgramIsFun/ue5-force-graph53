// Refactored from octreeeeeeeeee3.h in commit cfe5a2e49166b9a584b865688086bef641dc2862
// Octree spatial data structure for Barnes-Hut algorithm
// Reference: Barnes & Hut (1986) - "A hierarchical O(N log N) force-calculation algorithm"
//
// An octree divides 3D space into nested cubic cells (octants).
// Each node can have up to 8 children representing the 8 octants.
// This enables efficient spatial queries for force calculations.

#pragma once

#include <functional>

#include "CoreMinimal.h"
// #include "KnowledgeGraph.h"
#include "KnowledgeNode.h"
// #include "PointData.h" // Make sure this include points to your actual PointData structure file location

class Node77;

/**
 * Point data stored in octree leaf nodes
 * Represents a single graph node's position in 3D space
 */
struct PointData
{
	int32 nodeid;        // Index of the node in the graph
	FVector Position;    // 3D position of the node
	PointData* Next = nullptr;  // Linked list for multiple points in same cell

	PointData(int32 nodeid, FVector position)
		: nodeid(nodeid), Position(position)
	{
	}
};

/**
 * Octree node for Barnes-Hut spatial subdivision
 * 
 * Each node represents a cubic region of 3D space.
 * Internal nodes have 8 children (octants).
 * Leaf nodes contain point data.
 * 
 * For Barnes-Hut algorithm, each node stores:
 * - CenterOfMass: weighted average position of all contained points
 * - Strength: total charge/mass of all contained points
 * - TotalDataPoints: number of points in this subtree
 * 
 * These aggregated values allow treating distant clusters as single bodies.
 */
struct OctreeNode
{
	// Octree structure
	bool isCenterSet = false;
	FVector Center;                    // Center point of this cubic cell
	FVector Extent;                    // Half-size of the cube in each dimension
	TArray<OctreeNode*> Children;      // 8 children (octants), nullptr if leaf
	PointData* Data = nullptr;         // Point data (only in leaf nodes)

	// Barnes-Hut aggregated values
	FVector CenterOfMass;              // Weighted average position of all points in subtree
	int32 TotalDataPoints = 0;         // Number of points in this subtree
	float Strength;                    // Total charge/mass of all points in subtree
	bool StrengthSet = false;


	// default to be zero vector.  
	OctreeNode(FVector center
	           // =FVector(0,0,0)
	           ,
	           FVector extent
	           // =FVector(0,0,0)
	);

	OctreeNode();

	~OctreeNode();

	bool IsLeaf() const;
	bool ContainsPoint(const FVector point) const;

	// Print data that is stored in the octree node
	void PrintData() const;
	
	void Subdivide();
	void accumulate_with_recursion();
	void accumulate_without_recursion();
	void AccumulateStrengthAndComputeCenterOfMass();
	void Cover(float X0, float Y0, float Z0);
	bool check_contain_data_or_not();
	// void AddAll1(TMap<int32, AKnowledgeNode*> Map1, TArray<FVector> nodePositions);
	void AddAll1(
		TArray<Node77> all_nodes,
		TArray<FVector> nodePositions
	);

};

// void AddDataPoint(OctreeNode* node, AKnowledgeNode* newNode, FVector position);

void AddDataPoint(OctreeNode* node, FVector Location,int32 id);

// Define the Callback Type
using OctreeCallback = std::function<bool(OctreeNode*,float alpha, int32 id, TArray<FVector>&  nodePositions, TArray<FVector>&  nodeVelocities)>;

// Declare the BFS traversal function
// void TraverseBFS(OctreeNode* root, OctreeCallback callback);
void TraverseBFS(OctreeNode* root, OctreeCallback callback, float alpha, int32 id, TArray<FVector>&  nodePositions, TArray<FVector>&  nodeVelocities);

// bool SampleCallback(OctreeNode* node);
bool SampleCallback(OctreeNode* node, float alpha, int32 id, TArray<FVector>&  nodePositions, TArray<FVector>&  nodeVelocities);
