# Task 12 - Containers, Indexing, and Truthiness

## Goal

Implement containers as first-class values and finish the full truthiness model required by the language design.

## Design Links

- Container values, truthiness, and VM container opcodes: [../mslangc-design.md](../mslangc-design.md)
- Repository rules: [../AGENTS.md](../AGENTS.md)

## Dependencies

1. Task 03
2. Task 06
3. Task 08
4. Task 10

## Scope

1. Implement `list`, `tuple`, and `map` runtime objects.
2. Lower container literals and emit `BUILD_*` instructions.
3. Implement index read and write operations.
4. Complete `ms_value_is_falsey()` and implement `ms_value_length()`.
5. Support container printing and runtime error reporting.

## Implementation Boundaries

1. This task owns runtime and lowering behavior for containers.
2. Parser support for literal syntax is assumed from Tasks 05 and 06.
3. `map` in v1 supports string keys only.
4. String indexing is out of scope in v1 and must fail at runtime if routed to
   generic indexing support.
5. Tuple mutation is a runtime error with `MS4xxx`, not a static error.

## File Ownership

1. container-related runtime objects under `include/ms/runtime/` and `src/runtime/`
2. truthiness and length helpers under `src/runtime/`
3. container lowering under `src/frontend/`
4. container tests under `tests/unit/` and `tests/e2e/containers/`

## Diagnostics Contract

1. Invalid index target, non-integral indices, out-of-range access, missing map
   keys, and tuple writes are `phase=runtime` with `MS4xxx`.
2. Non-string map keys in v1 are `phase=runtime` with `MS4xxx`.
3. Truthiness behavior must be observable through tests, not only helper-level
   unit assertions.

## TDD Plan

1. Start with unit tests for the full falsey set: `nil`, `false`, `0`, `0.0`, `""`, and empty containers.
2. Add unit tests for container storage, indexing behavior, and string/map
   length semantics.
3. Add `.ms` scripts for literal construction, conditional truthiness, and
   index read/write behavior.
4. Add negative `.ms` scripts for tuple mutation, missing map keys, string
   indexing, and invalid key/index types.

## Acceptance

1. Empty string and empty containers are false in conditional execution.
2. `list` supports indexed reads and writes.
3. `tuple` is readable but not writable.
4. `map` supports string-key reads and writes in v1.
5. `ms_value_length()` works for `string`, `list`, `tuple`, and `map`.
6. The task is not complete until build passes, tests pass, `.ms` scripts run
   end to end, and all edited files are UTF-8 with LF and no trailing whitespace.

## Acceptance Commands

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure -R "containers|truthiness"
```

## Out of Scope

1. Module cache behavior.
2. GC optimization.
