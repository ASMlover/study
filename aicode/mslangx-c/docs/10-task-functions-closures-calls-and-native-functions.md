# Task 10 - Functions, Closures, Calls, and Native Functions

## Goal

Complete function semantics across resolver, lowering, and VM so the language gains real abstraction, recursion, and closure capture.

## Design Links

- Callable protocol, closures, upvalues, and VM call behavior: [../mslangc-design.md](../mslangc-design.md)
- Repository rules: [../AGENTS.md](../AGENTS.md)

## Dependencies

1. Task 07
2. Task 08
3. Task 09

## Scope

1. Support `fn` declarations and `fn` expressions end to end.
2. Implement parameter binding, call frames, and return values.
3. Implement closures, upvalue capture by cell, and upvalue closing.
4. Implement native function registration and the native call ABI.
5. Connect parser, resolver, lowering, and VM for function execution.

## Implementation Boundaries

1. Parser support for `fn` syntax already exists from earlier tasks; this task owns execution semantics.
2. Resolver provides capture metadata and lowering consumes it.
3. Arity is fixed in v1; defaults and variadics remain out of scope.
4. Runtime call failures are runtime errors; static `return` misuse remains a resolver error.
5. Class method dispatch is deferred to Task 11.

## File Ownership

1. runtime function object definitions under `include/ms/runtime/` and `src/runtime/`
2. function and closure lowering under `src/frontend/`
3. call-path support in `src/runtime/vm*.c`
4. function, closure, and native-call tests under `tests/unit/` and `tests/e2e/`

## Diagnostics Contract

1. Top-level `return` remains `phase=resolve` with `MS3xxx`.
2. Calling a non-callable value or using the wrong arity is `phase=runtime` with `MS4xxx`.
3. Closure capture behavior must be observable in stable tests rather than inferred from implementation details.

## TDD Plan

1. Start with recursion and return-value tests.
2. Add closure tests that capture locals after the outer function returns.
3. Add nested-closure and shadowing tests.
4. Add mutation tests that prove closures observe the captured cell rather than a
   copied value.
5. Add native-call tests for argc, argv, and return values.
6. Require `.ms` scripts for recursion, closure capture, nested closures, and function expressions.

## Acceptance

1. Recursive functions execute correctly.
2. Closures preserve captured values across scope exit and reflect later writes
   to the captured cell.
3. Native functions use one documented ABI and are covered by tests.
4. The task is not complete until build passes, tests pass, `.ms` scripts run
   end to end, and all edited files are UTF-8 with LF and no trailing whitespace.

## Acceptance Commands

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure -R "functions|closures|native"
build\Debug\mslangc.exe tests\e2e\functions\recursion.ms
build\Debug\mslangc.exe tests\e2e\closures\capture.ms
```

## Out of Scope

1. Classes, inheritance, and bound methods.
2. Module loading.
