# Maple Language — Next-Wave Optimization & Improvement

> Version: 1.0 | Date: 2026-03-22

All phases in `improve.md` (10–17) and all optimizations in `optimize.md` (O1–O4) are complete. This document defines the next wave of improvements from a programming language design expert perspective, drawing lessons from V8, LuaJIT, CPython 3.11, and Lua 5.4.

Each task is independently implementable and verifiable. Dependencies are explicitly noted.

---

## O5: Benchmark Harness

**Motivation:** Without measurement, no optimization can be validated or compared. Every serious language runtime (LuaJIT, V8, PyPy) gates performance commits on a stable benchmark suite.

**Status quo:** There is no dedicated benchmark infrastructure. Performance is evaluated informally.

### O5.1 Benchmark Scripts

Create `benchmarks/` directory with the following `.ms` scripts. Each covers a different bottleneck:

| File | What It Measures |
|------|-----------------|
| `fib_recursive.ms` | Function call overhead, stack frame allocation |
| `fib_iterative.ms` | Loop / integer arithmetic hot path |
| `binary_trees.ms` | GC allocation pressure, generational promotion |
| `field_access.ms` | Shape / IC hit rate on 1M property reads |
| `method_dispatch.ms` | IC polymorphism — single class vs. multiple classes |
| `string_concat.ms` | StringBuilder, string interning, GC |
| `list_ops.ms` | List allocation, `for-in`, `range()` |

Example `benchmarks/fib_recursive.ms`:
```maple
fun fib(n) {
  if (n < 2) return n
  return fib(n - 1) + fib(n - 2)
}
print fib(32)   // expect: 2178309
```

Example `benchmarks/field_access.ms`:
```maple
class Point { init(x, y) { this.x = x; this.y = y; } }
var p = Point(0, 0)
var i = 0
while (i < 1000000) {
  p.x = i
  var _ = p.x
  i += 1
}
print p.x   // expect: 999999
```

### O5.2 `--benchmark` Flag in main.cc

**Change:** Add `--benchmark N` CLI flag. When set:
- Suppress all `print` output (redirect to `/dev/null`)
- Run the script N times (default 5)
- Report: min/median/max wall time in µs, plus GC collection count

```cpp
// main.cc — add to arg parsing
if (arg == "--benchmark") {
  benchmark_runs = std::stoi(next_arg);  // default 5
}
// After each run:
auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
times.push_back(elapsed.count());
```

Expose GC stats via `Memory::get_instance().gc_count()` (add this accessor if missing).

### O5.3 Runner Script

Create `benchmarks/run_all.sh`:
```bash
#!/usr/bin/env bash
EXE=./build/mslang
for f in benchmarks/*.ms; do
  echo -n "$(basename $f): "
  $EXE --benchmark 5 "$f"
done
```

**Files:** `main.cc`, new `benchmarks/*.ms`, new `benchmarks/run_all.sh`
**Depends on:** nothing
**Test:**
1. `cmake --build build`
2. `./build/mslang --benchmark 5 benchmarks/fib_recursive.ms` → prints timing
3. `bash benchmarks/run_all.sh` → table of all benchmarks
4. All existing `ctest` tests still pass

---

## O6: Bytecode Quickening (Runtime Specialization)

**Motivation:** Currently every `OP_ADD` walks a 4-branch type cascade: string concat, integer, double, operator overload. CPython 3.11's "specializing adaptive interpreter" proves this is the single highest-ROI optimization — 20-40% on numeric-heavy workloads.

**Status quo:** `OP_ADD` handler in `VM.cc` always checks all type variants. No runtime type feedback.

### O6.1 Specialized Opcodes

Add the following opcodes to `Opcode.hh` in a new `// --- Quickened (runtime-specialized) ---` block:

