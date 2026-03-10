// KnowledgeGraph_DataIntegration.cpp
// Integration layer between KnowledgeGraph and GraphDataManager

#include "KnowledgeGraph.h"
#include "NBodyUtils.h"

void AKnowledgeGraph::OnGraphDataLoadedCallback(bool bSuccess)
{
	ll("OnGraphDataLoadedCallback called with success: " + FString(bSuccess ? "true" : "false"), true, 0);

	if (!bSuccess)
	{
		ll("Failed to load graph data!", true, 2);
		precheck_succeed = false;
		return;
	}

	// Data is loaded, now convert it to the old format for compatibility
	// In the future, we'll refactor the rest of the code to use DataManager directly

	const TArray<FNodeData>& LoadedNodes = DataManager->GetNodes();
	const TArray<FLinkData>& LoadedLinks = DataManager->GetLinks();

	jnodessss = LoadedNodes.Num();
	ll("Loaded " + FString::FromInt(jnodessss) + " nodes from DataManager", true, 0);

	// Build ID mappings (for backward compatibility)
	id_to_string.Empty();
	string_to_id.Empty();
	for (int32 i = 0; i < LoadedNodes.Num(); i++)
	{
		if (!LoadedNodes[i].StringId.IsEmpty())
		{
			string_to_id.Add(LoadedNodes[i].StringId, i);
			id_to_string.Add(i, LoadedNodes[i].StringId);
		}
	}

	// Copy predefined positions if available
	if (use_predefined_location)
	{
		predefined_positions.SetNum(jnodessss);
		for (int32 i = 0; i < jnodessss; i++)
		{
			predefined_positions[i] = LoadedNodes[i].Position;
		}

		// Center to current actor if needed
		if (use_predefined_locationand_then_center_to_current_actor)
		{
			FVector center = GetActorLocation();
			FVector aggregation = FVector(0, 0, 0);

			for (int32 i = 0; i < jnodessss; i++)
			{
				aggregation += predefined_positions[i];
			}

			aggregation /= jnodessss;
			for (int32 i = 0; i < jnodessss; i++)
			{
				predefined_positions[i] -= (aggregation - center);
			}
		}
	}

	// Extract node properties (for backward compatibility)
	AllNodeProperties.Empty();
	for (const FNodeData& Node : LoadedNodes)
	{
		AllNodeProperties.Add(Node.Properties);
	}

	// Now continue with the old flow
	initialize_arrays();
	
	// Copy links to old format and create their meshes
	// LoadedLinks already declared at the top of the function
	all_links2.SetNum(LoadedLinks.Num());
	
	for (int32 i = 0; i < LoadedLinks.Num(); i++)
	{
		// Use add_edge to properly create the link with its mesh
		add_edge(i, LoadedLinks[i].SourceIndex, LoadedLinks[i].TargetIndex);
	}
	
	if (generate_objects_for_node_and_link_new())
	{
		return;
	}

	ll("post generate graph", true, 0);
	post_generate_graph();
}

// Updated prepare() function to use DataManager
void AKnowledgeGraph::prepare()
{
	ll("prepare() called - using new DataManager", true, 0);

	if (cgm == CGM::GENERATE)
	{
		// Auto-generate mode - don't load data, just set up arrays
		ll("Auto-generate mode", true, 0);
		jnodessss = jnodes1;
		
		initialize_arrays();
		miscellaneous(); // Creates the links
		
		if (generate_objects_for_node_and_link())
		{
			return;
		}

		ll("post generate graph", true, 0);
		post_generate_graph();
	}
	else
	{
		// Use DataManager to load from JSON or Database
		ll("Loading graph via DataManager", true, 0);
		
		// Convert old enum to new enum
		EGraphCreationMode Mode = EGraphCreationMode::AutoGenerate;
		if (cgm == CGM::JSON)
		{
			Mode = EGraphCreationMode::FromJson;
		}
		else if (cgm == CGM::DATABASE)
		{
			Mode = EGraphCreationMode::FromDatabase;
		}

		// Request data - callback will be triggered when done
		DataManager->RequestGraphData(Mode, use_json_file_index, fileIndexToPath);
	}

	if (use_predefined_location)
	{
		// Predefined locations could be regarded as stable
		alpha = 0;
	}
}


// Updated generate_objects_for_node_and_link to work with DataManager
bool AKnowledgeGraph::generate_objects_for_node_and_link_new()
{
	bool log = true;
	
	if (cgm == CGM::GENERATE)
	{
		// Auto-generate mode - create simple text labels
		for (int32 i = 0; i < jnodessss; i++)
		{
			if (node_use_text_render_components)
			{
				FString name = "Sample Text : " + FString::FromInt(i);
				generate_text_render_component_and_attach(name, i);
			}
		}
	}
	else
	{
		// Use DataManager data
		const TArray<FNodeData>& Nodes = DataManager->GetNodes();
		
		for (int32 i = 0; i < jnodessss; i++)
		{
			if (node_use_text_render_components)
			{
				FString name = Nodes[i].Name;
				if (name.IsEmpty())
				{
					name = "Node " + FString::FromInt(i);
				}
				generate_text_render_component_and_attach(name, i);
			}
		}
		ll("Number of nodes generated: " + FString::FromInt(jnodessss), log);
		ll("Number of links: " + FString::FromInt(all_links2.Num()), log);
	}
	
	return false;
}
