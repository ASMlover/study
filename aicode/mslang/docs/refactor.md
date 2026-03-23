# Refactoring Plan: Code Structure Iteration

## Motivation

Two monolithic files account for 52% of the codebase (~14K lines):

| File | Lines | Mixed Responsibilities |
|------|------:|------------------------|
| `VM.cc` | 3752 | Native registration, GC, built-in methods, call machinery, import system, dispatch loop |
| `Compiler.cc` | 3431 | Parse-rule table, register allocation, constant folding, variable resolution, expression parsers, statement parsers |

**Goal:** Single responsibility per file; ≤ ~800 lines each (except the irreducible dispatch loop).

## Design Decisions

| Question | Decision | Rationale |
|----------|----------|-----------|
| Subdirectories under `src/`? | **Stay flat** | ~55 files post-split; subdirs add include-path and CMake friction with little benefit |
| Split `Object.hh` (610 lines)? | **Leave intact** | 17 tightly-coupled types; consumers need most of them; splitting only adds cross-includes |
| Share `Compiler` class across TUs? | **Extract `CompilerImpl.hh`** | Class lives inside `Compiler.cc`; internal header lets multiple `.cc` share it |
| Split VM dispatch loop (1786 lines)? | **Accept as-is** | computed-goto + macros + locals make it unsplittable; Lua/CPython/Ruby are the same |

## Target File Layout

### New files

| File | Est. Lines | Responsibility |
|------|:----------:|----------------|
| `src/VMNatives.cc` | ~400 | Native function registration |
| `src/VMGC.cc` | ~500 | GC: mark / sweep / minor / major / incremental |
| `src/VMCall.cc` | ~380 | call dispatch, invoke helpers, upvalue, coroutine resume |
| `src/VMImport.cc` | ~130 | Module import system |
| `src/VMBuiltins.cc` | ~550 | Built-in type methods (string / list / map / tuple / file / …) |
| `src/CompilerImpl.hh` | ~260 | Internal header: `Compiler` class + internal types |
| `src/CompilerExpr.cc` | ~1030 | Expression parsers + parse-rule table |
| `src/CompilerStmt.cc` | ~1170 | Statement and declaration parsers |

### Slimmed files

| File | Before | After | Remaining content |
|------|-------:|------:|-------------------|
| `VM.cc` | 3752 | ~2030 | Constructor/destructor, stack ops, error reporting, `invoke()` (thin dispatcher), `interpret()`, `run()` |
| `Compiler.cc` | 3431 | ~710 | Constructor, register alloc, constant folding, variable resolution, scoping, emit helpers, lifecycle |

## Implementation Phases

> **Rule:** After every phase, run `cmake --build build && ctest --test-dir build --output-on-failure`. All 62 tests must pass before proceeding.

### Phase 0 — Safety Net

**Deps:** none | **Type:** verification only

Run the full test suite on the unmodified codebase to establish a green baseline.

```bash
cmake --build build && ctest --test-dir build --output-on-failure
```

---

### Phase 1 — Extract `VMNatives.cc`

**Deps:** Phase 0 | **Type:** extract method body → new TU

**What moves:** The native-function registration block inside `VM::VM()` (lines 66–422 of `VM.cc` — all `define_native(...)` lambdas).

**Steps:**
1. Add `void register_natives() noexcept;` as a **private** method declaration in `VM.hh` (alongside existing `define_native`).
2. Create `src/VMNatives.cc` with `#include "VM.hh"`. Implement `VM::register_natives()` containing the moved lambda block.
3. In `VM.cc`, replace lines 66–422 with a single call: `register_natives();`

**What stays in `VM.cc`:** Constructor init-strings (lines 52–63), destructor, everything below line 423.

**Verification:** build + test. Key coverage: `tests/natives.ms`, `tests/type.ms`, `tests/closures.ms` (exercise `clock`, `type`, `assert`, `range`, `str`, etc.).

---

### Phase 2 — Extract `VMGC.cc`

**Deps:** Phase 1 | **Type:** move method implementations → new TU

**What moves:** All GC-related VM method implementations (lines 590–1064 of `VM.cc`):

