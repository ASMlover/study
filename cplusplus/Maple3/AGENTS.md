# Repository Guidelines

## Project Background

Maple3 is a lightweight scripting language interpreter project. It currently focuses on the frontend (lexer/scanner) and is evolving toward a full pipeline (parser, runtime, and CLI) using a simple, CMake-based C++20 codebase.
The project aims to stay small, readable, and portable while growing toward a complete interpreter stack, with emphasis on a clean C++20 codebase and straightforward CMake workflows.

## Project Structure & Module Organization

- Root directory contains all production sources as `*.cc` and `*.hh` (for example `Main.cc`, `Scanner.cc`, `Token.hh`).
- `CMakeLists.txt` defines the build and compiler flags.
- `build/` is a generated output directory (create via CMake; do not commit build artifacts).

## Build, Test, and Development Commands

- Configure: `cmake -S . -B build` (generates build files into `build/`).
- Build: `cmake --build build --config Release` (or `Debug` for symbols and checks).
- Run: `build/Maple` (executable name matches the `project(Maple)` target).

## Getting Started (Platform Notes)

- Windows (MSVC): use `cmake -S . -B build -G "Visual Studio 17 2022"` and build with `--config Debug` or `Release`.
- Linux/macOS (Clang/GCC): configure with `cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug` (or `Release`).
- If you change compiler flags, update `CMakeLists.txt` so both MSVC and non-MSVC paths stay aligned.

## Coding Style & Naming Conventions

- Indentation: 2 spaces; no tabs.
- Headers use `#pragma once` and are named `*.hh`; implementations are `*.cc`.
- Namespaces use short, lowercase prefixes (e.g., `ms`).
- Prefer `snake_case` for functions and `kCONSTANT`-style for constants (see `Consts.hh`).
- Keep includes ordered: standard headers first, then local headers.
- Keep comments brief and let code be self-explanatory where possible.

## Agent Workflow Rules

- At the start of every session, read `Plans.md` first.
- At the end of every task round, update `Plans.md` with progress and next steps.

## Testing Guidelines

- No dedicated test framework is configured in this repository yet.
- If you add tests, place them under a new `tests/` directory and wire them into `CMakeLists.txt`.
- Name tests by feature, for example `tests/scanner_tokenize.cc`.

## Commit & Pull Request Guidelines

- Commit messages follow a conventional pattern with emojis and scopes, e.g., `:construction: chore(scanner): updated the scanner implementation` and `:memo: docs(aicode): add README for aicode`.
- Use a short imperative subject, keep scope specific, and avoid multiple topics in one commit.
- PRs should include: a brief summary, the commands run (build/test), and any relevant context or screenshots for behavior changes.

## Configuration Notes

- The build uses C++20 (`-std=c++2a`/`/std:c++latest`) and treats warnings as errors on non-MSVC compilers.
- If you introduce new dependencies, document them in this file and update `CMakeLists.txt` accordingly.