```cpp
// --- Quickened arithmetic (set by runtime; de-quicken before serialize) ---
OP_ADD_II,    // iABC  R(A) := R(B) + R(C)  [both integers]
OP_ADD_FF,    // iABC  R(A) := R(B) + R(C)  [both doubles]
OP_ADD_SS,    // iABC  R(A) := R(B) + R(C)  [string concat]
OP_SUB_II,    // iABC  R(A) := R(B) - R(C)  [integers]
OP_SUB_FF,    // iABC  R(A) := R(B) - R(C)  [doubles]
OP_MUL_II,    // iABC  R(A) := R(B) * R(C)  [integers]
OP_MUL_FF,    // iABC  R(A) := R(B) * R(C)  [doubles]
OP_DIV_FF,    // iABC  R(A) := R(B) / R(C)  [doubles; int/int may produce float]
OP_LT_II,     // iABC  R(A) := R(B) < R(C)  [integers]
OP_LT_FF,     // iABC  R(A) := R(B) < R(C)  [doubles]
OP_EQ_II,     // iABC  R(A) := R(B) == R(C) [integers]
```

Also add `opcode_name()` cases for all new opcodes in the `switch` at the bottom of `Opcode.hh`.

### O6.2 Quickening in VM.cc

Use a **two-hit policy**: rewrite on the second execution, to avoid cold-start cost:

```cpp
// In the OP_ADD handler, after resolving lhs/rhs from RK:
if (lhs.is_integer() && rhs.is_integer()) {
  // Fast path already taken: rewrite instruction in-place
  auto* instr_ptr = const_cast<Instruction*>(ip - 1);
  *instr_ptr = encode_ABC(OpCode::OP_ADD_II, A, B, C);
  regs[A] = Value(lhs.as_integer() + rhs.as_integer());
} else if (lhs.is_double() && rhs.is_double()) {
  auto* instr_ptr = const_cast<Instruction*>(ip - 1);
  *instr_ptr = encode_ABC(OpCode::OP_ADD_FF, A, B, C);
  regs[A] = Value(lhs.as_double() + rhs.as_double());
} else { /* string / operator overload / etc. */ }
```

Each specialized handler (`OP_ADD_II`, etc.) has a cheap type guard:

```cpp
VM_CASE(OP_ADD_II): {
  auto lhs = rk_value(B, regs, constants);
  auto rhs = rk_value(C, regs, constants);
  if (__builtin_expect(lhs.is_integer() && rhs.is_integer(), 1)) {
    regs[A] = Value(lhs.as_integer() + rhs.as_integer());
    VM_NEXT();
  }
  // Deopt: revert to generic
  ip[-1] = encode_ABC(OpCode::OP_ADD, A, B, C);
  goto op_OP_ADD;  // re-execute as generic
}
```

**De-quicken before serialize:** In `Serializer.cc`, map any `OP_ADD_II`, `OP_ADD_FF`, … back to `OP_ADD` etc. before writing `.msc`.

**Files:** `Opcode.hh`, `VM.cc`, `Debug.cc` (disassembler), `Serializer.cc`
**Depends on:** O5 (measure impact)
**Test:**
1. Create `tests/quickening.ms`:
   ```maple
   var s = 0
   var i = 0
   while (i < 1000) { s = s + i; i = i + 1 }
   print s   // expect: 499500
   ```
2. `ctest --test-dir build --output-on-failure` — all tests pass
3. `./build/mslang --benchmark 5 benchmarks/fib_iterative.ms` — compare time vs. pre-O6 baseline
4. With `MAPLE_DEBUG_TRACE=ON`: verify `ADD_II` appears in trace after second-pass execution
5. Round-trip test: compile → serialize → deserialize → run → same output (quickened opcodes de-quickened in `.msc`)

---

## O7: Polymorphic Inline Caches (PIC)

**Motivation:** The current `InlineCache` struct (in `Object.hh`) holds exactly **one** class entry. Any polymorphic callsite (e.g., iterating a list of `Dog`/`Cat` objects) thrashes the cache to 0% hit rate. Real OOP code is frequently dimorphic or trimorphic. V8, SpiderMonkey, and Ruby YJIT all use 4-entry PICs as the sweet spot.

