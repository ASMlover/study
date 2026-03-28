# T24: Testing & Polish

**Phase**: 14 - Polish & Testing
**Dependencies**: T23 (Error Handling Polish)
**Estimated Complexity**: Medium

## Goal

Implement the test framework, write comprehensive unit tests and integration tests, create example scripts, and ensure the entire codebase passes strict compilation with no warnings and no memory leaks.

## Files to Create/Modify

| File | Action |
|------|--------|
| `tests/test_framework.h` | Create: test macros and suite management |
| `tests/test_framework.c` | Create: test runner implementation |
| `tests/CMakeLists.txt` | Rewrite: proper test targets |
| `tests/unit/test_scanner.c` | Complete scanner tests |
| `tests/unit/test_parser.c` | Complete parser tests |
| `tests/unit/test_compiler.c` | Complete compiler tests |
| `tests/unit/test_vm.c` | Complete VM tests |
| `tests/unit/test_gc.c` | Complete GC tests |
| `tests/unit/test_table.c` | Complete table tests |
| `tests/unit/test_logger.c` | Complete logger tests |
| `tests/basic/*.ms` | Basic integration test scripts |
| `tests/functions/*.ms` | Function integration test scripts |
| `tests/classes/*.ms` | Class integration test scripts |
| `tests/modules/*.ms` | Module integration test scripts |
| `examples/*.ms` | Example Maple programs |

## TDD Implementation Cycles

### Cycle 1: Test Framework — Core Macros and Suite

**RED** — Write failing test:
- Create `tests/unit/test_framework_self.c` — a test file that tests the test framework itself
- Write `test_suite_init_free`: init a MsTestSuite, assert count=0, passed=0, failed=0, then free
- Write `test_suite_add_and_run`: add a simple passing test via `RUN_TEST`, run suite, assert passed=1, failed=0
- Write `test_suite_failing_test`: add a test that calls `ASSERT_TRUE(0)`, run, assert failed=1, error message contains "Assertion failed"
- Write `test_assert_eq`: add test calling `ASSERT_EQ(1, 1)` (pass) and `ASSERT_EQ(1, 2)` (fail with "Expected 1, got 2")
- Write `test_assert_str_eq`: add test with `ASSERT_STR_EQ("hello", "hello")` (pass) and `ASSERT_STR_EQ("hello", "world")` (fail)
- Expected failure reason: `tests/test_framework.h` and `tests/test_framework.c` don't exist

**Verify RED**: `cmake --build build` → compile error: cannot find `test_framework.h`

**GREEN** — Minimal implementation:
- Create `tests/test_framework.h`:
  ```c
  typedef struct { const char* name; void (*func)(void); bool passed; char error[256]; } MsTest;
  typedef struct { MsTest* tests; int count; int capacity; int passed; int failed; } MsTestSuite;

  void ms_test_suite_init(MsTestSuite* suite);
  void ms_test_suite_free(MsTestSuite* suite);
  void ms_test_suite_add(MsTestSuite* suite, const char* name, void (*func)(void));
  int ms_test_suite_run(MsTestSuite* suite);
  void ms_test_suite_report(const MsTestSuite* suite);

  extern MsTestSuite* ms_current_suite;
  #define TEST(name) static void test_##name(void)
  #define RUN_TEST(suite, name) ms_test_suite_add(suite, #name, test_##name)
  #define ASSERT_TRUE(cond) do { if (!(cond)) { snprintf(ms_current_suite->tests[ms_current_suite->count-1].error, 256, "Assertion failed: %s", #cond); return; } } while(0)
  #define ASSERT_EQ(expected, actual) do { if ((expected) != (actual)) { snprintf(ms_current_suite->tests[ms_current_suite->count-1].error, 256, "Expected %d, got %d", (int)(expected), (int)(actual)); return; } } while(0)
  #define ASSERT_STR_EQ(expected, actual) do { if (strcmp((expected),(actual)) != 0) { snprintf(ms_current_suite->tests[ms_current_suite->count-1].error, 256, "Expected \"%s\", got \"%s\"", (expected), (actual)); return; } } while(0)
  ```
- Create `tests/test_framework.c`: Implement `ms_test_suite_init()` (zero fields), `ms_test_suite_free()` (free tests array), `ms_test_suite_add()` (grow array, store test), `ms_test_suite_run()` (iterate, call each func, track pass/fail), `ms_test_suite_report()` (print results)

**Verify GREEN**: `cmake --build build && build\test_framework_self` → all self-tests pass

**REFACTOR**: Consider adding `ASSERT_NULL`, `ASSERT_NOT_NULL`, `ASSERT_DOUBLE_EQ` macros as needed.

### Cycle 2: CMake Test Infrastructure

**RED** — Write failing test:
- Attempt to run `ctest --output-on-failure` from build directory → no tests registered
- Expected failure reason: `tests/CMakeLists.txt` not set up properly, no CTest targets

