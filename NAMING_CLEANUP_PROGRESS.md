# Naming Convention Cleanup Progress

**Date:** 2026-03-12  
**Task:** Clean up poor naming conventions to follow Unreal Engine and project standards

---

## Overview

Removing meaningless number suffixes and placeholder names throughout the codebase to improve code clarity and maintainability.

---

## Changes to Make

### 1. Class Names ✅ COMPLETE
- `Node77` → `GraphNode` (semantic name for graph node data)
- `Link77` → `GraphLink` (semantic name for graph edge/link data)

**Rationale:** The "77" suffix has no meaning. Using `GraphNode` and `GraphLink` follows the contribution guidelines for domain-specific naming and prevents conflicts with Unreal's base types.

**Files Modified:**
- ✅ KnowledgeGraph.h (class definitions)
- ✅ KnowledgeGraph_Physics.cpp
- ✅ GraphPhysicsSimulator.h/cpp
- ✅ GraphRenderer.h/cpp
- ✅ NBodyOctree.h
- ✅ NBodyOctreeImpl.cpp

**Status:** All references updated, zero diagnostics

### 2. Function Names with Number Suffixes
- `get_player_location727()` → `GetPlayerLocation()`
- `get_location_of_somewhere_in_front_of_player727()` → `GetLocationInFrontOfPlayer()`

**Rationale:** The "727" suffix is meaningless. Using PascalCase follows Unreal conventions for public member functions.

### 3. Blueprint Function Names
- `set_text_size_of_all_nodes1112()` → `SetNodeTextSize()`
- `increase_or_decrease_text_size_of_all_nodes1112()` → `AdjustNodeTextSize()`
- `increase_text_size_of_all_nodes1112()` → `IncreaseNodeTextSize()`
- `decrease_text_size_of_all_nodes1112()` → `DecreaseNodeTextSize()`
- `update_position_of_all_nodes_to_database1113()` → `SaveNodePositionsToDatabase()`
- `add_link_to_database1114()` → `AddLinkToDatabase()`
- `add_node_to_database1115()` → `AddNodeToDatabase()`
- `delete_node_from_database1116()` → `DeleteNodeFromDatabase()`
- `delete_link_from_database1117()` → `DeleteLinkFromDatabase()`
- `select_closest_node_from_player222()` → `SelectClosestNodeToPlayer()`

**Rationale:** Remove meaningless number suffixes, use PascalCase for Blueprint functions, make names more concise while remaining clear.

### 4. Blueprint Category
- `"YourCategory111111111111222222222"` → `"Knowledge Graph"`

**Rationale:** Replace placeholder with meaningful category name.

---

## Progress

- [x] Issue identified
- [x] Class renames (Node77, Link77) - COMPLETE
- [ ] Function renames (727 suffix)
- [ ] Blueprint function renames
- [ ] Blueprint category fix
- [ ] Compilation verification
- [ ] Update documentation

---

## Status

**Current:** Class renames complete, moving to function renames  
**Blocked:** None  
**Next:** Rename functions with 727 suffix