**Status quo:** `InlineCache` = `{ ObjClass*, ICKind, Value, u32_t shape_id, u32_t slot_index }` — 1 entry.

### O7.1 Expand InlineCache to 4 Entries

In `Object.hh`, replace `InlineCache` with:

```cpp
inline constexpr int kIC_PIC_SIZE = 4;

struct ICEntry {
  ObjClass* klass{nullptr};
  ICKind kind{ICKind::IC_NONE};
  Value cached{};
  u32_t shape_id{0};
  u32_t slot_index{0};
};

struct InlineCache {
  std::array<ICEntry, kIC_PIC_SIZE> entries{};
  u8_t count{0};          // number of valid entries (0..kIC_PIC_SIZE)
  bool megamorphic{false}; // true → skip IC entirely, use slow path
};
```

Accessors and helpers:
```cpp
// Find entry matching klass; returns pointer or nullptr
inline ICEntry* find_entry(ObjClass* klass) noexcept {
  for (u8_t i = 0; i < count; ++i)
    if (entries[i].klass == klass) return &entries[i];
  return nullptr;
}

// Append entry; returns false if full → caller sets megamorphic
inline bool append_entry(const ICEntry& e) noexcept {
  if (count == kIC_PIC_SIZE) return false;
  entries[count++] = e;
  return true;
}
```

### O7.2 Update VM Fast Paths

In `VM.cc`, the `OP_GETPROP` fast path becomes:

```cpp
// Fast path: PIC lookup
if (!ic.megamorphic) {
  if (auto* e = ic.find_entry(klass)) {
    if (e->kind == ICKind::IC_FIELD && inst->shape()->id() == e->shape_id) {
      regs[A] = inst->field_at(e->slot_index);
      VM_NEXT();
    }
    // other IC kinds (method, getter)…
  }
  // Miss: do full lookup, then append to PIC
  // … full lookup …
  ICEntry new_entry{klass, kind, cached_val, shape_id, slot};
  if (!ic.append_entry(new_entry))
    ic.megamorphic = true;
  VM_NEXT();
}
// Megamorphic: slow path (Table lookup)
```

Apply the same pattern to `OP_SETPROP` and `OP_INVOKE`.

### O7.3 GC Tracing

In `ObjFunction::trace_references()` (`Object.cc`), update the IC tracing loop:

```cpp
for (auto& ic : ic_) {
  for (u8_t i = 0; i < ic.count; ++i) {
    if (ic.entries[i].klass)
      Memory::get_instance().mark_object(ic.entries[i].klass);
    Memory::get_instance().mark_value(ic.entries[i].cached);
  }
}
```

**Files:** `Object.hh`, `VM.cc`, `Object.cc`, `Serializer.cc` (serialize IC as empty — runtime-only)
**Depends on:** O5 (measure dimorphic vs. monomorphic IC hit rates)
**Test:**
1. Create `tests/pic.ms`:
   ```maple
   class A { get_x() { return 1 } }
   class B { get_x() { return 2 } }
   var objs = [A(), B(), A(), B()]
   var sum = 0
   for (var o in objs) { sum = sum + o.get_x() }
   print sum   // expect: 6
   ```
2. `ctest --test-dir build --output-on-failure` — all tests pass
3. Benchmark `benchmarks/method_dispatch.ms` before/after — measure improvement at polymorphic sites

---

## O8: Peephole Optimization Pass

**Motivation:** The single-pass compiler necessarily produces suboptimal sequences because it cannot look ahead. A cheap O(n) peephole pass after compilation catches the most common waste without requiring an AST or SSA IR. This is what Lua 5.4, Erlang BEAM, and CPython all do.

**Status quo:** Compiler has `try_fold_unary()` / `try_fold_binary()` for compile-time constant folding via `chunk.truncate()`. No post-pass optimization.

### O8.1 Jump-Target Bitmap

Before any rewriting, build a `std::vector<bool> is_target(code.size(), false)`. Scan all branch instructions and mark their destination offsets:

