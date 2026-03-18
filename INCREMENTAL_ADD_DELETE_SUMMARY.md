# Incremental Node Add/Delete — Swap-Remove Implementation

## Overview

Implemented incremental add and delete for graph nodes using the swap-remove pattern.
This avoids a full graph reload (`ReloadTheWholeGraph()`) when adding or removing individual nodes.

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

- `KnowledgeGraph.h` — Added `RemoveGraphNodeByIndex()`, `AddGraphNodeLocal()`, `RemoveSelectedGraphNode()` declarations. Fixed `GraphLink` default constructor to initialize all members.
- `KnowledgeGraph_BlueprintAPI.cpp` — Implemented swap-remove delete, incremental add, and updated `LateAddNode` to use incremental path.
- `UI/GraphControlPanelWidget.cpp` — Added "Remove Selected Node" button.

## Testing Status

- [ ] Compile test
- [ ] Add node via `AddGraphNodeToDatabase` with `bRefreshGraphAfterEditing = false`
- [ ] Remove selected node via UI panel button
- [ ] Verify physics re-settles after add/delete
- [ ] Verify instanced mesh stays in sync
- [ ] Verify text labels stay in sync
- [ ] Verify link meshes are properly cleaned up on delete
- [ ] Stress test: rapid add/delete cycles

## Known Considerations

- `AddEdge` names link meshes as `CylinderMesh%d` using array index. After swap-removes, UE appends suffix to avoid name collisions — cosmetic only, no functional impact.
- GPU shader path (`bUseGPUShaders`) is not yet updated for incremental mutations — would need `SimParameters` rebuild.
- After add/delete, simulation is reheated (`Alpha = max(Alpha, 0.3)`) so the graph re-settles.
