# Maple Scripting Language

A pure **C11** bytecode-compiled scripting language, inspired by the `clox` interpreter from [Crafting Interpreters](https://craftinginterpreters.com/).

```
Source (.ms) → Scanner → Parser → Compiler → VM ←→ GC
```

## Features

- **Bytecode compilation** with single-pass compiler
- **Stack-based VM** with switch dispatch
- **Mark-and-sweep GC** with tri-color marking
- **First-class functions** with closures and upvalues
- **Classes & inheritance** with constructors, `this`, `super`
- **Module system** — `import` / `from ... import ... as`
- **List/array** support with literal syntax
- **Cross-platform** — Windows (MSVC) and Linux (GCC/Clang)
- **Zero dependencies** — C standard library only

## Quick Start

### Prerequisites

- C11 compiler: MSVC 2022+ / GCC 5+ / Clang 3.6+
- CMake 3.10+

### Build & Run

```bash
cmake -B build
cmake --build build

# Interactive REPL
./build/maple

# Execute a script
./build/maple script.ms
```

### CLI

```
maple                  Interactive REPL
maple script.ms        Run a script
maple --path dir script.ms    Add module search path
maple --version        Show version
maple --help           Show help
```

## Language Tour

```maple
// Variables
var x = 42
var greeting = "Hello, Maple!"

// Control flow
if (x > 10) {
    print "big"
} else {
    print "small"
}

for (var i = 0; i < 5; i = i + 1) {
    print i
}

// Functions & closures
fn makeCounter() {
    var count = 0
    fn increment() {
        count = count + 1
        return count
    }
    return increment
}

var c = makeCounter()
print c()  // 1
print c()  // 2

// Classes & inheritance
class Animal {
    init(name) {
        this.name = name
    }
    speak() {
        print this.name + " makes a sound"
    }
}

class Dog < Animal {
    speak() {
        print this.name + " barks"
    }
}

var dog = Dog("Rex")
dog.speak()

// Modules
import math
from utils import helper as myHelper

// Lists
var list = [1, 2, 3]
list[0] = 10
print len(list)
```

## Documentation

| Document | Purpose |
|----------|---------|
| [REQUIREMENTS.md](REQUIREMENTS.md) | Features, code style, testing, implementation phases |
| [DESIGN.md](DESIGN.md) | Architecture, data structures, APIs, algorithms |
| [AGENTS.md](AGENTS.md) | AI assistant configuration and code conventions |

### Implementation Tasks

The project is decomposed into 24 TDD-driven tasks in [docs/](docs/). Each task follows Red-Green-Refactor cycles and is independently compilable and testable.

| # | Task | Phase | Dependencies |
|---|------|-------|--------------|
| [T01](docs/T01-project-skeleton.md) | Project Skeleton & Build System | 1 Foundation | — |
| [T02](docs/T02-common-definitions.md) | Common Definitions | 1 Foundation | T01 |
| [T03](docs/T03-memory-subsystem.md) | Memory Subsystem | 1 Foundation | T02 |
| [T04](docs/T04-platform-layer.md) | Platform Layer | 1 Foundation | T02, T03 |
| [T05](docs/T05-logger.md) | Debug Logger | 1 Foundation | T03, T04 |
| [T06](docs/T06-token-types.md) | Token Types | 1 Foundation | T02 |
| [T07](docs/T07-value-system.md) | Value System | 2 Core Types | T02, T03 |
| [T08](docs/T08-object-system-strings.md) | Object System — Strings | 2 Core Types | T07 |
| [T09](docs/T09-hash-table.md) | Hash Table | 2 Core Types | T07, T08 |
| [T10](docs/T10-scanner.md) | Scanner (Lexer) | 3 Scanner | T06 |
| [T11](docs/T11-ast-nodes.md) | AST Nodes | 4 AST & Parser | T06 |
| [T12](docs/T12-bytecode-chunk.md) | Bytecode Chunk | 5 Bytecode | T07 |
| [T13](docs/T13-parser.md) | Parser | 4 AST & Parser | T10, T11 |
| [T14](docs/T14-compiler-basic.md) | Compiler — Basic | 6 Compiler & VM | T12, T13 |
| [T15](docs/T15-vm-core.md) | VM Core | 6 Compiler & VM | T14, T09 |
| [T16](docs/T16-functions-closures.md) | Functions & Closures | 7 Functions | T15 |
| [T17](docs/T17-garbage-collection.md) | Garbage Collection | 8 GC | T16 |
| [T18](docs/T18-classes-inheritance.md) | Classes & Inheritance | 9 Classes | T16, T17 |
| [T19](docs/T19-list-support.md) | List/Array Support | 10 Lists | T15, T17 |
| [T20](docs/T20-module-system.md) | Module System | 11 Modules | T15, T04 |
| [T21](docs/T21-builtin-functions.md) | Built-in Functions | 12 Builtins | T16 |
| [T22](docs/T22-main-repl.md) | Main Entry Point & REPL | 13 Integration | T21, T20 |
| [T23](docs/T23-error-handling.md) | Error Handling Polish | 13 Integration | T22 |
| [T24](docs/T24-testing-polish.md) | Testing & Polish | 14 Polish | T23 |

### Dependency Graph

```
T01 → T02 → T03 → T04 → T05
                ↘ T07 → T08 → T09 ─────────────────────────┐
         T06 → T10 ─┐                              T15 ←───┘
           ↘ T11 ──→ T13 → T14 → T15 → T16 → T17 → T18
           ↘ T12 ──────────→ T14      T15 → T19
                                    T16 → T21
                                    T15+T04 → T20
                                    T21+T20 → T22 → T23 → T24
```

## Project Structure

```
mslang4/
├── CMakeLists.txt          # Build configuration
├── src/
│   ├── main.c              # Entry point & CLI
│   ├── common.h            # Constants, result types, debug macros
│   ├── memory.h/c          # Allocation wrapper & GC
│   ├── platform.h/c        # Cross-platform file I/O, paths, colors
│   ├── logger.h/c          # Leveled, colored logging
│   ├── token.h             # Token type enum & struct
│   ├── scanner.h/c         # Lexer
│   ├── ast.h               # AST node definitions
│   ├── parser.h/c          # Pratt parser + recursive descent
│   ├── chunk.h/c           # Bytecode instruction set & storage
│   ├── value.h/c           # Tagged-union value representation
│   ├── object.h/c          # Object system (strings, functions, classes...)
│   ├── table.h/c           # Hash table (open addressing)
│   ├── compiler.h/c        # Single-pass AST → bytecode compiler
│   ├── vm.h/c              # Stack-based virtual machine
│   ├── module.h/c          # Module loader & import resolution
│   ├── builtins.h/c        # print, clock, type, len, input, str, num
│   └── platform.h/c        # Platform abstraction
├── tests/
│   ├── unit/               # C unit tests
│   ├── basic/              # .ms integration tests
│   ├── functions/          # Function/closure tests
│   ├── classes/            # OOP tests
│   └── modules/            # Import system tests
├── docs/                   # T01–T24 implementation task docs
└── examples/               # Example .ms programs
```

## Built-in Functions

| Function | Description |
|----------|-------------|
| `print(value)` | Print value to stdout |
| `clock()` | Return elapsed time in seconds |
| `type(value)` | Return type name as string |
| `len(string\|list)` | Return length |
| `input([prompt])` | Read line from stdin |
| `str(value)` | Convert to string |
| `num(value)` | Convert to number |

## Code Conventions

- **C11 only** — no C++ features, no external dependencies
- **Naming**: `MsPascalCase` types, `ms_snake_case` functions, `MS_UPPER_CASE` macros
- **Memory**: All allocation via `ms_reallocate()`, init/free lifecycle pairs
- **Style**: Linux Kernel Coding Style, 4-space indentation, UTF-8/LF
- **Error handling**: `MsResult` return codes with line/column info

See [AGENTS.md](AGENTS.md) and [REQUIREMENTS.md](REQUIREMENTS.md) §2.3 for full conventions.

## Acknowledgments

- Robert Nystrom's [Crafting Interpreters](https://craftinginterpreters.com/) — foundational design
