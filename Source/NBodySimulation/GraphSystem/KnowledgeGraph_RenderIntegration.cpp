// KnowledgeGraph_RenderIntegration.cpp
// Integration layer between KnowledgeGraph and GraphRenderer

#include "KnowledgeGraph.h"
#include "NBodyUtils.h"

// Wrapper functions that delegate to Renderer

void AKnowledgeGraph::update_node_world_position_according_to_position_array_new()
{
	if (!Renderer)
	{
		LogMessage("Renderer is null, falling back to old method", true, 1);
		update_node_world_position_according_to_position_array();
		return;
	}

	// Safety check: Ensure arrays are initialized
	if (nodePositions.Num() == 0 || GraphNodes.Num() == 0)
	{
		LogMessage("Arrays not initialized, skipping node position update", true, 1);
		return;
	}

	Renderer->UpdateNodePositions(nodePositions, GraphNodes);
}

void AKnowledgeGraph::update_link_position_new()
{
	if (!Renderer)
	{
		LogMessage("Renderer is null, falling back to old method", true, 1);
		update_link_position();
		return;
	}

	// Safety check: Ensure arrays are initialized
	if (GraphLinks.Num() == 0 || nodePositions.Num() == 0)
	{
		LogMessage("Arrays not initialized, skipping link position update", true, 1);
		return;
	}

	Renderer->UpdateLinkPositions(GraphLinks, nodePositions, GetWorld());
}

void AKnowledgeGraph::rotate_to_face_player_new()
{
	if (!Renderer)
	{
		LogMessage("Renderer is null, falling back to old method", true, 1);
		rotate_to_face_player111();
		return;
	}

	// Safety check: Ensure arrays are initialized
	if (nodePositions.Num() == 0 || GraphNodes.Num() == 0)
	{
		return;
	}

	FVector PlayerLocation = GetPlayerLocation();
	Renderer->RotateTextToFacePlayer(nodePositions, GraphNodes, PlayerLocation);
}

// Text size functions using Renderer
void AKnowledgeGraph::set_text_size_of_all_nodes1112(float size)
{
	if (Renderer)
	{
		Renderer->SetTextSize(size, GraphNodes);
	}
	else
	{
		// Fallback to old method
		for (int i = 0; i < TotalNodeCount; i++)
		{
			if (Config.bUseTextRenderComponents)
			{
				GraphNodes[i].textComponent->SetWorldSize(size);
			}
		}
	}
}

void AKnowledgeGraph::increase_or_decrease_text_size_of_all_nodes1112(bool increase, float size)
{
	float delta = increase ? size : -size;
	
	if (Renderer)
	{
		Renderer->AdjustTextSize(delta, GraphNodes);
	}
	else
	{
		// Fallback to old method
		for (int i = 0; i < TotalNodeCount; i++)
		{
			if (Config.bUseTextRenderComponents)
			{
				float current_size = GraphNodes[i].textComponent->WorldSize;
				GraphNodes[i].textComponent->SetWorldSize(current_size + delta);
			}
		}
	}
}
