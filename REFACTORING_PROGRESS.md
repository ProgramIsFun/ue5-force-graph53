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
| 2 | GraphDataManager | ⏸️ NOT STARTED | - |
| 3 | GraphPhysicsSimulator | ⏸️ NOT STARTED | - |
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

---

## Pending Steps

### ⏸️ Step 2: Create GraphDataManager
**Status:** NOT STARTED  
**Dependencies:** Step 1 (complete)  
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

**Current state:** Step 1 complete. Ready to begin Step 2 (GraphDataManager).

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
