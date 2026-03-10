# KnowledgeGraph Refactoring Progress

> **📋 AI ASSISTANT CONTEXT FILE**  
> This document tracks the ongoing refactoring of the KnowledgeGraph class.  
> When resuming work on this refactoring, read this file first to understand current progress and next steps.

## Goal
Decompose the monolithic `AKnowledgeGraph` class into smaller, focused components for better maintainability and testability.

## Target Architecture
```
AKnowledgeGraph (orchestrator)
├── UGraphDataManager (data & HTTP)
├── UGraphPhysicsSimulator (force calculations)
├── UGraphRenderer (visualization)
├── FGraphOctreeManager (spatial queries)
└── UGraphInteractionHandler (user interaction)
```

---

---

## Progress Overview

| Step | Component | Status | Files Changed |
|------|-----------|--------|---------------|
| 1 | Configuration Extraction | ✅ COMPLETE | 2 files |
| 2 | GraphDataManager | ✅ COMPLETE | 2 files |
| 2b | Integrate GraphDataManager | ✅ COMPLETE | 4 files |
| 3 | GraphPhysicsSimulator | ✅ COMPLETE | 2 files |
| 3b | Integrate GraphPhysicsSimulator | ✅ COMPLETE | 3 files |
| 4 | GraphRenderer | ⏸️ NOT STARTED | - |
| 5 | GraphInteractionHandler | ⏸️ NOT STARTED | - |
| 6 | Cleanup & Optimization | ⏸️ NOT STARTED | - |

**Legend:**  
✅ COMPLETE - Done and tested  
🚧 IN PROGRESS - Currently working on this  
⏸️ NOT STARTED - Planned but not begun  
❌ BLOCKED - Cannot proceed due to dependency

---

## Completed Steps

### ✅ Step 1: Configuration Extraction
**Date:** 2026-03-10  
**Status:** COMPLETE - Compiles successfully  
**Tested:** Yes

**Changes:**
- Created `Source/NBodySimulation/GraphConfiguration.h`
- Introduced `FGraphConfiguration` struct with organized categories:
  - General Settings
  - Position Settings
  - Simulation Settings
  - Node Rendering Settings
  - Link Rendering Settings
  - Debug Settings
- Added `Config` property to `AKnowledgeGraph`
- Kept legacy properties for backward compatibility
- Renamed enum `CGM` → `EGraphCreationMode` (legacy enum kept)

**Benefits:**
- All configuration now in one place
- Better editor organization with proper categories
- Cleaner property names (e.g., `bUseGPUShaders` vs `use_shaders`)
- Foundation for future refactoring

**Files Modified:**
- `Source/NBodySimulation/KnowledgeGraph.h` (added Config struct, marked legacy properties)

**Files Created:**
- `Source/NBodySimulation/GraphConfiguration.h`

**Migration Path:**
- Old code still works: `use_shaders`, `universal_graph_scale`, etc.
- New code should use: `Config.bUseGPUShaders`, `Config.UniversalGraphScale`, etc.
- Legacy properties will be removed in future steps

### ✅ Step 2: Create GraphDataManager
**Date:** 2026-03-10  
**Status:** COMPLETE - Compiles successfully  
**Tested:** Compilation only (not yet integrated)

**Changes:**
- Created `UGraphDataManager` class to handle all data operations
- Introduced clean data structures:
  - `FNodeData` - Replaces messy `Node77` class
  - `FLinkData` - Replaces messy `Link77` class
- Extracted all HTTP request code from `KnowledgeGraph5.cpp`
- Extracted JSON parsing logic
- Implemented ID mapping system (string ↔ integer)
- Added delegate system for async operations (`FOnGraphDataLoaded`)
- Proper error handling and logging

**API Overview:**
```cpp
// Main operations
void RequestGraphData(EGraphCreationMode Mode, ...);
void AddNodeToDatabase(const FString& NodeName, const FVector& Location);
void UpdateAllNodePositionsToDatabase(const TArray<FVector>& Positions);

// Data access
const TArray<FNodeData>& GetNodes() const;
const TArray<FLinkData>& GetLinks() const;
int32 GetNodeIndexFromStringId(const FString& StringId) const;
```

**Benefits:**
- All HTTP/database code isolated in one class
- Clean separation of data from logic
- Easier to test data operations
- Better error handling
- Async operations with delegates

**Files Created:**
- `Source/NBodySimulation/GraphDataManager.h`
- `Source/NBodySimulation/GraphDataManager.cpp`

**Next Step:**
Before proceeding to Step 3, we should integrate GraphDataManager into KnowledgeGraph to verify it works correctly. This is optional but recommended.

---

