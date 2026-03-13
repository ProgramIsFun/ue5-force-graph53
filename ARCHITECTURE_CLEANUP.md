# Architecture Cleanup - Remove Old/New Duplication

**Date:** 2026-03-12  
**Task:** Complete migration to new architecture, remove "_new" suffix and old implementations

---

## Overview

The codebase has dual implementations (old and new) from an incomplete refactoring. The new systems (Renderer, PhysicsSimulator, DataManager) are working and should become the standard implementation.

---

## Functions to Migrate

### 1. Physics Calculation ✅ TO DO
**Old:** `cpu_calculate()` - Direct physics calculations
**New:** `cpu_calculate_new()` - Uses PhysicsSimulator
**Action:** Replace old with new, remove "_new" suffix

### 2. Node Position Updates ✅ TO DO
**Old:** `update_node_world_position_according_to_position_array()` - Direct actor updates
**New:** `update_node_world_position_according_to_position_array_new()` - Uses Renderer
**Action:** Replace old with new, remove "_new" suffix

### 3. Link Position Updates ✅ TO DO
**Old:** `update_link_position()` - Direct mesh updates
**New:** `update_link_position_new()` - Uses Renderer
**Action:** Replace old with new, remove "_new" suffix

### 4. Text Rotation ✅ TO DO
**Old:** `rotate_to_face_player111()` - Direct component rotation
**New:** `rotate_to_face_player_new()` - Uses Renderer
**Action:** Replace old with new, remove "_new" suffix

### 5. Object Generation ✅ TO DO
**Old:** `generate_objects_for_node_and_link()` - Legacy generation
**New:** `generate_objects_for_node_and_link_new()` - Uses DataManager
**Action:** Replace old with new, remove "_new" suffix

---

## Migration Strategy

### Step 1: Remove Old Implementations
Delete the old function implementations from:
- `KnowledgeGraph_Core.cpp` - `cpu_calculate()`
- `KnowledgeGraph_Physics.cpp` - `update_link_position()`, `update_node_world_position_according_to_position_array()`, `generate_objects_for_node_and_link()`
- `KnowledgeGraph_Core.cpp` - `rotate_to_face_player111()`

### Step 2: Rename New Functions (Remove "_new" suffix)
- `cpu_calculate_new()` → `cpu_calculate()`
- `update_node_world_position_according_to_position_array_new()` → `update_node_world_position_according_to_position_array()`
- `update_link_position_new()` → `update_link_position()`
- `rotate_to_face_player_new()` → `rotate_to_face_player()`
- `generate_objects_for_node_and_link_new()` → `generate_objects_for_node_and_link()`

### Step 3: Update All Call Sites
Update calls in:
- `KnowledgeGraph_Core.cpp` - main_function()
- `KnowledgeGraph_DataIntegration.cpp` - prepare()
- Any other files that reference these functions

### Step 4: Update Header Declarations
Update function declarations in `KnowledgeGraph.h`

---

## Files to Modify

**Headers:**
- `Source/NBodySimulation/GraphSystem/KnowledgeGraph.h`

**Implementation:**
- `Source/NBodySimulation/GraphSystem/KnowledgeGraph_Core.cpp`
- `Source/NBodySimulation/GraphSystem/KnowledgeGraph_Physics.cpp`
- `Source/NBodySimulation/GraphSystem/KnowledgeGraph_PhysicsIntegration.cpp`
- `Source/NBodySimulation/GraphSystem/KnowledgeGraph_RenderIntegration.cpp`
- `Source/NBodySimulation/GraphSystem/KnowledgeGraph_DataIntegration.cpp`

---

## Safety Checks

Before removing old implementations, verify:
- ✅ Renderer is always initialized in BeginPlay()
- ✅ PhysicsSimulator is always initialized in BeginPlay()
- ✅ DataManager is always initialized in BeginPlay()

Current initialization in `KnowledgeGraph.cpp::BeginPlay()`:
```cpp
DataManager = NewObject<UGraphDataManager>(this);
PhysicsSimulator = NewObject<UGraphPhysicsSimulator>(this);
Renderer = NewObject<UGraphRenderer>(this);
```

All three are guaranteed to be created, so fallbacks are unnecessary.

---

## Progress

- [ ] Remove old implementations
- [ ] Rename new functions (remove "_new")
- [ ] Update call sites
- [ ] Update header declarations
- [ ] Verify compilation
- [ ] Test functionality

---

## Status

**Current:** Planning complete, ready to execute  
**Blocked:** None  
**Next:** Remove old implementations
