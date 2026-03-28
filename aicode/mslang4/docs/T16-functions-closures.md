# T16: Functions & Closures

**Phase**: 7 - Functions & Closures
**Dependencies**: T15 (VM Core)
**Estimated Complexity**: High

## Goal

Add first-class functions, closures with upvalues, and native function binding. This extends the compiler, object system, and VM with function declaration, call frames, upvalue capture, and closure execution.

## Files to Modify

| File | Changes |
|------|---------|
| `src/object.h` | Fully implement MsFunction, MsClosure, MsUpvalue, MsNative |
| `src/object.c` | Function/closure/upvalue/native creation and free |
| `src/compiler.h` | No changes needed |
| `src/compiler.c` | Add `compileFuncDecl`, upvalue resolution, OP_CLOSURE emission |
| `src/vm.h` | No changes needed (call frames already defined) |
| `src/vm.c` | Add call/callValue, OP_CALL, OP_CLOSURE, OP_CLOSE_UPVALUE, upvalue management |

## TDD Implementation Cycles

### Cycle 1: Object System Extensions (MsFunction, MsClosure, MsUpvalue, MsNative)

**RED** — Write failing test:
- Create `tests/unit/test_functions.c`
- Write `test_function_object()`: create `MsFunction` via `ms_function_new(vm, name)`, verify type is `MS_OBJ_FUNCTION`, chunk is initialized, name matches
- Write `test_closure_object()`: create `MsClosure` via `ms_closure_new(vm, function)`, verify type is `MS_OBJ_CLOSURE`, function pointer matches, upvalue array initialized
- Write `test_upvalue_object()`: create `MsUpvalue` via `ms_upvalue_new(vm, &slot)`, verify type is `MS_OBJ_UPVALUE`, location points to slot
- Write `test_native_object()`: create `MsNative` via `ms_native_new(vm, fn, name, arity)`, verify type is `MS_OBJ_NATIVE`, arity matches
- Write `test_type_macros()`: verify `MS_IS_FUNCTION`, `MS_IS_CLOSURE`, `MS_IS_NATIVE` return true for respective types; `MS_AS_FUNCTION`, `MS_AS_CLOSURE`, `MS_AS_NATIVE` cast correctly
- Write `test_object_free_chain()`: create function + closure + upvalue, free VM, verify no leak
- Expected failure: linker error — `ms_function_new`, `ms_closure_new`, `ms_upvalue_new`, `ms_native_new` undefined

**Verify RED**: `cmake --build build 2>&1 | grep "undefined reference"` — link errors for new object functions

**GREEN** — Minimal implementation:
- In `src/object.h`: add type check macros:
  - `MS_IS_FUNCTION(v)`, `MS_IS_CLOSURE(v)`, `MS_IS_NATIVE(v)`, `MS_IS_UPVALUE(v)`
  - `MS_AS_FUNCTION(v)`, `MS_AS_CLOSURE(v)`, `MS_AS_NATIVE(v)`, `MS_AS_UPVALUE(v)`
- In `src/object.c`:
  - `ms_function_new(vm, name)`: allocate `MsFunction`, init chunk via `ms_chunk_init()`, set name, return
  - `ms_closure_new(vm, function)`: allocate `MsClosure` with upvalue array of `function->upvalueCount` entries, set function, return
  - `ms_upvalue_new(vm, slot)`: allocate `MsUpvalue`, set `location = slot`, `closed = MS_VALUE_NIL`, `next = NULL`, return
  - `ms_native_new(vm, fn, name, arity)`: allocate `MsNative`, set function pointer, name, arity, return
  - Update `ms_object_free()`: handle `MS_OBJ_FUNCTION` (free chunk, free function), `MS_OBJ_CLOSURE` (free upvalues array, free closure), `MS_OBJ_UPVALUE` (free upvalue), `MS_OBJ_NATIVE` (free name ref, free native)
- Ensure `MsFunction` uses `MsChunk chunk` (not placeholder), has `int arity` and `int upvalueCount` fields, `MsString* name`

