// KnowledgeGraph_RenderIntegration.cpp
// Integration layer between KnowledgeGraph and GraphRenderer

#include "KnowledgeGraph.h"
#include "NBodyUtils.h"

// Wrapper functions that delegate to Renderer

void AKnowledgeGraph::update_node_world_position_according_to_position_array_new()
{
	if (!Renderer)
	{
		ll("Renderer is null, falling back to old method", true, 1);
		update_node_world_position_according_to_position_array();
		return;
	}

	Renderer->UpdateNodePositions(nodePositions, all_nodes2);
}

void AKnowledgeGraph::update_link_position_new()
{
	if (!Renderer)
	{
		ll("Renderer is null, falling back to old method", true, 1);
		update_link_position();
		return;
	}

	Renderer->UpdateLinkPositions(all_links2, nodePositions, GetWorld());
}

void AKnowledgeGraph::rotate_to_face_player_new()
{
	if (!Renderer)
	{
		ll("Renderer is null, falling back to old method", true, 1);
		rotate_to_face_player111();
		return;
	}

	FVector PlayerLocation = get_player_location727();
	Renderer->RotateTextToFacePlayer(nodePositions, all_nodes2, PlayerLocation);
}

// Text size functions using Renderer
void AKnowledgeGraph::set_text_size_of_all_nodes1112(float size)
{
	if (Renderer)
	{
		Renderer->SetTextSize(size, all_nodes2);
	}
	else
	{
		// Fallback to old method
		for (int i = 0; i < jnodessss; i++)
		{
			if (node_use_text_render_components)
			{
				all_nodes2[i].textComponent->SetWorldSize(size);
			}
		}
	}
}

void AKnowledgeGraph::increase_or_decrease_text_size_of_all_nodes1112(bool increase, float size)
{
	float delta = increase ? size : -size;
	
	if (Renderer)
	{
		Renderer->AdjustTextSize(delta, all_nodes2);
	}
	else
	{
		// Fallback to old method
		for (int i = 0; i < jnodessss; i++)
		{
			if (node_use_text_render_components)
			{
				float current_size = all_nodes2[i].textComponent->WorldSize;
				all_nodes2[i].textComponent->SetWorldSize(current_size + delta);
			}
		}
	}
}
