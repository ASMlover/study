# T20: Module System

**Phase**: 11 - Module System
**Dependencies**: T15 (VM Core), T04 (Platform Layer), T17 (Garbage Collection)
**Estimated Complexity**: High

## Goal

Implement the module system with `import module` and `from module import item (as alias)` syntax. Modules are loaded once (cached), have isolated scope, and support circular dependency detection.

## Files to Create/Modify

| File | Action |
|------|--------|
| `src/module.h` | Create: module loading API |
| `src/module.c` | Create: module loading implementation |
| `src/compiler.c` | Modify: add `compileImportStmt`, emit OP_IMPORT/OP_IMPORT_FROM |
| `src/vm.c` | Modify: add OP_IMPORT, OP_IMPORT_FROM dispatch, module loading |
| `src/vm.h` | Modify: add `ms_vm_import_module`, `ms_vm_import_from`, `ms_vm_add_module_path` |

## TDD Implementation Cycles

### Cycle 1: Module Struct and Cache Infrastructure

**RED** — Write failing test:
- Create `tests/unit/test_module.c`
- Write `test_module_cache_empty`: init VM, call `ms_module_get_loaded(vm, "math")` → returns NULL
- Write `test_module_cache_store`: create a MsModule (or mock), store via `vm->modules` table, retrieve via `ms_module_get_loaded()` → returns same pointer
- Write `test_module_search_path`: call `ms_module_add_search_path(vm, "/custom/path")` → verify path is appended
- Expected failure reason: `src/module.h` and `src/module.c` don't exist yet, `ms_module_get_loaded` and `ms_module_add_search_path` undefined

**Verify RED**: `cmake --build build` → linker error: undefined reference to module functions

**GREEN** — Minimal implementation:
- Create `src/module.h`:
  ```c
  #ifndef MS_MODULE_H
  #define MS_MODULE_H

  #include "vm.h"

  typedef struct {
      char* moduleName;
      char* message;
      int line;
  } MsModuleError;

  MsResult ms_module_load(MsVM* vm, const char* moduleName, MsModule** outModule);
  MsModule* ms_module_get_loaded(MsVM* vm, const char* moduleName);
  void ms_module_add_search_path(MsVM* vm, const char* path);
  char* ms_module_resolve_path(MsVM* vm, const char* moduleName);

  #endif
  ```
- Create `src/module.c`:
  - `ms_module_get_loaded()`: Look up in `vm->modules` hash table by name, return pointer or NULL
  - `ms_module_add_search_path()`: Append to `vm->modulePaths` dynamic array via `ms_reallocate()`
- In `src/vm.h`: Add `MsTable modules` and `char** modulePaths` / `int modulePathCount` / `int modulePathCapacity` to MsVM struct. Add `MsTable loadingStack` for circular detection.
- In `src/vm.c` `ms_vm_init()`: Initialize `modules` table and `modulePaths` array. In `ms_vm_free()`: free module paths array and modules table.

**Verify GREEN**: `cmake --build build && build\test_module` → cache and search path tests pass

**REFACTOR**: Ensure modulePaths dynamic array uses standard growth pattern.

### Cycle 2: Path Resolution

**RED** — Write failing test:
- Add to `tests/unit/test_module.c`
- Write `test_resolve_path_current_dir`: create a temp file `math.ms` in CWD, call `ms_module_resolve_path(vm, "math")` → returns path ending in `math.ms`
- Write `test_resolve_path_search_paths`: add a custom search path, place `utils.ms` there, resolve → finds it
- Write `test_resolve_path_not_found`: resolve `"nonexistent"` → returns NULL
- Expected failure reason: `ms_module_resolve_path` stub returns NULL

**Verify RED**: `cmake --build build && build\test_module` → assertion failure: expected non-NULL path, got NULL

