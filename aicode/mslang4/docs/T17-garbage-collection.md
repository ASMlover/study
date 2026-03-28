# T17: Garbage Collection

**Phase**: 8 - Garbage Collection
**Dependencies**: T16 (Functions & Closures)
**Estimated Complexity**: High

## Goal

Implement mark-and-sweep garbage collection with a gray worklist (tri-color marking). Integrate GC triggers into allocation paths and add debug/stress modes.

## Files to Modify

| File | Changes |
|------|---------|
| `src/memory.h` | Add GC function declarations, ms_alloc_object declaration, ms_free_objects |
| `src/memory.c` | Implement ms_gc_collect, mark, trace, sweep, ms_alloc_object, ms_free_objects |
| `src/object.c` | Implement ms_gc_blacken_object for each object type |
| `src/vm.c` | Add GC trigger on allocation, compiler root marking |

## TDD Implementation Cycles

### Cycle 1: GC Infrastructure — ms_alloc_object and Memory Tracking

**RED** — Write failing test:
- Create `tests/unit/test_gc.c`
- Write `test_vm_bytes_allocated()`: init VM, verify `bytesAllocated == 0`; allocate a string object, verify `bytesAllocated > 0`; free VM, verify no leak
- Write `test_alloc_object_linked()`: allocate 3 objects, verify `vm->objects` linked list contains all 3; free VM, verify no leak
- Write `test_alloc_object_fields()`: allocate object, verify `type` set correctly, `isMarked == false`, `next` points to previous head of list
- Write `test_alloc_tracks_size()`: allocate objects of different types, verify `bytesAllocated` increases by correct sizes
- Expected failure: linker error — `ms_alloc_object` not declared, or `bytesAllocated` not tracked

**Verify RED**: `cmake --build build 2>&1 | grep "undefined reference"` — link errors for GC symbols

**GREEN** — Minimal implementation:
- In `src/memory.h`:
  - Declare `ms_alloc_object(vm, size, type)` — macro or inline wrapping allocation with GC integration
  - Declare `ms_free_objects(vm)` — free all objects in linked list
  - Define `MS_GC_HEAP_GROW_FACTOR` (e.g., 2)
  - Define debug macros: `MS_DEBUG_LOG_GC`, `MS_DEBUG_STRESS_GC`
- In `src/memory.c`:
  - `ms_alloc_object(vm, size, type)`:
    1. `vm->bytesAllocated += size`
    2. `MsObject* obj = ms_reallocate(NULL, 0, size)`
    3. Set `obj->type = type`, `obj->next = vm->objects`, `obj->isMarked = false`
    4. Link into `vm->objects` list
    5. If `MS_DEBUG_STRESS_GC` defined, call `ms_gc_collect(vm)` on every allocation
    6. Otherwise, if `vm->bytesAllocated > vm->nextGC`, call `ms_gc_collect(vm)`
    7. Return `obj`
  - `ms_free_objects(vm)`: walk `vm->objects` list, call `ms_object_free()` on each
- Update `src/object.c`: ensure all `ms_*_new()` functions use `ms_alloc_object()` instead of raw `ms_reallocate()`

**Verify GREEN**: `cmake --build build && ./build/test_gc` — allocation and tracking tests pass

**REFACTOR**: Ensure all object creation functions go through `ms_alloc_object()`, not raw malloc

### Cycle 2: Mark Phase — Root Marking

**RED** — Write failing test:
- Add `test_mark_stack_values()`: push several values (numbers, strings, closures) onto stack, call `ms_gc_collect()`, verify stack values are still valid (not freed)
- Add `test_mark_globals()`: define global variable pointing to a string, collect GC, verify string still accessible via globals table
- Add `test_mark_call_frames()`: set up a call frame with closure, collect GC, verify closure and its function still valid
- Add `test_mark_open_upvalues()`: capture upvalue, collect GC, verify upvalue location still valid
- Add `test_mark_compiler_roots()`: during compilation (between parse and codegen), collect GC, verify constants and function being compiled survive
- Add `test_mark_module_table()`: store module in `vm->modules`, collect, verify module survives
- Expected failure: assertion failure — objects on stack/globals freed after collection

**Verify RED**: `./build/test_gc` — root marking tests fail (use-after-free or assertion errors)