**Verify RED**: `cmake -B build -DBUILD_TESTS=ON && cmake --build build && cd build && ctest` → "No tests were found"

**GREEN** — Minimal implementation:
- Rewrite `tests/CMakeLists.txt`:
  - Build each test file as a separate executable
  - Link against `maple_lib` (static library containing all src/ objects)
  - Add each as a CTest target via `add_test()`
  - Support `ctest --output-on-failure`
- Ensure top-level `CMakeLists.txt` includes `tests/CMakeLists.txt` when `BUILD_TESTS=ON`
- Add `enable_testing()` and `add_subdirectory(tests)` conditionally

**Verify GREEN**: `cmake -B build -DBUILD_TESTS=ON && cmake --build build && cd build && ctest --output-on-failure` → test framework self-test runs and passes

**REFACTOR**: Use a CMake function/macro to reduce boilerplate when adding new test targets.

### Cycle 3: Scanner Unit Tests

**RED** — Write failing test:
- Create `tests/unit/test_scanner.c`
- Write `test_scan_single_char`: scan `+` → token type `MS_TOKEN_PLUS`
- Write `test_scan_number`: scan `42` → token type `MS_TOKEN_NUMBER`, value "42"
- Write `test_scan_string`: scan `"hello"` → token type `MS_TOKEN_STRING`, value "hello"
- Write `test_scan_identifier`: scan `foo` → token type `MS_TOKEN_IDENTIFIER`
- Write `test_scan_keyword`: scan `var` → token type `MS_TOKEN_VAR`
- Write `test_scan_eof`: scan empty string → token type `MS_TOKEN_EOF`
- Write `test_scan_line_tracking`: scan `"a\nb"` → second token on line 2
- Add to CMake and CTest
- Expected failure reason: tests may not link (scanner functions not in lib) or assertions fail on existing bugs

**Verify RED**: `cmake --build build && build\test_scanner` → some tests may fail, revealing scanner bugs

**GREEN** — Minimal implementation:
- Write all scanner tests exercising: single-char tokens, two-char tokens (`==`, `!=`, `<=`, `>=`), numbers (int, float), strings (with escapes), identifiers, keywords, whitespace/newline tracking, EOF
- Fix any bugs discovered in scanner by the tests
- Ensure all tests pass

**Verify GREEN**: `cmake --build build && build\test_scanner` → all scanner tests pass

**REFACTOR**: Group related tests into sub-suites (tokens, numbers, strings, keywords).

### Cycle 4: Compiler and VM Unit Tests

**RED** — Write failing test:
- Create `tests/unit/test_compiler.c`
- Write `test_compile_arithmetic`: compile `"1 + 2"` → verify chunk contains OP_CONSTANT, OP_CONSTANT, OP_ADD, OP_RETURN
- Write `test_compile_variable`: compile `"var x = 42"` → verify OP_DEFINE_GLOBAL emitted
- Create `tests/unit/test_vm.c`
- Write `test_vm_arithmetic`: interpret `"1 + 2"` → verify result is 3
- Write `test_vm_variable`: interpret `"var x = 42\nprint x"` → verify output "42"
- Write `test_vm_comparison`: interpret `"1 < 2"` → verify result is true
- Write `test_vm_string_concat`: interpret `'"a" + "b"'` → verify result is "ab"
- Add to CMake and CTest
- Expected failure reason: tests may not compile/link or reveal compiler/VM bugs

**Verify RED**: `cmake --build build && build\test_compiler && build\test_vm` → some tests may fail

**GREEN** — Minimal implementation:
- Write comprehensive compiler tests: expression compilation, variable declaration/assignment, control flow, function definitions, class definitions
- Write comprehensive VM tests: arithmetic, variables, control flow (if/else, while, for), function calls, closures, class instantiation
- Fix any bugs discovered
- Ensure all tests pass

**Verify GREEN**: `cmake --build build && build\test_compiler && build\test_vm` → all tests pass

**REFACTOR**: Share VM setup/teardown code across VM tests via helper functions.

### Cycle 5: GC and Table Unit Tests

**RED** — Write failing test:
- Create `tests/unit/test_gc.c`
- Write `test_gc_collects_unreachable`: allocate object, remove references, trigger GC → verify object freed
- Write `test_gc_keeps_reachable`: allocate object, keep reference in variable, trigger GC → verify object alive
- Write `test_gc_mark_phase`: create linked objects, GC mark → verify all reachable objects marked
- Write `test_gc_sweep_phase`: verify unmarked objects freed, marked objects kept
- Create `tests/unit/test_table.c`
- Write `test_table_set_get`: set key "x" to 42, get "x" → 42
- Write `test_table_delete`: set and delete key → get returns nil
- Write `test_table_grow`: insert many entries → verify all retrievable
- Write `test_table_find`: find existing key → found, find missing → not found
- Create `tests/unit/test_logger.c`
- Write `test_logger_levels`: verify log output for each level
- Add to CMake and CTest
- Expected failure reason: GC/table bugs may surface

