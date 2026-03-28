# T21: Built-in Functions

**Phase**: 12 - Builtins
**Dependencies**: T16 (Functions & Closures)
**Estimated Complexity**: Medium

## Goal

Implement all built-in native functions: `print`, `clock`, `type`, `len`, `input`, `str`, `num`. These are registered as native functions in the VM's global scope during initialization.

## Files to Create/Modify

| File | Action |
|------|--------|
| `src/builtins.h` | Create: builtin declarations |
| `src/builtins.c` | Create: builtin implementations |
| `src/vm.c` | Modify: call `ms_builtins_define_all()` during init |

## TDD Implementation Cycles

### Cycle 1: Builtin Framework and `print`

**RED** — Write failing test:
- Create `tests/unit/test_builtins.c`
- Write `test_builtin_print_string`: call `ms_builtin_print(vm, 1, args)` with a string value `ms_string_val("hello")`, capture stdout output → assert output is `"hello\n"`
- Write `test_builtin_print_number`: call with number value `ms_number_val(42.0)` → assert output is `"42\n"`
- Write `test_builtin_print_nil`: call with nil → assert output is `"nil\n"`
- Write `test_builtins_define_all`: init VM, call `ms_builtins_define_all(&vm)`, look up "print" in globals → assert non-nil
- Expected failure reason: `src/builtins.h` and `src/builtins.c` don't exist, linker error

**Verify RED**: `cmake --build build` → linker error: undefined reference to `ms_builtins_define_all`, `ms_builtin_print`

**GREEN** — Minimal implementation:
- Create `src/builtins.h`:
  ```c
  #ifndef MS_BUILTINS_H
  #define MS_BUILTINS_H

  #include "vm.h"

  void ms_builtins_define_all(MsVM* vm);
  MsValue ms_builtin_print(MsVM* vm, int argCount, MsValue* args);
  MsValue ms_builtin_clock(MsVM* vm, int argCount, MsValue* args);
  MsValue ms_builtin_type(MsVM* vm, int argCount, MsValue* args);
  MsValue ms_builtin_len(MsVM* vm, int argCount, MsValue* args);
  MsValue ms_builtin_input(MsVM* vm, int argCount, MsValue* args);
  MsValue ms_builtin_str(MsVM* vm, int argCount, MsValue* args);
  MsValue ms_builtin_num(MsVM* vm, int argCount, MsValue* args);

  #endif
  ```
- Create `src/builtins.c`: Implement `ms_builtins_define_all()` — for each builtin, create MsString name, create MsNative via `ms_native_new()`, store in globals table using `ms_table_set()`.
- Implement `ms_builtin_print(vm, argCount, args)`: print each argument via `ms_print_value()` to stdout, print newline, return `ms_nil_val()`

**Verify GREEN**: `cmake --build build && build\test_builtins` → print tests pass

**REFACTOR**: Consider adding argCount validation (e.g., print accepts any count, others have fixed arity).

### Cycle 2: `clock` and `type` Builtins

**RED** — Write failing test:
- Add to `tests/unit/test_builtins.c`
- Write `test_builtin_clock`: call `ms_builtin_clock(vm, 0, NULL)` → assert return value is a number >= 0
- Write `test_builtin_type_number`: call `ms_builtin_type(vm, 1, &ms_number_val(42.0))` → assert return is MsString `"number"`
- Write `test_builtin_type_string`: call with string value → assert `"string"`
- Write `test_builtin_type_bool`: call with `ms_bool_val(true)` → assert `"bool"`
- Write `test_builtin_type_nil`: call with nil → assert `"nil"`
- Expected failure reason: `ms_builtin_clock` and `ms_builtin_type` return nil (stub)

**Verify RED**: `cmake --build build && build\test_builtins` → assertion failure: expected "number" string, got nil

**GREEN** — Minimal implementation:
- `ms_builtin_clock(vm, argCount, args)`: return `ms_number_val(ms_platform_get_time_seconds())`
- `ms_builtin_type(vm, argCount, args)`: inspect `args[0]` type tag, return MsString: `"nil"`, `"bool"`, `"number"`, `"string"`, `"function"`, `"class"`, `"instance"`, `"list"`

**Verify GREEN**: `cmake --build build && build\test_builtins` → clock and type tests pass

**REFACTOR**: Use a lookup table for type name strings instead of long if-else chain.

### Cycle 3: `len` Builtin

**RED** — Write failing test:
- Add to `tests/unit/test_builtins.c`
- Write `test_builtin_len_string`: call `ms_builtin_len(vm, 1, &ms_string_val("hello"))` → assert returns `ms_number_val(5.0)`
- Write `test_builtin_len_list`: create MsList with 3 elements, call `ms_builtin_len(vm, 1, &listVal)` → assert returns `ms_number_val(3.0)`
- Write `test_builtin_len_invalid`: call with number value → assert runtime error returned
- Expected failure reason: `ms_builtin_len` not implemented

**Verify RED**: `cmake --build build && build\test_builtins` → assertion failure: len returns wrong value or nil

