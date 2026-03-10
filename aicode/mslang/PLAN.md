# PLAN.md — Maple Language Implementation Tasks

Phased implementation plan. Each task has dependencies and verification criteria.

---

## Phase 1: Foundation

### Task 1.1: CMake Build System
- **Files**: `CMakeLists.txt`
- **Work**: cmake 3.22+, C++23 flags (MSVC/GCC/Clang), Debug/Release configs, GLOB_RECURSE `src/*.hh` `src/*.cc`, MSVC source_group_by_dir, debug options (TRACE/PRINT/STRESS_GC/LOG_GC), CTest integration (`tests/*.ms`)
- **Verify**: `cmake -B build && cmake --build build` compiles empty main.cc

### Task 1.2: Platform Headers
- **Files**: `src/Macros.hh`, `src/Types.hh`, `src/Consts.hh`, `src/Common.hh`
- **Depends**: 1.1
- **Work**:
  - `Macros.hh` — MAPLE_GNUC/MAPLE_MSVC detection, MAPLE_UNUSED
  - `Types.hh` — type aliases (u8_t, str_t, sz_t, ssz_t, ...)
  - `Consts.hh` — kSTACK_MAX=256, kFRAMES_MAX=64, kUINT8_COUNT=256, kGC_HEAP_GROW=2, kGC_INITIAL_SIZE=1MB, kALIGNMENT=8
  - `Common.hh` — Copyable, UnCopyable, Singleton\<T\>, as_type\<T\>, as_down\<T\>, as_ptr\<T\>, as_string()
- **Verify**: main.cc includes all headers and compiles

### Task 1.3: Colorful Output
- **Files**: `src/Colorful.hh`, `src/Colorful.cc`, `src/ColorfulHelper.hh`, `src/ColorfulHelper_POSIX.hh`, `src/ColorfulHelper_WIN32.hh`
- **Depends**: 1.2
- **Work**: Color enum, fg::/bg:: stream manipulators, reset_colorful(); POSIX via ANSI escapes, WIN32 via SetConsoleTextAttribute; `ColorfulHelper.hh` dispatches by platform
- **Verify**: main.cc outputs colored text

### Task 1.4: Logger
- **Files**: `src/Logger.hh`, `src/Logger.cc`
- **Depends**: 1.3
- **Work**: LogLevel enum (TRACE/DEBUG/INFO/WARN/ERROR), Logger : Singleton, set_level()/level(), variadic format methods, color mapping (TRACE=gray, DEBUG=cyan, INFO=green, WARN=yellow, ERROR=red), output to stderr: `[LEVEL] [tag] message`
- **Verify**: All log levels output with correct colors

### Task 1.5: Opcode
- **Files**: `src/Opcode.hh`
- **Depends**: 1.2
- **Work**: `enum class OpCode : u8_t` with 40+ opcodes including OP_IMPORT/OP_IMPORT_FROM/OP_IMPORT_ALIAS; optional opcode_name()
- **Verify**: Compiles

### Task 1.6: Value Type
- **Files**: `src/Value.hh`, `src/Value.cc`
- **Depends**: 1.2
- **Work**: ValueStorage = std::variant\<monostate, bool, double, Object*\>; constructors (nil/bool/double/Object*); type checks (is_nil/is_boolean/is_number/is_object); extractors (as_boolean/as_number/as_object); semantics (is_truthy/is_equal/stringify); convenience checks (is_string/is_function/is_closure/is_class/is_instance)
- **Verify**: Create various Values, verify type checks and stringify

### Task 1.7: Chunk
- **Files**: `src/Chunk.hh`, `src/Chunk.cc`
- **Depends**: 1.5, 1.6
- **Work**: code\_ (vector\<u8_t\>), constants\_ (vector\<Value\>), lines\_ (vector\<int\>); write(byte/OpCode, line), add_constant() -> sz_t, code_at/constant_at/line_at/count, operator[] for patching
- **Verify**: Create chunk, write instructions/constants, read back

