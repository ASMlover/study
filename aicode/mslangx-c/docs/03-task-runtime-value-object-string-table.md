# Task 03 - Runtime Value, Object Header, String, Table

## Status

Complete. Verified on 2026-03-29 with the acceptance commands in this document.

## Goal

Define the runtime core data model so later VM, classes, containers, and modules all share one stable object and table foundation.

## Design Links

- Runtime value/object model and truthiness baseline: [../mslangc-design.md](../mslangc-design.md)
- Repository rules: [../AGENTS.md](../AGENTS.md)

## Dependencies

1. Task 01
2. Task 02

## Scope

1. Define `MsValueType` and `MsValue`.
2. Define the shared `MsObject` header and object kind tags.
3. Implement string objects.
4. Implement a string-key hash table for module namespaces, fields, methods,
   exports, and interned strings.
5. Implement value printing, equality helpers, and baseline falsey checks for
   scalar values.

## Implementation Boundaries

1. This task owns runtime core types only; it does not execute bytecode.
2. Containers, classes, closures, modules, and GC tracing semantics are deferred.
3. Strings are heap objects under `MS_VAL_OBJECT`; do not introduce a separate
   non-object string value tag.
4. Table APIs must be stable after this task because later tasks will depend on
   them heavily.
5. Freeze `ms_value_is_falsey()` for `nil`, `false`, numeric zero, and empty
   string now; container cases extend the same API in Task 12.

## File Ownership

1. `include/ms/value.h`, `src/runtime/value.c`
2. `include/ms/object.h`, `src/runtime/object.c`
3. `include/ms/string.h`, `src/runtime/string.c`
4. `include/ms/table.h`, `src/runtime/table.c`
5. unit tests under `tests/unit/`

## API Contracts

1. Value constructors and accessors must be explicit and type-safe at the C
   boundary.
2. Every heap object starts with the shared object header.
3. String ownership and interning rules must be documented and testable.
4. Table probing, tombstone reuse, overwrite, and deletion semantics must be deterministic.
5. Scalar equality must compare by value, while object equality falls back to
   identity until later tasks introduce more specific helpers.
6. Value print output must be stable enough for tests and later CLI output checks.

## TDD Plan

1. Start with unit tests for value constructors, equality, and print helpers.
2. Add collision, overwrite, delete, and tombstone reuse tests for the table.
3. Add string interning or deduplication tests if interning is introduced in this task.
4. Add tests that prove strings are represented as heap objects and compare by
   byte content.
5. `.ms` scripts are optional here; primary coverage is C unit tests.

## Acceptance

1. Runtime core modules compile and link without VM execution support.
2. Unit tests cover table collision paths and stable value behavior.
3. `ms_value_is_falsey()` is locked for the scalar/string baseline.
4. String representation and equality behavior match the design baseline.
5. The task is not complete until build passes, tests pass, and all edited files
   are UTF-8 with LF and no trailing whitespace.

## Acceptance Commands

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure -R "runtime_core|value|string|table"
```

## Out of Scope

1. VM execution.
2. Class, module, and container behavior.
3. GC marking and sweeping.
