# Maple Language — Improvement Roadmap

> Version: 1.0 | Date: 2026-03-10 | Phase 1-9 complete

---

## Phase 10: Language Expressiveness

### 10.1 String Interpolation
```maple
print "Hello, ${name}!";       // Hello, World!
print "1 + 2 = ${1 + 2}";     // 1 + 2 = 3
```
- Scanner: recognize `${` inside strings, switch to expression mode
- Emit segment constants + expressions + N×`OP_ADD`, or dedicated `OP_STRING_INTERPOLATE N`
- **Files:** Scanner.cc, Compiler.cc | **Scope:** Medium

### 10.2 Anonymous Functions (Lambdas)
```maple
var double = fun(x) { return x * 2; };
```
- Allow `fun` without identifier in expression position, synthetic name `<lambda>`
- Wire `fun_expression()` as prefix handler for TOKEN_FUN
- **Files:** Compiler.cc | **Scope:** Small

### 10.3 Compound Assignment (`+=` `-=` `*=` `/=` `%=`)
```maple
x += 5;  x -= 3;  x *= 2;  x /= 4;  x %= 5;
```
- New 2-char tokens; desugar `x += e` → `x = x + e` (emit GET + expr + OP + SET)
- Works for locals, globals, upvalues, properties, index expressions
- **Files:** TokenTypes.hh, Scanner.cc, Compiler.cc | **Scope:** Medium

### 10.4 Ternary Operator
```maple
var x = cond ? a : b;
```
- Infix `?` above assignment; compile via `OP_JUMP_IF_FALSE` + `OP_JUMP` + patch
- **Files:** Compiler.cc | **Scope:** Small

### 10.5 Block Comments
```maple
/* multi-line comment */
```
- Scanner: `/*` → skip until `*/`, support nesting (counter), track line increments
- **Files:** Scanner.cc | **Scope:** Small

### 10.6 `else if` Chain
- After `else`, if next token is `if` → parse as `if_statement()` directly (no braces required)
- **Files:** Compiler.cc | **Scope:** Small

### 10.7 `switch` Statement
```maple
switch (value) { case 1: print "one"; default: print "other"; }
```
- Compile to cascading `OP_EQUAL` + `OP_JUMP_IF_FALSE`, or `OP_SWITCH` jump table
- **Files:** TokenTypes.hh, Scanner.cc, Compiler.cc | **Scope:** Medium

---

## Phase 11: Type System & Built-in Types

### 11.1 Integer Type
- Add `i64_t` to Value (NaN-boxing: second QNAN tag); auto-promote int+float→float
- Bitwise operators (`&` `|` `^` `~` `<<` `>>`) on integers only
- Alternative: keep f64 only + implicit truncation (Lua 5.3 style)
- **Files:** Value.hh/cc, VM.cc, Compiler.cc, Opcode.hh | **Scope:** Large

### 11.2 String Methods
```maple
s.len(); s.slice(0,5); s.find("x"); s.replace("a","b");
s.split(","); s.upper(); s.lower(); s.trim();
s.starts_with("H"); s.ends_with("E"); s.contains("W");
```
- VM `invoke()` checks `is_string(receiver)` → dispatch to C++ implementation via shared string prototype
- **Files:** Object.hh/cc, VM.cc | **Scope:** Medium

### 11.3 List Comprehensions
```maple
var squares = [x * x for x in range(10)];
```
- Desugar to `OP_BUILD_LIST 0` → loop → push → end; depends on 12.1 for-in
- **Files:** Compiler.cc | **Scope:** Medium

### 11.4 Tuple Type
```maple
var point = (1, 2);  var (x, y) = get_point();
```
- ObjTuple: fixed-size, immutable, hashable; destructuring assignment in compiler
- **Files:** Object.hh/cc, Compiler.cc, VM.cc | **Scope:** Medium

---

## Phase 12: Control Flow & Iteration

### 12.1 `for-in` Iterator Protocol
```maple
for (var x in [1, 2, 3]) { print x; }
for (var i in range(10)) { print i; }
```
- Objects with `iter()`/`next()` methods; desugar to while loop
- Built-in `range(start, stop, step)` native
- **Files:** TokenTypes.hh, Scanner.cc, Compiler.cc, VM.cc, Object.hh/cc | **Scope:** Large