### Task 1.8: Debug Disassembler
- **Files**: `src/Debug.hh`, `src/Debug.cc`
- **Depends**: 1.7
- **Work**: disassemble_chunk(chunk, name), disassemble_instruction(chunk, offset) -> sz_t; supports all formats (simple/byte/constant/jump/invoke/closure); consecutive same-line shown as `|`
- **Verify**: Hand-built chunk disassembles correctly

### Task 1.9: main.cc Skeleton
- **Files**: `src/main.cc`
- **Depends**: 1.8
- **Work**: REPL mode (argc==1), file mode (argc==2), usage error (argc>2), exit codes 0/64/65/70/74; Phase 1 validation: hand-built chunk with OP_CONSTANT + OP_ADD + OP_RETURN
- **Verify**: Build and run, outputs disassembly and colored logs

---

## Phase 2: Frontend — Scanner & Compiler (Expressions)

### Task 2.1: Token System
- **Files**: `src/TokenTypes.hh`, `src/Token.hh`, `src/Token.cc`
- **Depends**: Phase 1
- **Work**: X-macro definitions (PUNCTUATOR/TOKEN/KEYWORD) with extensions KEYWORD(FROM,"from"), KEYWORD(AS,"as"); Token struct (type, start, length, line); TokenType enum from X-macro; token_type_name()
- **Verify**: Compiles, enum values correct

### Task 2.2: Scanner
- **Files**: `src/Scanner.hh`, `src/Scanner.cc`
- **Depends**: 2.1
- **Work**: scan_token() returns single Token on demand; single chars `(){},.+-;/*`, double chars `! != = == > >= < <=`, literals (string/number), identifiers + keyword recognition (trie-style), whitespace/comment (`//`) skip, error tokens
- **Verify**: Scan `print 1 + 2;`, correct token sequence

### Task 2.3: Compiler — Expressions
- **Files**: `src/Compiler.hh`, `src/Compiler.cc`
- **Depends**: 2.2, 1.7
- **Work**:
  - Parser state: current\_, previous\_, had_error\_, panic_mode\_
  - advance/consume/check/match, error handling, synchronize
  - Bytecode emission: emit_byte/bytes/op/return/constant, make_constant
  - Pratt parser: ParseRule table (prefix, infix, precedence), parse_precedence, expression()
  - Parse functions: number, string, grouping, unary (! -), binary (+ - * / == != < > <= >=), literal (true/false/nil)
  - compile(source) -> ObjFunction* (initially nullptr, test via Chunk)
  - print_statement, expression_statement, declaration -> statement main loop
- **Verify**: Compile `print 1 + 2 * 3;`, correct bytecode disassembly

---

## Phase 3: VM Core

### Task 3.1: Table (Hash Map)
- **Files**: `src/Table.hh`, `src/Table.cc`
- **Depends**: Phase 1
- **Work**: Entry (ObjString* key + Value); linear probing; get/set/remove/add_all/find_string (interning lookup); adjust_capacity + rehash; tombstone deletion; GC: mark_entries/remove_white
- **Verify**: Insert, lookup, delete, resize tests

### Task 3.2: Object Basics
- **Files**: `src/Object.hh`, `src/Object.cc`
- **Depends**: 1.6, 3.1
- **Work**: ObjectType enum (STRING/FUNCTION/NATIVE/CLOSURE/UPVALUE/CLASS/INSTANCE/BOUND_METHOD/MODULE); Object base (type, is_marked, next, virtual stringify); ObjString (value, hash); ObjFunction (arity, upvalue_count, chunk, name); as_obj\<T\>() helper; hash_string() FNV-1a; remaining types added in later phases
- **Verify**: Create ObjString/ObjFunction, stringify correct

