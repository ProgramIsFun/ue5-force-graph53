# Node Property Panel Feature

## Overview
When a graph node is selected (via laser-select or look-at), a panel in the upper-right corner displays the node's key-value properties.

## Files Added
- `Source/NBodySimulation/GraphSystem/UI/NodePropertyPanelWidget.h` — Slate widget that renders property rows
- `Source/NBodySimulation/GraphSystem/UI/NodePropertyPanelWidget.cpp` — Implementation

## Files Modified
- `KnowledgeGraph.h` — Added `FOnGraphNodeSelectionChanged` delegate, `OnGraphNodeSelectionChanged` member
- `KnowledgeGraph_BlueprintAPI.cpp` — Broadcast delegate in `SelectGraphNodeByLookAt()`, `SelectClosestGraphNodeToPlayer()`, and `RemoveGraphNodeByIndex()`
- `GraphPlayerController.h` — Added `NodePropertyPanel` member and `OnSelectedGraphNodeChanged()` callback
- `GraphPlayerController.cpp` — Creates property panel in `FindAndBindGraphActor()`, subscribes to selection delegate

## How It Works
1. Selection functions broadcast `OnGraphNodeSelectionChanged` with the new index (or -1 on deselect)
2. `GraphPlayerController` receives the callback and calls `RefreshSelectedNodeProperties()` on the panel
3. The panel queries `DataManager->GetNodeProperties()` for the selected node's `TMap<FString, FString>` and renders each pair as a row
4. Built-in properties (index, database ID) are always shown; the panel hides when nothing is selected

## Status
- [x] Widget created
- [x] Delegate wired through selection flow
- [x] Player controller integration
- [ ] Compile and runtime testing in-editor
