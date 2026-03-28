# T23: Error Handling Polish

**Phase**: 13 - Integration
**Dependencies**: T22 (Main Entry Point & REPL)
**Estimated Complexity**: Medium

## Goal

Polish error handling across all components: compile errors with source context and line/column info, runtime errors with stack traces, and import errors with clear messages. Ensure all error paths produce helpful, colored output.

## Files to Modify

| File | Changes |
|------|---------|
| `src/compiler.c` | Enhance error messages with source context |
| `src/vm.c` | Implement stack trace printing for runtime errors |
| `src/scanner.c` | Ensure error tokens have accurate position info |
| `src/parser.c` | Add source line to error output |
| `src/module.c` | Clear error messages for import failures |

## TDD Implementation Cycles

### Cycle 1: Compile Errors with Line and Column

**RED** — Write failing test:
- Create `tests/errors/undefined_var.ms`:
  ```
  var x = 1
  print y
  ```
- Run: `build\maple tests\errors\undefined_var.ms` → capture stderr
- Assert stderr contains: line number (`2`), column number, message containing `Undefined variable 'y'`
- Expected failure reason: compile errors may only show generic message without column info

**Verify RED**: `cmake --build build && build\maple tests\errors\undefined_var.ms 2>&1` → error message missing column info

**GREEN** — Minimal implementation:
- In `src/scanner.c`: Ensure error tokens store accurate `line` and `column` fields (column = byte offset from line start)
- In `src/parser.c`: Store current source line text in parser struct for context display
- In `src/compiler.c` error reporting: Format as:
  ```
  error: line 3, column 10: Undefined variable 'x'
  ```
  Include line number, column number, and error message. Column comes from the token's position.

**Verify GREEN**: `cmake --build build && build\maple tests\errors\undefined_var.ms 2>&1` → shows `line 2, column 8: Undefined variable 'y'`

**REFACTOR**: Ensure column counting handles tab characters consistently.

### Cycle 2: Source Context and Pointer in Compile Errors

**RED** — Write failing test:
- Create `tests/errors/syntax_error_context.ms`:
  ```
  var x = 1
  print x +
  var y = 2
  ```
- Run and capture stderr → assert output includes:
  - The source line: `    print x +`
  - A `^` pointer at the error column
  Expected format:
  ```
  error: line 2, column 11: Expected expression
      print x +
              ^
  ```
- Expected failure reason: error output doesn't show source line or pointer

**Verify RED**: `cmake --build build && build\maple tests\errors\syntax_error_context.ms 2>&1` → no source line displayed

**GREEN** — Minimal implementation:
- In `src/parser.c`: Extract and store the current source line text. On error, include it in output.
- In `src/compiler.c` error function: Print the source line, then print spaces up to column followed by `^`
- Use error structures from DESIGN.md §4:
  ```c
  typedef struct {
      char message[256];
      MsToken token;
      char sourceLine[512];
  } MsCompileError;
  ```

**Verify GREEN**: `cmake --build build && build\maple tests\errors\syntax_error_context.ms 2>&1` → shows source line and `^` pointer

**REFACTOR**: Clip long source lines to avoid terminal overflow (show context window around error column).

### Cycle 3: Colored Error Output

**RED** — Write failing test:
- Run `build\maple tests\errors\undefined_var.ms 2>&1` → assert output contains ANSI escape sequences for red color on "error:" prefix
- When stdout is not a terminal (piped), colors should be disabled
- Expected failure reason: no ANSI color codes in error output

**Verify RED**: `cmake --build build && build\maple tests\errors\undefined_var.ms 2>&1` → no ANSI color codes

**GREEN** — Minimal implementation:
- Add color helpers in a shared location (e.g., `src/common.h` or new `src/terminal.h`):
  ```c
  #define MS_COLOR_RED     "\033[31m"
  #define MS_COLOR_YELLOW  "\033[33m"
  #define MS_COLOR_RESET   "\033[0m"
  ```
- Add `ms_platform_is_terminal(FILE* stream)` to check if output is a terminal
- In compile error output: wrap "error:" prefix in `MS_COLOR_RED` when stderr is a terminal
- In runtime error output: wrap error text in red, stack trace frames in yellow

**Verify GREEN**: `cmake --build build && build\maple tests\errors\undefined_var.ms 2>&1` → contains `\033[31m` before "error:"

**REFACTOR**: Abstract color output into a helper function that checks terminal and applies color.

### Cycle 4: Runtime Error Stack Traces

**RED** — Write failing test:
- Create `tests/errors/runtime_stack_trace.ms`:
  ```
  fun foo() {
      var x = unknown
  }
  fun bar() {
      foo()
  }
  bar()
  ```
