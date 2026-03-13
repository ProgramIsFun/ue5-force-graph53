# UE5 Force-Directed Graph

An Unreal Engine 5 implementation of force-directed graph visualization, replicating the functionality of the popular [3d-force-graph](https://github.com/vasturiano/3d-force-graph) library.

![Force-Directed Graph Visualization](https://github.com/user-attachments/assets/4e648745-8ee3-4fc7-9008-fd1d53785711)

## Overview

Force-directed graphs (also known as force-based graphs) are widely used for visualizing complex relationships in various domains:
- Network visualization
- Large graph visualization
- Knowledge representation
- System management
- Mesh visualization

This project brings these capabilities directly into Unreal Engine 5, enabling real-time 3D graph visualization with the power of UE5's rendering pipeline.

## Architecture

### Dependency Chain
```
vasturiano/3d-force-graph
  └─ vasturiano/three-forcegraph
      └─ vasturiano/d3-force-3d
          └─ vasturiano/d3-octree
```

[Live Demo](https://vasturiano.github.io/3d-force-graph/example/large-graph/)

### Shader Implementation
The compute shader code can be found at:
[NBodySimCS.cpp](https://github.com/ProgramIsFun/ue5-force-graph53/blob/master/Plugins/NBodySimShader/Source/NBodySim/Private/NBodySimCS.cpp)

## Project Status

✅ **Stable** - Recent refactoring and improvements have been completed and are ready for testing.

## Comparison to UE4 Force Graph

This project is an improved version of [thomaswall/ue4-force-graph](https://github.com/thomaswall/ue4-force-graph).

### Improvements
- **Modern Engine**: Built for Unreal Engine 5 (vs UE 4.24)
- **Enhanced Physics**: Different implementation of charge force calculations
- **Bug Fixes**: Corrected link bias calculations and other issues

### Trade-offs
- **Performance**: Custom octree implementation may be slower than UE's built-in spatial data structures

## Getting Started with Unreal Engine 5

### 1. Understanding Core Classes

**Actor** → **Pawn** → **Character**

All three are C++ classes with detailed implementations by Epic Games:

- **Actor**: The base class for all objects in the game world. Use for non-controlled objects like platforms, scenery, or interactive props.

- **Pawn**: A controllable entity that can receive input. Ideal for entities with custom movement logic that don't need the full Character feature set.

- **Character**: Extends Pawn with built-in movement capabilities (walking, jumping, etc.). Best for player avatars and NPCs requiring complex locomotion.

### 2. Project Structure

Source code lives in the `Source/` folder. Unlike some projects that separate headers and implementation files, this project keeps `.h` and `.cpp` files together in the same directory for simplicity.

### 3. Implementation Details

The force-directed graph is implemented as a C++ class inheriting from `Pawn`:

- `KnowledgeGraph.cpp`
- `KnowledgeGraph.h`

**Key Functions:**
- **Constructor/Destructor**: Resource management
- **BeginPlay()**: Initialization of nodes, links, and forces
- **Tick()**: Per-frame updates of node and link positions

**Adding to a Level:**
1. Locate `KnowledgeGraph` in the Content Browser
2. Drag it into your level viewport
3. Verify placement in the World Outliner panel

![Level Editor Setup](https://github.com/user-attachments/assets/2042450f-0d52-4c6f-97e2-a8dc2973de14)

### 4. Working with Maps

Open different maps by double-clicking map files in the Content Browser.

### 5. Development Workflow

**Compilation**: Recompile after any C++ source code changes.

**Rapid Iteration Tip**: Use the `UPROPERTY()` macro to expose member variables to the editor, allowing you to tweak values without recompiling.

```cpp
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Graph")
float NodeRepulsionStrength = 100.0f;
```

Learn more about [UPROPERTY macros](https://benui.ca/unreal/uproperty/)

## References

- [vasturiano/3d-force-graph](https://github.com/vasturiano/3d-force-graph)
- [thomaswall/ue4-force-graph](https://github.com/thomaswall/ue4-force-graph)
- [Unreal Engine UPROPERTY Documentation](https://benui.ca/unreal/uproperty/)