| Opcode | Target offset |
|--------|--------------|
| `OP_JMP` | `pc + 1 + decode_sBx(instr)` |
| `OP_TEST`, `OP_TESTSET` | `pc + 2` (skip next instr) |
| `OP_TRY` | `pc + 1 + decode_sBx(instr)` |
| `OP_FORITER` | `pc + 1 + decode_sBx(instr)` |

Jump targets mark basic block boundaries — the optimizer must not cross them.

### O8.2 Optimization Patterns

Add a new file `src/Optimize.hh` / `src/Optimize.cc`:

```cpp
namespace ms {
  // Run peephole pass on the given function (and all nested closures).
  // Called once after compile() returns, before execution or serialization.
  void peephole_optimize(ObjFunction* fn) noexcept;
}
```

Patterns (applied in a single linear pass with a sliding window of 2–3 instructions):

**P1 — Redundant MOVE:**
```
MOVE  R(A), R(B)   where A == B  → delete
```

**P2 — LOADNIL coalescing:**
```
LOADNIL  R(A), 0
LOADNIL  R(A+1), 0    → LOADNIL R(A), 1
```
(merge consecutive single-register LOADNIL into one ranged LOADNIL)

**P3 — Dead code after unconditional exit:**
```
RETURN R(A) [or OP_THROW]
<any instruction that is NOT a jump target>  → replace with OP_NOP
```
Add `OP_NOP` to `Opcode.hh`:
```cpp
OP_NOP,  // no-operation (padding after dead code elimination)
```

**P4 — LOADK + NEG folding:**
```
LOADK  R(A), K(i)    [K(i) is numeric]
NEG    R(A), R(A)    → LOADK R(A), K(-i)  [add negated constant to pool]
```
(This extends the existing compile-time folding for cases the compiler missed)

**P5 — MOVE + RETURN tail merge:**
```
MOVE    R(A), R(B)
RETURN  R(A)          → RETURN R(B)
```

### O8.3 Recursive Application

After optimizing the top-level function, recurse into all constants that are `ObjFunction*` (closures defined inline). This handles nested functions and lambdas.

### O8.4 Integration Point

In `Compiler.cc`, the `compile()` function (or wherever `ObjFunction` is finalized):
```cpp
ObjFunction* fn = compiler.end_compiler();
ms::peephole_optimize(fn);   // ← add this call
return fn;
```

**Files:** New `src/Optimize.hh`, new `src/Optimize.cc`, `CMakeLists.txt` (add new files), `Opcode.hh` (OP_NOP), `Compiler.cc` (call site), `Debug.cc` (OP_NOP name)
**Depends on:** O6 (run peephole on generic opcodes; quickened opcodes are set at runtime, after peephole)
**Test:**
1. Create `tests/peephole.ms`:
   ```maple
   // Dead code after return — should not crash
   fun f(x) {
     if (x > 0) return x
     return -x
     print "unreachable"   // dead
   }
   print f(3)    // expect: 3
   print f(-5)   // expect: 5
   ```
2. `ctest --test-dir build --output-on-failure` — all tests pass
3. With `MAPLE_DEBUG_PRINT=ON`, verify disassembly of `f` contains `OP_NOP` (or absent instructions) after the second `RETURN`

---

## O9: Enums

**Motivation:** Enums are the most commonly missing type in any Lox-derived language. They replace magic numbers/strings, improve switch-case readability, and have near-zero implementation cost when desugared to static class members.

**Status quo:** No `enum` keyword. Users manually define constants.

### O9.1 Syntax

```maple
enum Direction { NORTH, EAST, SOUTH, WEST }
print Direction.NORTH    // expect: 0
print Direction.WEST     // expect: 3

enum Status { PENDING = 1, ACTIVE = 2, DONE = 10 }
print Status.ACTIVE      // expect: 2
print Status.DONE        // expect: 10
```

### O9.2 Compiler Desugaring

