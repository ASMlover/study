# Task 14 - Mark-Sweep GC and Root Integration

## Goal

Replace ad-hoc heap lifetime management with one non-moving mark-sweep GC that covers the full runtime root set.

## Design Links

- GC roots, runtime heap boundaries, and memory model: [../mslangc-design.md](../mslangc-design.md)
- Repository rules: [../AGENTS.md](../AGENTS.md)

## Dependencies

1. Task 03
2. Task 10
3. Task 11
4. Task 12
5. Task 13

## Scope

1. Implement a non-moving mark-sweep collector.
2. Maintain the object list and mark bits.
3. Integrate roots from stack values, call frames, open upvalues, current
   module references, module cache, interned strings, builtin/native registries,
   and temporary roots.
4. Add a temporary-root mechanism for compile-to-runtime transitions.
5. Add GC stress tests and basic observability.

## Implementation Boundaries

1. This task owns GC correctness, not GC performance tuning.
2. Compile-time arena memory must stay outside the GC domain.
3. Existing runtime semantics must remain unchanged when GC is enabled.
4. Any optional stress mode should improve determinism for tests rather than act as a production feature.

## File Ownership

1. GC code under `include/ms/runtime/` and `src/runtime/`
2. GC root registration touchpoints in VM and allocators
3. GC unit tests under `tests/unit/`
4. GC stress `.ms` scripts under `tests/stress/gc/`

## Diagnostics and Observability Contract

1. Runtime allocation failures remain `phase=runtime` with `MS4xxx`.
2. Module-init failures triggered during import continue to surface as `MS5004`.
3. Expose minimal GC counters such as allocation count, free count, and collection count for tests.
4. If a `--gc-stress` or equivalent mode is added, keep it deterministic and test-only.

## TDD Plan

1. Start with root-survival tests for stack values, closures, class graphs,
   interned strings, builtin/native registries, and cached modules.
2. Add tests for reclaiming unreachable objects.
3. Add `.ms` churn scripts that allocate heavily under repeated collections.
4. Re-run existing suites with GC enabled as part of the task, not only isolated GC tests.

## Acceptance

1. Existing scripts keep working with GC enabled.
2. Reachable objects survive and unreachable objects are reclaimed.
3. Stress tests do not show use-after-free behavior or monotonic leaks in a fixed workload.
4. The task is not complete until build passes, tests pass, stress scripts run end to end, and all edited files are UTF-8 with LF and no trailing whitespace.

## Acceptance Commands

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure -R "gc|stress"
```

## Out of Scope

1. Moving or generational GC.
2. Runtime performance tuning beyond correctness instrumentation.
