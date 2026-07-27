# Enforce Context-Mode MCP Tools

## Rule
Always use the `context-mode` MCP tools instead of native tools for file inspection, command execution, and search.

## Tool Mappings
- **Command Execution:** Use `ctx_execute` (with `language: "shell"`) instead of `run_command`.
- **File Inspection / Analysis:** Use `ctx_execute_file` (to analyze in-sandbox) or `ctx_execute` instead of `view_file`.
- **Code/Pattern Search:** Use `ctx_search` or `ctx_batch_execute` instead of `grep_search`.
