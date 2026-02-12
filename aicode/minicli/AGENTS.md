# Repository Guidelines

## Project Structure & Module Organization
This repository (`aicode/minicli`) is currently a scaffold directory with no committed source files yet. Keep the layout simple and predictable as code is added:

- `src/`: application code (CLI entrypoint and modules)
- `tests/`: automated tests mirroring `src/` structure
- `assets/`: static fixtures or sample data used by tests/docs
- `docs/`: design notes and usage examples

Example: `src/commands/init.*` should have related tests in `tests/commands/test_init.*`.

## Build, Test, and Development Commands
No build system is defined yet in this directory. When introducing tooling, expose a minimal command set and document it in this file and `README.md`.

- `cmake -S . -B build` configures a C/C++ build tree (recommended if using C++).
- `cmake --build build --config Debug` builds local debug artifacts.
- `ctest --test-dir build --output-on-failure -C Debug` runs tests with verbose failures.

Only add commands that work from the repository root and keep them stable.

## Coding Style & Naming Conventions
Use consistent formatting and keep style automation mandatory once configured.

- Indentation: 2 or 4 spaces, no tabs; pick one per language and enforce with formatter config.
- Naming: `snake_case` for files, `PascalCase` for types/classes, `camelCase` for functions/variables.
- Keep modules focused; prefer small files with one clear responsibility.

If C/C++ is used, adopt `clang-format`; if Python is added, use `ruff` + `black`.

## Testing Guidelines
Place tests in `tests/` and mirror production paths. Name tests by behavior, e.g., `test_parse_empty_args`.

- Include unit tests for each new command/module.
- Add regression tests for every bug fix.
- Target meaningful coverage for core command parsing and error handling paths.

## Commit & Pull Request Guidelines
Recent history in the parent repo uses conventional-style subjects with scope and emoji, e.g.:
`:construction: chore(maple): update parser behavior`

- Commit format: `<emoji> <type>(<scope>): <summary>` (imperative, concise).
- Keep commits focused and reviewable.
- PRs should include: purpose, key changes, test evidence, and linked issue/task.
- Attach terminal output or screenshots when behavior/output changes.

## Agent Workflow Rules
- All code files must use UTF-8 encoding and LF line endings.
- All code files must automatically remove trailing whitespace.
- At the start of every task round, read `Plans.md`; at the end of every task round, update `Plans.md`.
- When a task round finishes successfully, update `Tests.md` in the same round.
- If the input is `go TXX`, implement the feature and tests defined in `tasks/TXX.md`.
  Example: `go T01` means implement `tasks/T01.md` functionality and its corresponding test cases.
- If the input is `ci TXX`, create a git commit for files implementing the feature and tests defined in `tasks/TXX.md`.
  Example: `ci T01` means commit the files that implement `tasks/T01.md` functionality and its corresponding test cases.
  Commit message must include the corresponding task's implemented functionality (not only task ID).