| Method | Line | Method | Line |
|--------|-----:|--------|-----:|
| `mark_roots` | 590 | `mark_remembered_set` | 710 |
| `push_gray` | 650 | `promote_object` | 717 |
| `remember` | 654 | `sweep_young` | 723 |
| `trace_references` | 658 | `nullify_weak_refs` | 776 |
| `sweep` | 673 | `needs_finalization` | 790 |
| `collect_garbage` | 704 | `mark_finalizable` | 801 |
| `run_pending_finalizers` | 829 | `incremental_mark_step` | 966 |
| `minor_gc` | 868 | `incremental_sweep_step` | 993 |
| `major_gc` | 912 | `finish_major_gc` | 1019 |
| `begin_major_gc` | 959 | `incremental_gc_step` | 1030 |
| `free_objects` | 1046 | | |

All 21 methods are already declared in `VM.hh` — no header changes needed.

**Includes for `VMGC.cc`:** `VM.hh`, `Compiler.hh` (for `mark_compiler_roots`), `Memory.hh`, `Logger.hh`

**Verification:** build + test. GC is exercised by nearly every test; pay extra attention to `tests/closures.ms`, `tests/classes.ms`, `tests/inheritance.ms` (deep object graphs).

---

### Phase 3 — Extract `VMCall.cc`

**Deps:** Phase 2 | **Type:** move method implementations → new TU

**What moves:** Call dispatch + upvalue management (lines 1066–1244, 1246–1275, 1771–1823 of `VM.cc`):

| Method | Line |
|--------|-----:|
| `concatenate` | 1066 |
| `call` | 1078 |
| `call_value` | 1139 |
| `resume_coroutine` | 1195 |
| `invoke_from_class` | 1246 |
| `invoke_operator` | 1262 |
| `bind_method` | 1771 |
| `capture_upvalue` | 1789 |
| `close_upvalues` | 1814 |

All 9 methods are already declared in `VM.hh` — no header changes needed.

**Includes for `VMCall.cc`:** `VM.hh`

**What stays in `VM.cc`:** `invoke()` (line 1276) — it calls `call_value` and `invoke_from_class` cross-TU, resolved by linker.

**Verification:** build + test. Key coverage: `tests/functions.ms`, `tests/closures.ms`, `tests/coroutines.ms`, `tests/inheritance.ms`, `tests/operator_overloading.ms`.

---

### Phase 4 — Extract `VMImport.cc`

**Deps:** Phase 3 | **Type:** move method implementations → new TU

**What moves:** Module import methods (lines 1825–1922 of `VM.cc`):

| Method | Line |
|--------|-----:|
| `register_io_module` | 1825 |
| `import_module` | 1857 |

Both are already declared in `VM.hh` — no header changes needed.

**Includes for `VMImport.cc`:** `VM.hh`, `Compiler.hh`, `Serializer.hh`, `Module.hh`

**Verification:** build + test. Key coverage: `tests/import.ms`, `tests/import_alias.ms`, `tests/from_import.ms`.

---

### Phase 5 — Extract `VMBuiltins.cc`

**Deps:** Phase 4 | **Type:** refactor + extract (the only phase that changes logic structure)

**What changes:** The monolithic `VM::invoke()` method (lines 1276–1769) is currently a 493-line function with inline type-specific dispatch. This phase **refactors** it into a thin dispatcher that delegates to per-type methods.

**Steps:**
1. Add 7 private method declarations to `VM.hh`:
   ```cpp
   bool invoke_string_method(ObjString* str, ObjString* name, int arg_count) noexcept;
   bool invoke_list_method(ObjList* list, ObjString* name, int arg_count) noexcept;
   bool invoke_tuple_method(ObjTuple* tuple, ObjString* name, int arg_count) noexcept;
   bool invoke_map_method(ObjMap* map, ObjString* name, int arg_count) noexcept;
   bool invoke_stringbuilder_method(ObjStringBuilder* sb, ObjString* name, int arg_count) noexcept;
   bool invoke_weakref_method(ObjWeakRef* ref, ObjString* name, int arg_count) noexcept;
   bool invoke_file_method(ObjFile* file, ObjString* name, int arg_count) noexcept;
   ```
