# Incremental Node Add/Delete — Swap-Remove Implementation

## Overview

Implemented incremental add and delete for graph nodes using the swap-remove pattern,
with mode-aware routing based on `Config.CreationMode`.

## Mode-Aware Routing

| Mode | Add Node | Delete Node |
|------|----------|-------------|
| AutoGenerate | Immediate local edit | Immediate local edit |
| FromJson | Immediate local edit | Immediate local edit |
| FromDatabase | HTTP POST first → local on success | HTTP DELETE first → local on success |

High-level entry points:
- `RequestAddGraphNode(NodeName, LinkTargetNodeIndex)` — routes based on mode
- `RequestRemoveSelectedGraphNode()` — routes based on mode

## Approach: Swap-Remove

All parallel arrays (`GraphNodes`, `nodePositions`, `nodeVelocities`, `BodyTransforms`) stay dense.

**Delete (index K):**
1. Destroy visuals for node K and any links referencing K
2. Copy last element into slot K
3. Fix up all `GraphLink` indices that pointed to the old last index
4. Shrink arrays by one
5. Rebuild instanced mesh instances
6. Recalculate link bias/strength

**Add:**
1. Append to all parallel arrays
2. Create visual components
3. Optionally create a link to an existing node
4. Recalculate link bias/strength

## Files Changed

- `KnowledgeGraph.h` — Added `RequestAddGraphNode()`, `RequestRemoveSelectedGraphNode()`, `OnDeleteGraphNodeHttpCompleted()`, `RemoveGraphNodeByIndex()`, `AddGraphNodeLocal()`. Fixed `GraphLink` default constructor. Removed `LateAddNode`, `bRefreshGraphAfterEditing`.
- `KnowledgeGraph_BlueprintAPI.cpp` — Mode-aware routing, swap-remove delete, incremental add, database HTTP placeholders with callbacks.
- `UI/GraphControlPanelWidget.cpp` — "Remove Selected Node" button calls `RequestRemoveSelectedGraphNode()`.

## Testing Status

- [ ] Compile test
- [ ] AutoGenerate mode: add node via `RequestAddGraphNode`
- [ ] AutoGenerate mode: select + remove node via UI panel
- [ ] FromJson mode: add and remove
- [ ] FromDatabase mode: verify HTTP fires before local edit
- [ ] Verify physics re-settles after add/delete
- [ ] Verify instanced mesh stays in sync
- [ ] Verify text labels stay in sync
- [ ] Verify link meshes are properly cleaned up on delete

## Known Considerations

- `DeleteGraphNodeFromDatabase()` uses a placeholder URL (`Config.GraphDatabaseQueryUrl + "/" + id`). Replace with actual delete endpoint.
- GPU shader path (`bUseGPUShaders`) not yet updated for incremental mutations.
- After add/delete, simulation is reheated (`Alpha = max(Alpha, 0.3)`) so the graph re-settles.