**GREEN** — Minimal implementation:
- In `src/memory.c`:
  - `ms_gc_mark_roots(vm)`:
    1. Mark all stack values: iterate `vm.stack` to `vm.stackTop`, call `ms_gc_mark_value()` on each
    2. Mark all call frame closures: for each frame, mark the closure; mark each upvalue in the closure
    3. Mark globals: `ms_table_mark(&vm->globals)`
    4. Mark open upvalues: walk `vm->openUpvalues` list, mark each
    5. Mark compiler roots: if `vm->compiler != NULL`, call `ms_compiler_mark_roots(vm->compiler)`
    6. Mark strings table: `ms_table_mark(&vm->strings)`
    7. Mark modules table: `ms_table_mark(&vm->modules)`
  - `ms_gc_mark_value(value)`: if value is an object, call `ms_gc_mark_object(obj)`
  - `ms_gc_mark_object(obj)`: if `obj == NULL` or `obj->isMarked`, return; set `obj->isMarked = true`; add to gray stack via `ms_gray_stack_push(vm, obj)`
  - `ms_gray_stack_push(vm, obj)`: grow gray stack if needed, push object, increment `grayCount`
  - `ms_table_mark(table)`: iterate table entries, mark each value (and key if it's a string object)

**Verify GREEN**: `cmake --build build && ./build/test_gc` — root marking tests pass

**REFACTOR**: Ensure gray stack growth uses `ms_reallocate()` and is tracked in `bytesAllocated`

### Cycle 3: Trace Phase — Gray Worklist Processing

**RED** — Write failing test:
- Add `test_trace_closure_references()`: create closure with function containing constants, mark the closure as root, collect GC, verify function and its constants survive
- Add `test_trace_class_references()`: create class with methods table (requires T18 structs, but can test with available types), mark class, verify method closures survive
- Add `test_trace_upvalue_closed()`: close an upvalue (copy value to `closed`), mark the upvalue, verify closed value survives
- Add `test_trace_nested()`: create chain of objects referencing each other (function → constants → strings), mark the function, verify entire chain survives
- Add `test_gray_stack_empty()`: after trace phase, verify `grayCount == 0` (all gray objects turned black)
- Expected failure: child objects not marked — trace phase not implemented

**Verify RED**: `./build/test_gc` — trace tests fail (referenced objects freed)

**GREEN** — Minimal implementation:
- In `src/object.c`:
  - `ms_gc_blacken_object(vm, obj)`: switch on `obj->type`:
    - `MS_OBJ_STRING`: no references to mark
    - `MS_OBJ_FUNCTION`: mark `function->name`; iterate `function->chunk.constants`, mark each constant value
    - `MS_OBJ_CLOSURE`: mark `closure->function`; iterate `closure->upvalues`, mark each upvalue
    - `MS_OBJ_UPVALUE`: mark `upvalue->closed` value
    - `MS_OBJ_NATIVE`: mark `native->name`
    - (Stubs for `MS_OBJ_CLASS`, `MS_OBJ_INSTANCE`, `MS_OBJ_BOUND_METHOD`, `MS_OBJ_MODULE`, `MS_OBJ_LIST` — to be completed in T18)
- In `src/memory.c`:
  - `ms_gc_trace_references(vm)`:
    1. While `vm->grayCount > 0`:
    2. Pop object from gray stack
    3. Call `ms_gc_blacken_object(vm, obj)` — this marks children, pushing new gray objects
    4. Repeat until gray stack empty

**Verify GREEN**: `cmake --build build && ./build/test_gc` — trace tests pass

**REFACTOR**: Ensure `ms_gc_blacken_object` is extensible for new object types

### Cycle 4: Sweep Phase and Collection Cycle

**RED** — Write failing test:
- Add `test_sweep_unreachable()`: allocate objects, remove all references, trigger GC, verify unreachable objects are freed (check `bytesAllocated` decreases)
- Add `test_sweep_keeps_reachable()`: allocate objects, keep some on stack, trigger GC, verify reachable objects survive
- Add `test_full_gc_cycle()`: run `ms_gc_collect()` explicitly, verify mark→trace→sweep phases execute in order
- Add `test_next_gc_adjustment()`: after collection, verify `vm->nextGC` is updated to `bytesAllocated * MS_GC_HEAP_GROW_FACTOR`
- Add `test_objects_list_after_sweep()`: allocate 5 objects, keep 2 referenced, collect, verify `vm->objects` list has exactly 2 entries
- Add `test_is_marked_reset()`: after sweep, verify surviving objects have `isMarked == false` (reset for next cycle)
- Expected failure: unreachable objects not freed, bytesAllocated not decreasing

**Verify RED**: `./build/test_gc` — sweep tests fail

**GREEN** — Minimal implementation:
- In `src/memory.c`:
  - `ms_gc_sweep(vm)`:
    1. Walk `vm->objects` linked list using pointer-to-pointer (`MsObject** object = &vm->objects`)
    2. If current object `isMarked`: reset `isMarked = false`, advance to `&(*object)->next`
    3. If not marked: save `unreached = *object`, `*object = unreached->next`, call `ms_object_free(unreached)`
    4. Continue until end of list
  - `ms_gc_collect(vm)`:
    1. `size_t before = vm->bytesAllocated` (for debug logging)
    2. `ms_gc_mark_roots(vm)`
    3. `ms_gc_trace_references(vm)`
    4. `ms_table_remove_white(&vm->strings)` — remove dead interned strings
    5. `ms_gc_sweep(vm)`
    6. `vm->nextGC = vm->bytesAllocated * MS_GC_HEAP_GROW_FACTOR`
    7. If `MS_DEBUG_LOG_GC`: print before/after bytes, objects collected
  - `ms_table_remove_white(table)`: iterate string table, remove entries where key object `isMarked == false`

**Verify GREEN**: `cmake --build build && ./build/test_gc` — sweep and full cycle tests pass

**REFACTOR**: Verify sweep correctly handles edge cases (empty list, all alive, all dead)

### Cycle 5: GC Integration with VM Operations

**RED** — Write failing test:
- Add `test_gc_triggered_on_threshold()`: allocate objects until `bytesAllocated > nextGC`, verify GC automatically triggers
- Add `test_gc_during_compilation()`: compile a program that allocates many constants, verify GC doesn't free compiler-allocated objects mid-compilation
- Add `test_gc_during_execution()`: run a script that creates many temporary strings, verify GC keeps execution correct
- Add `test_gc_with_closures()`: create closures with upvalues, trigger GC, verify closures and upvalues remain valid
- Add `test_gc_repeated_cycles()`: run multiple GC cycles, verify objects properly survive across cycles
- Add `test_no_double_free()`: allocate, collect, verify no double-free on subsequent collect
- Expected failure: GC triggers at wrong time, frees live objects, or crashes

**Verify RED**: `./build/test_gc` — integration tests fail

**GREEN** — Minimal implementation:
- Ensure `ms_alloc_object()` triggers GC correctly:
  - If `MS_DEBUG_STRESS_GC`: collect on every allocation (for testing)
  - If `vm->bytesAllocated > vm->nextGC`: collect
  - Don't trigger during GC itself (add `isCollecting` guard to prevent re-entrancy)
- `ms_compiler_mark_roots(compiler)`: walk compiler state chain, mark each state's `function`, mark function's chunk constants
- In `ms_vm_interpret()`: set `vm->compiler` before compilation, clear after
- Ensure `ms_gc_mark_value()` handles all value types correctly (nil, bool, number → no-op; object → mark)

**Verify GREEN**: `cmake --build build && ./build/test_gc` — all integration tests pass

**REFACTOR**: Add `isCollecting` flag to VM to prevent GC re-entrancy

### Cycle 6: Stress Testing and Sanitizer Verification

**RED** — Write failing test:
- Build with `MS_DEBUG_STRESS_GC` enabled
- Add `test_stress_basic_program()`: run simple program (`"var x = 1 + 2\nprint x"`) under stress GC, verify correct output "3"
- Add `test_stress_functions()`: run fibonacci(10) under stress GC, verify "55"
- Add `test_stress_closures()`: run closure counter example under stress GC, verify correct output
- Add `test_stress_string_ops()`: heavy string concatenation under stress GC, verify correct results
- Build with AddressSanitizer/UndefinedBehaviorSanitizer and run all tests
- Expected failure: stress GC exposes bugs — use-after-free, double-free, incorrect marking

**Verify RED**: `cmake -B build -DCMAKE_C_FLAGS="-fsanitize=address,undefined" && cmake --build build && ./build/test_gc` — potential sanitizer errors

**GREEN** — Minimal implementation:
- Fix any bugs exposed by stress testing:
  - Missing mark in `ms_gc_blacken_object` for a reference
  - Incorrect gray stack management
  - Race between allocation and GC (shouldn't exist in single-threaded, but verify)
  - Ensure all `MsValue` locations that can hold objects are properly marked as roots
- Verify `ms_vm_free()` properly frees all remaining objects (call `ms_free_objects()` then free gray stack)
- Ensure no memory leaks: init VM, run programs, free VM → clean under ASan

**Verify GREEN**: `cmake --build build && ./build/test_gc` — all tests pass, no sanitizer errors

**REFACTOR**: Final review of all mark/blacken functions — ensure every reference is traced

## Acceptance Criteria

- [ ] GC collects unreachable objects
- [ ] Reachable objects survive collection
- [ ] `vm->bytesAllocated` stays bounded under repeated allocation
- [ ] No use-after-free: all marked objects remain valid
- [ ] String interning table cleaned of dead strings
- [ ] Stress GC mode works (MS_DEBUG_STRESS_GC)
- [ ] GC log output works (MS_DEBUG_LOG_GC)
- [ ] No memory leaks: init VM, run script, free VM → clean valgrind/sanitizer report
- [ ] Performance: GC doesn't cause noticeable pauses on small programs

## Notes

- GC lifecycle: trigger → mark roots → trace references → remove white strings → sweep → adjust threshold
- `ms_gc_blacken_object` per-type marking: String (nothing), Function (name + constants), Closure (function + upvalues), Upvalue (closed value), Native (name), Class/Instance/BoundMethod/Module/List (T18)
- Gray stack is a dynamic array (`MsObject** grayStack`) that grows as needed during trace
- `MS_GC_HEAP_GROW_FACTOR` controls how aggressively the heap grows after each collection (default: 2x)
- Debug modes: `MS_DEBUG_LOG_GC` prints mark/sweep activity; `MS_DEBUG_STRESS_GC` forces collection on every allocation