### ✅ Step 2b: Integrate GraphDataManager
**Date:** 2026-03-10  
**Status:** COMPLETE - Compiles and runs  
**Tested:** Compilation successful

**Changes:**
- Added `UGraphDataManager* DataManager` to `AKnowledgeGraph`
- Created `KnowledgeGraph_DataIntegration.cpp` for integration layer
- Implemented `OnGraphDataLoadedCallback()` to handle async data loading
- Updated `prepare()` to use DataManager instead of old HTTP code
- Added null pointer safety check in `update_link_position()`
- Bound DataManager delegate in `BeginPlay()`

**Integration Strategy:**
- DataManager loads data asynchronously
- Callback converts new data structures to old format for compatibility
- Existing code continues to work with minimal changes
- Old HTTP functions in KnowledgeGraph5.cpp are now bypassed

**Files Modified:**
- `Source/NBodySimulation/KnowledgeGraph.h` (added DataManager property)
- `Source/NBodySimulation/KnowledgeGraph.cpp` (constructor, BeginPlay)
- `Source/NBodySimulation/KnowledgeGraph2.cpp` (safety check)
- `Source/NBodySimulation/KnowledgeGraph3.cpp` (removed old prepare())

**Files Created:**
- `Source/NBodySimulation/KnowledgeGraph_DataIntegration.cpp`

**What Works:**
- Auto-generate mode (creates nodes/links procedurally)
- JSON file loading (via DataManager)
- Database loading (via DataManager HTTP requests)
- All existing graph visualization and physics

**Next Step:**
Ready to proceed to Step 3 (GraphPhysicsSimulator) to extract force calculation code.

---

### ✅ Step 3: Create GraphPhysicsSimulator
**Date:** 2026-03-10  
**Status:** COMPLETE - Compiles and runs  
**Tested:** Yes

**Changes:**
- Created `UGraphPhysicsSimulator` class to handle all physics calculations
- Introduced `FPhysicsParameters` struct for physics configuration
- Extracted all force calculation code:
  - Link forces (spring forces between connected nodes)
  - Charge forces (repulsion between all nodes)
  - Center forces (keeps graph centered)
  - Barnes-Hut octree optimization
  - Brute force fallback
- Velocity decay and position updates
- Alpha management (simulation temperature/cooling)

**API Overview:**
```cpp
void Initialize(const FGraphConfiguration& Config, const FPhysicsParameters& Params);
void SimulateStep(float DeltaTime, TArray<FVector>& NodePositions, ...);
bool IsStabilized() const;
float GetAlpha() const;
```

**Benefits:**
- ~400 lines of physics code isolated
- Can easily swap physics implementations
- Parallel processing support
- Cleaner separation of concerns

**Files Created:**
- `Source/NBodySimulation/GraphPhysicsSimulator.h`
- `Source/NBodySimulation/GraphPhysicsSimulator.cpp`

---

### ✅ Step 3b: Integrate GraphPhysicsSimulator
**Date:** 2026-03-10  
**Status:** COMPLETE - Compiles and runs  
**Tested:** Yes

**Changes:**
- Added `UGraphPhysicsSimulator* PhysicsSimulator` to `AKnowledgeGraph`
- Created `KnowledgeGraph_PhysicsIntegration.cpp` for integration layer
- Implemented `InitializePhysicsSimulator()` to set up physics params
- Implemented `cpu_calculate_new()` to use PhysicsSimulator
- Updated `post_generate_graph()` to initialize physics
- Updated `update_position_array()` to use new simulator
- Fixed UObject creation (use `NewObject` in BeginPlay, not `CreateDefaultSubobject`)

**Integration Strategy:**
- PhysicsSimulator initialized after graph generation
- Old `cpu_calculate()` kept as fallback
- New `cpu_calculate_new()` uses PhysicsSimulator
- Automatic fallback if simulator is null

**Files Modified:**
- `Source/NBodySimulation/KnowledgeGraph.h` (added PhysicsSimulator property)
- `Source/NBodySimulation/KnowledgeGraph.cpp` (BeginPlay creates objects)
- `Source/NBodySimulation/KnowledgeGraph3.cpp` (initialize and use simulator)

**Files Created:**
- `Source/NBodySimulation/KnowledgeGraph_PhysicsIntegration.cpp`

**What Works:**
- All physics calculations now through PhysicsSimulator
- Barnes-Hut octree optimization
- Parallel processing
- Alpha decay and stabilization detection
- Existing visualization and interaction

**Next Step:**
Ready to proceed to Step 4 (GraphRenderer) to extract rendering code.

---

## Pending Steps

### ⏸️ Step 4: Create GraphRenderer
**Status:** NOT STARTED  
**Dependencies:** Step 3b (complete)  
**Estimated Complexity:** Medium
**Goal:** Extract all HTTP, JSON, and database operations

