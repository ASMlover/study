# Maple Language — Runtime Memory & Performance Optimization Plan

> Version: 1.1 | Date: 2026-03-27

Systematic optimization plan for memory layout and execution performance of the Maple runtime (mslang).
All tasks are independently implementable, buildable, and testable, ordered by priority.

---

## Task Overview

| Task ID | Title | Priority | Effort | Deps | Status |
|---------|-------|----------|--------|------|--------|
| OPT-P0-01 | Remove Object vtable (incl. virtual dtor) | P0 | Large | — | `[x]` |
| OPT-P0-02 | ObjString inline character data (FAM) | P0 | Large | OPT-P0-01 | `[x]` |
| OPT-P0-03 | ObjUpvalue/ObjBoundMethod slab pool allocator | P0 | Medium | OPT-P0-01 | `[x]` |
| OPT-P1-01 | ObjInstance inline field array (SBO) | P1 | Large | OPT-P0-01 | `[x]` |
| OPT-P1-02 | NativeFn as raw function pointer | P1 | Small | — | `[x]` |
| OPT-P1-03 | Constant pool deduplication | P1 | Small | — | `[x]` |
| OPT-P1-04 | Encode IC index directly in GETPROP/SETPROP/INVOKE | P1 | Medium | — | `[x]` |
| OPT-P1-05 | Fix ObjMap iteration O(n²) | P1 | Small | — | `[ ]` |
| OPT-P1-06 | ASCII single-char ObjString cache | P1 | Small | — | `[ ]` |
| OPT-P1-07 | ObjClosure inline upvalue array (FAM) | P1 | Medium | OPT-P0-01 | `[ ]` |
| OPT-P2-01 | ObjClass lazy-allocate sparse Tables | P2 | Small | — | `[ ]` |
| OPT-P2-02 | Shape: small linear map instead of unordered_map | P2 | Small | — | `[ ]` |
| OPT-P2-03 | Remembered set deduplication | P2 | Small | — | `[ ]` |
| OPT-P2-04 | ObjMap open-addressing hash table | P2 | Medium | OPT-P1-05 | `[ ]` |
| OPT-P2-05 | Coroutine SavedCallFrame direct save/restore | P2 | Small | — | `[ ]` |
| OPT-P2-06 | Peephole NOP compaction | P2 | Medium | — | `[ ]` |
| OPT-P3-01 | CallFrame deferred lazy allocation | P3 | Small | — | `[ ]` |
| OPT-P3-02 | Quickening deopt counter | P3 | Small | — | `[ ]` |
| OPT-P3-03 | Compiler local variable arrays dynamic | P3 | Small | — | `[ ]` |
| OPT-P3-04 | Coroutine independent stack segment | P3 | Large | OPT-P2-05 | `[ ]` |

---

## P0 — Core Wins (Implement First)

---

### `[x]` OPT-P0-01: Remove Object vtable (incl. virtual dtor)

**Goal**

`Object` carries an 8-byte vtable pointer due to three virtual methods (`stringify()`,
`trace_references()`, `size()`) plus `virtual ~Object()`. Every GC hot-path call goes through
virtual dispatch, causing unpredictable indirect jumps and I-cache pressure.
**Full devirtualization** (including the destructor) saves 8 bytes per object, shrinking Object
from ≥24 bytes to 16 bytes.

> ⚠️ **Critical constraint**: As long as `Object` has *any* `virtual` (including the destructor),
> the compiler emits a vtable and the 8-byte pointer remains. Both must be removed together.

**Background data**

- `trace_references()` call sites: `VMGC.cc:113` (main trace loop), `VMGC.cc:157` (remembered set), `VMGC.cc:422` (incremental GC)
- `size()` call sites: `VMGC.cc:142,185,212,452` (sweep path)
- `stringify()` call sites: `VMGC.cc:110,139,191,209,419` (debug log) + `Value::stringify()` + `OP_PRINT` + error reporting — not on hot path
- `delete obj` (sweep path): `VMGC.cc:143,215,453,495,504` — must all be replaced after removing virtual dtor
- 16 subclasses all override these 3 methods (ObjString/ObjNative/ObjStringBuilder/ObjFile/ObjWeakRef do not override `trace_references`)

**Files to modify**

- `src/Object.hh` — remove all `virtual` (incl. dtor), declare 4 dispatch functions
- `src/Object.cc` — implement the 4 dispatch functions
- `src/VMGC.cc` — replace trace/size call sites; replace all `delete obj` with `object_destroy`
- `src/Memory.cc` — replace `stringify` debug log calls
- `src/VM.cc` — replace `stringify` call sites; `free_objects()` `delete` replacements
- `src/Value.cc` — `Value::stringify()` routes through `object_stringify()`
- `src/VMBuiltins.cc`, `src/VMNatives.cc` — stringify call sites
- `src/Debug.cc` — disassembler stringify call sites

**Implementation steps**

1. **`src/Object.hh`**: Remove the four virtual methods (3 virtuals + virtual dtor), make dtor non-virtual:
   ```cpp
   // Remove these four:
   virtual ~Object() = default;
   virtual str_t stringify() const noexcept = 0;
   virtual void  trace_references() noexcept {}
   virtual sz_t  size() const noexcept = 0;
   // Replace with:
   ~Object() = default;   // non-virtual
   ```
   Declare **four** dispatch functions outside the `Object` class:
   ```cpp
   str_t   object_stringify(const Object* obj) noexcept;
   void    object_trace(Object* obj) noexcept;
   sz_t    object_size(const Object* obj) noexcept;
   void    object_destroy(Object* obj) noexcept;  // replaces delete obj
   ```

2. **`src/Object.cc`**: Implement the four dispatch functions with `switch(obj->type())`:
   - `object_trace`: trace branches for types with references; `default: break;` for others
   - `object_size`: each branch returns `sizeof(SubType)` (ObjString/ObjInstance/ObjClosure need FAM size)
   - `object_stringify`: each branch calls `static str_t do_stringify(const SubType*)` on the subclass
   - `object_destroy`: each branch explicitly invokes the subclass destructor, e.g. `static_cast<ObjString*>(obj)->~ObjString()`
   Rename each subclass's former `override` to a `static` private method (e.g. `ObjString::do_stringify`).

3. **`src/VMGC.cc`**:
   - `VMGC.cc:113,157,422`: `obj->trace_references()` → `object_trace(obj)`
   - `VMGC.cc:142,185,212,452`: `obj->size()` → `object_size(obj)`
   - `VMGC.cc:143,215,453,495,504`: `delete obj` → `object_destroy(obj); ::operator delete(obj);`

4. All debug-log `->stringify()` in `src/VMGC.cc` → `object_stringify(...)`

5. **`src/VM.cc`** (`free_objects` etc.): `delete obj` → `object_destroy(obj); ::operator delete(obj);`

6. **`src/Value.cc`**: `as_object()->stringify()` → `object_stringify(as_object())`

7. **`src/VMBuiltins.cc`**, **`src/VMNatives.cc`**, **`src/Debug.cc`**: all `->stringify()` → `object_stringify(...)`

8. Add layout assertions:
   ```cpp
   static_assert(!std::is_polymorphic_v<Object>, "Object must not have vtable");
   static_assert(sizeof(Object) == 16, "Object base must be 16B after devirt");
   ```

**Verification**

```bash
cmake --build build
ctest --test-dir build --output-on-failure
python tests/run_tests.py
# Focus: tests/classes.ms, tests/gc.ms, tests/inline_cache.ms
# Optional: mslang --benchmark 5 benchmarks/binary_trees.ms
```