`enum Name { V1, V2 = expr, V3 }` desugars at compile time (no new opcodes, no new object types):

1. Emit `OP_CLASS R(tmp), K("Name")`
2. For each variant in order:
   - Evaluate value (auto-increment integer, or explicit expression)
   - Emit `OP_LOADK R(val), K(n)` + `OP_STATICMETH R(tmp), K("VARIANT"), R(val)`
3. Emit `OP_DEFGLOBAL K("Name"), R(tmp)` (or local if inside a scope)

This reuses `OP_STATICMETH` exactly as static method definitions do. The enum value becomes a static field on the class.

### O9.3 Scanner Changes

Add `TOKEN_ENUM` keyword to `TokenTypes.hh` (or reuse existing `TOKEN_STATIC` handling pattern). Add `"enum"` → `TOKEN_ENUM` to the keyword map in `Scanner.cc`.

### O9.4 Compiler Changes

In `Compiler.cc`, add `enum_declaration()` method:
- Parse `enum Name { ... }`
- Track a running integer counter (reset to 0 at start)
- Each `VARIANT` without `=`: use counter, then increment
- Each `VARIANT = expr`: evaluate expr at compile time (constant only) or emit runtime assignment

**Files:** `TokenTypes.hh`, `Scanner.cc`, `Compiler.cc`
**Depends on:** nothing
**Test:**
1. Create `tests/enums.ms`:
   ```maple
   enum Color { RED, GREEN, BLUE }
   print Color.RED     // expect: 0
   print Color.GREEN   // expect: 1
   print Color.BLUE    // expect: 2

   enum Status { OK = 200, NOT_FOUND = 404 }
   print Status.OK         // expect: 200
   print Status.NOT_FOUND  // expect: 404

   switch (Color.GREEN) {
     case 0: print "red"
     case 1: print "green"   // expect: green
     default: print "other"
   }
   ```
2. `ctest --test-dir build --output-on-failure` — all tests pass

---

## O10: Default Parameters & Rest Parameters

**Motivation:** The most commonly missing function feature in Maple. Without defaults, users must write wrapper functions for optional arguments. Without rest parameters, variadic functions cannot be written in Maple itself (only in C++ natives).

**Status quo:** `ObjFunction` has only `arity_` (exact parameter count). Every call must pass exactly `arity_` arguments.

### O10.1 Default Parameters

**Syntax:**
```maple
fun greet(name, greeting = "Hello") {
  print greeting + ", " + name + "!"
}
greet("World")           // expect: Hello, World!
greet("World", "Hi")    // expect: Hi, World!
```

**Compiler changes (`Compiler.cc`):**
- In `function_()`, after parsing each parameter name, check for `TOKEN_EQUAL`
- If present, parse the default-value expression and emit it as a constant (or arbitrary expression at function entry)
- Track `min_arity_` (params without default) and `max_arity_` (all params)
- Store defaults as entries in the constant pool with a known base index, or as a separate `std::vector<Value>` on `ObjFunction`

**Preferred approach — constants-based:**
- Add to `ObjFunction`: `int min_arity_{0}` and `int default_base_{-1}` (index into constant pool where defaults start)
- Emit each default expression as a constant; store consecutive constants `K(default_base), K(default_base+1), ...`
- In `VM::call()`:
  ```cpp
  // Fill missing args with defaults
  for (int i = arg_count; i < fn->max_arity(); ++i) {
    frame->slots[i + 1] = fn->chunk().constant_at(fn->default_base() + i - fn->min_arity());
  }
  ```

**ObjFunction additions (`Object.hh`):**
```cpp
int min_arity_{0};     // params without defaults
int default_base_{-1}; // K(default_base..) = default values (-1 = none)
// existing arity_ becomes max_arity_
```

**Serializer (`Serializer.cc`):** Serialize `min_arity_` and `default_base_` alongside existing `arity_`.

### O10.2 Rest Parameters

