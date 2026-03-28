# AGENTS.md - AI Assistant Configuration

## Project Overview

**Maple Scripting Language** (prefix `ms`) — a pure C11 bytecode-compiled scripting language inspired by the `clox` interpreter from [Crafting Interpreters](https://craftinginterpreters.com/).

**Model**: GLM-5 (zai-coding-plan/glm-5)

## Build Commands

See [REQUIREMENTS.md §2.4](REQUIREMENTS.md) for build, run, and test commands.

## Code Standards

- **Coding style**: Linux Kernel Coding Style (https://www.kernel.org/doc/html/latest/process/coding-style.html)
- **File encoding**: UTF-8, LF line endings
- **Trailing whitespace**: No trailing whitespace; auto-strip on save
- **Language**: C11 only, no C++ features
- **Files**: `.h` / `.c`, header guards use `#ifndef`/`#define`/`#endif`
- **Naming**:
  - Types: `MsPascalCase` (e.g., `MsToken`, `MsVM`)
  - Functions: `ms_snake_case` (e.g., `ms_scanner_init`, `ms_vm_interpret`)
  - Variables/members: `snake_case`
  - Constants/macros: `MS_UPPER_CASE`
- **Memory**: `malloc`/`realloc`/`free` via `ms_reallocate()` wrapper; init/free lifecycle pattern
- **Includes**: `"common.h"` first, then `<stdlib.h>` etc., then project headers with `""`
- **No global state**: Pass context structs as parameters

For full style guidelines, examples, and conventions, see [REQUIREMENTS.md §2.3](REQUIREMENTS.md) and [DESIGN.md](DESIGN.md).

## Documentation Map

| Document | Purpose |
|---|---|
| [REQUIREMENTS.md](REQUIREMENTS.md) | Features, code style, testing requirements, implementation phases |
| [DESIGN.md](DESIGN.md) | Architecture, data structures, APIs, algorithms, build config |

## AI Assistant Instructions

1. **Pure C11**: No C++ features, no external dependencies
2. **Read the docs first**: Consult REQUIREMENTS.md and DESIGN.md before implementation
3. **Follow naming conventions**: `Ms` prefix for types, `ms_` prefix for functions, `MS_` for macros
4. **Memory safety**: Always pair `ms_xxx_init()` with `ms_xxx_free()`; GC objects via `ms_alloc_object()`
5. **Error handling**: Return `MsResult` with line/column info; output params for success values
6. **Cross-platform**: Use `platform.h` abstraction; `#ifdef` only in platform layer
7. **Test your code**: Write unit tests (`tests/unit/`) and Maple script tests
8. **Incremental progress**: Follow the 10 phases in REQUIREMENTS.md §4
9. **Gitmoji commits**: Use gitmoji identifiers and English messages
