# T22: Main Entry Point & REPL

**Phase**: 13 - Integration
**Dependencies**: T21 (Built-in Functions), T20 (Module System)
**Estimated Complexity**: Medium

## Goal

Implement the CLI entry point with argument parsing, script file execution, and interactive REPL mode. This connects all components into a usable command-line tool.

## Files to Modify

| File | Action |
|------|--------|
| `src/main.c` | Rewrite: full CLI with REPL, script execution, args |

## TDD Implementation Cycles

### Cycle 1: Basic VM Initialization and Teardown in main

**RED** — Write failing test:
- The "test" for main is running the executable and checking exit code
- Write `tests/cli/test_init.sh` (or `.bat`): run `maple` with no args — expect it to start REPL (will hang waiting for input), verify it starts without crash by piping empty input: `echo "" | maple` → exit code 0
- Expected failure reason: `src/main.c` doesn't properly init/free VM, or `ms_builtins_define_all` not called

**Verify RED**: `cmake --build build && echo "" | build\maple` → crash or undefined reference

**GREEN** — Minimal implementation:
- In `src/main.c`:
  ```c
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include "vm.h"
  #include "builtins.h"
  #include "platform.h"

  int main(int argc, char* argv[]) {
      MsVM vm;
      ms_vm_init(&vm);
      ms_builtins_define_all(&vm);

      /* placeholder — will add REPL/file logic next */

      ms_vm_free(&vm);
      return 0;
  }
  ```
- Ensure `ms_vm_init()` and `ms_vm_free()` are called, and `ms_builtins_define_all()` registers all builtins

**Verify GREEN**: `cmake --build build && echo "" | build\maple` → exits with code 0

**REFACTOR**: None needed for this minimal skeleton.

### Cycle 2: Script File Execution

**RED** — Write failing test:
- Create `tests/basic/hello.ms`: `print "Hello, World!"`
- Write test: `build\maple tests\basic\hello.ms` → expect stdout contains `Hello, World!` and exit code 0
- Write test: `build\maple nonexistent.ms` → expect stderr contains error message, exit code non-zero
- Expected failure reason: `run_file()` not implemented, main doesn't handle file argument

**Verify RED**: `cmake --build build && build\maple tests\basic\hello.ms` → no output or crash

**GREEN** — Minimal implementation:
- Add `run_file()` function to `src/main.c`:
  ```c
  static MsInterpretResult run_file(MsVM* vm, const char* path) {
      char* source = ms_platform_read_file(path);
      if (source == NULL) {
          fprintf(stderr, "Could not open file \"%s\".\n", path);
          return MS_INTERPRET_COMPILE_ERROR;
      }
      MsInterpretResult result = ms_vm_interpret(vm, source);
      free(source);
      if (result == MS_INTERPRET_COMPILE_ERROR) exit(65);
      if (result == MS_INTERPRET_RUNTIME_ERROR) exit(70);
      return result;
  }
  ```
- In `main()`: if `argc == 2` (one argument after program name), call `run_file(&vm, argv[1])`
- Exit codes: 0 = success, 65 = compile error, 70 = runtime error (lox convention)

**Verify GREEN**: `cmake --build build && build\maple tests\basic\hello.ms` → outputs `Hello, World!`, exit code 0

**REFACTOR**: None needed.

### Cycle 3: Exit Codes for Error Cases

**RED** — Write failing test:
- Create `tests/errors/syntax_error.ms`: `var x =` (incomplete statement)
- Create `tests/errors/runtime_error.ms`: `var x = unknown_var`
- Write test: `build\maple tests\errors\syntax_error.ms` → exit code 65
- Write test: `build\maple tests\errors\runtime_error.ms` → exit code 70
- Expected failure reason: error exit codes may not be set correctly

**Verify RED**: `cmake --build build && build\maple tests\errors\syntax_error.ms && echo "FAIL: expected non-zero"` → may print FAIL

**GREEN** — Minimal implementation:
- In `run_file()`: After `ms_vm_interpret()`, check result:
  - `MS_INTERPRET_COMPILE_ERROR` → `exit(65)`
  - `MS_INTERPRET_RUNTIME_ERROR` → `exit(70)`
  - `MS_INTERPRET_OK` → fall through, return result

**Verify GREEN**: `cmake --build build && build\maple tests\errors\syntax_error.ms || echo %ERRORLEVEL%` → prints 65

**REFACTOR**: Ensure error messages go to stderr, not stdout.

### Cycle 4: REPL Mode