**Syntax:**
```maple
fun sum(...args) {
  var total = 0
  for (var x in args) { total = total + x }
  return total
}
print sum(1, 2, 3, 4)   // expect: 10
```

**Design:**
- If the last parameter is `...name`, set `has_rest_param_ = true` on `ObjFunction`
- `arity_` in this case = number of fixed params (before `...`)
- In `VM::call()`, when `has_rest_param_`:
  ```cpp
  // Collect trailing args into ObjList
  int rest_start = fn->arity();
  auto* list = allocate<ObjList>();
  for (int i = rest_start; i < arg_count; ++i)
    list->push(args[i]);
  frame->slots[rest_start + 1] = Value(list);
  ```
- Minimum required args = `fn->min_arity()` (defaults can cover fixed params but not rest)

**ObjFunction additions (`Object.hh`):**
```cpp
bool has_rest_param_{false};
```

### O10.3 Spread at Call Site (Optional Extension)

```maple
var nums = [1, 2, 3]
print sum(...nums)   // equivalent to sum(1, 2, 3)
```

**Compiler:** When `...` precedes an argument expression, emit a new opcode `OP_SPREAD` that unpacks a list into the register sequence at the call site. This extends `OP_CALL`'s argument range.

**Files:** `Object.hh` (ObjFunction), `Compiler.cc`, `VM.cc`, `Serializer.cc`, `Opcode.hh` (OP_SPREAD, if O10.3 is included)
**Depends on:** nothing
**Test:**
1. Create `tests/default_params.ms`:
   ```maple
   fun greet(name, greeting = "Hello") {
     print greeting + ", " + name + "!"
   }
   greet("World")           // expect: Hello, World!
   greet("World", "Hi")     // expect: Hi, World!

   fun add(a, b = 10, c = 20) { return a + b + c }
   print add(1)        // expect: 31
   print add(1, 2)     // expect: 23
   print add(1, 2, 3)  // expect: 6
   ```
2. Create `tests/rest_params.ms`:
   ```maple
   fun sum(...args) {
     var total = 0
     for (var x in args) { total = total + x }
     return total
   }
   print sum()           // expect: 0
   print sum(1)          // expect: 1
   print sum(1, 2, 3)    // expect: 6
   print sum(1, 2, 3, 4) // expect: 10
   ```
3. `ctest --test-dir build --output-on-failure` — all tests pass

---

## O11: Coroutines / Generators (yield)

**Motivation:** The single most impactful missing language feature. Generators unlock lazy sequences, composable pipelines, and cooperative multitasking patterns. This is foundational in Python (PEP 255), Lua (coroutines), and JavaScript (ES6 generators).

**Status quo:** Iteration protocol uses `for-in` with `range()` or list iteration (`OP_FORITER`). There is no user-defined lazy iteration.

### O11.1 Syntax

```maple
fun* counter(start, stop) {
  var i = start
  while (i < stop) {
    yield i
    i += 1
  }
}

for (var n in counter(0, 5)) {
  print n       // expect: 0 1 2 3 4 (each on its own line)
}

// Manual .next() protocol
var gen = counter(10, 13)
print gen.next()   // expect: 10
print gen.next()   // expect: 11
print gen.next()   // expect: 12
print gen.next()   // expect: nil  (exhausted)
```

### O11.2 New Object: ObjCoroutine

Add to `Object.hh`:

```cpp
enum class CoroutineState : u8_t { CREATED, RUNNING, SUSPENDED, DEAD };

class ObjCoroutine final : public Object {
  ObjClosure* closure_;
  CoroutineState state_{CoroutineState::CREATED};

  // Saved VM state (captured on yield, restored on resume)
  std::vector<CallFrame> saved_frames_;
  std::vector<Value>     saved_stack_;
  int saved_frame_count_{0};
  Value* saved_stack_top_{nullptr};  // relative to saved_stack_.data()

  Value yielded_value_{};   // last yielded value (nil when dead)
  Value sent_value_{};      // value passed to .send(v) / .next()

public:
  explicit ObjCoroutine(ObjClosure* closure) noexcept;
  str_t stringify() const noexcept override;
  void trace_references() noexcept override;
  sz_t size() const noexcept override;

  ObjClosure* closure() const noexcept { return closure_; }
  CoroutineState state() const noexcept { return state_; }
  void set_state(CoroutineState s) noexcept { state_ = s; }
  Value& yielded_value() noexcept { return yielded_value_; }
  Value& sent_value() noexcept { return sent_value_; }
  // Save/restore VM frame state:
  void save_state(CallFrame* frames, int count, Value* stack, Value* top) noexcept;
  bool restore_state(CallFrame* frames, int& count, Value* stack, Value*& top) noexcept;
};
```