### Task 3.3: VM Core — Arithmetic & Stack
- **Files**: `src/VM.hh`, `src/VM.cc`
- **Depends**: 3.2, 2.3
- **Work**:
  - VM : Singleton; stack (array + stack_top\_ + push/pop/peek/reset); globals\_ + strings\_ Tables; objects\_ linked list
  - interpret(source) -> compile then run; run() main dispatch loop (for + switch)
  - Opcodes: CONSTANT, NIL, TRUE, FALSE, ADD/SUB/MUL/DIV/NEGATE, NOT/EQUAL/GREATER/LESS, POP, PRINT, RETURN
  - READ_BYTE/SHORT/CONSTANT/STRING helpers; runtime_error()
  - String allocation + interning; OP_ADD: number add / string concat
  - Wire Compiler -> VM -> output
- **Verify**: `print 1 + 2;` -> 3, `print "hello" + " world";` -> hello world

### Task 3.4: Global Variables
- **Files**: `src/Compiler.cc`, `src/VM.cc`
- **Depends**: 3.3
- **Work**: Compiler: var_declaration (`var name = expr;` / `var name;`), identifier_constant, parse_variable/define_variable, named_variable (read/assign), variable() prefix handler. VM: OP_DEFINE_GLOBAL/GET_GLOBAL/SET_GLOBAL via globals\_ Table
- **Verify**: `var x = 10; print x;` -> 10

### Task 3.5: Local Variables & Scope
- **Files**: `src/Compiler.cc`
- **Depends**: 3.4
- **Work**: Local struct (name, depth, is_captured); locals\_ array + local_count\_ + scope_depth\_; begin_scope/end_scope; block() `{ decls }`; add_local/declare_variable/mark_initialized/resolve_local. VM: OP_GET_LOCAL/SET_LOCAL via frame->slots[slot]
- **Verify**: `{ var a = 1; var b = 2; print a + b; }` -> 3; shadowing test

### Task 3.6: Control Flow
- **Files**: `src/Compiler.cc`, `src/VM.cc`
- **Depends**: 3.5
- **Work**: Compiler: if_statement (OP_JUMP_IF_FALSE + patch), while_statement (OP_LOOP), for_statement (desugar to while), and\_/or\_ (short-circuit), emit_jump/patch_jump/emit_loop. VM: OP_JUMP/JUMP_IF_FALSE/LOOP
- **Verify**: `tests/control_flow.ms` passes

### Task 3.7: Phase 3 Tests
- **Files**: `tests/arithmetic.ms`, `tests/variables.ms`, `tests/strings.ms`, `tests/control_flow.ms`
- **Depends**: 3.6
- **Work**: Test scripts with `// expect:` comments
- **Verify**: All pass manually

---

## Phase 4: Functions & Closures

### Task 4.1: Native Functions
- **Files**: `src/Object.hh/cc`, `src/VM.cc`
- **Depends**: Phase 3
- **Work**: ObjNative (NativeFn = Value(*)(int, Value*), name); VM::define_native(); clock() native; VM::call_value() handles ObjNative
- **Verify**: `print clock();` outputs timestamp

### Task 4.2: Function Compilation & Calls
- **Files**: `src/Compiler.cc`, `src/VM.cc`, `src/Object.hh/cc`
- **Depends**: 4.1
- **Work**:
  - Compiler: FunctionType enum (FUNCTION/SCRIPT/INITIALIZER/METHOD); fun_declaration, function(type) with nested Compiler, argument_list, call() infix, return_statement
  - VM: CallFrame (closure, ip, slots); frames\_ + frame_count\_; call/call_value; OP_CALL/RETURN
  - ObjClosure: function + upvalues (empty this phase); compile() returns ObjFunction*, VM wraps as ObjClosure
- **Verify**: `fun add(a, b) { return a + b; } print add(1, 2);` -> 3

