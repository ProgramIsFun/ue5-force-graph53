// KnowledgeGraph_DataIntegration.cpp
//
// Integration layer between KnowledgeGraph and GraphDataManager.
// Handles data loading from JSON files, HTTP endpoints, and auto-generation.
// Converts between new data structures (FNodeData, FLinkData) and legacy format.
//
// Key Functions:
// - OnGraphDataLoadedCallback(): Async callback when data is loaded
// - Prepare(): Initiates graph data loading based on creation mode
// - GenerateObjectsForNodeAndLinkNew(): Creates visual objects from data
//
// Part of the KnowledgeGraph refactoring - integrates GraphDataManager component

#include "KnowledgeGraph.h"
#include "NBodyUtils.h"

void AKnowledgeGraph::OnGraphDataLoadedCallback(bool bSuccess)
{
	LogMessage("OnGraphDataLoadedCallback called with success: " + FString(bSuccess ? "true" : "false"), true, 0);

	if (!bSuccess)
	{
		LogMessage("Failed to load graph data! Server connection problem detected.", true, 2);
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red,
			TEXT("ERROR: Failed to connect to graph database server."));
		bServerConnectionFailed = true;
		return;
	}

	// Data is loaded, now convert it to the old format for compatibility
	// In the future, we'll refactor the rest of the code to use DataManager directly

	const TArray<FNodeData>& LoadedNodes = DataManager->GetNodes();
	const TArray<FLinkData>& LoadedLinks = DataManager->GetLinks();

	TotalNodeCount = LoadedNodes.Num();
	LogMessage("Loaded " + FString::FromInt(TotalNodeCount) + " nodes from DataManager", true, 0);

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
	if (Config.bUsePredefinedLocation)
	{
		predefined_positions.SetNum(TotalNodeCount);
		for (int32 i = 0; i < TotalNodeCount; i++)
		{
			predefined_positions[i] = LoadedNodes[i].Position;
		}

		// Center to current actor if needed
		if (Config.bCenterPredefinedLocationToActor)
		{
			FVector center = GetActorLocation();
			FVector aggregation = FVector(0, 0, 0);

			for (int32 i = 0; i < TotalNodeCount; i++)
			{
				aggregation += predefined_positions[i];
			}

			aggregation /= TotalNodeCount;
			for (int32 i = 0; i < TotalNodeCount; i++)
			{
				predefined_positions[i] -= (aggregation - center);
			}
		}
	}

	// Node properties are stored in DataManager (FNodeData::Properties), no duplication needed

	// Now continue with the old flow
	InitializeArrays();
	
	// Copy links to old format and create their meshes
	// LoadedLinks already declared at the top of the function
	GraphLinks.SetNum(LoadedLinks.Num());
	
	for (int32 i = 0; i < LoadedLinks.Num(); i++)
	{
		// Use AddEdge to properly create the link with its mesh
		AddEdge(i, LoadedLinks[i].SourceIndex, LoadedLinks[i].TargetIndex);
	}
	
	if (GenerateObjectsForNodeAndLinkNew())
	{
		return;
	}

	LogMessage("post generate graph", true, 0);
	PostGenerateGraph();
}

// Updated prepare() function to use DataManager
void AKnowledgeGraph::Prepare()
{
	LogMessage("Prepare() called - using new DataManager", true, 0);

	if (Config.CreationMode == EGraphCreationMode::AutoGenerate)
	{
		// Auto-generate mode - don't load data, just set up arrays
		LogMessage("Auto-generate mode", true, 0);
		TotalNodeCount = Config.AutoGenerateNodeCount;
		
		InitializeArrays();
		Miscellaneous(); // Creates the links
		
		if (GenerateObjectsForNodeAndLink())
		{
			return;
		}

		LogMessage("post generate graph", true, 0);
		PostGenerateGraph();
	}
	else
	{
		// Use DataManager to load from JSON or Database
		LogMessage("Loading graph via DataManager", true, 0);
		
		// Convert old enum to new enum
		EGraphCreationMode Mode = EGraphCreationMode::AutoGenerate;
		if (Config.CreationMode == EGraphCreationMode::FromJson)
		{
			Mode = EGraphCreationMode::FromJson;
		}
		else if (Config.CreationMode == EGraphCreationMode::FromDatabase)
		{
			Mode = EGraphCreationMode::FromDatabase;
		}

		// Request data - callback will be triggered when done
		DataManager->RequestGraphData(Mode, Config.JsonFileIndex, fileIndexToPath);
	}

	if (Config.bUsePredefinedLocation)
	{
		// Predefined locations could be regarded as stable
		Config.Alpha = 0;
	}
}


// Updated generate_objects_for_node_and_link to work with DataManager
bool AKnowledgeGraph::GenerateObjectsForNodeAndLinkNew()
{
	bool log = true;
	
	if (Config.CreationMode == EGraphCreationMode::AutoGenerate)
	{
		// Auto-generate mode - create simple text labels
		for (int32 i = 0; i < TotalNodeCount; i++)
		{
			if (Config.bUseTextRenderComponents)
			{
				FString name = "Sample Text : " + FString::FromInt(i);
				GenerateTextRenderComponentAndAttach(name, i);
			}
		}
	}
	else
	{
		// Use DataManager data
		const TArray<FNodeData>& Nodes = DataManager->GetNodes();
		
		for (int32 i = 0; i < TotalNodeCount; i++)
		{
			if (Config.bUseTextRenderComponents)
			{
				FString name = Nodes[i].Name;
				if (name.IsEmpty())
				{
					name = "Node " + FString::FromInt(i);
				}
				GenerateTextRenderComponentAndAttach(name, i);
			}
		}
		LogMessage("Number of nodes generated: " + FString::FromInt(TotalNodeCount), log);
		LogMessage("Number of links: " + FString::FromInt(GraphLinks.Num()), log);
	}
	
	return false;
}

// Legacy HTTP request functions have been removed.
// All data loading is now handled by GraphDataManager via Prepare() -> OnGraphDataLoadedCallback().