**Verify RED**: `cmake --build build && build\test_gc && build\test_table && build\test_logger` → potential failures

**GREEN** — Minimal implementation:
- Write GC tests covering: allocation, marking, sweeping, cycle detection, stress testing with many objects
- Write table tests covering: insert, lookup, delete, resize, string interning, collision handling
- Write logger tests covering: log levels, output formatting, file output
- Fix any bugs discovered

**Verify GREEN**: `cmake --build build && build\test_gc && build\test_table && build\test_logger` → all pass

**REFACTOR**: Add a GC stress test mode that runs GC on every allocation.

### Cycle 6: Integration Test Scripts

**RED** — Write failing test:
- Create test scripts for each feature area:
  - `tests/basic/arithmetic.ms`: all arithmetic operators, precedence, edge cases
  - `tests/basic/strings.ms`: concatenation, comparison
  - `tests/basic/variables.ms`: declaration, assignment, scoping
  - `tests/basic/control_flow.ms`: if/else, while, for, break, continue
  - `tests/functions/basic.ms`: declaration, calls, returns
  - `tests/functions/closures.ms`: closure capture, nested closures
  - `tests/functions/recursion.ms`: Fibonacci, factorial
  - `tests/classes/basic.ms`: class creation, methods, fields
  - `tests/classes/inheritance.ms`: inheritance, super, method override
  - `tests/modules/test_import.ms`: import/export
  - `tests/modules/test_from_import.ms`: from-import with aliases
- Add CMake custom commands to run each .ms script via `maple` and check exit code 0
- Expected failure reason: some scripts may have bugs or test unimplemented features

**Verify RED**: `cmake --build build && cd build && ctest --output-on-failure` → some integration tests fail

**GREEN** — Minimal implementation:
- Write all integration test scripts with expected behavior
- Each script uses `print` assertions: `print` known values and compare output
- Add CTest targets that run each script and verify exit code 0
- Fix any language bugs discovered by the scripts
- Create example scripts:
  - `examples/hello.ms`: Hello world
  - `examples/fibonacci.ms`: Fibonacci sequence
  - `examples/classes.ms`: OOP demonstration
  - `examples/closures.ms`: Closure patterns

**Verify GREEN**: `cmake --build build && cd build && ctest --output-on-failure` → all integration tests pass

**REFACTOR**: Consider adding output comparison for integration tests (expected vs actual output).

### Cycle 7: Warning-Free Compilation and Leak Check

**RED** — Write failing test:
- Build with strict flags: `cmake -B build -DCMAKE_C_FLAGS="-Wall -Wextra -Wpedantic -Werror"` (GCC/Clang) or `/W4 /WX` (MSVC)
- Check for any compiler warnings → may fail on `-Werror`
- Run with ASan/valgrind: check for memory leaks
- Expected failure reason: may have unused variables, missing prototypes, or memory leaks

**Verify RED**: `cmake -B build -DCMAKE_C_FLAGS="-Wall -Wextra -Wpedantic -Werror" && cmake --build build` → possible compile error from -Werror

**GREEN** — Minimal implementation:
- Fix all compiler warnings: unused variables, missing prototypes, implicit conversions, sign comparison
- Fix all memory leaks: ensure every `ms_reallocate()` has matching free, every `ms_xxx_init()` has matching `ms_xxx_free()`
- Verify with ASan: `cmake -B build -DCMAKE_C_FLAGS="-fsanitize=address" && cmake --build build && cd build && ctest`
- Verify with valgrind (Linux): `valgrind --leak-check=full build/maple tests/basic/arithmetic.ms` → no leaks
- Run full test suite after fixes

**Verify GREEN**: `cmake -B build -DCMAKE_C_FLAGS="-Wall -Wextra -Wpedantic -Werror" && cmake --build build && cd build && ctest --output-on-failure` → clean build, all tests pass

**REFACTOR**: Add CI configuration to enforce warning-free builds automatically.

## Acceptance Criteria

- [ ] All unit tests pass (`ctest --output-on-failure`)
- [ ] All integration test scripts execute without error
- [ ] Test coverage includes: scanner, parser, compiler, VM, GC, table, logger, modules, builtins
- [ ] No compiler warnings with `-Wall -Wextra -Wpedantic` (GCC/Clang) or `/W4` (MSVC)
- [ ] No memory leaks (verified with valgrind on Linux or ASan on both platforms)
- [ ] Test framework reports pass/fail counts correctly
- [ ] Example scripts run correctly

## Notes

- Test framework uses `ms_reallocate()` for dynamic test array growth (consistent with project conventions)
- All test files link against `maple_lib` static library to access internal functions
- Integration tests run .ms scripts via the `maple` executable and check exit codes
- Example scripts serve as both documentation and regression tests
- Cross-platform testing: verify on both Windows (MSVC) and Linux (GCC/Clang)