**Verify GREEN**: `cmake --build build && ./build/test_functions` — object creation and free tests pass

**REFACTOR**: Verify all object types have consistent allocation pattern through `ms_alloc_object()`

### Cycle 2: Function Declaration Compilation

**RED** — Write failing test:
- Add `test_compile_function()`: compile `"fn add(a, b) { return a + b }\nprint add(1, 2)"`, verify chunk contains:
  - `OP_CLOSURE` with function constant index and upvalue operands
  - `OP_DEFINE_GLOBAL("add")`
  - `OP_CONSTANT(1)`, `OP_CONSTANT(2)`, `OP_CALL(2)`, `OP_PRINT`
- Add `test_compile_function_no_params()`: compile `"fn foo() { print 1 }"`, verify `OP_CLOSURE` emitted, arity 0
- Add `test_compile_nested_function()`: compile function containing another function declaration, verify `OP_CLOSURE` with upvalue operands
- Expected failure: compiler error — `compileFuncDecl` not implemented

**Verify RED**: `./build/test_functions` — compilation tests fail

**GREEN** — Minimal implementation:
- In `src/compiler.c`:
  - Replace stub `compileFuncDecl()` with full implementation:
    1. Consume `fn`, consume identifier (function name)
    2. Create new `MsCompilerState` linked to current (enclosing)
    3. Create new `MsFunction` with name
    4. Set `type = MS_FUNC_FUNCTION`
    5. Declare function name as local in enclosing scope
    6. Begin scope, declare parameters as locals
    7. Compile function body (block)
    8. Emit `OP_NIL` + `OP_RETURN` at end (implicit return)
    9. Emit `OP_CLOSURE` with function constant index
    10. For each upvalue, emit operand: `isLocal` bit + index
    11. Define as variable in enclosing scope
  - `addUpvalue(state, index, isLocal)`: check for duplicates in upvalues array, add if new, return upvalue index
  - `resolveUpvalue(state, name)`: walk enclosing compiler states looking for variable; if found in local scope, add as local upvalue; if found as upvalue in enclosing, add as non-local upvalue; return upvalue index or -1

**Verify GREEN**: `cmake --build build && ./build/test_functions` — function compilation tests pass

**REFACTOR**: Verify upvalue operand encoding is correct for nested closures

### Cycle 3: Function Calls in VM

**RED** — Write failing test:
- Add `test_call_simple_function()`: `ms_vm_interpret(vm, "fn add(a, b) { return a + b }\nprint add(1, 2)")` → "3"
- Add `test_call_no_args()`: `ms_vm_interpret(vm, "fn greet() { print \"hi\" }\ngreet()")` → "hi"
- Add `test_call_arity_error()`: `ms_vm_interpret(vm, "fn foo(a) {}\nfoo(1, 2)")` → `MS_INTERPRET_RUNTIME_ERROR`
- Add `test_call_non_callable()`: `ms_vm_interpret(vm, "var x = 1\nx()")` → runtime error
- Add `test_call_stack_overflow()`: deeply recursive function → `MS_INTERPRET_RUNTIME_ERROR`
- Expected failure: `OP_CALL` not handled in VM

**Verify RED**: `./build/test_functions` — call tests fail or crash

**GREEN** — Minimal implementation:
- In `src/vm.c`:
  - `OP_CLOSURE`: read constant index, create `MsClosure` from function, read upvalue operands and populate closure's upvalue array (from stack for local upvalues, from enclosing closure for non-local)
  - `OP_CALL`: read argCount, `callValue(peek(argCount), argCount)` — dispatch on callee type
  - `call(closure, argCount)`: check arity, check `frameCount < MS_FRAMES_MAX`, set up new call frame: `frame->closure = closure`, `frame->ip = closure->function->chunk.code`, `frame->slots = stackTop - argCount - 1`
  - `callValue(callee, argCount)`: if closure → `call()`; if native → execute native, push result; else runtime error "can only call functions"
  - `OP_RETURN`: close upvalues for current frame, pop call frame, push return value on caller's stack, return from `run()` if top-level frame popped