**GREEN** — Minimal implementation:
- In `src/module.c`: Implement `ms_module_resolve_path()`:
  1. Try `./<moduleName>.ms` relative to current working directory — use `ms_platform_file_exists()` to check
  2. Try each path in `vm->modulePaths`: `<path>/<moduleName>.ms`
  3. Return allocated path string if found, NULL if not found
- Internal static helper: `fileExists()` using platform abstraction or `fopen()` probe

**Verify GREEN**: `cmake --build build && build\test_module` → path resolution tests pass

**REFACTOR**: Consider whether to use `ms_platform_read_file()` helper or raw `fopen` for existence check.

### Cycle 3: Module Load and Execute

**RED** — Write failing test:
- Create `tests/modules/math.ms`:
  ```
  var pi = 3.14159
  ```
- Add to `tests/unit/test_module.c`
- Write `test_module_load_basic`: call `ms_module_load(vm, "math", &mod)` with test module path configured → returns `MS_RESULT_OK`, mod is non-NULL
- Write `test_module_load_caches`: load same module twice → second call returns cached version (same pointer)
- Expected failure reason: `ms_module_load` not fully implemented (only stub)

**Verify RED**: `cmake --build build && build\test_module` → assertion failure or compile error in ms_module_load

**GREEN** — Minimal implementation:
- In `src/module.c` `ms_module_load()`:
  1. Check cache: `ms_module_get_loaded()` → return if found
  2. Resolve path: `ms_module_resolve_path()`
  3. Read file via `ms_platform_read_file()`
  4. Compile via `ms_compiler_compile()`
  5. Execute module body in VM (in a new call frame with fresh globals scope)
  6. Capture top-level declarations as exports in module's exports table
  7. Cache module in `vm->modules`
  8. Return module
- Define MsModule struct in `src/module.h` (or `src/object.h`): includes name, exports table, compiled chunk

**Verify GREEN**: `cmake --build build && build\test_module` → load and cache tests pass

**REFACTOR**: Ensure module execution errors are properly propagated and don't leave partial state in cache.

### Cycle 4: Circular Dependency Detection

**RED** — Write failing test:
- Create `tests/modules/cycle_a.ms`: `import cycle_b`
- Create `tests/modules/cycle_b.ms`: `import cycle_a`
- Add to `tests/unit/test_module.c`
- Write `test_circular_dependency`: add module path to test dir, attempt to load `cycle_a` → returns `MS_RESULT_RUNTIME_ERROR` with message containing "Circular import detected"
- Expected failure reason: no circular dependency check in `ms_module_load`

**Verify RED**: `cmake --build build && build\test_module` → hangs or stack overflow, or no error returned

**GREEN** — Minimal implementation:
- In `src/module.c`: Add internal static `detectCircularDependency()` — track modules currently being loaded in `vm->loadingStack`
- In `ms_module_load()`: Before compiling, add module name to loading stack. After loading completes, remove from loading stack. If module name already in loading stack → return error with chain message: `Circular import detected: 'a' → 'b' → 'a'`

**Verify GREEN**: `cmake --build build && build\test_module` → circular dependency test returns error as expected

**REFACTOR**: Format circular dependency chain to show full import path.

### Cycle 5: Compiler — `import module` Syntax

**RED** — Write failing test:
- Create `tests/modules/greet.ms`:
  ```
  var greeting = "hello"
  ```
- Create `tests/integration/test_import.ms`:
  ```
  import greet
  ```
  Expected: compiles and runs without error (module loaded)
- Expected failure reason: parser doesn't recognize `import` keyword, compile error

**Verify RED**: `cmake --build build && build\maple tests\integration\test_import.ms` → compile error: unexpected token `import`

**GREEN** — Minimal implementation:
- In `src/compiler.c`: Add `OP_IMPORT` to opcode enum. Implement `compileImportStmt()`:
  - `import module`: consume `import` keyword, read identifier as module name, emit `OP_IMPORT` with module name constant
