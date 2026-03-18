// Refactored from octreeeeeeeeee3.cpp in commit cfe5a2e49166b9a584b865688086bef641dc2862
#include "NBodyOctree.h"

#include <queue>
#include <stack>

#include "NBodyUtils.h"
#include "Chaos/AABB.h"


OctreeNode::OctreeNode(FVector center, FVector extent)
	: Center(center),
	  Extent(extent)
	  // CenterOfMass(center)
{
	Children.SetNum(8, false);
	isCenterSet = true;
}

OctreeNode::OctreeNode()
{
	Children.SetNum(8, false);
}

OctreeNode::~OctreeNode()
{
	for (auto child : Children)
	{
		delete child;
	}
	Children.Empty(); // Clear the TArray, not strictly necessary since the node is being destroyed

	// Clean up the linked list in `Data`
	PointData* CurrentPoint = Data;
	while (CurrentPoint != nullptr)
	{
		PointData* NextPoint = CurrentPoint->Next;
		delete CurrentPoint;
		CurrentPoint = NextPoint;
	}
	Data = nullptr;
}

void OctreeNode::Subdivide()
{
	if (
		!IsLeaf() // 
		||
		Data == nullptr // 

			// These two conditions shall never matter, because dysfunctions before calling shall already have the checking. 
	)
		return;

	FVector NewExtent = Extent * 0.5;
	PointData* OldData = Data;


	Data = nullptr; // Clear current data as it's redistributed to children

	for (int i = 0; i < 8; ++i)
	{
		FVector NewCenter = Center + FVector(
			(i & 1) ? NewExtent.X : -NewExtent.X,
			(i & 2) ? NewExtent.Y : -NewExtent.Y,
			(i & 4) ? NewExtent.Z : -NewExtent.Z
		);

		Children[i] = new OctreeNode(NewCenter, NewExtent);
	}

	AddDataPoint(this, OldData->Position,OldData->nodeid ); // Add the old data point to the children
	delete OldData; // Deleting the old data after redistributing
}

void OctreeNode::accumulate_with_recursion(float NodeChargeStrength)
{
	FVector aggregatePosition = FVector(0);
	float aggregateStrength = 0.0;
	float totalWeight = 0;

	if (IsLeaf())
	{
		if (Data)
		{
			// FVector position = Data->Node->GetActorLocation();
			FVector position = Data->Position;
			double strength = NodeChargeStrength;

			LogMessageInternal("strength555555555: " + FString::SanitizeFloat(strength));
			Strength = strength;
			StrengthSet = true;

			// TotalWeight = FMath::Abs(strength);


			// In javascript implementations,
			// we extract the value of that node and assign directly xyz property  
			CenterOfMass = position; // Assign directly for leaf nodes
		}
		else
		{
			// If no data is associated with this Leaf node, we will never record the total weight and strength.
			// StrengthSet=false;
		}
	}
	else
	{
		// Recursive accumulation from children nodes
		for (OctreeNode* child : Children)
		{
			if (
				child != nullptr
			)
			{
				child->accumulate_with_recursion(NodeChargeStrength);


				if (
					child->StrengthSet || !child->IsLeaf()
				)
				{
					float c = FMath::Abs(child->Strength);

					aggregateStrength += child->Strength;


					totalWeight += c;
					LogMessageInternal("c: " + FString::SanitizeFloat(c));
					LogMessageInternal("child->CenterOfMass: " + child->CenterOfMass.ToString());
					aggregatePosition += c * child->CenterOfMass;
				}
			}
			else
			{
				// Should never happens here,
				// because if this is not a leaf note, all the child should not be an empty pointer. 
			}
		}

		// Calculate the center of mass based on total weight
		if (totalWeight > 0)
		{
			LogMessageInternal("aggregateStrength2222: " + FString::SanitizeFloat(aggregateStrength));
			aggregateStrength *= sqrt(4.0 / 8);
			LogMessageInternal("aggregateStrength: " + FString::SanitizeFloat(aggregateStrength));
			LogMessageInternal("aggregatePosition: " + aggregatePosition.ToString());
			LogMessageInternal("totalWeight: " + FString::SanitizeFloat(totalWeight));
			LogMessageInternal("aggregatePosition / totalWeight: " + (aggregatePosition / totalWeight).ToString());
			CenterOfMass = aggregatePosition / totalWeight;
			LogMessageInternal("CenterOfMass: " + CenterOfMass.ToString());
			Strength = aggregateStrength; // Optionally, adjust strength scaling here
			// TotalWeight = totalWeight;
		}
	}
}

void AddDataPoint(OctreeNode* node, FVector Location,int32 id)
{
	FVector newPoint = Location;

	if (!node->IsLeaf())
	{
		// Determine which octant the point belongs to by comparing against the node center
		int32 i = (newPoint.Z >= node->Center.Z ? 4 : 0) | (newPoint.Y >= node->Center.Y ? 2 : 0) | (
			newPoint.X >= node->Center.X ? 1 : 0);

		AddDataPoint(node->Children[i], Location,id);
	}
	else
	{
		if (
			node->Data != nullptr
			// If this leaf node already has data
		)
		{
			// If the node already has data, subdivide and add the new data point
			node->Subdivide();
			AddDataPoint(node, Location,id);
		}
		else
		{
			// No data is associated with the current node
			node->Data = new PointData(id, Location);

			// All the 8 pointers of children's property Should still be nullptr.
		}
	}
}


