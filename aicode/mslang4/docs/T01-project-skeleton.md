# T01: Project Skeleton & Build System

**Phase**: 1 - Foundation
**Dependencies**: None
**Estimated Complexity**: Low

## Goal

Set up the project directory structure, CMake build configuration, and a minimal `main.c` that compiles and runs. This establishes the build pipeline that all subsequent tasks depend on.

## Files to Create

| File | Purpose |
|------|---------|
| `CMakeLists.txt` | Root build configuration |
| `src/main.c` | Entry point (minimal hello-world stub) |
| `src/common.h` | Common definitions placeholder |
| `include/` | Include directory (empty) |
| `tests/CMakeLists.txt` | Test build configuration placeholder |
| `tests/unit/` | Unit test directory (empty) |
| `tests/basic/` | Basic integration test directory (empty) |
| `examples/` | Example scripts directory (empty) |
| `.gitignore` | Ignore build/ directory and binaries |

## TDD Implementation Cycles

Since there is no test framework yet, the "test" in each cycle is the build itself — cmake configure, compile, and run.

### Cycle 1: CMake Configuration and Empty Main

**RED** — Write failing test:
- Create `CMakeLists.txt` referencing `src/main.c` but do NOT create `src/main.c` yet
- Create the `src/` directory but leave it empty
- The build will fail because the source file is missing

**Verify RED**: 
```
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```
Expected: link/compile error — `main.c` not found or no `main` symbol

**GREEN** — Minimal implementation:
- Create `src/main.c` with the smallest possible program:

```c
#include <stdio.h>

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    return 0;
}
```

- Create `CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.10)
project(Maple LANGUAGES C)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4 /WX- /utf-8)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

file(GLOB_RECURSE SOURCES "src/*.c")
add_executable(maple ${SOURCES})
target_include_directories(maple PRIVATE src)

option(BUILD_TESTS "Build test suite" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
```

**Verify GREEN**: 
```
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```
Expected: compiles and links successfully with no errors

**REFACTOR**: Ensure no compiler warnings with `-Wall -Wextra -Wpedantic` (GCC/Clang) or `/W4` (MSVC). The `(void)argc; (void)argv;` suppresses unused-parameter warnings.

### Cycle 2: Version Output on Run

**RED** — Write failing test:
- Run the compiled binary and check its output — currently it prints nothing
- Expected failure: `./build/maple` produces no output, but we expect a version string

**Verify RED**: 
```
./build/maple
```
Expected: no output (empty stdout)

**GREEN** — Minimal implementation:
- Update `src/main.c` to print the version string:

```c
#include <stdio.h>

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    printf("Maple Scripting Language v0.1\n");
    return 0;
}
```

**Verify GREEN**: 
```
cmake --build build
./build/maple
```
Expected: prints `Maple Scripting Language v0.1` to stdout, exits with code 0

**REFACTOR**: No changes needed.

### Cycle 3: Common Header Placeholder and Test Infrastructure

**RED** — Write failing test:
- `CMakeLists.txt` references `add_subdirectory(tests)`, but `tests/CMakeLists.txt` does not exist yet
- Expected failure: CMake configure error — `tests/CMakeLists.txt` not found

**Verify RED**: 
```
cmake -B build
```
Expected: CMake error about missing `tests/CMakeLists.txt`

**GREEN** — Minimal implementation:
- Create `src/common.h` with empty include guards:

```c
#ifndef MS_COMMON_H
#define MS_COMMON_H

#endif
```

- Create `tests/CMakeLists.txt` as an empty placeholder file (zero content).

- Create empty directories: `tests/unit/`, `tests/basic/`, `examples/`, `include/`
  - Place a `.gitkeep` file in each empty directory so git tracks them.

- Create `.gitignore`:

```
build/
*.o
*.obj
*.exe
*.out
```

**Verify GREEN**: 
```
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
./build/maple
```
Expected: full configure + build + run succeeds, version string prints

**REFACTOR**: Verify the directory structure matches REQUIREMENTS.md §2.3.1. Confirm no warnings at any build step.

## Acceptance Criteria

- [ ] `cmake -B build` succeeds
- [ ] `cmake --build build` compiles without errors
- [ ] `./build/maple` (or `build\Debug\maple.exe`) prints "Maple Scripting Language v0.1"
- [ ] Directory structure matches REQUIREMENTS.md §2.3.1
- [ ] No warnings with `-Wall -Wextra -Wpedantic` (GCC/Clang) or `/W4` (MSVC)

## Notes

- This task creates no test framework — that comes in later tasks. The "test" here is the build pipeline itself.
- `common.h` is an empty placeholder; filled in T02.
- `tests/CMakeLists.txt` is empty; populated in T02 when the first unit test is written.