### O11.3 New Opcodes

Add to `Opcode.hh`:

```cpp
// --- Coroutines ---
OP_YIELD,    // iA    suspend coroutine; yield R(A) to caller
OP_RESUME,   // iABC  R(A) = resume coroutine R(B) with sent value R(C)
```

`OP_YIELD` in VM:
1. Save current frame stack into `coroutine->save_state(...)`
2. Set state → SUSPENDED; `coroutine->yielded_value() = regs[A]`
3. Restore caller's frame (the frame that called `.next()` / `OP_RESUME`)
4. Place yielded value into caller's result register and `VM_NEXT()`

`OP_RESUME` in VM:
1. Check `coroutine->state()`: CREATED → call closure; SUSPENDED → restore state; DEAD → push nil
2. Set state → RUNNING; `coroutine->sent_value() = regs[C]`
3. On CREATED: set up a new call frame for the closure
4. On SUSPENDED: call `coroutine->restore_state(...)`

### O11.4 Compiler Changes

**Compiler.cc:**
- Detect `fun*` (TOKEN_FUN followed by TOKEN_STAR) → set `FunctionType::TYPE_GENERATOR` on current compiler
- When compiling a `yield expr` statement:
  - Compile `expr` into a register
  - Emit `OP_YIELD R(result)`
  - Result register of the yield expression (sent value) is populated by `OP_RESUME`
- Calling a generator function (`OP_CALL` on a `TYPE_GENERATOR` closure): the VM creates an `ObjCoroutine` and returns it instead of executing the body

**New token:** Add `TOKEN_YIELD` to `TokenTypes.hh`; add `"yield"` to the keyword map in `Scanner.cc`.

### O11.5 `.next()` and for-in Integration

Register a native `.next()` method on `ObjCoroutine` (or handle it in `VM::invoke()` when the receiver is a coroutine):

- `.next()` → equivalent to `.send(nil)` → resumes with nil as the sent value
- `.send(v)` → resumes, v becomes the result of the `yield` expression inside the generator
- When state == DEAD, `.next()` returns nil

**`OP_FORITER` extension:** When the sequence register holds an `ObjCoroutine`, call `.next()` instead of the list iterator. Stop when result is nil and state is DEAD.

### O11.6 GC Integration

`ObjCoroutine::trace_references()` must mark:
- `closure_`
- All values in `saved_stack_`
- All `ObjClosure*` in `saved_frames_` (deferred closures, etc.)
- `yielded_value_`, `sent_value_`

**Files:** `TokenTypes.hh`, `Scanner.cc`, `Token.hh`, `Compiler.cc`, `Object.hh`, `Object.cc`, `Opcode.hh`, `VM.hh`, `VM.cc`, `Debug.cc`, `Serializer.cc`, `Memory.cc`
**Depends on:** O6 and O7 should be stable before implementing (coroutine yield/resume interacts with the dispatch loop)
**Scope:** Large
**Test:**
1. Create `tests/coroutines.ms`:
   ```maple
   fun* range_gen(n) {
     var i = 0
     while (i < n) { yield i; i += 1 }
   }

   for (var x in range_gen(3)) {
     print x
   }
   // expect: 0
   // expect: 1
   // expect: 2

   var g = range_gen(2)
   print g.next()   // expect: 0
   print g.next()   // expect: 1
   print g.next()   // expect: nil
   ```