void OctreeNode::accumulate_without_recursion(float NodeChargeStrength)
{
	// Instead of using recursion, we will traverse the tree using DFS and record the path,
	// then calculate center of mass in reverse order.
	std::stack<OctreeNode*> traversalOrder;
	std::stack<OctreeNode*> stack;

	if (this)
	{
		stack.push(this);
	}

	// DFS to record the nodes in traversal order using a stack
	while (!stack.empty())
	{
		OctreeNode* currentNode = stack.top();
		stack.pop();

		LogMessageInternal("currentNode Lower bound" + (currentNode->Center - currentNode->Extent).ToString() +
		   " Upper bound" + " " + (currentNode->Center + currentNode->Extent).ToString());


		if (!currentNode->IsLeaf())
		{
			// Push children to the stack in normal order
			for (int i = 0; i < currentNode->Children.Num(); i++)
			{
				if (
					currentNode->Children[i]->check_contain_data_or_not()
				)
				{
					LogMessageInternal("i: " + FString::FromInt(i));
					stack.push(currentNode->Children[i]);
				}
			}
		}

		traversalOrder.push(currentNode);
	}

	// Process in reverse traversal order (from last non-leaf to root)
	while (!traversalOrder.empty())
	{
		
		OctreeNode* node = traversalOrder.top();
		traversalOrder.pop();

		LogMessageInternal("node->Center: " + node->Center.ToString());
		if (node->IsLeaf())
		{
			if (node->Data)
			{
				// FVector position = node->Data->Node->GetActorLocation();
				
				FVector position = node->Data->Position;
				
				node->Strength = NodeChargeStrength;
				node->StrengthSet = true;
				node->CenterOfMass = position;
				
			}
		}
		else
		{
			FVector aggregatePosition = FVector(0);
			float aggregateStrength = 0.0;
			float totalWeight = 0;

			for (auto child : node->Children)
			{
				if (child && (child->StrengthSet || !child->IsLeaf()))
				{
					float strengthAbs = FMath::Abs(child->Strength);
					aggregateStrength += child->Strength;
					totalWeight += strengthAbs;
					aggregatePosition += strengthAbs * child->CenterOfMass;
					
				}
			}

			if (totalWeight > 0)
			{
				aggregateStrength *= sqrt(4.0 / 8);
				node->CenterOfMass = aggregatePosition / totalWeight;
				node->Strength = aggregateStrength;
			}
		}
	}
}

void OctreeNode::AccumulateStrengthAndComputeCenterOfMass(float NodeChargeStrength)
{
	bool using_recursion = false;
	if (using_recursion)
	{
		accumulate_with_recursion(NodeChargeStrength);
	}
	else{
		accumulate_without_recursion(NodeChargeStrength);
	}
}


void TraverseBFS(OctreeNode* root, OctreeCallback callback, float alpha, int32 id, TArray<FVector>& nodePositions, TArray<FVector>& nodeVelocities, float NodeChargeStrength, float ForceDistanceMin, float ForceDistanceMax, float BarnesHutThetaSquared)
{
	if (!root) return;

	std::stack<OctreeNode*> Stack1;
	Stack1.push(root);
	while (!Stack1.empty())
	{
		OctreeNode* currentNode = Stack1.top();
		Stack1.pop();

		// Execute the callback on the current node
		bool skipChildren = callback(currentNode, alpha, id, nodePositions, nodeVelocities, NodeChargeStrength, ForceDistanceMin, ForceDistanceMax, BarnesHutThetaSquared);

		// If callback returns true, do not enqueue children
		if (skipChildren)
		{
			continue;
		}

		// Reverse enqueue all non-null children that contain data
		for (int i = currentNode->Children.Num() - 1; i >= 0; --i)
		{
			OctreeNode* child = currentNode->Children[i];
			if (child && (child->Data || !child->IsLeaf()))
			{
				Stack1.push(child);
			}
		}
	}
}

bool SampleCallback(OctreeNode* node,
	float alpha,
	int32 id,
	TArray<FVector>& nodePositions,
	TArray<FVector>& nodeVelocities,
	float NodeChargeStrength,
	float ForceDistanceMin,
	float ForceDistanceMax,
	float BarnesHutThetaSquared)
{
	FVector width = node->Extent * 2;
	FVector dir = node->CenterOfMass - nodePositions[id];

	float l = dir.Size() * dir.Size();

	// Barnes-Hut opening angle and distance bounds from configuration
	float theta2 = BarnesHutThetaSquared;
	float distancemax = ForceDistanceMax;
	float distancemin = ForceDistanceMin;

	// Barnes-Hut criterion: if cell is small enough relative to distance, treat as single body
	if (width.X * width.X / theta2 < l)
	{
		if (l < distancemax)
		{
			if (l < distancemin)
				l = sqrt(distancemin * l);

			FVector Vector = dir * node->Strength * alpha;

			LogMessageInternal("original Velocity: " + nodeVelocities[id].ToString());

			nodeVelocities[id] += Vector / l;
			LogMessageInternal("velocity Updated: " + nodeVelocities[id].ToString());
		}
		return true;
	}

	// If not leaf or beyond max distance, continue traversal into children
	if (!node->IsLeaf() || l >= distancemax)
	{
		return false;
	}

	// Leaf node with no data — skip
	if (node->Data == nullptr)
	{
		return true;
	}

	// Leaf node: apply force if this is a different node
	bool bCond = node->Data->nodeid != id;

	if (bCond)
	{
		if (l < distancemin)
			l = sqrt(distancemin * l);
	}

	PointData* currentNode = node->Data;
	if (currentNode->nodeid != id)
	{
		float w = NodeChargeStrength * alpha / l;
		nodeVelocities[id] += dir * w;
		LogMessageInternal("velocity Updated: " + nodeVelocities[id].ToString());
	}

	return false;
}


