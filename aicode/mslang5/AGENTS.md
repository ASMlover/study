# AGENTS.md

## Model Marker

- Current model: `gpt-*-codex`

## Project

- Name: `Maple`
- Runtime namespace: `ms`
- Goal: implement a clox-style scripting runtime in modern C++.

## Implementation Rules

- Use `C++23+`.
- Keep a multi-file codebase; do not collapse runtime into one file.
- Use `.hh` headers and `.cc` sources.
- Prefer `#pragma once` in headers.
- Prefer the standard library; minimize third-party dependencies.
- Follow the Google C++ Style Guide.
- Use `lower_snake_case` for function names.
- Prefer using `noexcept` whenever it is semantically safe and can be verified.
- Prefer using `inline` whenever it is semantically safe and improves small-function call overhead/readability.
- Keep cross-platform support:
  - Windows (MSVC)
  - Linux (GCC)
- Use `CMake`.

## Language Scope

- Align with `clox` in Crafting Interpreters.
- Include VM and GC.
- Support module imports:
  - `import <module>`
  - `from <module> import <name> as <alias>`

## Logging

- Provide an internal leveled logger.
- Use distinct colors per level.
- Ensure colors work on Windows/Linux terminals.

## Testing

- Keep tests under `tests/`.
- Include tests for:
  - core language behavior
  - VM execution
  - GC behavior
  - module import behavior
  - CLI/REPL behavior

## Documentation-First Workflow

- Update planning/design docs before major changes.
- Keep requirement/design decisions in root Markdown docs.

## Git Commit Convention

- Commit messages must be English.
- Every commit must include a gitmoji.
- Format:
  - `:<gitmoji>: <type>(<scope>): <short english summary>`

## File Formatting Convention

- Use UTF-8 for all implementation code files.
- Use LF (`\n`) line endings in implementation code files.
- Auto-remove trailing whitespace.

## Task Progress Convention

- Update `PLAN.md` immediately after each functional task.
- `PLAN.md` updates must be accurate and traceable to delivered changes and tests.

## Document Simplification Command

- Trigger: user input matches `精简 xxx.md`.
- Requirement: simplify the original document without changing (adding or removing) design/specification content.
- Hard constraints:
  - Do not add items.
  - Do not remove items.
  - Do not change the intent of original design/specification statements.

## Git Commit Command

- Trigger: user input is `git`.
- Action: run `git commit` for modifications in the current directory.