**Risks**

- `object_destroy` must cover all 16 subclasses — any omission causes resource leaks (ObjFile unclosed, Shape not recursively freed, etc.). Add `MAPLE_UNREACHABLE()` or `assert(false)` in the `default:` branch to catch gaps.
- Ensure all subclass overrides renamed to `static` preserve identical return types and behavior.
- Subclasses with no `trace_references` (e.g. ObjString) map to `default: break;` in the switch.

---

### `[x]` OPT-P0-02: ObjString inline character data (FAM)

**Goal**

`ObjString::value_` (`std::string`) has a 32-byte header plus a separate heap allocation for
character data. Using a flexible array member makes each string a single contiguous allocation,
eliminating the `std::string` header overhead and the second level of indirection.

**Background data**

- `ObjString::value()` returns `const str_t&`; ~65+ call sites
- Hottest call sites: `Table.cc:161` (string intern lookup), `VMCall.cc:36` (string concat), `VM.cc:762` (OP_ADD_SS)
- String allocation paths: `VM::copy_string` (`VM.cc:182`), `VM::take_string` (`VM.cc:194`)
- `Serializer.cc:191,246` reads string content for serialization

**Depends on**: OPT-P0-01 (Object layout must be stable first)

**Files to modify**

- `src/Object.hh` — modify ObjString class definition
- `src/Object.cc` — modify ObjString construction/size/static methods
- `src/VM.cc` — copy_string, take_string allocation logic
- `src/Table.cc` — find_string (needs length + hash + char* comparison)
- `src/Serializer.cc` — serialize/deserialize string content
- All files calling `string_obj->value()` (return type changes from `const str_t&` to `strv_t`)

**Implementation steps**

1. **`src/Object.hh`**: Redefine `ObjString`:
   ```cpp
   class ObjString final : public Object {
     u32_t hash_{0};
     u32_t length_{0};
     char  data_[];   // flexible array member, immediately follows the struct
   public:
     ObjString(const ObjString&) = delete;
     ObjString& operator=(const ObjString&) = delete;

     strv_t value() const noexcept { return {data_, length_}; }
     u32_t  hash()  const noexcept { return hash_; }
     u32_t  length() const noexcept { return length_; }
     cstr_t c_str() const noexcept { return data_; }

     static ObjString* create(cstr_t chars, u32_t length, u32_t hash) noexcept;
     static u32_t hash_string(cstr_t chars, sz_t length) noexcept;
   };
   ```
   Remove `str_t value_` member and the original constructor.

2. **`src/Object.cc`**: Implement `ObjString::create`:
   ```cpp
   ObjString* ObjString::create(cstr_t chars, u32_t length, u32_t hash) noexcept {
     void* mem = ::operator new(sizeof(ObjString) + length + 1);
     ObjString* s = ::new(mem) ObjString();  // placement new, empty ctor
     s->hash_   = hash;
     s->length_ = length;
     std::memcpy(s->data_, chars, length);
     s->data_[length] = '\0';
     return s;
   }
   sz_t ObjString::do_size() const noexcept {
     return sizeof(ObjString) + length_ + 1;
   }
   ```

3. **`src/VM.cc`** — `copy_string` (~`VM.cc:180`) and `take_string` (~`VM.cc:194`):
   Call `ObjString::create` directly; no longer construct `str_t`.
   `take_string` semantics unchanged — still calls `create`.

4. **`src/Table.cc`** — `find_string` (~`Table.cc:155`):
   Change `entry->key->value() == str_t(chars, length)` to
   `entry->key->length() == length && std::memcmp(entry->key->c_str(), chars, length) == 0`.

5. **All `->value()` call sites**: return type changes from `const str_t&` to `strv_t`.
   Most comparison callers accept `strv_t` directly. Concat paths need special handling:
   - `VMCall.cc:36` (string concat hot path): avoid `str_t(a->value()) + str_t(b->value())` (two constructions); pre-allocate a single buffer:
     ```cpp
     str_t result;
     result.reserve(a->length() + b->length());
     result.append(a->c_str(), a->length());
     result.append(b->c_str(), b->length());
     // then call copy_string(result.c_str(), result.size())
     ```
     Or add a two-string `copy_string(a, b)` that allocates `sizeof(ObjString) + a->length() + b->length() + 1` and memcpys both segments.
   - `VM.cc:762` (OP_ADD_SS) and `VM.cc:1973` (OP_ADD_SS quickened): use same pre-allocation approach
   - `Serializer.cc:191,246`: use `obj->c_str()` + `obj->length()`

6. **`src/Serializer.cc`**: Deserialization uses `ObjString::create` instead of `copy_string` (preserves intern semantics).

**Verification**

```bash
cmake --build build
ctest --test-dir build --output-on-failure
python tests/run_tests.py
# Focus: tests/strings.ms, tests/string_methods.ms, tests/string_interpolation.ms
# Confirm serialization: run tests with imports (module cache reads/writes contain strings)
```

**Risks**

- Flexible array members cannot have constructor parameters (use placement new). ObjString destructor does nothing (data is POD); GC sweep's `object_destroy` calls `::operator delete(obj)` — not `delete obj`.
- `strv_t` lifetime must not outlive the `ObjString`. Do not hold bare `strv_t` across code paths where GC may trigger.

---

### `[x]` OPT-P0-03: ObjUpvalue/ObjBoundMethod slab pool allocator

**Goal**

`ObjUpvalue` (~40B) and `ObjBoundMethod` (~40B) are the two most frequently allocated objects
in call-intensive code (ObjUpvalue allocated on every closure capture, ObjBoundMethod on every
method lookup). A slab/freelist pool for these fixed-size objects reduces allocation from a
system malloc call to a linked-list dequeue (1–2 instructions).

**Files to modify**

- `src/Memory.hh` — add `ObjectPool<T>` template
- `src/Memory.cc` — implement `ObjectPool`
- `src/VM.hh` — add two pool members to VM
- `src/VM.cc` — `allocate<ObjUpvalue>` and `allocate<ObjBoundMethod>` use pool path
- `src/VMGC.cc` — sweep returns objects to pool instead of `delete`

**Implementation steps**

1. **`src/Memory.hh`**: Declare `ObjectPool<T>`:
   ```cpp
   // Slab pool for fixed-size objects; 64 objects per slab
   template <typename T>
   class ObjectPool {
     static constexpr sz_t kSlabSize = 64;
     struct Slab { alignas(T) char data[sizeof(T) * kSlabSize]; };
     struct FreeNode { FreeNode* next; };

     std::vector<std::unique_ptr<Slab>> slabs_;
     FreeNode* free_list_{nullptr};
     sz_t      allocated_{0};
   public:
     T*   alloc() noexcept;    // dequeue from freelist; allocate new slab when empty
     void free(T* obj) noexcept;  // return to freelist (no destructor call)
     void destroy_all() noexcept; // release all slabs at shutdown
     sz_t size_bytes() const noexcept; // current bytes used (for GC accounting)
   };
   ```

2. **`src/Memory.cc`**: Implement `alloc` and `free`:
   - `alloc`: if `free_list_` is non-null, pop and return; otherwise create a new slab, link all 64 slots into `free_list_`, then pop one.
   - `free`: cast the object pointer to `FreeNode*` and push to head of `free_list_` (object memory reused to store the link pointer).

3. **`src/VM.hh`**: Add two pool members:
   ```cpp
   ObjectPool<ObjUpvalue>     upvalue_pool_;
   ObjectPool<ObjBoundMethod> bound_method_pool_;
   ```

