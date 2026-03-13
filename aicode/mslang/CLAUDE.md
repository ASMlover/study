# CLAUDE.md — Maple Language (mslang)

## Project Overview

Maple is a bytecode VM scripting language based on clox (Crafting Interpreters), implemented in C++23.
Core pipeline: single-pass compiler (Pratt Parser) → stack-based VM → Mark-and-Sweep GC.

## Build Commands

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug   # Configure
cmake --build build                        # Build
./build/mslang tests/arithmetic.ms         # Run script
./build/mslang                             # Run REPL
ctest --test-dir build --output-on-failure # Run tests

# Debug options (append to cmake configure)
-DMAPLE_DEBUG_TRACE=ON        # Trace VM execution
-DMAPLE_DEBUG_PRINT=ON        # Print compiled bytecode
-DMAPLE_DEBUG_STRESS_GC=ON    # GC on every allocation
-DMAPLE_DEBUG_LOG_GC=ON       # Log GC activity
```

## Code Conventions

### C++ Style Guide
- Based on [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html), with project-specific overrides below
- Headers: `.hh`, Implementation: `.cc`, Scripts: `.ms`
- Source in `src/`, tests in `tests/`, examples in `examples/`, namespace: `ms`
- `#pragma once` exclusively — no `#ifndef` guards
- Minimize `#if`/`#define`/`#endif` in headers (platform macros in `Macros.hh` only)

### Naming

| Category | Style | Examples |
|----------|-------|---------|
| Classes/Structs | `PascalCase` | `ObjString`, `CallFrame` |
| Functions/Methods | `lower_snake_case` | `scan_token()`, `emit_byte()` |
| Member variables | `snake_case_` (trailing `_`) | `stack_top_`, `frame_count_` |
| Constants | `kPascalCase` | `kSTACK_MAX`, `kFRAMES_MAX` |
| Enum values | `kPascalCase` / `OP_UPPER_CASE` | opcodes use `OP_` prefix |
| Type aliases | `snake_t` | `u8_t`, `str_t`, `sz_t`, `cstr_t` |

### Type Aliases (Types.hh)
```cpp
nil_t = std::nullptr_t;    byte_t/u8_t = std::uint8_t;
i32_t = std::int32_t;      u32_t = std::uint32_t;
sz_t = std::size_t;        str_t = std::string;
strv_t = std::string_view; cstr_t = const char*;
ss_t = std::stringstream;
```

### Base Classes (Common.hh)
`Copyable` (default copy/move) · `UnCopyable` (deleted copy/move) · `Singleton<T>` (`T::get_instance()`)

### C++ Idioms
- Prefer `noexcept`; `std::variant` for Value, `std::vector` for dynamic arrays, `std::array` for fixed-size
- `std::format` / `std::print` where available (C++23)
- `static_cast` for hot-path downcasts, `as_obj<T>()` helper
- Raw `Object*` for GC-managed objects — never `shared_ptr`
- Mark GC roots properly when allocating during compilation
- Prefer `inline` for functions whenever feasible (short functions, header-defined helpers, etc.)

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

## Architecture

```
Source → Scanner → Compiler (Pratt) → Chunk (bytecode) → VM (stack) → Output
                                                           ↕
                                                        GC (M&S)
```

### Key Files
| File | Role |
|------|------|
| `VM.hh/cc` | Execution engine, stack, dispatch loop |
| `Compiler.hh/cc` | Single-pass Pratt parser (largest file) |
| `Object.hh/cc` | GC-managed object hierarchy |
| `Value.hh/cc` | Runtime value (`std::variant` or NaN-boxing) |
| `Memory.hh/cc` | Mark-and-Sweep GC |
| `Table.hh/cc` | Hash table for string interning |
| `Scanner.hh/cc` | Lexer, one token at a time |
| `Chunk.hh/cc` | Bytecode container (code + constants + lines) |
| `Module.hh/cc` | Import system, module loading/caching |
| `Logger.hh/cc` | Colored logger (TRACE/DEBUG/INFO/WARN/ERROR) |
| `Debug.hh/cc` | Bytecode disassembler |

### GC Rules
- All `Object*` linked via intrusive `Object::next` list
- Roots: VM stack, globals, call frames, open upvalues, compiler state
- Triggers when `bytes_allocated_ > next_gc_`
- After any allocation that might trigger GC, ensure new objects are reachable from a root

## Test Format

Tests use `// expect:` comments:
```maple
print 1 + 2;    // expect: 3
print "hello";  // expect: hello
```

## Git Commit Convention

- English only, prefixed with [gitmoji](https://gitmoji.dev/): `<gitmoji> <description>`
- Examples: `:sparkles: add string interpolation` · `:bug: fix recursive stack overflow` · `:recycle: refactor parse rules`

## Workflow

- When a task from any `.md` document is implemented and verified, automatically update its status from `[ ]` to `[x]` without further user instruction.
- When the user types `实现 xxx.md a.b`, implement and verify the feature described in section `a.b` of `xxx.md`, then update its status.
- When the user types `精简 xxx.md`, streamline the source document **without adding or removing any designs or rules**: compress redundant wording, merge duplicate sections, remove filler text, while preserving the original structure and semantics.
- When the user types `git`, stage all changes in the current directory and create a git commit following the Git Commit Convention above.

## Reference
- [Crafting Interpreters — clox](https://github.com/munificent/craftinginterpreters) (c/ directory)
- Design doc: `docs/DESIGN.md`
- Requirements: `docs/REQUIREMENTS.md`