### 12.2 Error Handling: `try` / `catch` / `throw`
```maple
try { var x = num("bad"); } catch (e) { print e; }
```
- ObjError (message + stack trace); `OP_TRY`/`OP_CATCH`/`OP_END_TRY` opcodes
- ExceptionHandler stack parallel to call frames (frame index, stack depth, catch IP)
- Runtime error unwinds to nearest handler instead of aborting
- **Files:** Opcode.hh, Compiler.cc, VM.hh/cc, Object.hh/cc | **Scope:** Large

### 12.3 Deferred Execution (`defer`)
```maple
defer f.close();  // runs at function return, LIFO order
```
- Per-frame defer list; `OP_DEFER` pushes closure, `OP_RETURN` executes all
- **Files:** Opcode.hh, Compiler.cc, VM.cc | **Scope:** Medium

---

## Phase 13: OOP Enhancements

### 13.1 Static Methods
```maple
class Math { static max(a, b) { if (a > b) return a; return b; } }
print Math.max(3, 5);  // 5
```
- `static` keyword → store on ObjClass, not instance methods; forbid `this`
- **Files:** TokenTypes.hh, Scanner.cc, Compiler.cc, VM.cc | **Scope:** Small-Medium

### 13.2 Getter / Setter Properties
```maple
class Circle {
  get area() { return 3.14 * this._r * this._r; }
  set radius(v) { this._r = v; }
}
print c.area;     // auto-call getter, no parens
c.radius = 10;    // auto-call setter
```
- Separate getter/setter tables on ObjClass; `OP_GET/SET_PROPERTY` checks after fields/methods
- **Files:** Compiler.cc, VM.cc, Object.hh/cc | **Scope:** Medium

### 13.3 Abstract Methods (Optional)
- `abstract` keyword → calling base method raises runtime error; or convention-only
- **Scope:** Small

### 13.4 Operator Overloading
```maple
class Vec2 { __add(o) { return Vec2(this.x+o.x, this.y+o.y); } }
print a + b;  // dispatches to __add
```
- `OP_ADD` etc. check ObjInstance → look up `__add`/`__sub`/`__mul`/`__div`/`__eq`/`__lt`/`__str`
- Number/string fast paths unchanged
- **Files:** VM.cc, Object.hh | **Scope:** Medium

---

## Phase 14: VM & Compiler Optimizations

### 14.1 Constant Folding
- Compile-time evaluate constant expressions: `1 + 2 * 3` → single `OP_CONSTANT 7`
- Extend to unary (`-3`) and string concat (`"a" + "b"`)
- **Files:** Compiler.cc | **Scope:** Small-Medium

### 14.2 Register-Based Bytecode
- Instructions name src/dst registers: `ADD R0, R1, R2`; locals ARE registers
- Eliminates most push/pop; ~30% fewer instructions (Lua 5, Dalvik)
- **Scope:** Very Large (research)

### 14.3 Inline Caching for Property Access
- Per-instruction cache: (class pointer, slot offset); monomorphic → O(1) access
- On class mismatch → full lookup + update cache; polymorphic IC for hot spots
- **Files:** VM.cc, Chunk.hh | **Scope:** Medium-Large

### 14.4 Superinstruction Fusion
- Peephole pass fuses common sequences: `GET_LOCAL+GET_LOCAL+ADD` → `ADD_LOCAL_LOCAL`
- Candidates: `BINARY_LOCAL_LOCAL`, `CALL_LOCAL_0`, `RETURN_LOCAL`
- **Files:** Opcode.hh, Compiler.cc, VM.cc | **Scope:** Medium

### 14.5 Copy-on-Write Strings / StringBuilder
- ObjStringBuilder for amortized O(1) append in loops (avoids O(n^2) concat)
- Or expose `StringBuilder` as standard library type
- **Files:** Object.hh/cc | **Scope:** Medium

---

## Phase 15: Standard Library

### 15.1 Core Natives
```maple
// Math:    abs ceil floor round sqrt pow min max random
// Convert: int(value) bool(value)   (str/num already exist)
// I/O:     read_file(path) write_file(path, data)
// Assert:  assert(cond, message?)
```
- Register in `VM::define_natives()` or new `stdlib.cc`
- **Scope:** Small per function

### 15.2 File I/O Module
```maple
import "io";
var f = io.open("data.txt", "r"); var content = f.read(); f.close();
```
- Built-in module; ObjFile wraps std::fstream with GC integration
- **Files:** new Stdlib.hh/cc, VM.cc | **Scope:** Medium