**RED** — Write failing test:
- Write test: `echo "print 1 + 2" | build\maple` → expect output `3`
- Write test: `printf "var x = 42\nprint x\n" | build\maple` → expect output `42`
- Write test: `echo "" | build\maple` → exits cleanly, no crash
- Expected failure reason: REPL mode not implemented, no stdin reading loop

**Verify RED**: `cmake --build build && echo "print 1 + 2" | build\maple` → no output `3`

**GREEN** — Minimal implementation:
- Add `repl()` function to `src/main.c`:
  ```c
  static void repl(MsVM* vm) {
      char line[1024];
      for (;;) {
          printf("> ");
          if (!fgets(line, sizeof(line), stdin)) {
              printf("\n");
              break;
          }
          ms_vm_interpret(vm, line);
      }
  }
  ```
- In `main()`: if `argc == 1` (no arguments), call `repl(&vm)`

**Verify GREEN**: `cmake --build build && echo "print 1 + 2" | build\maple` → outputs `3`

**REFACTOR**: Handle multi-line input for incomplete blocks (optional enhancement for later).

### Cycle 5: `--version` and `--help` Flags

**RED** — Write failing test:
- Write test: `build\maple --version` → expect stdout contains `Maple v0.1`, exit code 0
- Write test: `build\maple --help` → expect stdout contains `Usage: maple`, exit code 0
- Write test: `build\maple -h` → same as `--help`
- Expected failure reason: argument parsing not implemented, flags treated as filenames

**Verify RED**: `cmake --build build && build\maple --version` → error: "Could not open file --version"

**GREEN** — Minimal implementation:
- In `main()`, add argument parsing loop for `argc > 1`:
  ```c
  for (int i = 1; i < argc; i++) {
      if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
          printf("Usage: maple [script.ms] [options]\n");
          printf("Options:\n");
          printf("  --path <dir>    Add module search path\n");
          printf("  --log-level <l> Set log level\n");
          printf("  --version        Show version\n");
          printf("  --help           Show this help\n");
          return 0;
      } else if (strcmp(argv[i], "--version") == 0) {
          printf("Maple v0.1\n");
          return 0;
      } else if (strcmp(argv[i], "--path") == 0 && i + 1 < argc) {
          ms_vm_add_module_path(&vm, argv[++i]);
      } else {
          run_file(&vm, argv[i]);
      }
  }
  ```

**Verify GREEN**: `cmake --build build && build\maple --version` → outputs `Maple v0.1`

**REFACTOR**: Consider extracting argument parsing into a separate function if it grows.

### Cycle 6: `--path` Flag for Module Search Paths

**RED** — Write failing test:
- Create `tests/modules/greet.ms`: `var msg = "hi"`
- Create `tests/cli/test_path_flag.ms`:
  ```
  import greet
  print "ok"
  ```
- Write test: `build\maple --path tests\modules tests\cli\test_path_flag.ms` → expect output `ok`
- Expected failure reason: `--path` flag may not properly add search path before file execution

**Verify RED**: `cmake --build build && build\maple --path tests\modules tests\cli\test_path_flag.ms` → module not found error

**GREEN** — Minimal implementation:
- In the argument loop, handle `--path <dir>`: call `ms_vm_add_module_path(&vm, argv[++i])` to add the search path
- Ensure `--path` is processed before the script file argument

**Verify GREEN**: `cmake --build build && build\maple --path tests\modules tests\cli\test_path_flag.ms` → outputs `ok`

**REFACTOR**: Handle missing path argument after `--path` gracefully.

## Acceptance Criteria

- [ ] `maple script.ms` runs a Maple script file
- [ ] `maple` starts REPL, evaluates expressions interactively
- [ ] `maple --version` prints version string
- [ ] `maple --help` prints usage information
- [ ] `maple --path /custom/path script.ms` adds module search path
- [ ] Compile error in script → exit code 65
- [ ] Runtime error in script → exit code 70
- [ ] Successful execution → exit code 0
- [ ] REPL handles Ctrl+D (EOF) gracefully
- [ ] No memory leaks on exit (VM freed properly)

## Notes

- Exit codes follow lox convention: 65 for compile error, 70 for runtime error
- REPL uses a simple line-based approach with `fgets()` and a 1024-byte buffer
- Multi-line REPL support (for incomplete blocks) is an optional future enhancement
- All resources are freed via `ms_vm_free()` before exit
- Argument parsing supports mixing flags with script paths: flags are processed in order