4. **`src/VM.cc`** — `allocate<T>` template (~`VM.cc:218`):
   Specialize (or branch) for `ObjUpvalue` and `ObjBoundMethod` to use pool alloc + placement new:
   ```cpp
   template<>
   ObjUpvalue* VM::allocate<ObjUpvalue>(Value* slot) {
     void* mem = upvalue_pool_.alloc();
     ObjUpvalue* obj = ::new(mem) ObjUpvalue(slot);
     young_bytes_ += sizeof(ObjUpvalue);
     bytes_allocated_ += sizeof(ObjUpvalue);
     obj->set_generation(GcGeneration::YOUNG);
     obj->set_next(young_objects_);
     young_objects_ = obj;
     return obj;
   }
   ```

5. **`src/VMGC.cc`** — sweep path:
   After OPT-P0-01, sweep no longer uses `delete obj`; all destruction goes through `object_destroy`.
   In the switch-case for ObjUpvalue and ObjBoundMethod, return to pool instead:
   ```cpp
   case ObjectType::OBJ_UPVALUE: {
     auto* uv = static_cast<ObjUpvalue*>(obj);
     uv->~ObjUpvalue();
     upvalue_pool_.free(uv);
     return;
   }
   case ObjectType::OBJ_BOUND_METHOD: {
     auto* bm = static_cast<ObjBoundMethod*>(obj);
     bm->~ObjBoundMethod();
     bound_method_pool_.free(bm);
     return;
   }
   ```
   Simplification: let `object_destroy` return `false` for pool-managed objects (already freed internally)
   and `true` for regular objects (caller then calls `::operator delete`).
   Add compile-time guards:
   ```cpp
   static_assert(std::is_trivially_destructible_v<ObjUpvalue>,
                 "ObjUpvalue pool free skips destructor side effects");
   static_assert(std::is_trivially_destructible_v<ObjBoundMethod>,
                 "ObjBoundMethod pool free skips destructor side effects");
   ```
   In `free_objects()` (VM shutdown): call `upvalue_pool_.destroy_all()` and `bound_method_pool_.destroy_all()`.

**Verification**

```bash
cmake -B build -DMAPLE_DEBUG_STRESS_GC=ON && cmake --build build
ctest --test-dir build --output-on-failure
python tests/run_tests.py
# STRESS_GC triggers GC on every allocation — quickly exposes pool/GC interaction bugs
cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build
mslang --benchmark 5 benchmarks/method_dispatch.ms   # ObjBoundMethod hot path
mslang --benchmark 5 benchmarks/binary_trees.ms      # ObjUpvalue + closures
```

**Risks**

- Pool object memory is reused to store `FreeNode*`. Confirm `sizeof(FreeNode*) <= sizeof(T)` and alignment is satisfied (ObjUpvalue ~40B, ObjBoundMethod ~40B — both well above an 8-byte pointer; safe).
- If either type gains a non-trivial member (e.g. `std::vector`) in the future, the `static_assert` will fail at compile time, forcing an update to the pool release logic.

---

## P1 — Important Improvements

---

### `[x]` OPT-P1-01: ObjInstance inline field array (SBO)

**Goal**

`ObjInstance::fields_` (`std::vector<Value>`) has a 24-byte header plus a second heap allocation
for field data. Use Small Buffer Optimization (SBO): store up to `kInlineFields` fields inline;
spill to a heap `Value*` only when exceeded. Eliminates the vector header and heap allocation
for the vast majority of instances (≤8 fields) **without changing pointer semantics**
(no dangling pointer risk).

> ⚠️ **No pure FAM + realloc approach**: after a realloc, old pointers are invalid and all
> stack slots, upvalues, globals, and GC lists holding the old pointer would need updating —
> not safely implementable in a mark-sweep GC without forwarding pointers.

**Background data**

- `ObjInstance` allocated in `VMCall.cc:133`
- Field access hot path: `Object.hh:335` (`get_field(u32_t slot)`) and `Object.hh:336` (`set_field`)
- Shape's `slot_count()` is known at allocation time; new fields call `shape_->add_transition()` + `fields_.push_back`

**Depends on**: OPT-P0-01

**Files to modify**

- `src/Object.hh` — modify ObjInstance definition, remove `std::vector<Value> fields_`
- `src/Object.cc` — modify field access, append, GC trace, size
- `src/VMCall.cc` — no special changes needed (default ctor suffices)

**Implementation steps**

1. **`src/Object.hh`** — modify `ObjInstance`:
   ```cpp
   class ObjInstance final : public Object {
     static constexpr u32_t kInlineFields = 8;
     ObjClass* klass_{nullptr};
     Shape*    shape_{nullptr};
     u32_t     field_count_{0};
     u32_t     capacity_{kInlineFields};
     Value     inline_fields_[kInlineFields]{};  // inline field slots
     Value*    overflow_{nullptr};               // heap array, only when > 8 fields
   public:
     Value  get_field(u32_t slot) const noexcept;
     void   set_field(u32_t slot, Value val) noexcept;
     bool   get_field(ObjString* name, Value* out) const noexcept;
     void   add_field(ObjString* name, Value val) noexcept;
     ObjClass* klass() const noexcept { return klass_; }
     Shape*    shape() const noexcept { return shape_; }
     u32_t     field_count() const noexcept { return field_count_; }
   private:
     Value* fields_ptr() noexcept {
       return overflow_ ? overflow_ : inline_fields_;
     }
   };
   ```

2. **`src/Object.cc`** — implement field operations:
   - `get_field(u32_t slot)`: `return fields_ptr()[slot]`
   - `set_field(u32_t slot, Value val)`: `fields_ptr()[slot] = val`
   - `add_field(ObjString* name, Value val)`:
     - Call `shape_->add_transition(name)` to get new shape and slot index
     - If `field_count_ < capacity_`: write `fields_ptr()[field_count_++]`
     - Otherwise: `capacity_ *= 2`, reallocate `overflow_`, copy from old location (`overflow_` pointer itself changes; ObjInstance pointer stays fixed)

3. **`src/VMCall.cc:133`** — no change needed; inline buffer zero-initializes to nil.

4. **`trace_references`**: iterate `fields_ptr()[0..field_count_-1]`, call `mark_value`.
   Note: `overflow_` is a raw non-GC pointer; free with `delete[] overflow_` in destructor only.

5. **`size`**: `sizeof(ObjInstance) + (overflow_ ? capacity_ * sizeof(Value) : 0)`

6. **Destructor**: in `object_destroy`, call `delete[] overflow_` if non-null.

**Verification**

```bash
cmake -B build -DMAPLE_DEBUG_STRESS_GC=ON && cmake --build build
python tests/run_tests.py
# Focus: tests/classes.ms, tests/inline_cache.ms, tests/gc.ms
mslang --benchmark 5 benchmarks/field_access.ms
mslang --benchmark 5 benchmarks/binary_trees.ms
```

**Risks**

- `overflow_` is not GC-managed — GC traces its *contents* (Values) but not the pointer itself. Ensure `object_destroy` calls `delete[] overflow_` to prevent leaks.
- `kInlineFields=8` is a tuning parameter; each ObjInstance gains 64 bytes of inline storage — weigh memory footprint vs. allocation reduction.

---

### `[x]` OPT-P1-02: NativeFn as raw function pointer

**Goal**

`NativeFn = std::function<Value(int,Value*)>` triggers a heap allocation when the capture
exceeds the small buffer. All 23 native functions capture `[this]` (the VM pointer).
Change to a raw function pointer taking `VM&` explicitly, eliminating `std::function`
type-erasure dispatch and potential heap allocation.

