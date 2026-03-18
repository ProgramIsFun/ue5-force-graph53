# Contribution Guidelines

## ParallelFor Usage

**DO NOT flag ParallelFor as a race condition concern** in this project's physics code. Each thread writes only to its own index in the shared arrays (e.g., `nodeVelocities[Index]`), so there is no data race. This applies to both the octree and brute force paths in `CalculateChargeForceAndUpdateVelocity()`, as well as `UpdatePositionArrayAccordingToVelocityArray()`. This is an intentional design decision — do not suggest removing or replacing ParallelFor with sequential loops.

## Naming Conventions

### Variable and Function Naming

When refactoring or introducing new variables, functions, or class members:

- Use **conventional naming patterns** that follow C++ and Unreal Engine standards
- **Avoid overly generic names** that might conflict with core library implementations (Unreal Engine, STL, etc.)
- Prefer **domain-specific prefixes or context** to prevent naming collisions

#### Examples:

**Good:**
- `GraphNodePosition` instead of `Position`
- `SimulationDeltaTime` instead of `DeltaTime`
- `KnowledgeGraphEdges` instead of `Edges`
- `PhysicsBodyMass` instead of `Mass`

**Avoid:**
- Generic names like `Data`, `Value`, `Index`, `Count` without context
- Names that shadow Unreal types: `Transform`, `Vector`, `Actor` (without prefix/context)
- STL-like names: `begin`, `end`, `size` (unless implementing iterator interface)

### Rationale

Unreal Engine has extensive core libraries with common names. Using specific, contextual names:
- Prevents ambiguous symbol resolution
- Reduces risk of shadowing engine types
- Makes code more self-documenting
- Avoids subtle bugs from name conflicts

## Project Structure

This project uses a modular architecture with split implementation files:
- `KnowledgeGraph.h` - Main header
- `KnowledgeGraph_Core.cpp` - Core functionality
- `KnowledgeGraph_Physics.cpp` - Physics integration
- `KnowledgeGraph_BlueprintAPI.cpp` - Blueprint exposure
- `KnowledgeGraph_DataIntegration.cpp` - Data handling
- `KnowledgeGraph_Utilities.cpp` - Helper functions

When adding functionality, place it in the appropriate module file.

### Architecture: Non-Actor Based Graph System

**IMPORTANT:** For performance reasons, nodes and edges in this graph system are **NOT** implemented as Actors.

#### Key data types:
- `GraphNode` — holds `int id` and `UTextRenderComponent* textComponent`
- `GraphLink` — holds source/target indices, bias/strength/distance, and `UStaticMeshComponent* EdgeMeshComponent`
- `FNodeData` / `FLinkData` (in `GraphDataManager.h`) — serializable data structs for the data manager

#### Rationale:
- Actor-based implementations have significant performance overhead
- Large graphs with thousands of nodes/edges would cause severe performance degradation
- The current architecture uses lightweight data structures managed by the `KnowledgeGraph` component
- Nodes and edges are represented as structs/data within the graph manager, not as individual actors

When implementing graph features, always work with the data-oriented approach in `KnowledgeGraph` rather than creating actor instances.

### UI System

The project includes a C++ Slate-based UI system for in-game graph controls:
- `Source/NBodySimulation/GraphSystem/UI/GraphControlPanelWidget` — Main panel with toggle/action buttons
- `Source/NBodySimulation/GraphSystem/UI/GraphPlayerController` — Handles Home key to show/hide panel
- `Source/NBodySimulation/GraphSystem/UI/GraphGameMode` — Wires in the custom PlayerController

The panel directly modifies `FGraphConfiguration` values at runtime. Adding new buttons requires only one line in `InitializePanel()`.

## Progress Tracking for Major Changes

When making significant changes to the codebase (refactoring, migrations, architectural changes, etc.):

1. **Create a progress tracking file** in the project root with a descriptive name:
   - Format: `FEATURE_NAME_SUMMARY.md` or `CHANGE_TYPE_PROGRESS.md`
   - Examples: `CONFIG_MIGRATION_SUMMARY.md`, `ARRAY_BOUNDS_FIX.md`, `PHYSICS_REFACTOR_PROGRESS.md`

2. **Document in the tracking file:**
   - Overview of the change
   - Files affected
   - Progress checklist
   - Known issues or blockers
   - Testing status
   - Completion criteria

3. **Reference in README:**
   - Add a link to the tracking file in the root README.md
   - Include a brief description of the ongoing work
   - Remove or archive the reference once work is complete

### Benefits:
- Provides clear visibility into ongoing work
- Helps resume work after interruptions
- Documents decisions and rationale
- Makes it easier for others to understand changes
- Creates a historical record of major modifications

## Command Execution

This project uses **Windows CMD** (not PowerShell) for command execution.

When executing commands, use CMD syntax:
- List files: `dir`
- Remove file: `del file.txt`
- Remove directory: `rmdir /s /q dir`
- Copy file: `copy source.txt destination.txt`
- Create directory: `mkdir dir`
- View file content: `type file.txt`
- Command separator: `&` (not `&&` or `;`)