### Task 4.3: Closures & Upvalues
- **Files**: `src/Compiler.cc`, `src/VM.cc`, `src/Object.hh/cc`
- **Depends**: 4.2
- **Work**:
  - Compiler: Upvalue struct (index, is_local); resolve_upvalue (recursive), add_upvalue; OP_CLOSURE emission with N*(isLocal+index) metadata; mark Local::is_captured; OP_CLOSE_UPVALUE in end_scope for captured locals
  - VM: ObjUpvalue (location, closed, next); open_upvalues\_ list (descending stack order); capture_upvalue/close_upvalues; OP_CLOSURE/GET_UPVALUE/SET_UPVALUE/CLOSE_UPVALUE
- **Verify**: `tests/closures.ms` passes (makeCounter etc.)

### Task 4.4: Phase 4 Tests
- **Files**: `tests/functions.ms`, `tests/closures.ms`
- **Depends**: 4.3
- **Work**: Recursion, native functions, closure capture, upvalue sharing tests
- **Verify**: All pass

---

## Phase 5: OOP — Classes & Inheritance

### Task 5.1: Classes & Instances
- **Files**: `src/Compiler.cc`, `src/VM.cc`, `src/Object.hh/cc`
- **Depends**: Phase 4
- **Work**: ObjClass (name, methods map); ObjInstance (klass, fields map). Compiler: class_declaration, ClassCompiler (enclosing, has_superclass), OP_CLASS. VM: OP_CLASS creates ObjClass; call_value handles ObjClass -> ObjInstance; OP_GET/SET_PROPERTY; lookup: fields then methods
- **Verify**: `class Foo {} var f = Foo(); f.x = 42; print f.x;` -> 42

### Task 5.2: Methods & this
- **Files**: `src/Compiler.cc`, `src/VM.cc`, `src/Object.hh/cc`
- **Depends**: 5.1
- **Work**: ObjBoundMethod (receiver, method). Compiler: method(), this\_() as local slot 0, TYPE_INITIALIZER for init. VM: OP_METHOD adds to class.methods; bind_method creates ObjBoundMethod; OP_INVOKE (optimized, skip bind); init auto-called, returns this
- **Verify**: `class Cat { init(n) { this.name = n; } speak() { print this.name; } } Cat("Tom").speak();` -> Tom

### Task 5.3: Inheritance & super
- **Files**: `src/Compiler.cc`, `src/VM.cc`
- **Depends**: 5.2
- **Work**: Syntax: `class Sub : Super { ... }`; super\_() expression; OP_INHERIT (copy parent methods), OP_GET_SUPER (find+bind from parent), OP_SUPER_INVOKE (optimized)
- **Verify**: `tests/classes.ms` passes (inheritance + super tests)

### Task 5.4: Phase 5 Tests
- **Files**: `tests/classes.ms`
- **Depends**: 5.3
- **Work**: Classes, instances, methods, this, inheritance, super, method override tests
- **Verify**: All pass

---

## Phase 6: Garbage Collection

### Task 6.1: GC Infrastructure
- **Files**: `src/Memory.hh`, `src/Memory.cc`, `src/VM.hh/cc`
- **Depends**: Phase 5
- **Work**: VM GC state (bytes_allocated\_, next_gc\_, gray_stack\_); allocate_object\<T\>() tracks bytes + checks trigger; collect_garbage() entry; Logger output (MAPLE_DEBUG_LOG_GC)
- **Verify**: GC framework ready, no actual collection yet

### Task 6.2: Mark Phase
- **Files**: `src/Memory.cc`
- **Depends**: 6.1
- **Work**: mark_roots() traverses all roots (stack, globals, frame closures, open_upvalues, compiler function\_, init_string\_); mark_object (set is_marked, push to gray_stack); mark_value; mark_table
- **Verify**: Mark phase correctly marks reachable objects