2. Create `src/VMBuiltins.cc` implementing all 7 methods (extracted from the body of `invoke()`).
3. Rewrite `VM::invoke()` in `VM.cc` as a ~50-line dispatcher: check receiver type → delegate to the appropriate `invoke_*_method()`. The module / static-class / instance / field parts remain inline.

**Includes for `VMBuiltins.cc`:** `VM.hh`

**Why this phase is last among VM splits:** It is the only one that restructures logic rather than just moving existing method implementations. All prior VM phases are pure moves, making them lower-risk and independently verifiable.

**Verification:** build + test. Key coverage: `tests/strings.ms`, `tests/lists.ms`, `tests/maps.ms`, `tests/tuples.ms`, `tests/string_builder.ms`, `tests/file_io.ms`, `tests/weak_ref.ms`.

---

### Phase 6 — Create `CompilerImpl.hh`

**Deps:** Phase 0 (independent of VM phases — can be done in parallel) | **Type:** extract definitions → new internal header

**What moves:** Lines 39–272 of `Compiler.cc`:
- `Precedence` enum (lines 45–62)
- `ExprDesc` struct (lines 68–81)
- `ParseFn` alias, `ParseRule`, `Local`, `Upvalue`, `LoopContext`, `ClassCompiler` (lines 87–118)
- `Compiler` class definition (lines 124–270)

**Key detail — `active_parse_state_`:**
Currently `static ParseState* active_parse_state_ = nullptr;` (line 272, internal linkage). Change to a C++17 inline variable so all compiler TUs share a single instance:
```cpp
// CompilerImpl.hh, inside namespace ms
inline ParseState* active_parse_state_ = nullptr;
```

**Steps:**
1. Create `src/CompilerImpl.hh` with copyright header, `#pragma once`, includes (`<array>`, `<charconv>`, `<cmath>`, `"Compiler.hh"`, `"VM.hh"`, `"Memory.hh"`), and the moved definitions.
2. In `Compiler.cc`, replace lines 39–272 with `#include "CompilerImpl.hh"`.
3. `Compiler.hh` (public API) is **unchanged**.

**Verification:** build + test. This is a pure structural extraction — zero behavioral change. All 62 tests must still pass.

---

### Phase 7 — Extract `CompilerExpr.cc`

**Deps:** Phase 6 | **Type:** move method implementations → new TU

**What moves:** Expression parsers + parse infrastructure from `Compiler.cc`:

| Section | Content | Est. Lines |
|---------|---------|:----------:|
| Parse-rule table | `static const std::array<ParseRule, kTOKEN_COUNT> rules` | ~153 |
| `get_rule`, `parse_precedence`, `expression` | Precedence-driven dispatch | ~35 |
| Expression parsers | `grouping` · `number` · `integer` · `string` · `string_interpolation` · `variable` · `unary` · `binary` · `bitwise` · `literal` · `and_` · `or_` · `call` · `dot` · `this_` · `super_` · `fun_expression` · `list_` · `map_` · `subscript_` · `ternary` · `argument_list` | ~850 |

**Includes for `CompilerExpr.cc`:** `CompilerImpl.hh`, `VM.hh`, `Memory.hh`

**Note:** `parse_precedence` and `get_rule` must be in the same TU as the `rules` table (they directly index it). Statement parsers call `expression()` cross-TU — resolved by linker since they are all `Compiler` member functions.

**Verification:** build + test. Key coverage: `tests/arithmetic.ms`, `tests/strings.ms`, `tests/string_interpolation.ms`, `tests/compound_assignment.ms`, `tests/ternary.ms`, `tests/lists.ms`, `tests/maps.ms`.

---

### Phase 8 — Extract `CompilerStmt.cc`

**Deps:** Phase 6 | **Type:** move method implementations → new TU

**What moves:** All statement and declaration parsers from `Compiler.cc`:

`block` · `var_declaration` · `function` · `fun_declaration` · `method` · `enum_declaration` · `class_declaration` · `import_declaration` · `expression_statement` · `print_statement` · `if_statement` · `while_statement` · `for_statement` · `for_in_statement` · `list_comprehension_` · `break_statement` · `continue_statement` · `return_statement` · `try_statement` · `throw_statement` · `yield_statement` · `defer_statement` · `switch_statement` · `synchronize` · `statement` · `declaration`

**Includes for `CompilerStmt.cc`:** `CompilerImpl.hh`, `VM.hh`, `Memory.hh`