**Background data**

- Type definition: `Object.hh:236`
- Only call site: `VMCall.cc:124-127`
- All native definitions: `VMNatives.cc` (23 lambdas, all capturing `[this]`)
- `VMImport.cc:56`: one additional NativeFn construction in the io module

**Files to modify**

- `src/Object.hh` — change `NativeFn` type alias, `ObjNative` constructor
- `src/VMCall.cc` — call site: pass `*this` (VM)
- `src/VMNatives.cc` — all lambdas → `[](VM& vm, int argc, Value* args)` form
- `src/VMImport.cc` — io module native construction

**Implementation steps**

1. **`src/Object.hh`**:
   ```cpp
   class VM;  // forward declaration
   using NativeFn = Value(*)(VM& vm, int arg_count, Value* args);
   ```

2. **`src/VMCall.cc:124-127`**:
   ```cpp
   // Before:
   Value result = native(arg_count, stack_top_ - arg_count);
   // After:
   Value result = native(*this, arg_count, stack_top_ - arg_count);
   ```

3. **`src/VMNatives.cc`**: Change all `[this](int argc, Value* args)` lambdas to captureless lambdas accepting `VM& vm`:
   ```cpp
   // Before:
   define_native("strlen", [this](int argc, Value* args) { ... use this-> ... });
   // After (captureless lambda implicitly converts to function pointer):
   define_native("strlen", [](VM& vm, int argc, Value* args) { ... use vm. ... });
   ```
   Replace `this->runtime_error(...)` with `vm.runtime_error(...)`, `this->copy_string(...)` with `vm.copy_string(...)`, etc.

4. **`src/VMImport.cc:56`**: Same transformation — captureless lambda with `VM&`.

5. Captureless lambdas convert implicitly to function pointers; all `define_native` calls should compile without changes. If any native genuinely needs extra state (confirmed: none do), add a `void* userdata` field to `ObjNative`.

**Verification**

```bash
cmake --build build
python tests/run_tests.py
# Natives are used across all tests — any error surfaces immediately
```

**Risks**

- Low risk: signature change only, behavior unchanged.
- If the compiler has issues with captureless lambda → function pointer conversion, convert the lambda to a `static` file-scope function.

---

### `[x]` OPT-P1-03: Constant pool deduplication

**Goal**

`Chunk::add_constant()` unconditionally push_backs; if the same identifier is referenced N times
in a function body, N constant pool slots are created (even though the underlying `ObjString*`
is the same interned pointer). Deduplication shrinks the constant pool, compacts bytecode, and
improves I-cache efficiency.

**Background data**

- `Chunk::add_constant`: `Chunk.cc:42`
- `Compiler::make_constant`: `Compiler.cc:478`
- `Compiler::identifier_constant`: `Compiler.cc:491-494` (most common source of duplicates)

**Files to modify**

- `src/CompilerImpl.hh` — add string constant dedup cache member
- `src/Compiler.cc` — check cache in `make_constant` or `identifier_constant`

**Implementation steps**

1. **`src/CompilerImpl.hh`**: Add to `Compiler` class:
   ```cpp
   std::unordered_map<ObjString*, int> str_const_cache_;  // intern ptr → constant pool index
   ```

2. **`src/Compiler.cc`** — `identifier_constant` (`Compiler.cc:491`):
   ```cpp
   int Compiler::identifier_constant(const Token& name) noexcept {
     ObjString* interned = VM::get_instance().copy_string(
         name.lexeme.data(), name.lexeme.length());
     auto it = str_const_cache_.find(interned);
     if (it != str_const_cache_.end()) return it->second;
     int idx = make_constant(Value::make_object(interned));
     str_const_cache_[interned] = idx;
     return idx;
   }
   ```
   Numeric literals (integer/double) are rarely duplicated — no dedup needed.
   String literals (`"foo"`) could also benefit but the gain is smaller.

3. `str_const_cache_` is naturally destroyed when the `Compiler` object is destroyed at `end_compiler()`.
   Nested function `Compiler` instances are independent; each cache is scoped to one function (desired behavior).

**Verification**

```bash
cmake --build build
python tests/run_tests.py
# All tests pass, confirming dedup doesn't change semantics
# Optional: print constant pool size at compile time to confirm reduction
# Focus: tests/globals.ms, tests/functions.ms (many global variable references)
```

**Risks**

- Extremely low risk: only affects constant pool slot count, not semantics.
- Jump offset backpatching patches the `code_` array, not the constant pool — no interaction.

---

### `[x]` OPT-P1-04: Encode IC index directly in GETPROP/SETPROP/INVOKE

**Goal**

Each property access currently compiles to 2 instructions: `OP_GETPROP(A,B)` + `OP_EXTRAARG(ic_slot)`.
`OP_EXTRAARG` exists solely to carry the IC slot index (0–255 range is sufficient).
Encoding the IC index into `OP_GETPROP`'s C field reduces instruction count by ~50% in class-heavy code.

**Background data**

- `OP_GETPROP` current format: iABC, A=dest reg, B=object reg, C=name constant idx (RK encoded)
- EXTRAARG consumption: `VM.cc:550-551` (GETPROP), `VM.cc:656-657` (SETPROP), `VM.cc:1176-1177` (INVOKE)
- IC slot count: `ic_.size()` is typically well below 255 (per-function independent count)
- Compiler emit sites: `CompilerExpr.cc` (dot access), `CompilerStmt.cc` (method call)

**Files to modify**

- `src/CompilerExpr.cc` — modify dot_access and method call emit logic
- `src/CompilerStmt.cc` — modify method call emit logic
- `src/VM.cc` — GETPROP/SETPROP/INVOKE handlers: read IC index from C field, remove `READ_INSTR()`
- `src/Debug.cc` — disassembler: update GETPROP/SETPROP/INVOKE display format
- `src/Serializer.cc` — stores instructions verbatim; no change needed

**Implementation steps**

1. **Confirm instruction format feasibility**: iABC's C field is 8 bits (0–255); currently stores name's RK encoding (0–127 = register, 128–255 = constant pool). C is already occupied.
   **Adjustment**: move name to Bx field (16 bits), use iABx format (A=dest, Bx upper 8 bits = name const idx, lower 8 bits = IC slot):
   ```
   OP_GETPROP: iABx   [Bx:16 = (name_const:8 | ic_slot:8)][A:8][Op:8]
   ```
   Name constant pool index is limited to 256 (function-local name count is well below this in practice). IC slot 0–254; `0xFF` means "use EXTRAARG fallback".

2. **`src/CompilerExpr.cc`** (dot access emit):
   ```cpp
   int ic_slot = current_function->add_ic();
   if (name_const <= 0xFF && ic_slot <= 0xFE) {
     u16_t bx = (static_cast<u16_t>(name_const) << 8) | static_cast<u16_t>(ic_slot);
     emit_ABx(OpCode::OP_GETPROP, dest, bx);
   } else {
     // fallback: old two-instruction format
     emit_ABC(OpCode::OP_GETPROP, dest, name_rk, 0xFF);
     emit_ABx(OpCode::OP_EXTRAARG, 0, ic_slot);
   }
   ```

3. **`src/VM.cc`** — GETPROP handler (~`VM.cc:548`):
   ```cpp
   u16_t bx = decode_Bx(instr);
   u8_t name_const = static_cast<u8_t>(bx >> 8);
   u8_t ic_slot    = static_cast<u8_t>(bx & 0xFF);
   if (ic_slot == 0xFF) {
     Instruction extra = READ_INSTR();
     ic_slot = static_cast<u8_t>(decode_Bx(extra));
   }
   ```
   Apply same change to SETPROP and INVOKE handlers.

