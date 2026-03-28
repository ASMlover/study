# T19: List/Array Support

**Phase**: 10 - Lists
**Dependencies**: T15 (VM Core), T17 (Garbage Collection)
**Estimated Complexity**: Medium

## Goal

Add list/array support with literal syntax `[1, 2, 3]`, subscript access `list[0]`, subscript assignment `list[1] = 10`, and the MsList object type with GC integration.

## Files to Create/Modify

| File | Changes |
|------|---------|
| `src/object.h` | Fully implement MsList struct, add macros |
| `src/object.c` | Implement ms_list_new, ms_list_append, ms_list_get, ms_list_set, ms_list_length, free/print |
| `src/compiler.c` | Compile list literals (OP_BUILD_LIST), subscript get/set |
| `src/vm.c` | Add OP_BUILD_LIST, OP_GET_SUBSCRIPT, OP_SET_SUBSCRIPT dispatch |

## TDD Implementation Cycles

### Cycle 1: MsList Object — Creation and Append

**RED** — Write failing test:
- Create `tests/unit/test_list.c`
- Write `test_list_create_and_append`: create a MsList via `ms_list_new()`, append 3 values with `ms_list_append()`, assert count is 3 and elements are correct
- Write `test_list_empty_create`: create empty list, assert count is 0 and capacity is 0
- Expected failure reason: `ms_list_new`, `ms_list_append`, `ms_list_get` not yet implemented (linker error)

**Verify RED**: `cmake --build build` → linker error: undefined reference to `ms_list_new`, `ms_list_append`, `ms_list_get`

**GREEN** — Minimal implementation:
- In `src/object.h`: Add `MS_OBJ_LIST` to `MsObjectType` enum. Define MsList struct:
  ```c
  typedef struct {
      MsObject obj;
      int count;
      int capacity;
      MsValue* elements;
  } MsList;
  ```
  Add macros: `MS_LIST_VAL(obj)`, `MS_AS_LIST(val)`, `MS_IS_LIST(val)`.
  Declare: `MsList* ms_list_new(MsVM* vm); void ms_list_append(MsVM* vm, MsList* list, MsValue value); MsValue ms_list_get(MsList* list, int index); int ms_list_length(MsList* list);`
- In `src/object.c`: Implement `ms_list_new()` — allocate via `ms_alloc_object()`, init elements=NULL, count=0, capacity=0. Implement `ms_list_append()` — grow array if needed using `MS_GROW_CAPACITY` pattern via `ms_reallocate()`, append value. Implement `ms_list_get()` — return `elements[index]`. Implement `ms_list_length()` — return `count`.

**Verify GREEN**: `cmake --build build && build\test_list` → tests pass

**REFACTOR**: Consider extracting dynamic array growth into a helper macro if reused elsewhere.

### Cycle 2: MsList — Set and Bounds Checking

**RED** — Write failing test:
- Add to `tests/unit/test_list.c`
- Write `test_list_set`: create list, append values, use `ms_list_set()` to change element at index 1, assert new value
- Write `test_list_get_out_of_bounds`: create list, append 1 value, call `ms_list_get(list, 5)` — assert it returns a sentinel or triggers error
- Write `test_list_set_out_of_bounds`: create list, call `ms_list_set(list, 10, value)` — assert error behavior
- Expected failure reason: `ms_list_set` not yet implemented

**Verify RED**: `cmake --build build` → linker error: undefined reference to `ms_list_set`

**GREEN** — Minimal implementation:
- In `src/object.h`: Declare `void ms_list_set(MsList* list, int index, MsValue value);`
- In `src/object.c`: Implement `ms_list_set()` — bounds check (if index >= count or index < 0 → return error `MS_RESULT_RUNTIME_ERROR`), set `elements[index] = value`. Update `ms_list_get()` to add bounds check → runtime error on out of range.

**Verify GREEN**: `cmake --build build && build\test_list` → tests pass

**REFACTOR**: Ensure bounds check error messages include the index and list length.

### Cycle 3: MsList — GC Integration (Free and Mark)

**RED** — Write failing test:
- Add to `tests/unit/test_list.c`
- Write `test_list_gc_mark`: create list with values including a GC-managed string, run GC mark phase, verify string is marked reachable
- Write `test_list_free`: create list, append values, free via `ms_object_free()`, verify no crash/leak
- Expected failure reason: `ms_object_free()` and `ms_gc_blacken_object()` don't handle `MS_OBJ_LIST` yet

**Verify RED**: `cmake --build build && build\test_list` → assertion failure or crash — MsList not freed/marked

**GREEN** — Minimal implementation:
- In `src/object.c` `ms_object_free()`: Add case for `MS_OBJ_LIST`: `MS_FREE_ARRAY(MsValue, list->elements, list->capacity)`, then `ms_reallocate(NULL, sizeof(MsList), 0)` to free MsList
- In `src/object.c` `ms_gc_blacken_object()`: Add case for `MS_OBJ_LIST`: iterate `list->elements[0..count-1]`, call `ms_gc_mark_value()` on each
- In `src/object.c` `ms_object_print()`: Add case for `MS_OBJ_LIST`: print `[`, iterate elements printing each, print `]`

**Verify GREEN**: `cmake --build build && build\test_list` → GC tests pass, no leaks

