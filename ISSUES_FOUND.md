# Issues Found - March 12, 2026

## Critical Issues

### 1. Timer Handle Leak (FIXED)
**Location**: `GraphDataManager.cpp` - `OnDatabaseRequestComplete()`
**Issue**: `RetryTimerHandle` is set but never cleared (FIXED)
**Impact**: Potential crash if object destroyed during retry
**Status**: ✅ Fixed - Timer now cleared in `ClearAllData()`

### 2. Array Bounds Safety (FIXED)
**Location**: `KnowledgeGraph_DataIntegration.cpp` - `OnGraphDataLoadedCallback()`
**Issue**: Links created without validating source/target indices (FIXED)
**Impact**: Potential crash if JSON has invalid link references
**Status**: ✅ Fixed - Added bounds checking and validation in `BuildIdMappings()`

## Code Quality Issues

### 3. Naming Convention Violations
**Violates**: `contribution-guidelines.md` naming standards

**Examples**:
- `id` → should be `NodeId` or `GraphNodeId`
- `node` → should be `GraphNode` or `SimulationNode`
- `edge` → should be `GraphEdge` or `KnowledgeEdge`
- `source`, `target` → should be `SourceNodeIndex`, `TargetNodeIndex`
- `bias`, `strength` → should be `LinkBias`, `LinkStrength`

**Files Affected**:
- `KnowledgeGraph.h` (GraphLink, GraphNode classes)
- Multiple implementation files

### 4. Performance: Debug Drawing
**Location**: `KnowledgeGraph.cpp` - `Tick()`
**Issue**: `DrawDebugGrid()` called every frame unconditionally
**Impact**: Unnecessary performance overhead
**Fix**: Add config flag to enable/disable, or only draw when needed

### 5. Legacy Code Duplication
**Issue**: Multiple versions of same functions:
- `generate_objects_for_node_and_link()` vs `generate_objects_for_node_and_link_new()`
- `cpu_calculate()` vs `cpu_calculate_new()`
- `update_node_world_position_according_to_position_array()` vs `*_new()`

**Impact**: Code maintenance burden, confusion
**Fix**: Remove old versions after migration complete

## Minor Issues

### 6. Inconsistent Logging
- Some functions use `LogMessage()` with config check
- Others use `UE_LOG()` directly
- `GraphDataManager` has its own logging system

**Fix**: Standardize on one logging approach

### 7. Magic Numbers
- HTTP timeout: 30.0f (should be config constant)
- Retry count: 3 (should be config constant)
- Retry delay: 2.0f (should be config constant)

### 8. Comment Quality
- Some comments are outdated ("formerly: jnodessss")
- Some are too verbose
- Some are missing entirely

## Recommendations

### Immediate Actions (High Priority)
1. ✅ Fix timer handle leak in GraphDataManager
2. ✅ Add array bounds validation for links
3. ✅ Reduce HTTP timeout to 10 seconds
4. Add config flag for debug grid drawing (optional)

### Short Term (Medium Priority)
1. Refactor naming conventions in GraphLink/GraphNode
2. Add config flag for debug grid drawing
3. Remove legacy function versions
4. Standardize logging

### Long Term (Low Priority)
1. Complete migration to new architecture
2. Add comprehensive error handling
3. Add unit tests for data loading
4. Document all public APIs

## Testing Checklist

- [ ] Test with database server offline (should fail gracefully)
- [ ] Test with invalid JSON file
- [ ] Test with missing JSON file
- [ ] Test with malformed link references
- [ ] Test with empty nodes array
- [ ] Test with very large graphs (1000+ nodes)
- [ ] Test hot-reload during data loading
- [ ] Test multiple rapid requests

## Files Requiring Changes

1. `Source/NBodySimulation/GraphSystem/GraphDataManager.cpp` (Critical)
2. `Source/NBodySimulation/GraphSystem/GraphDataManager.h` (Critical)
3. `Source/NBodySimulation/GraphSystem/KnowledgeGraph_DataIntegration.cpp` (Critical)
4. `Source/NBodySimulation/GraphSystem/KnowledgeGraph.h` (Naming)
5. `Source/NBodySimulation/GraphSystem/KnowledgeGraph.cpp` (Performance)
6. `Source/NBodySimulation/GraphSystem/GraphConfiguration.h` (Add constants)