4. **`src/Debug.cc`**: Update GETPROP/SETPROP/INVOKE disassembly to parse name and ic_slot from Bx and display both.

**Verification**

```bash
cmake -B build -DMAPLE_DEBUG_PRINT=ON && cmake --build build
# Run class tests, verify disassembly shows GETPROP no longer followed by EXTRAARG
mslang tests/inline_cache.ms
python tests/run_tests.py
mslang --benchmark 5 benchmarks/field_access.ms
mslang --benchmark 5 benchmarks/method_dispatch.ms
# Stress-test fallback path: construct a function with >256 distinct property names
# Confirm ic_slot=0xFF fallback generates correct two-instruction format
```

**Risks**

- Instruction format change requires updating serialization (Serializer). Existing `.msc` cache files use the old format — increment `kMSC_VERSION_MINOR` in `Serializer.hh` (currently 1.1) to auto-invalidate stale caches.
- Bx name_const is limited to 8 bits (0–255). If a single function accesses >255 distinct property names, it automatically falls back to the two-instruction format (`ic_slot=0xFF` signals the next EXTRAARG). Verify the fallback path works correctly under stress testing.

---

## P1 (Addendum) — Important Improvements (Supplemental)

---

### `[ ]` OPT-P1-05: Fix ObjMap iteration O(n²)

**Goal**

`OP_FORITER` (`VM.cc:1737-1747`) iterates `ObjMap` using `std::advance(it, idx)`, traversing
`idx` steps from the beginning on every iteration — O(n²) total. Provide a standalone fix
before OPT-P2-04 (open-addressing ObjMap).

**Background data**

- `VM.cc:1743-1744`: `auto it = entries.begin(); std::advance(it, idx);`
- `ObjMap::entries()` returns `std::unordered_map<Value,Value,...>&`; iterator is forward-only
- For a 1000-element map: 500K total steps — O(n²)

**Files to modify**

- `src/Object.hh` — add lazy key-snapshot helper to `ObjMap`
- `src/VM.cc` — `OP_FORITER` OBJ_MAP branch

**Implementation steps**

1. **`src/Object.hh`** — add iteration helper to `ObjMap`:
   ```cpp
   class ObjMap final : public Object {
     ValueMap entries_;
     mutable std::vector<Value> iter_keys_;
     mutable bool iter_dirty_{true};
   public:
     const std::vector<Value>& iter_snapshot() const noexcept {
       if (iter_dirty_) {
         iter_keys_.clear();
         for (auto& [k, _] : entries_) iter_keys_.push_back(k);
         iter_dirty_ = false;
       }
       return iter_keys_;
     }
     void mark_dirty() noexcept { iter_dirty_ = true; }
   };
   ```
   All map-modifying operations (`map_set`, `map_del`, `VMBuiltins.cc`) call `map->mark_dirty()`.

2. **`src/VM.cc`** — `OP_FORITER` OBJ_MAP branch (`VM.cc:1737`):
   ```cpp
   const auto& keys = map->iter_snapshot();
   if (idx >= static_cast<int>(keys.size())) {
     frame->ip += sBx;
   } else {
     frame->slots[A + 2] = keys[static_cast<sz_t>(idx)];  // O(1) random access
     index_val = Value(static_cast<double>(idx + 1));
   }
   ```

**Note**: When OPT-P2-04 (open-addressing ObjMap) is complete, this workaround can be removed —
direct index access on an open-addressing table gives O(n) iteration naturally.

**Verification**

```bash
cmake --build build
python tests/run_tests.py
# Add a performance test that iterates a 1000-key map; confirm time is near O(n)
```

---

### `[ ]` OPT-P1-06: ASCII single-char ObjString cache

**Goal**

String iteration (`VM.cc:1733-1734`) calls `copy_string(char_ptr, 1)` for each character,
allocating a new ObjString. Although interning ensures only one copy per character, the *first*
traversal of an ASCII string still allocates up to 128 small objects. Pre-cache all 128 ASCII
single-char ObjStrings so string iteration becomes allocation-free O(1).

**Files to modify**

- `src/VM.hh` — add `ascii_char_cache_[128]` member
- `src/VM.cc` — populate cache during VM init; `copy_string` fast-path for length==1 && char<128

**Implementation steps**

1. **`src/VM.hh`**:
   ```cpp
   ObjString* ascii_char_cache_[128]{};  // ASCII single-char ObjString cache
   ```

2. **`src/VM.cc`** — during VM construction (or `init()`):
   ```cpp
   for (int c = 0; c < 128; ++c) {
     char buf[1] = {static_cast<char>(c)};
     ascii_char_cache_[c] = copy_string(buf, 1);  // intern + allocate, permanently resident
   }
   ```
   These 128 objects are GC roots (permanently old gen); mark them in `mark_roots`.

3. **`src/VM.cc`** — `copy_string` fast path:
   ```cpp
   ObjString* VM::copy_string(cstr_t chars, sz_t length) noexcept {
     if (length == 1 && static_cast<u8_t>(chars[0]) < 128 && ascii_char_cache_[chars[0]])
       return ascii_char_cache_[chars[0]];
     // ...existing logic
   }
   ```

4. **`src/VMGC.cc`** — `mark_roots`:
   ```cpp
   for (ObjString* s : ascii_char_cache_)
     if (s) mark_object(s);
   ```

**Verification**

```bash
cmake --build build
python tests/run_tests.py
# Verify string iteration (for c in "hello") works correctly
# Verify cached objects are not collected by GC
```

---

### `[ ]` OPT-P1-07: ObjClosure inline upvalue array (FAM)

**Goal**

`ObjClosure::upvalues_` (`std::vector<ObjUpvalue*>`) has a 24-byte header plus a heap
allocation, even though the element count is known at compile time
(`function->upvalue_count()`). Converting to a flexible array member eliminates one heap
allocation per closure creation.

**Background data**

- `Object.hh:271`: `std::vector<ObjUpvalue*> upvalues_`
- `ObjClosure` construction: `Object.cc` — `upvalues_.resize(function->upvalue_count())`
- Access pattern: random access `upvalues_[i]`, no resizing

**Depends on**: OPT-P0-01

**Files to modify**

- `src/Object.hh` — modify ObjClosure definition, remove `std::vector<ObjUpvalue*>`
- `src/Object.cc` — modify construction, trace_references, size; add `ObjClosure::create`
- `src/VM.cc` — change allocation call to `ObjClosure::create(function)`

**Implementation steps**

1. **`src/Object.hh`** — modify `ObjClosure`:
   ```cpp
   class ObjClosure final : public Object {
     ObjFunction* function_{nullptr};
     int upvalue_count_{0};
     // upvalues_[] immediately follows (flexible array member pattern)
   public:
     static ObjClosure* create(ObjFunction* function) noexcept;
     ObjUpvalue*& upvalue(int idx) noexcept {
       return reinterpret_cast<ObjUpvalue**>(this + 1)[idx];
     }
     ObjUpvalue* upvalue(int idx) const noexcept {
       return reinterpret_cast<ObjUpvalue* const*>(this + 1)[idx];
     }
     int upvalue_count() const noexcept { return upvalue_count_; }
     ObjFunction* function() const noexcept { return function_; }
   };
   ```