**REFACTOR**: Verify `ms_print_value()` handles list printing with proper comma separation.

### Cycle 4: Compiler — List Literal Syntax

**RED** — Write failing test:
- Create `tests/integration/test_list_literal.ms`:
  ```
  var list = [1, 2, 3]
  print list
  ```
  Expected output: `[1, 2, 3]`
- Write `test_list_empty_literal.ms`:
  ```
  var list = []
  print list
  ```
  Expected output: `[]`
- Expected failure reason: compiler doesn't recognize `[` as list literal, parse error

**Verify RED**: `cmake --build build && build\maple tests\integration\test_list_literal.ms` → compile error: unexpected token `[`

**GREEN** — Minimal implementation:
- In `src/compiler.c`: Add `OP_BUILD_LIST` to opcode enum. Implement list literal compilation:
  - When parser sees `[`, enter list literal mode
  - Compile each element expression, emitting `OP_CONSTANT` for each
  - On `]`, emit `OP_BUILD_LIST` with element count operand
- In `src/vm.c`: Add `OP_BUILD_LIST` dispatch — pop `operand` values from stack, create MsList via `ms_list_new()`, `ms_list_append()` each value (in correct order), push list onto stack

**Verify GREEN**: `cmake --build build && build\maple tests\integration\test_list_literal.ms` → outputs `[1, 2, 3]`

**REFACTOR**: Ensure element order is preserved (first element = index 0).

### Cycle 5: Subscript Get Access

**RED** — Write failing test:
- Create `tests/integration/test_subscript_get.ms`:
  ```
  var list = [10, 20, 30]
  print list[0]
  print list[2]
  ```
  Expected output: `10` then `30`
- Write `test_nested_subscript.ms`:
  ```
  var m = [[1,2],[3,4]]
  print m[0][1]
  ```
  Expected output: `2`
- Expected failure reason: subscript operator `[]` not compiled, parse error on `list[0]`

**Verify RED**: `cmake --build build && build\maple tests\integration\test_subscript_get.ms` → compile error: unexpected token after expression

**GREEN** — Minimal implementation:
- In `src/compiler.c`: Add `OP_GET_SUBSCRIPT` to opcode enum. In expression parsing (after primary), when `[` follows an expression, compile index expression, then emit `OP_GET_SUBSCRIPT`. Allow chaining for nested subscripts.
- In `src/vm.c`: Add `OP_GET_SUBSCRIPT` dispatch — pop index and object from stack. If object is list and index is number → call `ms_list_get()`. Error otherwise with message.

**Verify GREEN**: `cmake --build build && build\maple tests\integration\test_subscript_get.ms` → outputs `10` then `30`

**REFACTOR**: Check that subscript works as a left-hand-side expression context for assignment (prepare for next cycle).

### Cycle 6: Subscript Assignment

**RED** — Write failing test:
- Create `tests/integration/test_subscript_set.ms`:
  ```
  var list = [1, 2, 3]
  list[1] = 10
  print list[1]
  ```
  Expected output: `10`
- Write `test_subscript_runtime_error.ms`:
  ```
  var list = [1, 2, 3]
  print list[10]
  ```
  Expected: runtime error (out of bounds)
- Write `test_subscript_type_error.ms`:
  ```
  var list = [1, 2]
  print list["hello"]
  ```
  Expected: runtime error (non-number index)
- Expected failure reason: `list[1] = 10` parse fails — subscript assignment not supported

**Verify RED**: `cmake --build build && build\maple tests\integration\test_subscript_set.ms` → compile error or subscript not assignable

**GREEN** — Minimal implementation:
- In `src/compiler.c`: Add `OP_SET_SUBSCRIPT` to opcode enum. In assignment parsing, detect subscript on LHS: compile object, compile index, compile RHS value, emit `OP_SET_SUBSCRIPT`
- In `src/vm.c`: Add `OP_SET_SUBSCRIPT` dispatch — pop value, index, object. If list and number → `ms_list_set()`. Error otherwise.
- Add runtime error for out-of-bounds in `ms_list_get()` and `ms_list_set()`
- Add runtime error for non-number subscript index

**Verify GREEN**: `cmake --build build && build\maple tests\integration\test_subscript_set.ms` → outputs `10`

**REFACTOR**: Consolidate subscript type checking into a shared helper if get/set share similar validation.

## Acceptance Criteria

- [ ] `var list = [1, 2, 3]\nprint list` → "[1, 2, 3]"
- [ ] `print [1,2,3][0]` → "1"
- [ ] `var list = [1,2,3]\nlist[1] = 10\nprint list[1]` → "10"
- [ ] `var list = []\nprint list` → "[]"
- [ ] Out-of-bounds access → runtime error
- [ ] Subscript with non-number index → runtime error
- [ ] Lists are GC-managed (no leaks after collection)
- [ ] Nested lists work: `var m = [[1,2],[3,4]]\nprint m[0][1]` → "2"

## Notes

- MsList uses a dynamic array (`MsValue* elements`) with `MS_GROW_CAPACITY` / `ms_reallocate()` pattern for growth
- All list values must be marked during GC to prevent premature collection of referenced objects
- Subscript access compiles to postfix `[expr]` — similar to function call syntax in the parser
