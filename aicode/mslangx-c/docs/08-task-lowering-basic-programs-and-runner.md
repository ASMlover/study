# Task 08 - Lowering for Basic Programs and End-to-End Runner

## Goal

Connect the frontend to the VM so source code can compile and run end to end for the first non-trivial language subset.

## Design Links

- Frontend phase ordering, loop lowering, and diagnostics phases: [../mslangc-design.md](../mslangc-design.md)
- Repository rules: [../AGENTS.md](../AGENTS.md)

## Dependencies

1. Task 06
2. Task 07
3. Task 09

## Scope

1. Lower literals, arithmetic, equality, logical expressions, assignment, `var`,
   `print`, blocks, `if`, `while`, and `for`.
2. Lower `break`, `continue`, and short-circuit logical expressions using the
   documented control-flow shape.
3. Add the top-level compile pipeline from source to chunk.
4. Wire the CLI to execute a file or inline source.
5. Add end-to-end `.ms` smoke tests.

## Implementation Boundaries

1. Lowering input is `AST + ResolutionTable`, not raw AST alone.
2. This task owns basic control-flow lowering and runner wiring only.
3. Function, closure, class, module, and container lowering is deferred.
4. `for` should be lowered through a documented loop shape rather than ad-hoc
   bytecode emission.
5. Evaluation order must remain left-to-right as specified by the design doc.

## File Ownership

1. `include/ms/frontend/lowering.h`
2. `src/frontend/lowering_basic.c`
3. `src/main.c` runner wiring
4. basic end-to-end tests under `tests/e2e/basic/`
5. optional lowering snapshot tests under `tests/unit/`

## Diagnostics Contract

1. Parse failures remain `phase=parse` with `MS2xxx` codes.
2. Resolver failures remain `phase=resolve` with `MS3xxx` codes.
3. Runtime failures remain `phase=runtime` with `MS4xxx` codes.
4. The CLI must preserve phase separation in exit status and user-visible reporting.

## TDD Plan

1. Start with failing end-to-end `.ms` scripts for arithmetic, variables,
   logical short-circuiting, and control flow.
2. Add bytecode snapshot tests for representative AST inputs.
3. Add loop tests that exercise `break` and `continue` on both `while` and `for`.
4. Add evaluation-order tests where assignment targets and right-hand sides are
   all observable.
5. Add exit-code tests for parse, resolve, and runtime failure paths.

## Acceptance

1. Basic scripts execute correctly from the CLI.
2. `break` and `continue` work correctly in supported loops.
3. Logical `and` and `or` short-circuit and preserve operand-value semantics.
4. End-to-end tests prove that parser, resolver, lowering, and VM are connected
   in the intended order.
5. The task is not complete until build passes, tests pass, the CLI runs `.ms`
   scripts end to end, and all edited files are UTF-8 with LF and no trailing whitespace.

## Acceptance Commands

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure -R "lowering_basic|runner_basic|e2e_basic"
build\Debug\mslangc.exe tests\e2e\basic\smoke.ms
```

## Out of Scope

1. Functions and closures.
2. Classes, modules, and containers.