2. **`src/Object.cc`** — implement `ObjClosure::create`:
   ```cpp
   ObjClosure* ObjClosure::create(ObjFunction* function) noexcept {
     int n = function->upvalue_count();
     void* mem = ::operator new(sizeof(ObjClosure) + n * sizeof(ObjUpvalue*));
     ObjClosure* c = ::new(mem) ObjClosure();
     c->function_ = function;
     c->upvalue_count_ = n;
     auto** upvalues = reinterpret_cast<ObjUpvalue**>(c + 1);
     for (int i = 0; i < n; ++i) upvalues[i] = nullptr;
     return c;
   }
   ```

3. **`trace_references`**: iterate `upvalue(0..upvalue_count_-1)`, call `mark_object`.

4. **`size`**: `sizeof(ObjClosure) + upvalue_count_ * sizeof(ObjUpvalue*)`

5. **`src/VM.cc`**: replace all `allocate<ObjClosure>(function)` with `ObjClosure::create(function)`.

**Verification**

```bash
cmake -B build -DMAPLE_DEBUG_STRESS_GC=ON && cmake --build build
python tests/run_tests.py
# Focus: tests/closures.ms, tests/upvalues.ms
mslang --benchmark 5 benchmarks/binary_trees.ms  # closure-intensive workload
```

---

## P2 — Medium Priority

---

### `[ ]` OPT-P2-01: ObjClass lazy-allocate sparse Tables

**Goal**

`ObjClass` holds 5 `Table` members: `methods_`, `static_methods_`, `getters_`, `setters_`,
`abstract_methods_`. The last three are empty in the vast majority of user-defined classes.
Change these three to `std::unique_ptr<Table>`, saving ~72 bytes per class when unused.

**Files to modify**

- `src/Object.hh` — change three member types in ObjClass
- `src/Object.cc` — modify construction/destruction/trace_references/size
- `src/VM.cc` — getter/setter/abstract access sites need null check (or lazy init)
- `src/VMCall.cc` — same

**Implementation steps**

1. **`src/Object.hh`**:
   ```cpp
   class ObjClass final : public Object {
     ObjString* name_{nullptr};
     Table methods_;
     Table static_methods_;
     std::unique_ptr<Table> getters_;
     std::unique_ptr<Table> setters_;
     std::unique_ptr<Table> abstract_methods_;
     ...
   public:
     Table& getters() noexcept {
       if (!getters_) getters_ = std::make_unique<Table>();
       return *getters_;
     }
     bool has_getters() const noexcept { return getters_ != nullptr; }
     // same pattern for setters_ and abstract_methods_
   };
   ```

2. **`src/VM.cc`** — all `klass->getters()` / `klass->setters()` access sites:
   - Read operations: check `klass->has_getters()` first (avoid creating empty Table on read)
   - Write operations (`OP_METHOD` registering a getter): call `klass->getters()` directly (triggers lazy init)

3. **`src/Object.cc`** — `trace_references`:
   ```cpp
   if (getters_) getters_->mark_table();
   if (setters_) setters_->mark_table();
   if (abstract_methods_) abstract_methods_->mark_table();
   ```

4. **`size()`**:
   ```cpp
   return sizeof(ObjClass)
     + (getters_ ? getters_->capacity() * sizeof(Table::Entry) : 0)
     + ...;
   ```

**Verification**

```bash
cmake --build build
python tests/run_tests.py
# Focus: tests/getter_setter.ms, tests/abstract_methods.ms, tests/classes.ms
```

---

### `[ ]` OPT-P2-02: Shape small linear map (replace unordered_map)

**Goal**

`Shape::slots_` and `Shape::transitions_` are both `std::unordered_map`, with ~50 bytes of
node overhead per entry. For typical 2–5 field classes, switch to a linear flat array for ≤8
entries; pointer equality suffices for `ObjString*` keys (interned).

**Files to modify**

- `src/Object.hh` — modify Shape class definition
- `src/Object.cc` — modify find_slot, add_transition, mark_keys, destructor

**Implementation steps**

1. **`src/Object.hh`** — define `SmallMap` helper and replace Shape members:
   ```cpp
   template <typename K, typename V, int N = 8>
   struct SmallMap {
     struct Entry { K key; V value; };
     Entry  data[N]{};
     int    count{0};
     // spill to heap when > N entries; unique_ptr ensures automatic cleanup
     std::unique_ptr<std::unordered_map<K,V>> extra;

     bool overflow() const noexcept { return extra != nullptr; }
     V* find(K key) noexcept { /* linear scan data[0..count-1]; query *extra when overflow */ }
     void insert(K key, V val) noexcept {
       /* count < N: write to data; count == N: allocate extra, migrate data, then insert */
     }
     void mark_keys() noexcept { /* iterate all keys, call mark_object */ }
     // default dtor handles unique_ptr cleanup automatically
   };

   class Shape {
     u32_t id_;
     SmallMap<ObjString*, u32_t>  slots_;        // property name → slot index
     SmallMap<ObjString*, Shape*> transitions_;  // property name → child Shape
     u32_t slot_count_{0};
   };
   ```
   > Shape is not a GC-managed object; ObjClass destructor recursively deletes it.
   > `SmallMap` uses `unique_ptr` to ensure the overflow heap map is freed automatically on Shape destruction.

2. **`src/Object.cc`**:
   - `find_slot`: call `slots_.find(name)`
   - `add_transition`: call `transitions_.insert(name, child)`
   - `mark_keys`: call `slots_.mark_keys()` and `transitions_.mark_keys()`

**Verification**

```bash
cmake --build build
python tests/run_tests.py
# Focus: tests/classes.ms, tests/inline_cache.ms
mslang --benchmark 5 benchmarks/field_access.ms
```

---

### `[ ]` OPT-P2-03: Remembered set deduplication

**Goal**

`VM::remember(Object*)` at `VMGC.cc:98` unconditionally push_backs without checking for
duplicates. Repeatedly mutated old-generation objects (e.g. a global list that is frequently
appended) can be added to the remembered set multiple times, causing redundant tracing during
minor GC.

**Files to modify**

- `src/Object.hh` — add `in_remembered_` bit to Object base (using existing padding)
- `src/VMGC.cc` — check the bit in `remember()`; clear all bits before `remembered_set_.clear()`

**Implementation steps**

1. **`src/Object.hh`**: Add `bool in_remembered_{false}` to Object (fits in existing padding after `age_`):
   ```cpp
   class Object {
     ObjectType type_;          // 4B
     bool is_marked_{false};    // 1B
     bool finalized_{false};    // 1B
     GcGeneration generation_;  // 1B
     u8_t age_{0};              // 1B
     bool in_remembered_{false};// 1B (fits in existing padding)
     // 3B padding
     Object* next_{nullptr};    // 8B
   };
   ```
   Add accessors: `bool in_remembered() const noexcept` and `void set_in_remembered(bool v) noexcept`.

2. **`src/VMGC.cc`** — `VM::remember(Object* object)` (`VMGC.cc:98`):
   ```cpp
   void VM::remember(Object* object) noexcept {
     if (object->in_remembered()) return;
     object->set_in_remembered(true);
     remembered_set_.push_back(object);
   }
   ```

3. **`src/VMGC.cc`** — before `remembered_set_.clear()` at end of minor GC (`VMGC.cc:345`):
   ```cpp
   for (Object* obj : remembered_set_)
     obj->set_in_remembered(false);
   remembered_set_.clear();
   ```

**Verification**

```bash
cmake -B build -DMAPLE_DEBUG_STRESS_GC=ON && cmake --build build
python tests/run_tests.py
# Focus: tests/gc.ms (old-gen write barrier tests)
```

---

### `[ ]` OPT-P2-04: ObjMap open-addressing hash table

