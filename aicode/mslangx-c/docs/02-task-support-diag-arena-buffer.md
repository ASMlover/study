# Task 02 - Support Layer, Diagnostics, Arena, Buffer

## Goal

Implement the shared support layer that later frontend and runtime tasks can build on without redesigning memory and diagnostics APIs.

## Design Links

- Diagnostics, compile-time arena, and testing baseline: [../mslangc-design.md](../mslangc-design.md)
- Repository rules: [../AGENTS.md](../AGENTS.md)

## Dependencies

1. Task 01

## Scope

1. Define `MsSpan`, `MsDiagnostic`, and a diagnostic list API.
2. Implement an arena allocator for AST and compile-time temporary structures.
3. Implement a growable buffer/array utility.
4. Freeze the boundary between compile-time arena memory and runtime heap memory.

## Implementation Boundaries

1. This task owns support code only; it must not depend on VM or object-model code.
2. Arena allocation is compile-time only and must never manage GC objects.
3. Diagnostics must be structured data first; string formatting helpers are secondary.
4. The minimal diagnostic schema must be stable after this task so Task 15 golden tests can lock it.

## File Ownership

1. `include/ms/diag.h`, `src/support/diag.c`
2. `include/ms/arena.h`, `src/support/arena.c`
3. `include/ms/buffer.h`, `src/support/buffer.c`
4. support-layer unit tests under `tests/unit/`

## API Contracts

1. `MsSpan` records file, line, column, and length.
2. `MsDiagnostic` records phase, code, message, and span.
3. Diagnostics support append, iterate, clear, and bulk free without losing insertion order.
4. Arena allocation supports many small allocations and one bulk release.
5. Buffer growth must check overflow and keep a valid length/capacity invariant.

## TDD Plan

1. Write diagnostics tests for append order, stable spans, and reset behavior first.
2. Write arena tests for many allocations followed by one bulk free.
3. Write buffer tests for growth, overwrite, and boundary conditions.
4. `.ms` scripts are not required here because this task introduces infrastructure rather than language behavior.

## Acceptance

1. Support modules compile independently.
2. Diagnostics, arena, and buffer tests pass under `ctest`.
3. The diagnostic schema is documented and stable enough for later golden tests.
4. The task is not complete until build passes, tests pass, and all edited files are UTF-8 with LF and no trailing whitespace.

## Acceptance Commands

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure -R "support|diag|arena|buffer"
```

## Out of Scope

1. Token, AST, parser, resolver, or VM semantics.
2. GC implementation.