- Native function type definition: `typedef MsValue (*MsNativeFn)(MsVM* vm, int argCount, MsValue* args)`

**Verify GREEN**: `cmake --build build && ./build/test_functions` — function call tests pass

**REFACTOR**: Ensure call frame slot pointers are correct (callee's stack starts at receiver position)

### Cycle 4: Return Values and Recursion

**RED** — Write failing test:
- Add `test_return_value()`: `ms_vm_interpret(vm, "fn double(x) { return x * 2 }\nprint double(5)")` → "10"
- Add `test_implicit_return_nil()`: `ms_vm_interpret(vm, "fn noop() {}\nvar x = noop()\nprint x")` → "nil"
- Add `test_return_from_middle()`: `ms_vm_interpret(vm, "fn abs(x) { if (x < 0) return -x\nreturn x }\nprint abs(-3)")` → "3"
- Add `test_recursion_fib()`: `ms_vm_interpret(vm, "fn fib(n) { if (n <= 1) return n\nreturn fib(n-1) + fib(n-2) }\nprint fib(10)")` → "55"
- Add `test_recursion_factorial()`: `ms_vm_interpret(vm, "fn fact(n) { if (n <= 1) return 1\nreturn n * fact(n-1) }\nprint fact(5)")` → "120"
- Expected failure: recursion tests may fail due to incorrect return handling

**Verify RED**: `./build/test_functions` — return and recursion tests fail

**GREEN** — Minimal implementation:
- Ensure `OP_RETURN` in VM:
  - If returning from `MS_FUNC_SCRIPT` (top level), pop frame, return `MS_INTERPRET_OK`
  - Otherwise: capture return value, close upvalues for current frame (`closeUpvalues(frame->slots)`), pop frame, discard all locals, push return value on caller's stack
- Ensure `compileFuncDecl()` emits implicit `OP_NIL` + `OP_RETURN` at end of function body if no explicit return
- Verify `OP_RETURN` in compiler emits `OP_RETURN` opcode

**Verify GREEN**: `cmake --build build && ./build/test_functions` — return and recursion tests pass

**REFACTOR**: Verify implicit return doesn't double-emit if last statement is already return

### Cycle 5: Upvalue Capture and Closures

**RED** — Write failing test:
- Add `test_closure_capture()`: `ms_vm_interpret(vm, "fn makeCounter() { var count = 0\nfn inc() { count = count + 1\nreturn count }\nreturn inc }\nvar c = makeCounter()\nprint c()\nprint c()")` → "1" then "2"
- Add `test_closure_multiple()`: function returning two closures that share a captured variable, verify both see mutations
- Add `test_nested_closure()`: closure capturing variable from two levels up, verify correct value
- Add `test_closure_outlive_scope()`: returned closure still works after enclosing function returns
- Add `test_upvalue_close()`: `"fn f() { var x = 1\n{ var y = 2\nreturn x + y }\n}\nprint f()"` → "3"
- Expected failure: upvalues not captured, closures don't see enclosing variables

**Verify RED**: `./build/test_functions` — closure tests fail

**GREEN** — Minimal implementation:
- In `src/vm.c`:
  - `captureUpvalue(vm, slot)`: walk `vm->openUpvalues` list; if upvalue for this slot exists, return it; otherwise create new `MsUpvalue` pointing to slot, insert into sorted list, return it
  - `closeUpvalues(vm, last)`: walk open upvalues list; for each upvalue at or above `last`, move value to `closed`, set `location = &closed`, remove from open list
  - `OP_CLOSE_UPVALUE`: pop value, close upvalue at current stack top
  - `OP_CLOSURE`: when populating upvalues — if `isLocal`, call `captureUpvalue(vm, &frame->slots[index])`; if not local, copy from enclosing closure's upvalues
  - Maintain `vm->openUpvalues` linked list (sorted by stack slot, highest first)
- In `src/compiler.c`:
  - `resolveUpvalue()`: recursively search enclosing states; if variable found as local, add as local upvalue (isLocal=true); if found as upvalue, add as non-local upvalue (isLocal=false)

**Verify GREEN**: `cmake --build build && ./build/test_functions` — all closure tests pass

**REFACTOR**: Ensure open upvalues list is correctly maintained and sorted for efficient lookup

### Cycle 6: Native Function Binding

**RED** — Write failing test:
- Add `test_native_clock()`: define native `clock()` returning current time, call it, verify result is a number
- Add `test_native_custom()`: define a custom native `double(x)` that returns `x * 2`, call `double(5)`, verify output "10"
- Add `test_native_string_ops()`: define native `strLen(s)` returning string length, call `strLen("hello")`, verify "5"
- Add `test_native_arity_error()`: call native with wrong number of args → runtime error
- Expected failure: no native function mechanism

**Verify RED**: `./build/test_functions` — native function tests fail

**GREEN** — Minimal implementation:
- In `src/vm.c`:
  - `defineNative(vm, name, fn, arity)`: create `MsNative` object, store in globals table
  - In `callValue()`: if `MS_OBJ_NATIVE`, check arity, call `native->function(vm, argCount, args)`, push result
  - Native functions receive `MsVM*`, `int argCount`, `MsValue* args`; return `MsValue`
- In `ms_vm_init()`: define standard natives (e.g., `clock`, `print` as native if not opcode-based)
- Write test helpers to register custom natives during tests

**Verify GREEN**: `cmake --build build && ./build/test_functions` — native function tests pass

**REFACTOR**: Move native definitions to a separate `natives.c`/`natives.h` for organization

### Cycle 7: Integration Tests

**RED** — Write failing test:
- Write `tests/functions/basic.ms`: function declarations, calls, returns, multiple parameters
- Write `tests/functions/closures.ms`: closure capture, multiple closures sharing state, nested closures
- Write `tests/functions/recursion.ms`: fibonacci, factorial, mutual recursion (if supported)
- Expected failure: integration test scripts produce wrong output

**Verify RED**: `./build/test_functions` or run `.ms` scripts — integration tests fail

**GREEN** — Minimal implementation:
- Run each `.ms` script through full pipeline (source → scanner → parser → compiler → VM)
- Compare output against expected output files (`.ms.expected`)
- Fix any issues discovered by integration tests (edge cases in upvalue capture, return handling, etc.)
- Ensure `ms_compiler_mark_roots()` correctly marks function objects during GC (even though full GC is T17, this prevents premature collection)

**Verify GREEN**: `cmake --build build && ./build/test_functions` — all tests pass including integration scripts

**REFACTOR**: Final cleanup — verify all function/closure paths are robust

## Acceptance Criteria

- [ ] Define and call a function: `fn add(a, b) { return a + b }\nprint add(1, 2)` → "3"
- [ ] Recursive function: `fn fib(n) { if (n <= 1) return n\nreturn fib(n-1) + fib(n-2) }\nprint fib(10)` → "55"
- [ ] Closures: `fn makeCounter() { var count = 0\nfn increment() { count = count + 1\nreturn count }\nreturn increment }\nvar c = makeCounter()\nprint c()\nprint c()` → "1" then "2"
- [ ] Upvalues capture and mutate enclosing variables
- [ ] Native functions can be defined and called
- [ ] Arity checking: wrong number of args → runtime error
- [ ] Stack overflow from deep recursion → runtime error
- [ ] No memory leaks with closures and upvalues

## Notes

- Object types to fully implement: `MsFunction` (with `MsChunk chunk`, `int arity`, `int upvalueCount`, `MsString* name`), `MsClosure` (with `MsFunction* function`, `MsUpvalue** upvalues`), `MsUpvalue` (with `MsValue* location`, `MsValue closed`, `MsUpvalue* next`), `MsNative` (with `MsNativeFn function`, `MsString* name`, `int arity`)
- Open upvalues form a linked list on the VM, sorted by stack slot (descending), to allow efficient closing
- `ms_compiler_mark_roots()` marks the current compiler state's function and constants as GC roots
- `OP_CLOSURE` operands: for each upvalue, one byte with bit 0 = isLocal, remaining bits = index