### 15.3 `range()` Function
- ObjRange (start, stop, step) — lazy, implements iterator protocol
- **Files:** Object.hh/cc, VM.cc | **Scope:** Small (depends on 12.1)

---

## Phase 16: Tooling & Developer Experience

### 16.1 REPL Enhancements
- Multi-line input (detect unmatched `{`/`(`), history (readline), tab completion
- Meta-commands: `@help`, `@quit`, `@load "file.ms"`
- **Files:** main.cc | **Scope:** Medium

### 16.2 Source Maps & Better Stack Traces
```
RuntimeError: Undefined variable 'foo'.
  at bar() [script.ms:15:10]
  |   print foo;
  |         ^^^
```
- Track column in Token/Chunk; `runtime_error()` reads source and prints context
- **Files:** Token.hh, Scanner.cc, Chunk.hh/cc, VM.cc | **Scope:** Medium

### 16.3 Bytecode Serialization
- `.msc` binary format: magic + version + constants + bytecode + line info + function graph
- `mslang --compile script.ms` → `script.msc`; `mslang script.msc` → direct execution
- **Files:** new Serializer.hh/cc, main.cc | **Scope:** Medium-Large

### 16.4 Language Server Protocol (LSP)
- Optional 2-pass compiler (AST mode) for tooling; semantic tokens, symbol table, diagnostics
- **Scope:** Very Large (long-term)

---

## Phase 17: GC Improvements

### 17.1 Incremental / Generational GC
- **Generational:** young nursery (frequent) + old generation (promoted after N cycles) + write barrier
- **Incremental:** amortize mark phase across bytecode instructions + write barrier
- **Files:** Memory.hh/cc, VM.cc | **Scope:** Large

### 17.2 Weak References
```maple
var weak = weak_ref(obj); var strong = weak.get();  // nil if collected
```
- ObjWeakRef: GC does not trace; sweep nulls out refs to collected objects
- **Files:** Object.hh/cc, Memory.cc | **Scope:** Medium

### 17.3 Finalizers
- `__finalize()` method called during sweep before `delete`, or destructor callback on ObjNativeData
- **Files:** VM.cc, Memory.cc | **Scope:** Small-Medium

---

## Progress Tracking

| Sprint | Task | Description | Status |
|--------|------|-------------|--------|
| **1 — Quick Wins** | 10.6 | `else if` chain | [x] |
| | 10.5 | Block comments `/* */` | [x] |
| | 10.2 | Anonymous functions (lambdas) | [x] |
| | 10.3 | Compound assignment `+= -= *= /= %=` | [x] |
| **2 — Expressiveness** | 10.1 | String interpolation `"${expr}"` | [x] |
| | 10.4 | Ternary operator `? :` | [x] |
| | 14.1 | Constant folding | [x] |
| **3 — Built-in Types** | 11.2 | String methods (len/slice/find/replace/split/...) | [x] |
| | 15.1 | Core natives (math/convert/assert/file I/O) | [x] |
| | 15.3 | `range()` function | [x] |
| **4 — Iteration** | 12.1 | `for-in` iterator protocol | [x] |
| | 11.3 | List comprehensions | [x] |
| **5 — OOP** | 13.1 | Static methods | [x] |
| | 13.4 | Operator overloading (`__add`/`__str`/...) | [x] |
| | 13.2 | Getter / setter properties | [x] |
| **6 — Robustness** | 12.2 | `try` / `catch` / `throw` | [x] |
| | 12.3 | `defer` statement | [x] |
| | 16.2 | Source maps & better stack traces | [x] |
| **7 — Performance** | 14.3 | Inline caching for property access | [x] |
| | 14.4 | Superinstruction fusion | [x] |
| | 14.5 | COW strings / StringBuilder | [x] |
| **8 — Tooling** | 16.1 | REPL enhancements (multi-line/history/completion) | [x] |
| | 16.3 | Bytecode serialization (`.msc`) | [x] |
| **Backlog** | 10.7 | `switch` statement | [x] |
| | 11.1 | Integer type + bitwise operators | [x] |
| | 11.4 | Tuple type + destructuring | [x] |
| | 13.3 | Abstract methods | [x] |
| | 14.2 | Register-based bytecode | [x] |
| | 15.2 | File I/O module | [x] |
| | 16.4 | Language Server Protocol (LSP) | [x] |
| | 17.1 | Incremental / generational GC | [x] |
| | 17.2 | Weak references | [x] |
| | 17.3 | Finalizers | [x] |
