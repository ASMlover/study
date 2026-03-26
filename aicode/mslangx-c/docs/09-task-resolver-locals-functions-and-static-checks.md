# Task 09 - Resolver for Locals, Functions, and Static Checks

## Goal

Implement the resolver as a standalone analysis pass responsible for name binding, capture analysis, and static semantic checks.

## Design Links

- Static semantics, `ResolutionTable`, and phase separation: [../mslangc-design.md](../mslangc-design.md)
- Repository rules: [../AGENTS.md](../AGENTS.md)

## Dependencies

1. Task 05
2. Task 06

## Scope

1. Maintain scope, function, loop, and class context stacks.
2. Resolve locals, lexical depth, slots, and upvalue metadata.
3. Reject top-level `return`, duplicate declarations in one scope including
   module scope, self-read during initialization, and illegal `break` or
   `continue`.
4. Produce a `ResolutionTable` that lowering can consume without mutating AST nodes.

## Implementation Boundaries

1. The resolver is a pure analysis pass over AST.
2. It must not emit bytecode or rewrite the AST.
3. Class-specific legality checks for `self`, `super`, and self-inheritance are fully exercised in Task 11, but the data structures needed for them should be designed here.
4. Static errors must be explicit and stable because Task 15 will lock them in golden tests.

## File Ownership

1. `include/ms/frontend/resolver.h`
2. `include/ms/frontend/resolution_table.h`
3. `src/frontend/resolver.c`
4. `src/frontend/resolution_table.c`
5. resolver tests under `tests/unit/` and `tests/ms/resolver/`

## Diagnostics Contract

1. All resolver errors use `phase=resolve` and `MS3xxx` codes.
2. Resolution output must be stable enough for snapshot-style tests of slots, depths, and captures.
3. Failure cases should be keyed to rule categories so later golden tests do not depend on ad-hoc messages alone.

## TDD Plan

1. Start with failing resolver tests for top-level `return`, duplicate locals
   and module-scope bindings, self-read in initializer, and illegal `break` or
   `continue`.
2. Add positive resolution snapshots for nested scopes and nested functions.
3. Add `.ms` fixtures that isolate one static rule at a time.
4. Keep runtime behavior out of this task even when a resolver error could also surface later during execution.

## Acceptance

1. The resolver produces a separate `ResolutionTable` and does not mutate AST structure.
2. Static checks from the design baseline are enforced with `MS3xxx` diagnostics.
3. Lowering can consume the produced resolution data without redoing name analysis.
4. The task is not complete until build passes, tests pass, and all edited files are UTF-8 with LF and no trailing whitespace.

## Acceptance Commands

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure -R "resolver|resolution"
```

## Out of Scope

1. Function call runtime behavior.
2. Class runtime semantics and inheritance dispatch.
