# Maple Scripting Language - Requirements Specification

## 1. Project Overview

- **Language Name**: Maple
- **Prefix**: `ms` (Maple Script)
- **Implementation Language**: C11
- **Build System**: CMake 3.10+
- **Target Platforms**: Windows (MSVC 2022+) and Linux (GCC 5+ / Clang 3.6+)
- **Goal**: Production-ready scripting language interpreter inspired by `clox` from [Crafting Interpreters](https://craftinginterpreters.com/), in pure C with no external dependencies.
- **Reference Features**: Bytecode compilation, stack-based VM, mark-and-sweep GC, closures/upvalues, classes/inheritance, first-class functions.

## 2. Core Requirements

### 2.1 Language Features

**Data Types**: `nil`, bool (`true`/`false`), double-precision numbers, heap-allocated immutable strings, functions (first-class, closures), classes (with inheritance), instances, lists/arrays (optional).

**Statement Termination**: Newlines terminate statements (Go-style). Semicolons `;` may be used to separate multiple statements on the same line.

**Control Flow**: `if`/`else`, `while`, `for` (C-style), `break`, `continue`, `return`.

**Functions**: Declared with `fn` keyword. User-defined, closures with upvalues, variadic (optional), native/built-in, recursion.

**OOP**: Class declarations, `init` constructor, instance methods, field access (get/set), single inheritance, `this`, `super`.

**Operators**: Arithmetic (`+`,`-`,`*`,`/`,`%`), comparison (`==`,`!=`,`<`,`<=`,`>`,`>=`), logical (`and`,`or`,`!`), assignment (`=`), string concatenation (`+`).

### 2.2 Enhanced Features (Beyond clox)

#### 2.2.1 Module System

```maple
import math
from math import sqrt, pow
from math import sqrt as squareRoot
from collections import List, Map as HashMap
```

- Each `.ms` file is a module; modules are singletons (loaded once, cached)
- Module scope isolation, circular import detection
- Relative and absolute import paths, configurable search paths
- File resolution: `.ms` extension auto-added; current directory + configured paths
- Top-level declarations are exported; clear errors for missing modules

#### 2.2.2 Debugging Logger

**Levels**: TRACE (Gray), DEBUG (Blue), INFO (Green), WARN (Yellow), ERROR (Red), FATAL (Magenta).

**Features**: ANSI/Win32 colored output, optional timestamps, source location (`__FILE__`/`__LINE__`/`__func__`), console/file output, runtime level config, compile-time filtering.

```c
ms_logger_set_level(MS_LOG_DEBUG);
ms_logger_info("VM initialized");
ms_logger_error("Undefined variable '%s' at line %d", name, line);
```

#### 2.2.3 Built-in Functions

`print(value)`, `clock()`, `type(value)`, `len(string|array)`, `input([prompt])` (optional), `str(value)`, `num(value)`.

#### 2.2.4 List/Array Support (Optional)

```maple
var list = [1, 2, 3]
list.append(4)
print(list[0])
list[1] = 10
print(list.length)
```

### 2.3 Implementation Requirements

#### 2.3.1 Code Organization

```
mslang4/
├── CMakeLists.txt              # Build configuration
├── README.md, REQUIREMENTS.md, DESIGN.md
├── src/
│   ├── main.c                  # Entry point
│   ├── common.h                # Common definitions, macros, types
│   ├── logger.h / .c           # Logger system
│   ├── token.h                 # Token definitions
│   ├── scanner.h / .c          # Lexer
│   ├── ast.h                   # AST node definitions
│   ├── parser.h / .c           # Parser
│   ├── compiler.h / .c         # Compiler
│   ├── chunk.h / .c            # Bytecode chunk
│   ├── value.h / .c            # Value representation
│   ├── object.h / .c           # Object system
│   ├── table.h / .c            # Hash table
│   ├── memory.h / .c           # Memory management
│   ├── vm.h / .c               # Virtual machine
│   ├── module.h / .c           # Module system
│   ├── builtins.h / .c         # Built-in functions
│   └── platform.h / .c         # Platform utilities
├── tests/
│   ├── unit/                   # C unit tests
│   ├── basic/, functions/, classes/, modules/, regression/
└── examples/
```

#### 2.3.2 C11 Features

Anonymous struct/union, `_Generic`, `static_assert`, `_Alignof`/`alignas`, designated initializers, `snprintf`, `<stdbool.h>`, `<stdint.h>`. Avoid VLAs; use dynamic arrays.

#### 2.3.3 Cross-Platform

File I/O, path manipulation (`/` vs `\\`), console colors (ANSI/Win32), time functions. Platform-specific code only in `platform.h`/`.c` via `#ifdef` guards.

#### 2.3.4 Memory Management

`ms_realloc` wrapper over `realloc`/`free`. Mark-and-sweep GC (incremental/generational optional). Debug allocation tracking. Init/free lifecycle pairs for all types.

#### 2.3.5 Error Handling

- **Return Codes**: `MsResult` enum (`MS_OK`, `MS_COMPILE_ERROR`, `MS_RUNTIME_ERROR`)
- **Output Parameters**: Success values via pointer params
- **Error Context**: `MsError` type with line/column, source snippet
- **Recovery**: Lexer skips to next token; parser synchronizes to statement boundary; VM unwinds stack with trace

#### 2.3.6 Code Style

- **Headers**: `#ifndef`/`#define`/`#endif` guards; `.h`/`.c` extensions
- **Naming**: Types `MsPascalCase`, functions `ms_snake_case`, variables/members `snake_case`, macros `MS_UPPER_CASE`
- **Formatting**: 4-space indentation, no tabs, no trailing whitespace, UTF-8/LF
- **No global state**: Pass context structs as parameters
- **Linux Kernel Coding Style** (https://www.kernel.org/doc/html/latest/process/coding-style.html)

#### 2.3.7 Testing

- **Unit Tests**: Custom lightweight C framework with macros
- **Integration Tests**: `.ms` script test suite
- **Categories**: Lexer, parser, compiler, VM, GC, modules, builtins, error handling, performance benchmarks (optional)
- **Coverage**: Aim for high coverage; all bugs get regression tests

#### 2.3.8 Performance Targets

Startup < 50ms; competitive with Lua/Python; GC pause < 10ms.

### 2.4 Build and CLI

**CMake Targets**: `maple` executable, `maple_lib` static library (optional). Build types: Debug, Release, RelWithDebInfo.

```bash
maple script.ms              # Run a script
maple                        # Interactive REPL
maple -c script.ms -o script.mbc   # Compile to bytecode (optional)
maple -x script.mbc                # Execute bytecode (optional)
maple --path /usr/lib/maple script.ms
maple --log-level debug script.ms
maple --version / --help
```

## 3. Non-Functional Requirements

- No external dependencies (C standard library only)
- Pure C11, no C++ features
- Binary size < 2MB
- Open-source license

## 4. Implementation Phases

1. **Foundation**: Project structure, CMake, common.h, logger, platform layer, tokens
2. **Lexer & Parser**: Scanner, token stream, AST nodes, recursive descent + Pratt parser, error recovery
3. **Bytecode & Compiler**: Instruction set, chunk (dynamic array), single-pass compiler, debug info
4. **Value System**: Tagged union values, object system, string interning, hash table
5. **Virtual Machine**: VM core + stack, instruction dispatch, call frames, stack operations
6. **Memory Management**: Allocation wrapper, mark-and-sweep GC, gray stack, stress mode
7. **Functions & Closures**: Function/upvalue/closure objects, native function binding
8. **Classes & Inheritance**: Class/instance/bound method objects, inheritance chain, `super` lookups
9. **Module System**: Module loader, import compilation, caching, circular dependency detection
10. **Polish & Testing**: Comprehensive tests, optimization, documentation, examples

## 5. Success Criteria

- Compiles on Windows (MSVC) and Linux (GCC)
- Passes craftinginterpreters test suite
- Module system with all import forms
- Colored logger output
- Working GC, classes, inheritance
- Clear error messages with source locations
- No memory leaks (verified with sanitizers)

## 6. Future Enhancements (Out of Scope)

JIT compilation, static types, exceptions, coroutines/async, standard library, package manager, debugger, LSP, WebAssembly target, multi-threading, FFI.