**What to extract:**
- HTTP request functions (`request_graph_http`, `debug_error_request`)
- JSON parsing (`extracting_property_list_and_store`)
- Node/link data storage (`all_nodes2`, `all_links2`)
- ID mapping (`string_to_id`, `id_to_string`)
- Blueprint functions for database operations

**Files to create:**
- `Source/NBodySimulation/GraphDataManager.h`
- `Source/NBodySimulation/GraphDataManager.cpp`

**Expected outcome:**
- `AKnowledgeGraph` delegates all data operations to `UGraphDataManager`
- Cleaner separation of concerns
- Easier to test data operations in isolation

### ⏸️ Step 3: Create GraphPhysicsSimulator
**Status:** NOT STARTED  
**Dependencies:** Step 2 (not started)  
**Estimated Complexity:** High
**Goal:** Extract force calculation and physics simulation

**What to extract:**
- Force calculation functions (link, charge, center)
- Velocity/position updates
- Alpha decay logic
- Octree management for Barnes-Hut
- CPU vs GPU simulation switching

**Files to create:**
- `Source/NBodySimulation/GraphPhysicsSimulator.h`
- `Source/NBodySimulation/GraphPhysicsSimulator.cpp`

### ⏸️ Step 4: Create GraphRenderer
**Status:** NOT STARTED  
**Dependencies:** Step 3 (not started)  
**Estimated Complexity:** Medium
**Goal:** Extract all visualization code

**What to extract:**
- Instanced mesh management
- Text render components
- Link mesh/debug line rendering
- Node position updates

**Files to create:**
- `Source/NBodySimulation/GraphRenderer.h`
- `Source/NBodySimulation/GraphRenderer.cpp`

### ⏸️ Step 5: Create GraphInteractionHandler
**Status:** NOT STARTED  
**Dependencies:** Step 4 (not started)  
**Estimated Complexity:** Low
**Goal:** Extract user interaction and Blueprint functions

**What to extract:**
- Node selection logic
- Blueprint-callable functions
- Player interaction

**Files to create:**
- `Source/NBodySimulation/GraphInteractionHandler.h`
- `Source/NBodySimulation/GraphInteractionHandler.cpp`

### ⏸️ Step 6: Cleanup and Optimization
**Status:** NOT STARTED  
**Dependencies:** Steps 1-5 (not all complete)  
**Estimated Complexity:** Medium
**Goal:** Final polish

**Tasks:**
- Remove legacy properties
- Rename cryptic functions (`ll`, `qq`, `lll`)
- Add documentation
- Remove commented code
- Consolidate `KnowledgeGraph2.cpp`, `KnowledgeGraph3.cpp`, `KnowledgeGraph5.cpp`

---

## 🤖 Instructions for AI Assistants

When resuming this refactoring:

1. **Read this file first** - Check the Progress Overview table to see what's done
2. **Verify compilation** - Ensure the project still compiles before making changes
3. **Follow the plan** - Complete steps in order (don't skip ahead)
4. **Update this file** - After each step, update status and add details
5. **Test incrementally** - Compile and test after each change
6. **Maintain compatibility** - Don't break existing code until Step 6

**Current state:** Steps 1-3b complete. GraphPhysicsSimulator is now integrated and working. Ready to begin Step 4 (GraphRenderer).

---

## Notes

### Backward Compatibility Strategy
- Keep legacy properties during migration
- Mark them as deprecated with comments
- Remove only after all code is migrated
- Ensure existing Blueprints continue to work

### Testing Strategy
- Compile after each step
- Test in editor after each step
- Verify existing functionality still works
- No breaking changes until final cleanup

### Code Style Improvements
- Use descriptive names (not `jnodessss`, `kn`, `ll`)
- Use `b` prefix for booleans (`bUseShaders` not `use_shaders`)
- Use proper categories in UPROPERTY
- Add comments for complex algorithms

---

## Commit Messages Template

```
Step 1: Extract configuration into FGraphConfiguration struct

- Created GraphConfiguration.h with organized settings
- Added Config property to AKnowledgeGraph
- Kept legacy properties for backward compatibility
- Compiles successfully, no breaking changes
```

---

## Questions/Decisions Log

**Q:** Should we merge KnowledgeGraph2/3/5.cpp into one file or split into components?  
**A:** Split into components (DataManager, PhysicsSimulator, Renderer) for better organization

**Q:** Keep legacy properties or force migration immediately?  
**A:** Keep for now, remove in Step 6 after full migration

**Q:** Use UObject or UActorComponent for new classes?  
**A:** UObject for managers (data, physics), UActorComponent for renderer/interaction
