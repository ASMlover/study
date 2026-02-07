# Maple3 Implementation Architecture

## 1. Scope

This document describes the current and planned implementation architecture of Maple3.
It is the source of truth for module responsibilities, pipeline flow, and test strategy.

## 2. Current Architecture (as of 2026-02-06)

### 2.1 Layered Pipeline

1. Source Text Input
2. Scanner/Lexer
3. Token Stream
4. Parser (expression-level + program recovery)
5. AST (expression nodes)
6. Test/Debug Output

### 2.2 Implemented Modules

1. `Scanner.hh` / `Scanner.cc`
- Converts source text into tokens.
- Handles comments, strings, numbers, identifiers, keywords.
- Emits scanner diagnostics as `TK_ERR` token payloads.

2. `Token.hh` / `Token.cc` / `TokenTypes.hh`
- Token enum and token container.
- Includes source location metadata (`line`, `column`, `length`).

3. `Errors.hh` / `Errors.cc`
- Unified diagnostic formatting:
  - `<file>:<line>:<column> error: <message>`
- Runtime and parse error containers.

4. `Ast.hh` / `Ast.cc`
- Expression AST nodes:
  - literal, grouping, unary, binary
- Debug string rendering for parser tests.

5. `Parser.hh` / `Parser.cc`
- Recursive-descent expression parsing with precedence.
- Program parse entry with synchronization-based recovery.

6. `Main.cc`
- Placeholder executable entrypoint (non-final CLI behavior).

### 2.3 Build and Test Topology

1. Build system: CMake (`C++20`)
2. Registered tests in CTest:
- `MapleScanTest`
- `MapleParserTest`
3. Test style:
- Lightweight executable assertions under `tests/`
- Driven via `ctest`

## 3. Target Architecture (Execution Roadmap)

The project evolves toward a full interpreter with complete CLI and script regression suite.

### 3.1 Final Pipeline

1. Source (file/REPL/module)
2. Scanner
3. Parser (declarations + statements + expressions)
4. Resolver (scope/binding checks)
5. Interpreter runtime
6. CLI output + diagnostics

### 3.2 Runtime/Core Modules

1. `Value.*`
- Runtime value representation (primitive + callable/object/module kinds).

2. `Environment.*`
- Lexical scope chain and symbol storage.

3. `Interpreter.*`
- AST evaluation/execution.
- Runtime control flow (including function return flow).

4. `Resolver.*`
- Static scope binding and semantic checks.

5. `ModuleLoader.*`
- Import resolution, cache, and cycle detection.

6. `Cli.*` (or expanded `Main.cc`)
- `run <file>`
- `repl`
- `test <path>`

### 3.3 Testing Architecture

1. Component Tests (C++ executables)
- Fast checks for scanner/parser/resolver/runtime internals.

2. Script Regression Tests
- Script fixtures in `tests/scripts/`
- Expected outputs in `tests/expected/`
- Suite runner target integrated with CTest.

3. Completion Gates per task
- Configure/build success
- CTest full pass
- Executable runtime behavior validated for the milestone

## 4. Dependency Flow (Design Constraints)

1. Scanner must not depend on Parser/AST.
2. Parser depends on Token and AST, not on runtime execution.
3. Resolver depends on AST and symbol rules, not on execution side effects.
4. Interpreter depends on AST + resolver binding metadata + runtime value/environment.
5. CLI is the orchestration layer and must not own language semantics.

## 5. Document Maintenance Rules

1. Any implementation change that affects module responsibilities, pipeline flow, or testing topology must update this file in the same task round.
2. `Designs.md`, `Plans.md`, and `Todos.md` must stay consistent:
- `Plans.md`: timeline/progress
- `Todos.md`: execution status
- `Designs.md`: architecture truth
