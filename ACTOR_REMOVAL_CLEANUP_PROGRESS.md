# Actor Reference Removal - Progress Tracking

## Overview
Removing `AKnowledgeNode*` and `AKnowledgeEdge*` references from the active codebase. These actor-based types are deprecated in favor of lightweight data structs.

## Changes Made

### KnowledgeGraph.h
- Removed `#include "KnowledgeNode.h"` and `#include "KnowledgeEdge.h"`
- `GraphNode`: removed `AKnowledgeNode* node` field and the constructor that took it; added default constructor
- `GraphLink`: removed `AKnowledgeEdge* EdgeActor` field and the constructor that took it; initialized `EdgeMeshComponent` to nullptr
- Removed declarations: `generate_actor_and_register()`, `generate_actor_for_a_link()`

### KnowledgeGraph_Physics.cpp
- Removed `generate_actor_and_register()` function body (dead code, never called)
- Removed `generate_actor_for_a_link()` function body (dead code, never called)
- Brute force charge calculation: replaced `node.node` identity comparison with index comparison (`NodeIdx != OtherNodeIdx`)
- `update_position_array_according_to_velocity_array`: removed unused `auto kn = node.node;` and cleaned up loop to use index-based iteration

### GraphDataManager.h
- `FNodeData`: removed `AKnowledgeNode* NodeActor` field and forward declaration
- `FLinkData`: removed `class AKnowledgeEdge* EdgeActor` field

### NBodyOctree.h
- Removed `#include "KnowledgeNode.h"` (was unused — only forward-declares `GraphNode`)

## Remaining (not touched)
- `KnowledgeNode.h/.cpp` and `KnowledgeEdge.h/.cpp` have been **deleted** from the repository
- Commented-out references in `NBodyOctree.h` have been cleaned up

## Status
- [x] All active code references removed
- [x] Deprecated files deleted
- [x] Commented-out references cleaned up
- [ ] Compile verification
- [ ] Runtime testing