**What stays in `Compiler.cc` (~710 lines):**
- Constructor
- Register allocation (`alloc_reg`, `free_reg`, `check_stack`, `discharge`, `to_rk`, `expr_to_reg`, `expr_to_nextreg`)
- Constant folding (`record_constant`, `invalidate_constants`, `try_fold_unary`, `try_fold_binary`)
- Error reporting (`error_at`, `error`, `error_at_current`)
- Token consumption (`advance`, `consume`, `consume_semi`, `check`, `match`)
- Bytecode emission (`current_chunk`, `emit_instr`, `emit_jump`, `patch_jump`, `emit_loop`, `emit_return`, `make_constant`)
- Variable resolution (`identifier_constant` through `named_variable`)
- Scoping (`begin_scope`, `end_scope`)
- Lifecycle (`end_compiler`, `compile` ×2, `mark_compiler_roots`)

**Verification:** build + test. Key coverage: `tests/control_flow.ms`, `tests/classes.ms`, `tests/inheritance.ms`, `tests/closures.ms`, `tests/coroutines.ms`, `tests/exceptions.ms`, `tests/import.ms`, `tests/enum.ms`, `tests/switch.ms`, `tests/defer.ms`.

## Phase Dependency Graph

```
Phase 0 (baseline)
  ├── Phase 1 (VMNatives)
  │     └── Phase 2 (VMGC)
  │           └── Phase 3 (VMCall)
  │                 └── Phase 4 (VMImport)
  │                       └── Phase 5 (VMBuiltins)  ← only refactoring phase
  │
  └── Phase 6 (CompilerImpl.hh)   ← can start independently of VM phases
        ├── Phase 7 (CompilerExpr)
        └── Phase 8 (CompilerStmt)  ← independent of Phase 7
```

VM track (Phases 1–5) and Compiler track (Phases 6–8) are **fully independent** — they can be done in parallel.

## Include Dependency Map (post-refactor)

```
VMNatives.cc     ──►  VM.hh
VMGC.cc          ──►  VM.hh, Compiler.hh, Memory.hh, Logger.hh
VMCall.cc        ──►  VM.hh
VMImport.cc      ──►  VM.hh, Compiler.hh, Serializer.hh, Module.hh
VMBuiltins.cc    ──►  VM.hh
VM.cc            ──►  VM.hh, Compiler.hh, Memory.hh, Debug.hh, Serializer.hh

CompilerImpl.hh  ──►  Compiler.hh, VM.hh, Memory.hh
CompilerExpr.cc  ──►  CompilerImpl.hh
CompilerStmt.cc  ──►  CompilerImpl.hh
Compiler.cc      ──►  CompilerImpl.hh, Debug.hh, Optimize.hh
```

> The existing circular `.cc`-level dependency (`VM.cc` ↔ `Compiler.cc`) is link-time only — no header-level circularity is introduced.

## Progress Tracking

| Phase | Description | Type | Files Touched | Status |
|:-----:|-------------|------|---------------|:------:|
| 0 | Baseline — confirm 62 tests pass | verify | — | [x] |
| 1 | Extract `VMNatives.cc` | extract | `VM.hh` · `VM.cc` · `VMNatives.cc` *(new)* | [x] |
| 2 | Extract `VMGC.cc` | move | `VM.cc` · `VMGC.cc` *(new)* | [x] |
| 3 | Extract `VMCall.cc` | move | `VM.cc` · `VMCall.cc` *(new)* | [x] |
| 4 | Extract `VMImport.cc` | move | `VM.cc` · `VMImport.cc` *(new)* | [x] |
| 5 | Extract `VMBuiltins.cc` | **refactor** | `VM.hh` · `VM.cc` · `VMBuiltins.cc` *(new)* | [x] |
| 6 | Create `CompilerImpl.hh` | extract | `Compiler.cc` · `CompilerImpl.hh` *(new)* | [x] |
| 7 | Extract `CompilerExpr.cc` | move | `Compiler.cc` · `CompilerExpr.cc` *(new)* | [x] |
| 8 | Extract `CompilerStmt.cc` | move | `Compiler.cc` · `CompilerStmt.cc` *(new)* | [ ] |