**Goal**

`ObjMap` uses `std::unordered_map<Value,Value,...>` (chained hashing, per-entry heap node).
Replace with an open-addressing hash table (modeled on the existing `Table` implementation):
contiguous memory layout, GC trace becomes a linear scan, cache-friendly.

**Files to modify**

- `src/Object.hh` — define `ValueTable` (open-addressing) to replace `ValueMap`
- `src/Object.cc` — implement `ValueTable` operations, update ObjMap
- `src/VMBuiltins.cc` — all ObjMap operation call sites (set/get/del/keys/values/len/contains)

**Implementation steps**

1. **`src/Object.hh`**: Define `ValueTable`:
   ```cpp
   struct ValueEntry { Value key{}; Value value{}; bool tombstone{false}; };

   class ValueTable {
     std::vector<ValueEntry> entries_;
     int count_{0};
     static constexpr double kMAX_LOAD = 0.75;
   public:
     bool   get(const Value& key, Value* out) const noexcept;
     bool   set(const Value& key, Value val) noexcept;   // returns true on new insert
     bool   del(const Value& key) noexcept;
     void   mark_entries() noexcept;  // GC: linear scan to mark key/value
     int    count() const noexcept { return count_; }
   private:
     ValueEntry* find_entry(const Value& key) noexcept;
     void   grow() noexcept;
   };
   ```

2. **`src/Object.cc`**: Implement `ValueTable` using `ValueHash` (already available) for hashing,
   linear probing, tombstone deletion (or backward-shift), 2× capacity growth on rehash.

3. **`src/Object.hh`** — update `ObjMap`:
   ```cpp
   class ObjMap final : public Object {
     ValueTable entries_;
   public:
     ValueTable& entries() noexcept { return entries_; }
     sz_t len() const noexcept { return entries_.count(); }
   };
   ```

4. **`src/VMBuiltins.cc`**: Update all ObjMap operations from `entries_.find/insert/erase/size`
   to `entries_.get/set/del/count`.

5. **`trace_references`**: call `entries_.mark_entries()` (linear scan).

**Verification**

```bash
cmake --build build
python tests/run_tests.py
# Focus: tests/maps.ms (if it exists) — test set/get/del/keys/values/len
```

---

### `[ ]` OPT-P2-05: Coroutine SavedCallFrame direct save/restore

**Goal**

On yield/resume, `CallFrame` (containing absolute `Value* slots` pointer) is saved as
`SavedCallFrame` (containing `ptrdiff_t slots_offset`). Each resume recomputes the absolute
pointer from the relative offset, and `SavedCallFrame::deferred` is a full `std::vector<ObjClosure*>` copy.
Eliminate the offset↔pointer conversion and the deferred vector copy by using a simplified
`SavedCallFrame` that stores only what is needed.

**Background data**

- `SavedCallFrame` definition: `Object.hh:498-505`, contains slots_offset and deferred vector
- OP_YIELD save: `VM.cc:1826-1848` (constructs SavedCallFrame field by field)
- OP_RESUME restore: `VM.cc:1906+` (restores CallFrame field by field)

**Files to modify**

- `src/Object.hh` — simplify `SavedCallFrame` (store `u32_t slots_offset`, drop deferred vector)
- `src/VM.cc` — OP_YIELD / OP_RESUME: simplified save/restore, no deferred copy

**Implementation steps**

1. **`src/Object.hh`** — simplify `SavedCallFrame`:
   ```cpp
   struct SavedCallFrame {
     ObjClosure*  closure{nullptr};
     Instruction* ip{nullptr};
     u32_t        slots_offset{0};   // index into stack_.data(); restore as stack_.data() + offset
     Value        pending_return{};
     bool         returning{false};
     // deferred not saved — Maple semantics guarantee defer executes before yield
   };
   ```

2. **`src/VM.cc`** — OP_YIELD (`VM.cc:1826`):
   ```cpp
   SavedCallFrame sf;
   sf.closure  = f.closure;
   sf.ip       = f.ip;
   sf.slots_offset = static_cast<u32_t>(f.slots - stack_.data());
   sf.pending_return = f.pending_return;
   sf.returning = f.returning;
   // assert(f.deferred.empty());
   coro->saved_frames().push_back(sf);
   ```

3. **`src/VM.cc`** — OP_RESUME restore (`VM.cc:1906`):
   ```cpp
   CallFrame& f = frames_[frame_count_++];
   f.closure  = sf.closure;
   f.ip       = sf.ip;
   f.slots    = stack_.data() + sf.slots_offset;
   f.pending_return = sf.pending_return;
   f.returning = sf.returning;
   f.deferred.clear();
   ```

**Verification**

```bash
cmake -B build -DMAPLE_DEBUG_STRESS_GC=ON && cmake --build build
python tests/run_tests.py
# Focus: tests/coroutines.ms
```

**Note**: True O(1) yield/resume via independent coroutine stack segments is deferred to
**OPT-P3-04** (low priority) due to its impact on hundreds of stack access paths.

---

### `[ ]` OPT-P2-06: Peephole NOP compaction

**Goal**

The `Optimize.cc` peephole optimizer replaces dead code with `OP_NOP` but never removes them.
Residual NOPs still go through the dispatch loop (decode + skip) on every execution, wasting
I-cache. Add a compaction pass that removes all NOPs and fixes up jump offsets.

**Background data**

- NOP generation sites: `Optimize.cc:84,105,117,150,171`
- VM NOP handler: `VM.cc:1803` (`VM_CASE(OP_NOP) { VM_DISPATCH(); }`)
- Chunk jump instructions use relative offsets (sBx field); NOP removal requires offset fixup

**Files to modify**

- `src/Chunk.hh` — add `compact_nops()` method declaration
- `src/Chunk.cc` — implement compaction pass
- `src/Optimize.cc` — call `chunk.compact_nops()` after the peephole pass

**Implementation steps**

1. **`src/Chunk.cc`** — implement `compact_nops()`:
   - Pass 1: build `offset_map[old_offset] = new_offset` (skipping all NOPs)
   - Pass 2: copy non-NOP instructions to new sequence; for all jump instructions (OP_JUMP, OP_JUMP_IF_FALSE, OP_FORITER, etc.) recompute `sBx = offset_map[target] - current_new_offset - 1`
   - Sync `lines_` (RLE line info): reduce run counts for NOP instructions

2. **IC slot offset concern**: IC slots are stored in `ObjFunction::ic_` and accessed by ic_index, not bytecode offset — unaffected by compaction. Confirm: `OP_GETPROP/SETPROP/INVOKE` ic_slot is an index into `ic_` vector, independent of instruction position in `code_`.

3. **Jump instructions to handle**: scan `Opcode.hh` for all instructions using relative jumps (sBx field): OP_JUMP, OP_JUMP_IF_FALSE, OP_JUMP_IF_TRUE, OP_FORITER, OP_LOOP, etc. — fix up all of them during compaction.

**Verification**

```bash
cmake -B build -DMAPLE_DEBUG_PRINT=ON && cmake --build build
# Run any script; confirm disassembly contains no (or very few) NOPs
# Confirm jump targets are correct (if/else/while/for loops)
python tests/run_tests.py
# Focus: tests/control_flow.ms, tests/loops.ms, tests/coroutines.ms
```

---

## P3 — Low Priority

---

### `[ ]` OPT-P3-01: CallFrame deferred lazy allocation

**Goal**

`CallFrame::deferred` (`std::vector<ObjClosure*>`) is empty in the vast majority of frames
but costs 24 bytes per frame unconditionally. 64 pre-allocated frames waste 1.5 KB. Switch to
lazy allocation.