**GREEN** — Minimal implementation:
- `ms_builtin_len(vm, argCount, args)`: if args[0] is string → return `ms_number_val(string->length)`. If args[0] is list → return `ms_number_val(list->count)`. Otherwise → runtime error "len() expects string or list"

**Verify GREEN**: `cmake --build build && build\test_builtins` → len tests pass

**REFACTOR**: None needed.

### Cycle 4: `str` and `num` Conversion Builtins

**RED** — Write failing test:
- Add to `tests/unit/test_builtins.c`
- Write `test_builtin_str_number`: call `ms_builtin_str(vm, 1, &ms_number_val(42.0))` → assert returns MsString `"42"`
- Write `test_builtin_str_bool`: call with `ms_bool_val(true)` → assert returns MsString `"true"`
- Write `test_builtin_num_string`: call `ms_builtin_num(vm, 1, &ms_string_val("3.14"))` → assert returns `ms_number_val(3.14)`
- Write `test_builtin_num_number`: call with number → returns same number
- Write `test_builtin_num_bool`: call with `ms_bool_val(true)` → returns `ms_number_val(1.0)`
- Write `test_builtin_num_invalid`: call with list → runtime error
- Expected failure reason: `ms_builtin_str` and `ms_builtin_num` not implemented

**Verify RED**: `cmake --build build && build\test_builtins` → assertion failure

**GREEN** — Minimal implementation:
- `ms_builtin_str(vm, argCount, args)`: convert value to string representation using `ms_print_value()` output, wrap as MsString, return
- `ms_builtin_num(vm, argCount, args)`: if string → parse with `strtod()`, return number. If number → return as-is. If bool → return 1.0 or 0.0. Otherwise → runtime error "num() expects string, number, or bool"

**Verify GREEN**: `cmake --build build && build\test_builtins` → str and num tests pass

**REFACTOR**: Ensure `ms_print_value()` formatting matches expected output (e.g., integer vs float display).

### Cycle 5: `input` Builtin

**RED** — Write failing test:
- Add to `tests/unit/test_builtins.c`
- Write `test_builtin_input_no_prompt`: pipe "hello world\n" to stdin, call `ms_builtin_input(vm, 0, NULL)` → assert returns MsString `"hello world"`
- Write `test_builtin_input_with_prompt`: call with prompt arg `"Name: "`, verify prompt printed to stdout (no newline), then reads from piped stdin
- Write `test_builtin_input_strips_newline`: pipe "test\n" → assert returned string does not end with `\n`
- Expected failure reason: `ms_builtin_input` not implemented

**Verify RED**: `cmake --build build && build\test_builtins` → assertion failure

**GREEN** — Minimal implementation:
- `ms_builtin_input(vm, argCount, args)`: if argCount > 0, print args[0] as prompt (no newline via `fputs`). Read line from stdin via `fgets()` into buffer. Strip trailing newline (`\n` or `\r\n`). Return as MsString via `ms_string_copy()`.

**Verify GREEN**: `cmake --build build && build\test_builtins` → input tests pass

**REFACTOR**: Handle edge cases: EOF before newline, very long input lines, empty input.

### Cycle 6: Integration — Builtins Accessible from Maple Scripts

**RED** — Write failing test:
- Create `tests/integration/test_builtins.ms`:
  ```
  print("hello")
  print(type(42))
  print(len("hello"))
  print(str(3.14))
  print(num("42") + 1)
  ```
  Expected output:
  ```
  hello
  number
  5
  3.14
  43
  ```
- Expected failure reason: `ms_builtins_define_all()` not yet called during VM init in main/REPL, or builtins not in globals

**Verify RED**: `cmake --build build && build\maple tests\integration\test_builtins.ms` → runtime error: undefined variable 'print'

**GREEN** — Minimal implementation:
- In `src/vm.c` `ms_vm_init()`: Call `ms_builtins_define_all(vm)` after globals table initialization
- Verify all 7 builtins are registered: `print`, `clock`, `type`, `len`, `input`, `str`, `num`

**Verify GREEN**: `cmake --build build && build\maple tests\integration\test_builtins.ms` → correct output

**REFACTOR**: Ensure builtins are registered before any user code can run (including module loading).

## Acceptance Criteria

- [ ] `print("hello")` outputs "hello" followed by newline
- [ ] `var t = clock()` returns a number >= 0
- [ ] `type(42)` returns "number"
- [ ] `type("hello")` returns "string"
- [ ] `type(true)` returns "bool"
- [ ] `len("hello")` returns 5
- [ ] `len([1,2,3])` returns 3
- [ ] `str(42)` returns "42" as a string
- [ ] `num("3.14")` returns 3.14 as a number
- [ ] `input()` reads from stdin (test with piped input)
- [ ] All builtins accessible without import

## Notes

- All builtins are MsNative objects registered in the VM's global table during init
- `print` uses `ms_print_value()` for consistent value formatting
- `clock` depends on `ms_platform_get_time_seconds()` from platform layer
- `input` uses `fgets()` for stdin reading — platform layer may abstract this further
- Each builtin should validate argument count and types, producing clear runtime errors