### Task 6.3: Trace & Sweep
- **Files**: `src/Memory.cc`
- **Depends**: 6.2
- **Work**:
  - trace_references() processes gray_stack; blacken_object by type (Closure->function+upvalues, Class->name+methods, Instance->klass+fields, BoundMethod->receiver+method, Upvalue->closed, Function->name+constants, Module->name+exports)
  - sweep() walks objects\_ list: unmarked -> remove+delete, marked -> reset is_marked
  - table remove_white(); update next_gc\_ = bytes_allocated\_ * kGC_HEAP_GROW
- **Verify**: All Phase 1-5 tests pass with GC enabled

### Task 6.4: GC Stress Test
- **Files**: (build option)
- **Depends**: 6.3
- **Work**: MAPLE_DEBUG_STRESS_GC triggers GC on every allocation; run all tests; ensure compiler GC safety (push objects to stack before allocation)
- **Verify**: All tests pass under stress GC

---

## Phase 7: Import System

### Task 7.1: Module Loader
- **Files**: `src/Module.hh`, `src/Module.cc`, `src/Object.hh/cc`
- **Depends**: Phase 6
- **Work**: ObjModule (name, exports map); ModuleLoader: resolve_path (relative, std::filesystem), read_source (ifstream); VM::modules\_ cache; circular import detection (loading set)
- **Verify**: Can read module files

### Task 7.2: import Statement
- **Files**: `src/Compiler.cc`, `src/VM.cc`
- **Depends**: 7.1
- **Work**: Compiler: import_statement `import "path";` -> OP_CONSTANT(path) + OP_IMPORT. VM: OP_IMPORT loads (or cache-hits), compiles+executes module, collects top-level decls as exports, creates ObjModule, defines global by module name. Member access via OP_GET_PROPERTY on ObjModule
- **Verify**: `import "math.ms"; print math.add(1, 2);` -> 3

### Task 7.3: from-import-as
- **Files**: `src/Compiler.cc`, `src/VM.cc`
- **Depends**: 7.2
- **Work**: Compiler: from_import_statement parses `from "path" import name;` and `from "path" import name as alias;`; OP_IMPORT_FROM/OP_IMPORT_ALIAS. VM handles both opcodes
- **Verify**: `from "math.ms" import add as plus; print plus(1, 2);` -> 3

### Task 7.4: Phase 7 Tests
- **Files**: `tests/import_test.ms`, `tests/modules/math.ms`, `tests/modules/utils.ms`
- **Depends**: 7.3
- **Work**: math.ms (add etc.), utils.ms (greet etc.), import_test.ms tests all import forms
- **Verify**: All pass

---

## Phase 8: Polish

### Task 8.1: REPL
- **Files**: `src/main.cc`, `src/VM.cc`
- **Depends**: Phase 7
- **Work**: `maple> ` prompt, line-by-line compile+execute, error recovery (no exit), skip blank lines
- **Verify**: Interactive REPL test

### Task 8.2: Error Messages
- **Files**: `src/Compiler.cc`, `src/VM.cc`
- **Depends**: 8.1
- **Work**: Compile errors: `[line N] Error at 'token': message`; runtime errors: message + call stack trace; common errors: undefined var, type mismatch, arity, div-by-zero, missing property
- **Verify**: `tests/errors.ms` passes

### Task 8.3: Test Runner
- **Files**: `tests/run_tests.py`
- **Depends**: Phase 7
- **Work**: Parse `// expect:` and `// expect runtime error:` comments; run mslang binary; compare actual vs expected output; report pass/fail/total; support single test or directory
- **Verify**: Runs all tests and reports

### Task 8.4: Example Scripts
- **Files**: `examples/fibonacci.ms`, `examples/counter.ms`, `examples/oop.ms`
- **Depends**: Phase 7
- **Work**: fibonacci (recursive + iterative), counter (closure), oop (inheritance + polymorphism)
- **Verify**: All examples run

### Task 8.5: Documentation
- **Files**: `docs/REQUIREMENTS.md`, `docs/DESIGN.md`
- **Depends**: Phase 7
- **Work**: Update docs to match implementation, add usage instructions