- Run and capture stderr → assert output includes:
  - Error message (e.g., `Runtime error: Undefined variable 'unknown'`)
  - Stack trace with function names and line numbers:
    ```
    Runtime error: Undefined variable 'unknown'.
      in function 'foo' at runtime_stack_trace.ms:2:8
      in function 'bar' at runtime_stack_trace.ms:5:4
      in script at runtime_stack_trace.ms:7:1
    ```
- Expected failure reason: runtime errors may only show error message without stack trace

**Verify RED**: `cmake --build build && build\maple tests\errors\runtime_stack_trace.ms 2>&1` → no stack trace

**GREEN** — Minimal implementation:
- In `src/vm.c`: Implement stack trace printing:
  - Walk call frames from top (current) to bottom (main)
  - For each frame: get function name from closure, get line from chunk's line info array using frame IP
  - Print `  in function 'name' at file:line:col` or `  in script at file:line:col` for top-level
- Use error structures from DESIGN.md §4:
  ```c
  typedef struct {
      char message[256];
      MsCallFrame* frames[MS_FRAMES_MAX];
      int frameCount;
  } MsRuntimeError;
  ```
- Use line info from chunk to map IP to source line

**Verify GREEN**: `cmake --build build && build\maple tests\errors\runtime_stack_trace.ms 2>&1` → shows stack trace with line numbers

**REFACTOR**: Consider limiting stack trace depth to avoid very long output for deep recursion.

### Cycle 5: Import Error Messages with Searched Paths

**RED** — Write failing test:
- Create `tests/errors/import_not_found.ms`: `import nonexistent`
- Run and capture stderr → assert output includes:
  - Module name `nonexistent`
  - List of searched paths (e.g., `./nonexistent.ms`)
  - Clear "not found" message
- Create `tests/errors/circular_import_a.ms`: `import circular_import_b`
- Create `tests/errors/circular_import_b.ms`: `import circular_import_a`
- Run and capture stderr → assert output shows circular chain: `Circular import detected: 'circular_import_a' → 'circular_import_b' → 'circular_import_a'`
- Expected failure reason: import errors may show generic message without paths or chain

**Verify RED**: `cmake --build build && build\maple tests\errors\import_not_found.ms 2>&1` → generic error, no searched paths listed

**GREEN** — Minimal implementation:
- In `src/module.c`: Enhance error messages:
  - Module not found: `Error: Module 'foo' not found. Searched: ./foo.ms, /path1/foo.ms`
  - Circular import: `Error: Circular import detected: 'a' → 'b' → 'a'`
  - Missing export: `Error: Module 'math' has no export 'sqrt'`
- In `ms_module_resolve_path()`: accumulate list of attempted paths
- In `detectCircularDependency()`: build chain string from loading stack

**Verify GREEN**: `cmake --build build && build\maple tests\errors\import_not_found.ms 2>&1` → shows searched paths

**REFACTOR**: Ensure import error messages use same colored format as other errors.

### Cycle 6: End-to-End Error Format Validation

**RED** — Write failing test:
- Run several error scenarios and validate the full output format:
  - Syntax error: verify line, column, source line, pointer, color
  - Runtime error: verify message, stack trace, color
  - Import error: verify searched paths or circular chain
- Write a test script or manual procedure that checks each format
- Expected failure reason: some error types may still have inconsistent formatting

**Verify RED**: Manual review of error output — identify any inconsistencies

**GREEN** — Minimal implementation:
- Ensure all error paths use consistent format: `[color]error:[reset] line X, column Y: message`
- Verify error messages are actionable (tell user what's wrong and hint at fix)
- Ensure first error stops compilation (current single-error behavior maintained)
- Review all `fprintf(stderr, ...)` calls for consistency

**Verify GREEN**: All error scenarios produce well-formatted, colored, helpful output

**REFACTOR**: Consider unifying error output through a single `ms_error_print()` function.

## Acceptance Criteria

- [ ] Compile error shows file, line, column, message, source line with pointer
- [ ] Runtime error shows error message and full stack trace
- [ ] Import errors show search paths attempted
- [ ] Circular import error shows the dependency chain
- [ ] Error output uses colored text (when terminal supports it)
- [ ] Multiple errors: first error stops compilation (current behavior)
- [ ] Error messages are clear and actionable

## Notes

- Error structures from DESIGN.md §4: `MsCompileError` and `MsRuntimeError`
- Color output uses ANSI escape codes, disabled when stderr is not a terminal
- Stack trace walks `MsCallFrame` array from current frame back to main
- Source line display requires storing line text in the parser/compiler during parsing
- The `^` pointer column must account for multi-byte characters or tabs
