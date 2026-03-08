# CLAUDE.md — Maple Language (mslang) Project Guide

## Project Overview

Maple 是一门基于 clox (Crafting Interpreters) 的字节码虚拟机脚本语言，使用 C++23 实现。
核心组件：单遍编译器 (Pratt Parser) → 栈式 VM → Mark-and-Sweep GC。

## Build Commands

```bash
# Configure (from project root)
cmake -B build -DCMAKE_BUILD_TYPE=Debug

# Build
cmake --build build

# Run script
./build/mslang tests/arithmetic.ms

# Run REPL
./build/mslang

# Run tests
ctest --test-dir build --output-on-failure

# Debug options (append to cmake configure)
-DMAPLE_DEBUG_TRACE=ON        # Trace VM execution
-DMAPLE_DEBUG_PRINT=ON        # Print compiled bytecode
-DMAPLE_DEBUG_STRESS_GC=ON    # GC on every allocation
-DMAPLE_DEBUG_LOG_GC=ON       # Log GC activity
```

## Code Conventions

### File Organization
- Headers: `.hh`, Implementation: `.cc`, Scripts: `.ms`
- All source in `src/`, tests in `tests/`, examples in `examples/`
- Namespace: `ms` for all code

### Header Style
- Use `#pragma once` exclusively — no `#ifndef` guards
- Minimize `#if`/`#define`/`#endif` in headers (platform macros in `Macros.hh` only)

### Naming Conventions
- Classes/Structs: `PascalCase` — `ObjString`, `CallFrame`, `Scanner`
- Methods/Functions: `snake_case` — `scan_token()`, `emit_byte()`, `is_truthy()`
- Member variables: `snake_case_` with trailing underscore — `stack_top_`, `frame_count_`
- Constants: `kPascalCase` — `kSTACK_MAX`, `kFRAMES_MAX`, `kGC_HEAP_GROW`
- Enum values: `kPascalCase` or `OP_UPPER_CASE` for opcodes
- Type aliases: `snake_t` — `u8_t`, `str_t`, `sz_t`, `cstr_t`

### Type Aliases (defined in Types.hh)
```cpp
nil_t = std::nullptr_t;   byte_t/u8_t = std::uint8_t;
i32_t = std::int32_t;     u32_t = std::uint32_t;
sz_t = std::size_t;       str_t = std::string;
strv_t = std::string_view; cstr_t = const char*;
ss_t = std::stringstream;
```

### Base Classes (defined in Common.hh)
- `Copyable` — default copy/move semantics
- `UnCopyable` — deleted copy/move
- `Singleton<T>` — `T::get_instance()` pattern

### C++ Style
- Prefer `noexcept` on all methods unless they intentionally throw
- Use `std::variant` for Value type, `std::vector` for dynamic arrays
- Use `std::array` for fixed-size arrays (VM stack, frames)
- Use `std::format` / `std::print` where available (C++23)
- Use `static_cast` for object downcasts in hot paths, `as_obj<T>()` helper
- Raw `Object*` pointers for GC-managed objects — never `shared_ptr`
- Mark GC roots properly when allocating during compilation

### Copyright Header
```cpp
// Copyright (c) 2026 ASMlover. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//  * Redistributions of source code must retain the above copyright
//    notice, this list ofconditions and the following disclaimer.
//
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in
//    the documentation and/or other materialsprovided with the
//    distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
```

## Architecture Quick Reference

```
Source → Scanner → Compiler (Pratt) → Chunk (bytecode) → VM (stack) → Output
                                                           ↕
                                                        GC (M&S)
```

### Key Files
| File | Role |
|------|------|
| `VM.hh/cc` | Core execution engine, stack, dispatch loop |
| `Compiler.hh/cc` | Single-pass compiler, Pratt parser (largest file) |
| `Object.hh/cc` | GC-managed object hierarchy (ObjString, ObjClosure, ...) |
| `Value.hh/cc` | Runtime value type (`std::variant<monostate, bool, double, Object*>`) |
| `Memory.hh/cc` | Mark-and-Sweep garbage collector |
| `Table.hh/cc` | Custom hash table for string interning |
| `Scanner.hh/cc` | Lexer, returns one token at a time |
| `Chunk.hh/cc` | Bytecode container (code + constants + lines) |
| `Module.hh/cc` | Import system, module loading and caching |
| `Logger.hh/cc` | Colored debug logger (TRACE/DEBUG/INFO/WARN/ERROR) |
| `Debug.hh/cc` | Bytecode disassembler |

### GC Rules
- All `Object*` linked via intrusive `Object::next` list
- GC roots: VM stack, globals table, call frames, open upvalues, compiler state
- Trigger: when `bytes_allocated_ > next_gc_`
- After any allocation that might trigger GC, ensure new objects are reachable from a root

## Test Format

Tests use `// expect:` comments:
```maple
print 1 + 2;    // expect: 3
print "hello";  // expect: hello
```

## Git Commit Convention

- Commit messages MUST be written in English
- Each commit message MUST start with a [gitmoji](https://gitmoji.dev/) emoji prefix
- Format: `<gitmoji> <description>`
- Examples:
  - `:sparkles: add string interpolation support`
  - `:bug: fix stack overflow in recursive calls`
  - `:memo: update architecture documentation`
  - `:recycle: refactor compiler parse rules`
  - `:white_check_mark: add tests for closure upvalues`

## Workflow

- 当一个 PLAN.md 中的 Task 实现并验证完毕后，自动将该 Task 在 PLAN.md 进度表中的状态从 `[ ]` 更新为 `[x]`，无需用户额外指示。

## Reference
- [Crafting Interpreters — clox](https://github.com/munificent/craftinginterpreters) (c/ directory)
- Design doc: `docs/DESIGN.md`
- Requirements: `docs/REQUIREMENTS.md`
