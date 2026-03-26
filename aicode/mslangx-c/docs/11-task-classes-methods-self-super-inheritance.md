# Task 11 - Classes, Methods, self, super, Inheritance

## Goal

Implement the lightweight object system so classes, instances, methods, and single inheritance run end to end.

## Design Links

- Object model, `self`/`super`, and class semantics: [../mslangc-design.md](../mslangc-design.md)
- Repository rules: [../AGENTS.md](../AGENTS.md)

## Dependencies

1. Task 09
2. Task 10

## Scope

1. Implement class, instance, and bound-method runtime objects.
2. Support field read/write, field-before-method lookup, and automatic `self`
   binding.
3. Support single inheritance and `super.method()` dispatch.
4. Support `init` initializer semantics.
5. Enforce `self`/`super` legality, `init` return restrictions, and
   self-inheritance checks.

## Implementation Boundaries

1. This task owns class runtime behavior and the resolver/lowering hooks for class semantics.
2. It must reuse the unified runtime object header rather than create a parallel object model.
3. Runtime property/method failures are runtime errors, not resolver errors.
4. Property assignment mutates instance fields only; it must not overwrite class
   method tables.
5. Module loading and container internals stay outside this task.

## File Ownership

1. class-related runtime objects under `include/ms/runtime/` and `src/runtime/`
2. class-specific resolver logic under `src/frontend/`
3. class and super lowering support under `src/frontend/`
4. class integration tests under `tests/e2e/class/`
5. class resolver tests under `tests/unit/`

## Diagnostics Contract

1. `self` outside class context is `phase=resolve` with `MS3xxx`.
2. `super` outside subclass method context is `phase=resolve` with `MS3xxx`.
3. `return <expr>` inside `init` is `phase=resolve` with `MS3xxx`.
4. A class inheriting from itself is `phase=resolve` with `MS3xxx`.
5. Missing properties, bad method lookups, and invalid runtime dispatch paths use `phase=runtime` with `MS4xxx`.

## TDD Plan

1. Start with resolver failure tests for invalid `self`, invalid `super`,
   illegal `return <expr>` in `init`, and self-inheritance.
2. Add `.ms` scripts for instance fields, bound methods, override behavior, and `super.method()`.
3. Add tests that prove instance fields shadow methods of the same name.
4. Add negative `.ms` scripts for invalid runtime property access.
5. Keep at least one test that proves `init` returns the instance rather than an arbitrary value.

## Acceptance

1. Classes can be instantiated and instance fields can be read and written.
2. Method calls bind `self` correctly.
3. Subclass override plus `super.method()` works.
4. Instance fields shadow methods on lookup, and class method tables remain
   immutable through instance property assignment.
5. The task is not complete until build passes, tests pass, `.ms` scripts run
   end to end, and all edited files are UTF-8 with LF and no trailing whitespace.

## Acceptance Commands

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure -R "resolver_class|class_integration"
```

## Out of Scope

1. Modules.
2. Container implementation.