---

## Phase 9: Improvements — Bugs, Quality & Features

### 9.1 Bug Fixes

#### Task 9.1.1: GC `remove_white()` Timing [Critical]
- **Files**: `src/VM.cc`
- **Problem**: Non-stress path calls `strings_.remove_white()` before `collect_garbage()`, when all `is_marked_` are false — removes all interned strings.
- **Fix**: Move `remove_white()` into `Memory.cc::collect_garbage()` after `trace_references()`, before `sweep()`.
- **Verify**: All tests pass with MAPLE_DEBUG_STRESS_GC.

#### Task 9.1.2: Stack Overflow Check in `push()`
- **Files**: `src/VM.cc`
- **Problem**: `push()` writes `*stack_top_++` without bounds check.
- **Fix**: Add `stack_top_ < stack_.data() + stack_.size()` assert or runtime check.
- **Verify**: Deep recursion script errors instead of crashing.

#### Task 9.1.3: `import_module` Bypasses `ModuleLoader`
- **Files**: `src/VM.cc`, `src/Module.hh/cc`
- **Problem**: Uses raw ifstream instead of ModuleLoader — no relative path resolution.
- **Fix**: Refactor to use ModuleLoader with current file path for resolution.
- **Verify**: Cross-directory import test (e.g. `tests/nested/`).

#### Task 9.1.4: `stringify()` Integer Overflow
- **Files**: `src/Value.cc`
- **Problem**: Casts integer double to `i32_t`, overflows beyond INT32_MAX.
- **Fix**: Use `i64_t` or `long long` with range check.
- **Verify**: `print 2147483648;` -> 2147483648 (not negative).

### 9.2 Code Quality

#### Task 9.2.1: Eliminate Compiler Static State
- **Files**: `src/Compiler.hh`, `src/Compiler.cc`
- **Problem**: scanner\_, current\_, previous\_, had_error\_, panic_mode\_, current_class\_, current_compiler\_ are all static — non-reentrant, not thread-safe.
- **Fix**: Extract to `ParseState` struct shared via pointer across nested Compilers.
- **Verify**: All tests pass.

#### Task 9.2.2: Decouple GC — Remove Global `vm_*` Functions
- **Files**: `src/VM.hh/cc`, `src/Memory.hh/cc`
- **Problem**: Memory.cc accesses VM GC state via 6 global `vm_*()` free functions.
- **Fix**: Introduce `GCContext` struct or move GC logic into VM methods.
- **Verify**: All tests pass.

#### Task 9.2.3: Object Member Encapsulation
- **Files**: `src/Object.hh`
- **Problem**: All Object subclass members are public.
- **Fix**: Make private/protected with accessors; use friend for hot paths.
- **Verify**: Compiles.

#### Task 9.2.4: Eliminate `const_cast` in `Table::get()`
- **Files**: `src/Table.hh/cc`
- **Problem**: const `get()` uses `const_cast` to call non-const `find_entry()`.
- **Fix**: Split find_entry into const/non-const, or mark const since it doesn't mutate.
- **Verify**: Compiles.

#### Task 9.2.5: Unify Storage — Table for All ObjString*->Value Maps
- **Files**: `src/Object.hh/cc`, `src/VM.cc`
- **Problem**: globals\_/strings\_ use custom Table, but methods\_/fields\_/exports\_ use std::unordered_map — inconsistent GC interaction.
- **Fix**: Unify on Table, or provide unified GC mark utility for unordered_map.
- **Verify**: All tests + GC stress pass.

#### Task 9.2.6: SafeGuard ParseRule Table
- **Files**: `src/Compiler.cc`
- **Problem**: C-style rules[] indexed by TokenType enum — silently breaks if enum order changes.
- **Fix**: Use std::array with static_assert on size, or designated initializers with explicit enum indices.
- **Verify**: Compiles.

