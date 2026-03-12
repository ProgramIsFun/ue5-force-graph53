// Refactored from octreeeeeeeeee4.cpp in commit cfe5a2e49166b9a584b865688086bef641dc2862
// Implementation of octree spatial data structure for Barnes-Hut algorithm
// Reference: Barnes & Hut (1986) - "A hierarchical O(N log N) force-calculation algorithm"

#include "NBodyOctree.h"

#include <queue>
#include <stack>

#include "KnowledgeGraph.h"
#include "NBodyUtils.h"
#include "Chaos/AABB.h"

bool OctreeNode::IsLeaf() const
{
	return Children[0] == nullptr;
}

bool OctreeNode::ContainsPoint(const FVector point) const
{
	return (FMath::Abs(point.X - Center.X) <= Extent.X &&
		FMath::Abs(point.Y - Center.Y) <= Extent.Y &&
		FMath::Abs(point.Z - Center.Z) <= Extent.Z);
}

void OctreeNode::PrintData() const
{
	bool log = true;
	if (Data)
	{
		if (Data->nodeid)
		{
			LogMessageInternal
			("77777777777777Data: " + FString::FromInt(Data->nodeid));
		}
	}

	for (auto child : Children)
	{
		if (child)
		{
			child->PrintData();
		}
	}
}


/**
 * Expand octree bounds to contain a given point
 * This is called during octree construction to ensure all points fit within the tree
 * 
 * Algorithm:
 * 1. If octree is empty, initialize with point as lower bound
 * 2. While point is outside bounds, double the octree size
 * 3. Adjust center to maintain containment
 * 
 * @param x, y, z - Coordinates of point to contain
 */
void OctreeNode::Cover(float x, float y, float z)
{
	// Intended to be called on the lower bound and the highest bound of all the data combined
	FVector point(x, y, z);
	
	if (!isCenterSet)
	{
		// Initialize octree bounds with first point
		// Set the initial bounds as the data point acting as the lowest bound
		FVector minBound = FVector(FMath::FloorToFloat(x), FMath::FloorToFloat(y), FMath::FloorToFloat(z));

		// Start with unit cube (1,1,1)
		FVector initialExtent = FVector(1, 1, 1);

		// Center is minBound plus half extent
		Center = minBound + initialExtent * 0.5f;
		Extent = initialExtent * 0.5f;

		isCenterSet = true;
		return;
	}

	// Expand octree until it contains the point
	while (!ContainsPoint(point))
	{
		FVector temp = Center - Extent;

		// Double the size of the octree
		Extent *= 2;
		
		// Adjust center to maintain containment
		Center = temp + Extent;

		// Alternative approach (commented out):
		// Determine the direction to adjust the center based on where the point lies
		// FVector direction = point - Center;
		// Center += FVector(
		//     (direction.X < 0) ? -Extent.X / 2 : Extent.X / 2,
		//     (direction.Y < 0) ? -Extent.Y / 2 : Extent.Y / 2,
		//     (direction.Z < 0) ? -Extent.Z / 2 : Extent.Z / 2
		// );
	}
}

bool OctreeNode::check_contain_data_or_not()
{

	if (Data||!IsLeaf())
	{
		
		return true;
		
	}

	return false;
}


/**
 * Add all nodes to the octree
 * This is the main octree construction function
 * 
 * Process:
 * 1. Find bounding box of all nodes (min/max X, Y, Z)
 * 2. Expand octree to contain all points using Cover()
 * 3. Insert each node into the octree
 * 
 * Complexity: O(n log n) for balanced tree
 * 
 * @param all_nodes - Array of all graph nodes
 * @param nodePositions - Current positions of all nodes
 */
void OctreeNode::AddAll1(
	TArray<Node77> all_nodes,
	TArray<FVector> nodePositions
)
{
	// Note: This assumes node IDs are sequential from 0 to N-1
	int32 N = all_nodes.Num();

	// Pre-allocate arrays for efficiency
	TArray<float> Xz, Yz, Zz;
	Xz.Reserve(N);
	Yz.Reserve(N);
	Zz.Reserve(N);

	// Initialize bounding box with infinity values
	float X0 = std::numeric_limits<float>::infinity();
	float Y0 = std::numeric_limits<float>::infinity();
	float Z0 = std::numeric_limits<float>::infinity();
	float X1 = -std::numeric_limits<float>::infinity();
	float Y1 = -std::numeric_limits<float>::infinity();
	float Z1 = -std::numeric_limits<float>::infinity();

	// Step 1: Compute bounding box of all points
	int i = 0;
	for (auto& node : all_nodes)
	{
		FVector D = nodePositions[i];
		float X = D.X;
		float Y = D.Y;
		float Z = D.Z;
		Xz.Add(X);
		Yz.Add(Y);
		Zz.Add(Z);

		// Track min/max bounds
		X0 = FMath::Min(X0, X);
		Y0 = FMath::Min(Y0, Y);
		Z0 = FMath::Min(Z0, Z);
		X1 = FMath::Max(X1, X);
		Y1 = FMath::Max(Y1, Y);
		Z1 = FMath::Max(Z1, Z);

		i++;
	}

	// Cover the extent


	Cover(X0, Y0, Z0);
	Cover(X1, Y1, Z1);

	// ll("!!!!New center and you extend will be set to: " + Center.ToString() + " " + Extent.ToString());
	// Add the new points
	// for (int i = 0; i < N; ++i)
	// {
	// 	AddDataPoint(this,  nodePositions[i],i);
	// }
	i = 0;
	for (auto& node : all_nodes)
	{
		AddDataPoint(this,  nodePositions[i],i);
		i++;
	}
}
