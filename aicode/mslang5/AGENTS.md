# AGENTS.md

## Model Marker

- Current model: `gpt-*-codex`

## Project

- Name: `Maple`
- Language runtime namespace: `ms`
- Goal: implement a clox-style scripting language runtime in modern C++.

## Implementation Rules

- Use `C++23` or newer.
- Keep code multi-file. Do not collapse the runtime into a single file.
- Use `.hh` for headers and `.cc` for source files.
- Prefer `#pragma once` in headers.
- Prefer the C++ standard library; minimize third-party dependencies.
- Keep the project cross-platform for:
  - Windows (MSVC)
  - Linux (GCC)
- Use `CMake` as the build system.

## Language Scope

- Baseline should align with `clox` capabilities from Crafting Interpreters.
- Include VM and garbage collection.
- Add Maple module import features:
  - `import <module>`
  - `from <module> import <name> as <alias>`

## Logging

- Provide an internal logger with log levels.
- Use different colors per log level.
- Ensure color behavior works on both Windows terminals and Linux terminals.

## Testing

- Keep tests in a dedicated `tests/` directory.
- Include detailed test scripts for:
  - core language behavior
  - VM execution
  - GC behavior
  - module import behavior
  - CLI/REPL behavior

## Documentation-First Workflow

- Before major implementation changes, update planning/design docs first.
- Keep requirement/design decisions in Markdown documents in the repo root.

## Git Commit Convention

- Commit messages must be written in English.
- Every commit message must include a gitmoji marker.
- Recommended format:
  - `:<gitmoji>: <type>(<scope>): <short english summary>`

## File Formatting Convention

- All implementation code files must be encoded as UTF-8.
- Use LF (`\n`) line endings for implementation code files.
- Automatically remove trailing whitespace at the end of lines.

## Task Progress Convention

- After each functional task is implemented, update task status in `PLAN.md` immediately.
- Task status updates in `PLAN.md` must be accurate and traceable to the delivered changes/tests.