#### Task 9.2.7: Zero-Allocation Number Parsing
- **Files**: `src/Compiler.cc`
- **Problem**: `number()` constructs temporary std::string from strv_t for strtod.
- **Fix**: Use std::from_chars() (C++17) directly on strv_t pointer.
- **Verify**: Number parsing tests pass.

### 9.3 Language Features

#### Task 9.3.1: `break` / `continue`
- **Files**: `src/TokenTypes.hh`, `src/Scanner.cc`, `src/Compiler.cc`, `src/Opcode.hh`, `src/VM.cc`
- **Work**: New TOKEN_BREAK/TOKEN_CONTINUE keywords; Compiler tracks loop context (loop_start, scope_depth, break patch list); break -> OP_JUMP past loop, continue -> OP_LOOP to increment/condition; patch all break jumps at loop end
- **Verify**: `tests/break_continue.ms` passes.

#### Task 9.3.2: Modulo Operator `%`
- **Files**: `src/TokenTypes.hh`, `src/Scanner.cc`, `src/Opcode.hh`, `src/Compiler.cc`, `src/VM.cc`
- **Work**: TOKEN_PERCENT, OP_MODULO at PREC_FACTOR; VM uses std::fmod(a, b)
- **Verify**: `print 10 % 3;` -> 1.

#### Task 9.3.3: String Escape Sequences
- **Files**: `src/Scanner.cc` or `src/Compiler.cc`
- **Work**: Handle `\n`, `\t`, `\\`, `\"`, `\r`, `\0` in scan_string() or string() compilation
- **Verify**: `print "hello\tworld\n";` outputs with tab and newline.

#### Task 9.3.4: OP_CONSTANT_LONG
- **Files**: `src/Opcode.hh`, `src/Chunk.hh/cc`, `src/Compiler.cc`, `src/VM.cc`, `src/Debug.cc`
- **Problem**: make_constant() limits to u8_t (256 constants max).
- **Work**: OP_CONSTANT_LONG with 3-byte operand (24-bit, up to 16M); auto-switch when index > 255; disassembler support
- **Verify**: Script with 256+ constants.

#### Task 9.3.5: Line Number RLE Compression
- **Files**: `src/Chunk.hh/cc`
- **Problem**: lines\_ stores one int per bytecode — wasteful.
- **Work**: RLE encoding via vector\<pair\<int,int\>\> or vector\<LineStart\>; line_at() via binary search
- **Verify**: All tests pass, reduced memory.

#### Task 9.3.6: More Native Functions
- **Files**: `src/VM.cc`
- **Work**: type(value)->"type string", strlen(str), str(value), num(str), input([prompt])
- **Verify**: Each native function tested.

#### Task 9.3.7: List (Array) Type
- **Files**: new ObjList, modify Scanner/Compiler/VM
- **Work**: ObjList with vector\<Value\> (GC traces elements); syntax: `[1,2,3]`, `a[0]`, `a[1]=42`, `a.push(5)`, `a.len()`; TOKEN_LEFT/RIGHT_BRACKET; OP_BUILD_LIST/INDEX_GET/INDEX_SET
- **Verify**: `tests/lists.ms` passes.

#### Task 9.3.8: Map (Dictionary) Type
- **Files**: new ObjMap, modify Compiler/VM
- **Work**: ObjMap with unordered_map\<Value,Value\> or Table-based; syntax: `{"a":1}`, `m["a"]`, `m["c"]=3`; reuse OP_INDEX_GET/SET
- **Verify**: `tests/maps.ms` passes.

### 9.4 Performance (Optional)

#### Task 9.4.1: NaN-Boxing
- **Files**: `src/Value.hh/cc`
- **Problem**: std::variant is 16 bytes with branch overhead.
- **Work**: Encode nil/bool/Object* in IEEE 754 quiet NaN space; Value shrinks to 8 bytes; type checks become bit ops; API unchanged
- **Verify**: All tests pass, fibonacci benchmark improves.

