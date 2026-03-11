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