2. Create `tests/coroutine_send.ms` (send values into generator):
   ```maple
   fun* accumulator() {
     var total = 0
     while (true) {
       var n = yield total
       if (n == nil) return
       total = total + n
     }
   }
   var acc = accumulator()
   acc.next()         // start
   print acc.send(10) // expect: 10
   print acc.send(20) // expect: 30
   print acc.send(5)  // expect: 35
   ```
3. `ctest --test-dir build --output-on-failure` — all tests pass

---

## Execution Order

```
O5  Benchmark Harness             [Small]    ← enable measurement first
 ↓
O6  Bytecode Quickening           [Medium]   ← highest perf ROI (20-40% numeric)
 ↓
O7  Polymorphic IC (4-entry PIC)  [Medium]   ← OOP polymorphism perf
 ↓
O8  Peephole Optimization Pass    [Medium]   ← compile-time code quality
 ↓
O9  Enums                         [Small]    ← quick expressiveness win (pure desugaring)
 ↓
O10 Default/Rest Parameters       [Medium]   ← major function usability
 ↓
O11 Coroutines / Generators       [Large]    ← biggest new language feature
```

---

## Progress Tracking

| ID | Description | Files | Depends | Status |
|----|-------------|-------|---------|--------|
| O5.1 | Benchmark scripts (7 .ms files) | `benchmarks/*.ms` | — | [x] |
| O5.2 | `--benchmark` flag in main.cc | `main.cc` | O5.1 | [x] |
| O5.3 | Runner script `run_all.sh` | `benchmarks/run_all.sh` | O5.1 | [x] |
| O6.1 | Specialized opcodes (OP_ADD_II etc.) | `Opcode.hh` | — | [x] |
| O6.2 | Quickening in VM dispatch loop | `VM.cc`, `Debug.cc`, `Serializer.cc` | O6.1, O5 | [x] |
| O7.1 | Expand InlineCache to 4-entry PIC | `Object.hh` | — | [x] |
| O7.2 | Update GETPROP/SETPROP/INVOKE fast paths | `VM.cc` | O7.1 | [x] |
| O7.3 | GC tracing for PIC entries | `Object.cc` | O7.1 | [x] |
| O8.1 | Jump-target bitmap builder | `Optimize.cc` | — | [x] |
| O8.2 | Peephole patterns P1–P5 | `Optimize.hh/cc`, `Opcode.hh` | O8.1 | [x] |
| O8.3 | Recursive closure optimization | `Optimize.cc` | O8.2 | [x] |
| O8.4 | Integration call in Compiler.cc | `Compiler.cc`, `CMakeLists.txt` | O8.3 | [x] |
| O9 | Enum declaration (desugaring) | `TokenTypes.hh`, `Scanner.cc`, `Compiler.cc` | — | [x] |
| O10.1 | Default parameters | `Object.hh`, `Compiler.cc`, `VM.cc`, `Serializer.cc` | — | [x] |
| O10.2 | Rest parameters (`...args`) | `Object.hh`, `Compiler.cc`, `VM.cc` | O10.1 | [x] |
| O10.3 | Spread at call site (`...list`) | `Opcode.hh`, `Compiler.cc`, `VM.cc` | O10.2 | [x] |
| O11.1 | `ObjCoroutine` object type | `Object.hh/cc`, `Memory.cc` | — | [x] |
| O11.2 | `OP_YIELD` / `OP_RESUME` opcodes | `Opcode.hh`, `VM.cc`, `Debug.cc` | O11.1 | [x] |
| O11.3 | Compiler: `fun*` / `yield` compilation | `TokenTypes.hh`, `Scanner.cc`, `Compiler.cc` | O11.2 | [x] |
| O11.4 | `.next()` / `.send()` + for-in integration | `VM.cc`, `Object.cc` | O11.3 | [x] |
| O11.5 | Serializer support | `Serializer.cc` | O11.3 | [x] |