#### Task 9.4.2: Computed Goto Dispatch (GCC/Clang)
- **Files**: `src/VM.cc`
- **Problem**: switch dispatch has indirect jump overhead.
- **Work**: GCC/Clang `&&label` threaded dispatch; `#ifdef MAPLE_GNUC` conditional, MSVC falls back to switch
- **Verify**: Performance benchmark comparison.

---

## Progress Tracking

| Phase | Task | Status |
|-------|------|--------|
| 1 | 1.1 CMake | [x] |
| 1 | 1.2 Platform headers | [x] |
| 1 | 1.3 Colorful output | [x] |
| 1 | 1.4 Logger | [x] |
| 1 | 1.5 Opcode | [x] |
| 1 | 1.6 Value | [x] |
| 1 | 1.7 Chunk | [x] |
| 1 | 1.8 Debug disassembler | [x] |
| 1 | 1.9 main.cc skeleton | [x] |
| 2 | 2.1 Token system | [x] |
| 2 | 2.2 Scanner | [x] |
| 2 | 2.3 Compiler (expressions) | [x] |
| 3 | 3.1 Table | [x] |
| 3 | 3.2 Object basics | [x] |
| 3 | 3.3 VM core | [x] |
| 3 | 3.4 Global variables | [x] |
| 3 | 3.5 Local variables & scope | [x] |
| 3 | 3.6 Control flow | [x] |
| 3 | 3.7 Phase 3 tests | [x] |
| 4 | 4.1 Native functions | [x] |
| 4 | 4.2 Function compilation | [x] |
| 4 | 4.3 Closures & upvalues | [x] |
| 4 | 4.4 Phase 4 tests | [x] |
| 5 | 5.1 Classes & instances | [x] |
| 5 | 5.2 Methods & this | [x] |
| 5 | 5.3 Inheritance & super | [x] |
| 5 | 5.4 Phase 5 tests | [x] |
| 6 | 6.1 GC infrastructure | [x] |
| 6 | 6.2 Mark phase | [x] |
| 6 | 6.3 Trace & sweep | [x] |
| 6 | 6.4 GC stress test | [x] |
| 7 | 7.1 Module loader | [x] |
| 7 | 7.2 import statement | [x] |
| 7 | 7.3 from-import-as | [x] |
| 7 | 7.4 Phase 7 tests | [x] |
| 8 | 8.1 REPL | [x] |
| 8 | 8.2 Error messages | [x] |
| 8 | 8.3 Test runner | [x] |
| 8 | 8.4 Example scripts | [x] |
| 8 | 8.5 Documentation | [x] |
| 9 | 9.1.1 GC remove_white timing | [x] |
| 9 | 9.1.2 push overflow check | [x] |
| 9 | 9.1.3 import use ModuleLoader | [x] |
| 9 | 9.1.4 stringify overflow | [x] |
| 9 | 9.2.1 Compiler static state | [x] |
| 9 | 9.2.2 GC decoupling | [x] |
| 9 | 9.2.3 Object encapsulation | [x] |
| 9 | 9.2.4 Table const_cast | [x] |
| 9 | 9.2.5 Unified Table storage | [x] |
| 9 | 9.2.6 ParseRule safety | [x] |
| 9 | 9.2.7 Zero-alloc number parse | [x] |
| 9 | 9.3.1 break/continue | [x] |
| 9 | 9.3.2 Modulo % | [x] |
| 9 | 9.3.3 String escapes | [x] |
| 9 | 9.3.4 OP_CONSTANT_LONG | [x] |
| 9 | 9.3.5 Line RLE compression | [x] |
| 9 | 9.3.6 More natives | [x] |
| 9 | 9.3.7 List type | [x] |
| 9 | 9.3.8 Map type | [x] |
| 9 | 9.4.1 NaN-boxing | [x] |
| 9 | 9.4.2 Computed goto | [x] |