- In `src/vm.c`: Add `OP_IMPORT` dispatch — read module name constant, call `ms_module_load()`, push module value (or sentinel) onto stack
- Ensure `import` is added as a keyword in scanner keyword table

**Verify GREEN**: `cmake --build build && build\maple tests\integration\test_import.ms` → runs without error

**REFACTOR**: Consider whether to push the module object or just load it as a side effect.

### Cycle 6: Compiler — `from module import item as alias` Syntax

**RED** — Write failing test:
- Create `tests/modules/mathlib.ms`:
  ```
  var sqrt = 42
  var pi = 3.14
  ```
- Create `tests/integration/test_from_import.ms`:
  ```
  from mathlib import sqrt
  print sqrt
  ```
  Expected output: `42`
- Create `tests/integration/test_from_import_alias.ms`:
  ```
  from mathlib import sqrt as square_root
  print square_root
  ```
  Expected output: `42`
- Expected failure reason: parser doesn't recognize `from` keyword syntax

**Verify RED**: `cmake --build build && build\maple tests\integration\test_from_import.ms` → compile error: unexpected token `from`

**GREEN** — Minimal implementation:
- In `src/compiler.c`: Add `OP_IMPORT_FROM` to opcode enum. Extend `compileImportStmt()` to handle `from`:
  - `from module import item`: consume `from`, read module name, consume `import`, read item name, emit `OP_IMPORT_FROM` with module name, item name constants
  - `from module import item as alias`: additionally read `as` and alias name, store under alias in current scope
- In `src/vm.c`: Add `OP_IMPORT_FROM` dispatch — load module, look up specific item in module's exports table, push item value, bind to local/alias name
- Ensure `from` and `as` are added as keywords in scanner

**Verify GREEN**: `cmake --build build && build\maple tests\integration\test_from_import.ms` → outputs `42`

**REFACTOR**: Consolidate import statement parsing into a single function with branching for `import` vs `from`.

### Cycle 7: Import Error Messages

**RED** — Write failing test:
- Create `tests/integration/test_import_missing.ms`:
  ```
  import nonexistent_module
  ```
  Expected: runtime error with message like `Module 'nonexistent_module' not found. Searched: ./nonexistent_module.ms`
- Create `tests/integration/test_import_missing_export.ms`:
  ```
  from mathlib import not_a_thing
  ```
  Expected: runtime error `Module 'mathlib' has no export 'not_a_thing'`
- Expected failure reason: error messages may be generic, not showing search paths or export names

**Verify RED**: `cmake --build build && build\maple tests\integration\test_import_missing.ms` → error message missing searched paths detail

**GREEN** — Minimal implementation:
- In `src/module.c`: Enhance `ms_module_resolve_path()` error to include all searched paths in error message
- In `ms_module_load()`: When resolve returns NULL, format error: `Module 'foo' not found. Searched: ./foo.ms, /path1/foo.ms`
- In VM `OP_IMPORT_FROM` dispatch: When item not found in exports, format error: `Module 'math' has no export 'sqrt'`

**Verify GREEN**: `cmake --build build && build\maple tests\integration\test_import_missing.ms` → shows detailed error with searched paths

**REFACTOR**: Ensure error messages are consistent with compile/runtime error format from T23.

## Acceptance Criteria

- [ ] Basic import: `import math` loads math.ms and caches it
- [ ] From-import: `from math import sqrt` imports specific symbol
- [ ] Alias: `from math import sqrt as square_root` works
- [ ] Module caching: importing same module twice only loads once
- [ ] Circular dependency detection: A imports B, B imports A → clear error
- [ ] Module not found: import nonexistent → compile/runtime error with message
- [ ] Module exports are accessible from importing module
- [ ] Module search paths: custom paths via `ms_vm_add_module_path()`

## Notes

- Modules use `ms_platform_read_file()` for file I/O abstraction
- The module loading stack for circular detection lives in `vm->loadingStack`
- Module exports are captured by recording top-level variable declarations after execution
- Import errors should include all searched paths for better debugging
