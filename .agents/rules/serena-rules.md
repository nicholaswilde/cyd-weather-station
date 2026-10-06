# Serena Semantic Navigation & LSP Integration

## Rule
Always activate and use Serena (`serena`) MCP tools for semantic navigation, AST symbol inspection, LSP diagnostics, and project memory management.

## Initialization
Before modifying or analyzing code, ensure the project is active:
1. Call `activate_project(project="cyd-weather-station")` if not already activated.
2. Call `read_memory(memory_name="critical_info")` to load essential project constraints.

## Tool Mappings & Capabilities
- **Symbol Overview & Definitions:** Use `get_symbols_overview` and `find_symbol` to inspect class/function signatures and bodies without loading entire files into context.
- **Reference & Caller Tracking:** Use `find_referencing_symbols` and `find_implementations` to trace symbol usage across the codebase.
- **LSP Diagnostics:** Use `get_diagnostics_for_file` (Clangd / Pyright LSP) to verify type consistency and catch syntax/lint errors before committing.
- **Safe Refactoring:** Use `rename_symbol` and `safe_delete_symbol` for atomic, reference-safe AST refactors across all files.
- **Project Memories:** Use `read_memory` and `write_memory` to access and persist durable project context in `.serena/memories/`.

## CodeGraph vs. Serena Boundary
- **CodeGraph (`codegraph_explore`):** Use for multi-hop repository architecture, cross-language blast radius, and system-level call paths.
- **Serena (`serena`):** Use for symbol-level semantic navigation, LSP diagnostics, AST-aware refactoring, and durable memories.