**Files to modify**

- `src/VM.hh` — modify CallFrame struct
- `src/VM.cc` — OP_DEFER and OP_RETURN handling
- `src/VMGC.cc` — mark_roots deferred traversal
- `src/VMCall.cc` — clear deferred on new frame

**Implementation steps**

1. **`src/VM.hh`**:
   ```cpp
   struct CallFrame {
     ObjClosure*  closure{nullptr};
     Instruction* ip{nullptr};
     Value*       slots{nullptr};
     Value        pending_return{};
     bool         returning{false};
     // replace std::vector<ObjClosure*> deferred
     ObjClosure** deferred_buf{nullptr};   // heap-allocated on demand
     u8_t         deferred_count{0};
     u8_t         deferred_capacity{0};
   };
   ```

2. **`src/VM.cc`** — OP_DEFER (`VM.cc:1796`):
   If `deferred_buf == nullptr`, `new ObjClosure*[4]` (initial capacity 4).
   If full, realloc (capacity × 2).

3. **`src/VMGC.cc`** — mark_roots: iterate `frames_[i].deferred_buf[0..deferred_count-1]`.

4. **`src/VMCall.cc`** — reset frame: `delete[] frame.deferred_buf; frame.deferred_buf = nullptr; frame.deferred_count = 0;`

**Verification**

```bash
cmake --build build
python tests/run_tests.py
# Focus: tests/defer.ms (if defer syntax tests exist)
```

---

### `[ ]` OPT-P3-02: Quickening deopt counter

**Goal**

Currently, a quickened instruction (e.g. `OP_ADD_II`) that encounters a type mismatch reverts
to the generic form, which quickens again on the next execution — infinite quicken→deopt loop
at polymorphic call sites. Add a deopt counter; after 3 deopt events, lock the instruction
permanently as generic.

**Background data**

- No deopt counter exists (confirmed in VMGC.cc/VM.cc)
- Deopt sites: `VM.cc:1949,1963,1978,2002,2014,2026,2038,2050,2062,2074` (each quickened handler)

**Files to modify**

- `src/Object.hh` — add `deopt_count` field to `ICEntry` (reuse existing padding)
- `src/VM.cc` — increment counter at deopt sites; lock at threshold

**Implementation steps**

1. **`src/Object.hh`** — add to `ICEntry` (uses existing padding, zero size increase):
   ```cpp
   struct ICEntry {
     ObjClass* klass{nullptr};
     ICKind    kind{ICKind::UNINITIALIZED};
     Value     cached{};
     u32_t     shape_id{0};
     u32_t     slot_index{0};
     u8_t      deopt_count{0};  // new: locked to generic when >= 3
   };
   ```
   Note: quickened arithmetic instructions (OP_ADD_II etc.) do not use IC slots. Use a separate
   lazy-allocated `std::vector<u8_t> arith_deopt_` in `ObjFunction` (indexed by bytecode offset).

2. **`src/VM.cc`** — all deopt sites (e.g. `VM.cc:1949`):
   ```cpp
   u32_t instr_offset = static_cast<u32_t>(
       frame->ip - 1 - frame->closure->function()->chunk().code().data());
   u8_t count = frame->closure->function()->increment_arith_deopt(instr_offset);
   if (count < 3) {
     // not locked: revert, allow requickening next time
     const_cast<Instruction*>(frame->ip)[-1] = encode_ABC(OpCode::OP_ADD, A, B, C);
   }
   // count >= 3: do not revert; execute generic logic each time (stable state)
   ```

**Verification**

```bash
cmake --build build
python tests/run_tests.py
# Focus: tests/quickening.ms
# Test polymorphic scenarios with mixed-type calls
```

---

### `[ ]` OPT-P3-03: Compiler local variable arrays dynamic

**Goal**

`CompilerImpl.hh:148/150` has `Local locals_[256]` and `Upvalue upvalues_[256]` as fixed stack
arrays. Each nested function scope consumes ~10 KB on the C++ call stack. At 10+ nesting levels,
this is 100 KB+ with stack overflow risk. Switch to `std::vector` with on-demand growth, reducing
cost from 10 KB to 48 bytes (two empty vectors).

**Files to modify**

- `src/CompilerImpl.hh` — change Local/Upvalue array types
- `src/Compiler.cc` — all `locals_[i]` / `upvalues_[i]` access sites and `local_count_` logic

**Implementation steps**

1. **`src/CompilerImpl.hh`**:
   ```cpp
   // Before:
   Local   locals_[kUINT8_COUNT];
   Upvalue upvalues_[kUINT8_COUNT];
   // After:
   std::vector<Local>   locals_;    // ctor: locals_.reserve(16)
   std::vector<Upvalue> upvalues_;  // ctor: upvalues_.reserve(8)
   ```

2. **`src/Compiler.cc`**: Replace `locals_[local_count_]` with `locals_.emplace_back()` or direct index access.
   The semantic limit check (`local_count_ == 256` → error) becomes `locals_.size() == 256`.
   Same treatment for `upvalues_`.

3. In constructor: `locals_.reserve(16); upvalues_.reserve(8);`

**Verification**

```bash
cmake --build build
python tests/run_tests.py
# All tests pass; use a deeply nested closure script to confirm no stack overflow
```

---

### `[ ]` OPT-P3-04: Coroutine independent stack segment (O(1) yield/resume)

**Goal**

Build on OPT-P2-05 to give each coroutine its own stack buffer; yield/resume becomes a pointer
swap — true O(1).

**Depends on**: OPT-P2-05

**Background data**

- Current yield still copies `saved_stack_` (`std::vector<Value>`) and `saved_frames_`
- Independent stack plan: each coroutine holds its own `Value stack_[kCoroStackSize]` and
  `CallFrame frames_[kCoroFrameMax]`; VM switch only swaps a few pointers

**Files to modify**

- `src/Object.hh` — modify ObjCoroutine, add independent stack buffer
- `src/VM.cc` — OP_YIELD / OP_RESUME: pointer swap instead of data copy
- `src/VMGC.cc` — mark_roots must trace Values on each coroutine's independent stack

**⚠️ Important**

Independent stack requires VM's `stack_top_`, `frames_`, etc. to switch to different buffers on
yield. All code paths that access the stack via `stack_top_` (hundreds of sites) must
transparently operate on the currently active stack buffer.
Ensure coroutine test coverage is 100% (`tests/coroutines.ms` + edge cases) before starting.

**Verification**

```bash
cmake -B build -DMAPLE_DEBUG_STRESS_GC=ON && cmake --build build
python tests/run_tests.py
# Focus: tests/coroutines.ms
# Performance comparison: deep coroutine nesting yield/resume benchmark
```

---

## Appendix: Quick Reference Commands

```bash
# Build (Debug)
cmake -B build -DCMAKE_BUILD_TYPE=Debug && cmake --build build

# Build (Release, for performance comparison)
cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build

# GC stress test
cmake -B build -DMAPLE_DEBUG_STRESS_GC=ON && cmake --build build

# Full test suite
ctest --test-dir build --output-on-failure
python tests/run_tests.py

# Individual scripts
./build/mslang tests/classes.ms
./build/mslang tests/gc.ms
./build/mslang tests/coroutines.ms

# Benchmarks
./build/mslang --benchmark 5 benchmarks/binary_trees.ms
./build/mslang --benchmark 5 benchmarks/field_access.ms
./build/mslang --benchmark 5 benchmarks/method_dispatch.ms
```
