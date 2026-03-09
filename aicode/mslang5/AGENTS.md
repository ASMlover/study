# AGENTS.md

## Model Marker

- Current model: `gpt-*-codex`

## Project

- Name: `Maple`
- Runtime namespace: `ms`
- Goal: implement a clox-style scripting runtime in modern C++.

## Implementation Rules

- Use `C++23+`.
- Keep a multi-file codebase; do not merge runtime into one file.
- Use `.hh` headers and `.cc` sources.
- Prefer `#pragma once` in headers.
- Prefer the standard library; minimize third-party deps.
- Follow the Google C++ Style Guide as baseline.
- Use `lower_snake_case` for function names.
- Keep cross-platform support for:
  - Windows (MSVC)
  - Linux (GCC)
- Use `CMake`.

## Language Scope

- Align baseline with `clox` in Crafting Interpreters.
- Include VM and GC.
- Add Maple module imports:
  - `import <module>`
  - `from <module> import <name> as <alias>`

## Logging

- Provide an internal logger with levels.
- Use distinct colors by level.
- Ensure colors work on Windows and Linux terminals.

## Testing

- Keep tests under `tests/`.
- Include detailed scripts for:
  - core language behavior
  - VM execution
  - GC behavior
  - module import behavior
  - CLI/REPL behavior

## Documentation-First Workflow

- Update planning/design docs before major implementation changes.
- Keep requirement/design decisions in root Markdown docs.

## Git Commit Convention

- Commit messages must be English.
- Every commit message must include a gitmoji.
- Recommended format:
  - `:<gitmoji>: <type>(<scope>): <short english summary>`

## File Formatting Convention

- Encode all implementation code files as UTF-8.
- Use LF (`\n`) line endings for implementation code files.
- Auto-remove trailing whitespace.

## Task Progress Convention

- Update `PLAN.md` status immediately after each functional task.
- `PLAN.md` status updates must be accurate and traceable to delivered changes/tests.

## Document Simplification Command

- Trigger: user input matches `精简 xxx.md`.
- Requirement: simplify wording as much as possible without changing the original item set.
- Hard constraints:
  - Do not add items.
  - Do not remove items.
  - Do not change the intent of design/rule/specification statements.

## Git Commit Command

- Trigger: user input is `git`.
- Action: commit all changes in the current directory.
