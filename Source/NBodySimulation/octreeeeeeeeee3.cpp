#include "octreeeeeeeeee3.h"

#include <queue>
#include <stack>

#include "utillllllssss.h"
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

	// Now clean up the linked list in `Data`
	PointData* current = Data;
	if (0)
	{
		while (current != nullptr)
		{
			delete current;
			PointData* next = current->Next;
			current = next;
		}
	}
	else
	{
		delete current;
	}
	Data = nullptr; // Reset pointer to nullptr after deletion
}

void OctreeNode::Subdivide()
{
	bool log = true;
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

void OctreeNode::accumulate_with_recursion()
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
			double strength = -60;

			ll2("strength555555555: " + FString::SanitizeFloat(strength));
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
				child->accumulate_with_recursion();


				if (
					child->StrengthSet || !child->IsLeaf()
				)
				{
					float c = FMath::Abs(child->Strength);

					aggregateStrength += child->Strength;


					totalWeight += c;
					ll2("c: " + FString::SanitizeFloat(c));
					ll2("child->CenterOfMass: " + child->CenterOfMass.ToString());
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
			ll2("aggregateStrength2222: " + FString::SanitizeFloat(aggregateStrength));
			aggregateStrength *= sqrt(4.0 / 8);
			ll2("aggregateStrength: " + FString::SanitizeFloat(aggregateStrength));
			ll2("aggregatePosition: " + aggregatePosition.ToString());
			ll2("totalWeight: " + FString::SanitizeFloat(totalWeight));
			ll2("aggregatePosition / totalWeight: " + (aggregatePosition / totalWeight).ToString());
			CenterOfMass = aggregatePosition / totalWeight;
			ll2("CenterOfMass: " + CenterOfMass.ToString());
			Strength = aggregateStrength; // Optionally, adjust strength scaling here
			// TotalWeight = totalWeight;
		}
	}
}

