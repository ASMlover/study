# Task 07 - Bytecode Chunk and VM Core

## Goal

Implement the minimal bytecode container and stack VM needed to execute the first lowered programs.

## Design Links

- Bytecode VM, opcode families, and runtime diagnostics: [../mslangc-design.md](../mslangc-design.md)
- Repository rules: [../AGENTS.md](../AGENTS.md)

## Dependencies

1. Task 03

## Scope

1. Define `MsChunk`, constant storage, and line mapping.
2. Define the initial opcode set needed by Task 08.
3. Implement the VM value stack, a minimal frame skeleton, current-module
   context, and the execution loop.
4. Support constants, arithmetic, comparison, logical operations, equality,
   module-scoped globals, jumps, and `print`.
5. Provide a disassembler or equivalent debugging view for bytecode tests.

## Implementation Boundaries

1. This task owns only the VM core needed for basic programs.
2. Closures, upvalues, method dispatch, full modules, and GC integration are deferred.
3. The initial opcode set should stay minimal but stable enough for lowering tests.
4. `GET_GLOBAL / DEFINE_GLOBAL / SET_GLOBAL` already refer to the current
   module namespace, even before Task 13 adds import loading.
5. Runtime failures from this task must use `phase=runtime` and `MS4xxx` codes.

## File Ownership

1. `include/ms/runtime/opcode.h`
2. `include/ms/runtime/chunk.h`
3. `include/ms/runtime/vm.h`
4. `src/runtime/chunk.c`
5. `src/runtime/vm.c`
6. `src/runtime/disasm.c`
7. VM-core tests under `tests/unit/`

## Diagnostics Contract

1. Invalid opcode streams, type errors, and invalid jump targets must fail deterministically.
2. Stack underflow and bad module-global access paths must be testable.
3. Disassembler output must be stable enough for small snapshot tests.

## TDD Plan

1. Start with hand-authored chunk execution tests before any lowering integration.
2. Add tests for arithmetic, comparison, equality, module-scoped globals, and
   jump behavior.
3. Add negative tests for stack underflow, invalid jumps, and type mismatch cases.
4. Keep the test data small so failures isolate VM issues rather than parser or lowering issues.

## Acceptance

1. Hand-authored chunks can execute arithmetic and conditional control flow.
2. The current module namespace can define, read, and update top-level bindings.
3. Runtime failures emit `MS4xxx` diagnostics.
4. The task is not complete until build passes, tests pass, and all edited files
   are UTF-8 with LF and no trailing whitespace.

## Acceptance Commands

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure -R "vm_core|chunk|disasm"
```

## Out of Scope

1. Functions and closures.
2. Classes, modules, containers, and GC.
