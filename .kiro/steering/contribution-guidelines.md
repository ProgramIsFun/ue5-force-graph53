# Contribution Guidelines

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