void AddDataPoint(OctreeNode* node, FVector Location,int32 id)
{
	// FVector newPoint = kn->GetActorLocation();
	FVector newPoint = Location;

	if (!node->IsLeaf())
	{
		// found the leaf node to add the data point


		if (0)
		{
			for (auto* child : node->Children)
			{
				if (1)
				{
					// This method is slow. 
					if (child->ContainsPoint(newPoint))
					{
						AddDataPoint(child, Location,id);
						return;
					}
				}
				else
				{
				}
			}
		}
		else
		{
			// FVector NewCenter = Center + FVector(
			// (i & 1) ? NewExtent.X : -NewExtent.X,
			// (i & 2) ? NewExtent.Y : -NewExtent.Y,
			// (i & 4) ? NewExtent.Z : -NewExtent.Z

			// We determine which region by just comparing the center of the node with the new point.
			// int i = (z >= zm ? 4 : 0) | (y >= ym ? 2 : 0) | (x >= xm ? 1 : 0);
			int32 i = (newPoint.Z >= node->Center.Z ? 4 : 0) | (newPoint.Y >= node->Center.Y ? 2 : 0) | (
				newPoint.X >= node->Center.X ? 1 : 0);

			AddDataPoint(node->Children[i], Location,id);
		}
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


void OctreeNode::accumulate_without_recursion()
{

	bool log=true;
	// Instead of using recursion, we will traverse the tree using bfs and record the path, And then we will calculate center of mass based on the Reverse order.
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

		ll2("currentNode Lower bound" + (currentNode->Center - currentNode->Extent).ToString() +
		   " Upper bound" + " " + (currentNode->Center + currentNode->Extent).ToString(),log);


		if (!currentNode->IsLeaf())
		{
			// Push children to the stack in normal order
			for (int i = 0; i < currentNode->Children.Num(); i++)
			{
				if (
					currentNode->Children[i]->check_contain_data_or_not()
				)
				{
					ll2("i: " + FString::FromInt(i),log);
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

		ll2("node->Center: " + node->Center.ToString(),log);
		if (node->IsLeaf())
		{
			if (node->Data)
			{
				// FVector position = node->Data->Node->GetActorLocation();
				
				FVector position = node->Data->Position;
				
				node->Strength = -60;
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

void OctreeNode::AccumulateStrengthAndComputeCenterOfMass()
{
	bool using_recursion = false;
	bool log = true;
	if (using_recursion)
	{
		accumulate_with_recursion();
	}
	else{
		accumulate_without_recursion();
	}
}


void TraverseBFS(OctreeNode* root, OctreeCallback callback, float alpha, int32 id, TArray<FVector>& nodePositions, TArray<FVector>& nodeVelocities)
{
	bool log = false;


	// return;
	if (!root) return; // If the root is null, return immediately

	// std::queue<OctreeNode*> nodeQueue;
	std::stack<OctreeNode*> Stack1;
	Stack1.push(root);
	while (!Stack1.empty())
	{
		OctreeNode* currentNode = Stack1.top();
		Stack1.pop();


		ll2("--------------------Right now, dealing with:  Lower bound" +
		   (currentNode->Center - currentNode->Extent).ToString() +
		   " Upper bound" + " " + (currentNode->Center + currentNode->Extent).ToString(), log);
		ll2("Prepare to call the call back functions with this node. ", true);


		// Execute the callback on the current node
		bool skipChildren = callback(currentNode, alpha, id, nodePositions, nodeVelocities);

		// If callback returns true, do not enqueue children
		if (skipChildren)
		{
			continue;
		}

		if (0)
		{
			// Otherwise, enqueue all non-null children
			for (OctreeNode* child : currentNode->Children)
			{
				if (child)
				{
					Stack1.push(child);
				}
			}
		}
		else
		{
			// Reverse enqueue all non-null children using TArray
			for (int i = currentNode->Children.Num() - 1; i >= 0; --i)
			{
				OctreeNode* child = currentNode->Children[i];
				if (child)
				{
					if (child->Data || !child->IsLeaf())
					{
						ll2("i" + FString::FromInt(i), log);
						ll2("Lower bound" + (child->Center - child->Extent).ToString() +
						   " Upper bound" + (child->Center + child->Extent).ToString(), log);

						if (0)
						{
							ll2("printing the data of the child", log);
							child->PrintData();
							ll2("finished printing the data of the child", log);
						}
						Stack1.push(child);
					}
				}
			}
		}
	}
}

bool SampleCallback(OctreeNode* node,
	float alpha,
	int32 id,
	TArray<FVector>& nodePositions,
	TArray<FVector>& nodeVelocities)
{
	bool log = false;
	bool log2 = false;
	bool log3 = true;
	ll2("-----------------", log);
	// ll2("SampleCallback", log);

	if (0)
	{
	}
	else
	{
		// FVector center = CurrentBounds.Center;
		FVector width = node->Extent * 2;


		// FVector dir = node->CenterOfMass - kn->GetActorLocation();

		FVector dir = node->CenterOfMass - nodePositions[id];

		// Remember that direction is the sum of all the Actor locations of the elements in that note. 
		float l = dir.Size() * dir.Size();

		float theta2 = 0.81;
		float distancemax = 1000000000;
		float distancemin = 1;
		// ll2("bounds: " + node->Center.ToString() + " " + node->Extent.ToString());

		ll2(FString::SanitizeFloat(node->Center.X - node->Extent.X) + " " +
		   FString::SanitizeFloat(node->Center.Y - node->Extent.Y) + " " +
		   FString::SanitizeFloat(node->Center.Z - node->Extent.Z) + " " +
		   FString::SanitizeFloat(node->Center.X + node->Extent.X) + " " +
		   FString::SanitizeFloat(node->Center.Y + node->Extent.Y) + " " +
		   FString::SanitizeFloat(node->Center.Z + node->Extent.Z), log);


		// ll2("lower: " + (node->Center - node->Extent).ToString() +
		// 	" upper: " + (node->Center + node->Extent).ToString(), log);

		// ll2("width: " + width.ToString(), log);
		ll2("dir: " + dir.ToString(), log2);
		ll2("l: " + FString::SanitizeFloat(l), log2);
		ll2("width.X * width.X / theta2: " + FString::SanitizeFloat(width.X * width.X / theta2), log2);


		// if size of current box is less than distance between nodes
		// This is used to stop recurring down the tree.
		if (width.X * width.X / theta2 < l)
		{
			//        print("GOING IN HERE");
			if (l < distancemax)
			{
				if (0)
				{
					if (dir.X == 0)
					{
						// Assign a random value   // return (random() - 0.5) * 1e-6;
						dir.X = (FMath::RandRange(0, 1) - 0.5f) * 1e-6;
						// l += x * x;
						l += dir.X * dir.X;
					}
					if (dir.Y == 0)
					{
						// Assign a random value   // return (random() - 0.5) * 1e-6;
						dir.Y = (FMath::RandRange(0, 1) - 0.5f) * 1e-6;
						// l += x * x;
						l += dir.Y * dir.Y;
					}
					if (dir.Z == 0)
					{
						// Assign a random value   // return (random() - 0.5) * 1e-6;
						dir.Z = (FMath::RandRange(0, 1) - 0.5f) * 1e-6;
						// l += x * x;
						l += dir.Z * dir.Z;
					}
				}
				else
				{
				}


				if (l < distancemin)
					l = sqrt(distancemin * l);


				//print(FString::SanitizeFloat(ns.strength));


				FVector Vector = dir
					*
					node->Strength
					*
					alpha;

				if (1)
				{
					ll2("l: " + FString::SanitizeFloat(l), log2);
					ll2("dir: " + dir.ToString(), log2);
					ll2("node->Strength: " + FString::SanitizeFloat(node->Strength), log2);
					ll2("alpha: " + FString::SanitizeFloat(alpha), log2);
					ll2("original Velocity: " + nodeVelocities[id].ToString(), log3);
					// ll2("vector: " + Vector.ToString() + " velocity: " + kn->
					//                                                     velocity.ToString(), log2);

				}

				// float mult = pow(ns.strength / nodeStrength, 1.0);

				// kn->velocity += Vector / l;
				nodeVelocities[id] += Vector / l;
				ll2("velocity Updated: " + nodeVelocities[id].ToString(), log3);
				// ll2("velocity Updated: " + kn->velocity.ToString(), log);
				//
				// if (1)
				// {
				// 	if (kn->velocity.Size() > 100000000000000)
				// 	{
				// 		ll2("velocity is too large. eeeeeeeeeeeee ");
				// 		eeeee();
				// 	}
				// }
			}
			ll2("11111111111111 Early termination. ", log3);
			return true;
		}

		// if not leaf, get all children

		if (!node->IsLeaf()
			||
			l >= distancemax)
		{
			ll2("22222222222222222 You need to return false here. ", log3);
			// ll2("l: " + FString::SanitizeFloat(l), log);
			return false;
		}


		// For the function to reach here, it has to be a leaf node

		if (node->Data == nullptr)

		{
			ll2("Data is null", log, 2);
			return true;
		}
		

		// bool bCond = node->Data->Node != kn;
		bool bCond = node->Data->nodeid != id;

		if (
			// The data is not same as the current node. 
			bCond

			// ||
			// node->Data->Next != nullptr
			//
		)
		{
			// ll2("Need to randomize something here.", log);

			//print("IM LEAF");
			if (0)
			{
				if (dir.X)
				{
					// (random() - 0.5) * 1e-6;
					dir.X = (FMath::RandRange(0, 1) - 0.5f) * 1e-6;
					l += dir.X * dir.X;
				}
				if (dir.Y)
				{
					// (random() - 0.5) * 1e-6;
					dir.Y = (FMath::RandRange(0, 1) - 0.5f) * 1e-6;
					l += dir.Y * dir.Y;
				}
				if (dir.Z)
				{
					// (random() - 0.5) * 1e-6;
					dir.Z = (FMath::RandRange(0, 1) - 0.5f) * 1e-6;
					l += dir.Z * dir.Z;
				}
			}
			else
			{
			}
			// if (l < distanceMin2) l = Math.sqrt(distanceMin2 * l);
			if (l < distancemin)
				l = sqrt(distancemin * l);
		}


		// do if (
		// 			treeNode.data !== node
		// 		) {
		// 	w = strengths[treeNode.data.index] * alpha / l;
		// 	node.vx += x * w;
		// 	if (nDim > 1) {
		// 		node.vy += y * w;
		// 	}
		// 	if (nDim > 2) {
		// 		node.vz += z * w;
		// 	}
		// 		} while (treeNode = treeNode.next);
		//

		if (0)
		{
			PointData* currentNode = node->Data;
			while (1)
			{
				if (
					// currentNode->Node != kn
					currentNode->nodeid != id
				)
				{
					float w = -60 * alpha / l;


					// kn->velocity += dir * w;
					nodeVelocities[id] += dir * w;
				}
				else
				{
				}
				if (currentNode->Next == nullptr)
				{
					break;
				}
				currentNode = currentNode->Next;
			}
		}
		else
		{
			PointData* currentNode = node->Data;

			if (
				// currentNode->Node != kn
				currentNode->nodeid != id
			)
			{
				// float w = currentNode->Node->strength * alpha / l;
				float w = -60 * alpha / l;


				// kn->velocity += dir * w;
				nodeVelocities[id] += dir * w;
				ll2("velocity Updated: " + nodeVelocities[id].ToString(), log3);
				// ll2("velocity Updated: " + kn->velocity.ToString(), log);
			}
		}
		ll2("3333333333333333 Returning false at the very end. ", log3);
		return false;
	}
}

// Assuming `root` is the root of your Octree and it's properly initialized
// TraverseBFS(root, SampleCallback);
