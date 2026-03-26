# Task 13 - Modules and Import System

## Goal

Implement module loading, caching, and import binding so multi-file programs can run with stable module diagnostics.

## Design Links

- Module names, state machine, import behavior, and module diagnostics: [../mslangc-design.md](../mslangc-design.md)
- Repository rules: [../AGENTS.md](../AGENTS.md)

## Dependencies

1. Task 06
2. Task 08
3. Task 10

## Scope

1. Implement module objects, per-module namespaces, and module cache state.
2. Map dotted module names to file paths.
3. Support `import` and `from ... import ... as ...` end to end.
4. Support default export of top-level `var`, `fn`, and `class` bindings.
5. Bind imported modules and symbols according to the design baseline.
6. Emit module diagnostics `MS5001` through `MS5004`.

## Implementation Boundaries

1. This task owns import resolution, cache management, and export binding.
2. It does not introduce explicit `export` syntax.
3. The module state machine must be explicit: `UNSEEN -> INITIALIZING -> INITIALIZED`, and any failure moves the module to `FAILED`.
4. Re-entry into a module already in `INITIALIZING` is a cycle and must report `MS5003`.
5. Cache keys must use the canonical resolved file path, not only the raw import
   string.
6. `import a.b` binds the module object to `b` unless `as` overrides the local
   name.
7. `from ... import ...` binds a snapshot value after successful module
   initialization; imports are not live bindings in v1.
8. Support for importing classes or containers can be added as later integration
   coverage once those features land, but module core must not wait on them.

## File Ownership

1. module runtime code under `include/ms/runtime/` and `src/runtime/`
2. import lowering support under `src/frontend/`
3. CLI search-path wiring under `src/`
4. module fixtures under `tests/fixtures/modules/`
5. module integration and golden tests under `tests/e2e/` and `tests/golden/`

## Diagnostics Contract

1. `MS5001`: module file not found.
2. `MS5002`: exported symbol not found.
3. `MS5003`: cyclic dependency.
4. `MS5004`: module initialization failed because parse, resolve, or runtime
   execution failed inside the imported module.
5. Imported-module failures should preserve the originating diagnostic and may
   add an import-site `MS5004` wrapper.
6. Module golden tests must lock `phase + code + line`.

## TDD Plan

1. Start with failing tests for each `MS500x` case.
2. Add passing multi-file `.ms` fixtures for `import a`, `import a.b as c`, and
   `from a import b as c`.
3. Add a cache test that proves repeated import does not re-run top-level side
   effects.
4. Add tests that prove imported symbols are snapshots, not live bindings.
5. Keep fixture layout deterministic so path-dependent bugs are easy to reproduce.

## Acceptance

1. Multi-file imports execute correctly.
2. Module cache behavior is deterministic and avoids duplicate initialization.
3. Module namespaces stay isolated except through explicit imports.
4. All module error codes are exercised by tests.
5. The task is not complete until build passes, tests pass, `.ms` fixtures run
   end to end, and all edited files are UTF-8 with LF and no trailing whitespace.

## Acceptance Commands

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure -R "modules|module_errors"
```

## Out of Scope

1. Explicit `export` syntax.
2. GC collection policy details.
