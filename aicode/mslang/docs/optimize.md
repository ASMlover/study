# Maple Language — Optimization Design

> Version: 1.1 | Date: 2026-03-21

Each task is independently implementable and verifiable. Dependencies are explicitly noted; tasks without dependencies can be done in any order.

---

## O1: UTF-8 Source Encoding

### O1.1 BOM Detection

**Status quo:** `ModuleLoader::read_source()` reads raw bytes; a UTF-8 BOM (`\xEF\xBB\xBF`) leaks into the Scanner as garbage.

**Change:** After reading the file into `source`, strip a leading 3-byte BOM if present.

```cpp
// Module.cc — inside read_source(), after file.read()
if (source.size() >= 3
    && static_cast<u8_t>(source[0]) == 0xEF
    && static_cast<u8_t>(source[1]) == 0xBB
    && static_cast<u8_t>(source[2]) == 0xBF) {
  source.erase(0, 3);
}
```

**Files:** Module.cc
**Depends on:** nothing
**Test:**
1. Create `tests/utf8_bom.ms` — a script starting with BOM `\xEF\xBB\xBF` followed by `print "ok" // expect: ok`
2. `ctest --test-dir build --output-on-failure` — new test must pass
3. All existing tests still pass (BOM-free files unaffected)

---

### O1.2 Scanner UTF-8 Pass-Through

**Status quo:** `is_alpha()` rejects bytes >= 0x80 → multi-byte identifiers produce `TOKEN_ERROR`.

**Change:**
- Add UTF-8 helpers in Scanner.cc:

```cpp
static inline int utf8_seq_len(char c) noexcept {
  u8_t b = static_cast<u8_t>(c);
  if (b < 0x80) return 1;
  if ((b & 0xE0) == 0xC0) return 2;
  if ((b & 0xF0) == 0xE0) return 3;
  if ((b & 0xF8) == 0xF0) return 4;
  return 0; // invalid
}
```

- Extend `is_alpha(c)`: return `true` for `c >= 0x80` (treats all multi-byte starts as identifier chars)
- In `scan_identifier()`: when `peek()` byte >= 0x80, consume `utf8_seq_len()` bytes as a unit; report `TOKEN_ERROR` on invalid sequence length or orphan continuation bytes
- `scan_string()` already passes multi-byte through (only ASCII `"`, `$`, `\` are special) — no change needed

**Non-goals:** No Unicode normalization (NFC/NFD); no UAX#31 category checks.

**Files:** Scanner.hh, Scanner.cc
**Depends on:** O1.1 (BOM must not corrupt the first identifier)
**Test:**
1. Create `tests/utf8_identifiers.ms`:
   ```maple
   var 名前 = "hello"
   print 名前      // expect: hello
   ```
2. Create `tests/utf8_strings.ms`:
   ```maple
   print "你好世界"  // expect: 你好世界
   ```
3. `ctest` — new tests pass, all existing tests unaffected

---

## O2: Automatic .msc Cache (Python-style)

### O2.1 Header Extension (source_hash)

**Status quo:** .msc header is 8 bytes: `"MSC\0"(4B) + major(1B) + minor(1B) + reserved(2B)`.

**New header (16B):**
```
Offset  Size  Field
0       4     Magic "MSC\0"
4       1     version_major
5       1     version_minor
6       2     flags (bit 0: has_source_hash)
8       8     source_hash (FNV-1a 64-bit of .ms source)
```

**Changes:**
- Bump `kMSC_VERSION_MINOR` → 1
- Add constants + hash function in Serializer.hh:

```cpp
inline constexpr u8_t kMSC_VERSION_MINOR = 1;
inline constexpr u16_t kMSC_FLAG_HAS_HASH = 0x0001;

inline u64_t fnv1a_hash(strv_t source) noexcept {
  u64_t h = 14695981039346656037ULL;
  for (char c : source) {
    h ^= static_cast<u8_t>(c);
    h *= 1099511628211ULL;
  }
  return h;
}
```

- `serialize()`: write flags=`kMSC_FLAG_HAS_HASH` + 8-byte hash after version
- `deserialize()`: read flags; if `has_source_hash`, read 8 bytes (ignore for now — validation happens in O2.2)
- Old .msc files (minor=0, no flags field) still load — detect by checking `minor < 1`, treat reserved 2B + next 8B as the original payload offset

**Files:** Serializer.hh, Serializer.cc
**Depends on:** nothing
**Test:**
1. `./build/mslang --compile tests/arithmetic.ms` → produces `tests/arithmetic.msc`
2. `./build/mslang tests/arithmetic.msc` → runs correctly
3. Hex-dump first 16 bytes: verify magic + version 1.1 + flags 0x0001 + 8-byte hash
4. All existing serialize/deserialize paths still work

---

### O2.2 Auto-Cache in run_file()

**Status quo:** `run_file()` either compiles `.ms` or loads `.msc` — no automatic caching.

**Change:** Add three helpers in Serializer.hh/cc:

```cpp
str_t msc_path_for(strv_t ms_path) noexcept;
ObjFunction* try_load_cache(strv_t msc_path, strv_t source) noexcept;
void compile_and_cache(strv_t source, strv_t ms_path, strv_t msc_path) noexcept;
```

- `msc_path_for`: append `"c"` to `.ms` path (or `".msc"` otherwise)
- `try_load_cache`: open .msc → validate header → compare `fnv1a_hash(source)` with stored hash → if match, `deserialize()` and return; else return nullptr
- `compile_and_cache`: `compile(source, path)` → `serialize(fn, msc_path)` (best-effort, silently ignore write failure)

Update `run_file()` in main.cc:

```cpp
// For .ms files:
auto source = *source_opt;
str_t msc = ms::msc_path_for(path);
if (auto* fn = ms::try_load_cache(msc, source)) {
  // cache hit — skip compilation
  auto r = vm.interpret_bytecode(fn);
  ...
} else {
  // cache miss — compile, execute, then write cache
  auto* fn = ms::compile(source, path);
  if (!fn) std::exit(65);
  ms::serialize_with_hash(fn, msc, source);  // best-effort
  auto r = vm.interpret_bytecode(fn);
  ...
}
```

**Constraints:** REPL produces no cache. `--compile` retains current behavior (always compiles).

**Files:** Serializer.hh, Serializer.cc, main.cc
**Depends on:** O2.1
**Test:**
1. Delete any existing `tests/arithmetic.msc`
2. `./build/mslang tests/arithmetic.ms` — runs, creates `tests/arithmetic.msc`
3. `./build/mslang tests/arithmetic.ms` again — loads from cache (verify with `MAPLE_DEBUG_PRINT` OFF: no "== <script> ==" disassembly printed, meaning no compilation occurred)
4. Edit `tests/arithmetic.ms` (add a space) → run again → recompiles (hash mismatch) → new `.msc` written
5. All `ctest` tests pass

---

### O2.3 Import Module Cache

**Status quo:** `VM::import_module()` always compiles imported `.ms` files.

**Change:** In `import_module()`, after resolving the `.ms` path, apply the same `try_load_cache` / `compile_and_cache` logic before calling `compile()`.

**Files:** VM.cc
**Depends on:** O2.2
**Test:**
1. Create `tests/import_cache_lib.ms` and `tests/import_cache_main.ms` (main imports lib)
2. Run main → both `.msc` files created
3. Run main again → both loaded from cache
4. Modify lib → lib recompiled, main still cached

---

## O3: Keyword Lookup Optimization

### O3.1 Replace switch-trie with Static HashMap

**Status quo:** `identifier_type()` is a 109-line nested switch-trie (Scanner.cc:246-354) dispatching on `start_[0]`, `start_[1]`, etc. for 28 keywords. Correct but verbose; adding a keyword requires careful nesting.

**Change:** Replace with a `static const std::unordered_map<strv_t, TokenType>`:

```cpp
// Scanner.cc
static const std::unordered_map<strv_t, TokenType> kKeywords = {
  {"abstract", TokenType::TOKEN_ABSTRACT}, {"and",      TokenType::TOKEN_AND},
  {"as",       TokenType::TOKEN_AS},       {"break",    TokenType::TOKEN_BREAK},
  {"case",     TokenType::TOKEN_CASE},     {"catch",    TokenType::TOKEN_CATCH},
  {"class",    TokenType::TOKEN_CLASS},    {"continue", TokenType::TOKEN_CONTINUE},
  {"default",  TokenType::TOKEN_DEFAULT},  {"defer",    TokenType::TOKEN_DEFER},
  {"else",     TokenType::TOKEN_ELSE},     {"false",    TokenType::TOKEN_FALSE},
  {"for",      TokenType::TOKEN_FOR},      {"from",     TokenType::TOKEN_FROM},
  {"fun",      TokenType::TOKEN_FUN},      {"if",       TokenType::TOKEN_IF},
  {"import",   TokenType::TOKEN_IMPORT},   {"in",       TokenType::TOKEN_IN},
  {"nil",      TokenType::TOKEN_NIL},      {"or",       TokenType::TOKEN_OR},
  {"print",    TokenType::TOKEN_PRINT},    {"return",   TokenType::TOKEN_RETURN},
  {"static",   TokenType::TOKEN_STATIC},   {"super",    TokenType::TOKEN_SUPER},
  {"switch",   TokenType::TOKEN_SWITCH},   {"this",     TokenType::TOKEN_THIS},
  {"throw",    TokenType::TOKEN_THROW},    {"true",     TokenType::TOKEN_TRUE},
  {"try",      TokenType::TOKEN_TRY},      {"var",      TokenType::TOKEN_VAR},
  {"while",    TokenType::TOKEN_WHILE},
};

TokenType Scanner::identifier_type() const noexcept {
  strv_t lexeme(start_, static_cast<sz_t>(current_ - start_));
  auto it = kKeywords.find(lexeme);
  return (it != kKeywords.end()) ? it->second : TokenType::TOKEN_IDENTIFIER;
}
```

109 lines → 3. `check_keyword()` becomes dead code — remove it.

**Performance note:** 31 keywords in a hashmap with `strv_t` keys (zero-alloc lookup). For 31 entries, hash + single comparison per lookup. If extreme perf is needed later, swap to a compile-time perfect hash — the API stays the same.

**Files:** Scanner.cc (also remove `check_keyword()` declaration from Scanner.hh if exposed)
**Depends on:** nothing
**Test:**
1. `ctest --test-dir build --output-on-failure` — all 51+ existing tests pass (keywords tokenize identically)
2. Spot-check: scripts using every keyword category (control flow, OOP, error handling) produce identical output

---

## O4: Table Performance

### O4.1 Backward-Shift Deletion

**Status quo:** `Table::remove()` sets a tombstone (`key=null, value=true`). Tombstones inflate `count_`, bloat load factor, and lengthen probe chains. Only cleared on resize.

**Change:** Replace tombstone deletion with backward-shift deletion:

```cpp
bool Table::remove(ObjString* key) noexcept {
  if (count_ == 0) return false;
  sz_t mask = entries_.size() - 1;
  sz_t idx = key->hash() & mask;
  // Linear probe to find entry
  while (entries_[idx].key != nullptr) {
    if (entries_[idx].key == key) goto found;
    idx = (idx + 1) & mask;
  }
  return false;

found:
  entries_[idx] = Entry{};
  count_--;
  // Shift subsequent displaced entries backward
  sz_t empty = idx;
  for (sz_t i = (empty + 1) & mask;
       entries_[i].key != nullptr;
       i = (i + 1) & mask) {
    sz_t natural = entries_[i].key->hash() & mask;
    // Check if `natural` is in (empty, i] wrapping — if not, entry belongs before `empty`
    bool displaced = (empty < i)
        ? (natural <= empty || natural > i)
        : (natural <= empty && natural > i);
    if (displaced) {
      entries_[empty] = entries_[i];
      entries_[i] = Entry{};
      empty = i;
    }
  }
  return true;
}
```

Also update `remove_white()`: collect keys to remove first, then delete one by one (backward-shift is safe when done sequentially).

Remove all tombstone logic from `find_entry()` (no more `tombstone` tracking).

**Files:** Table.hh, Table.cc
**Depends on:** nothing
**Test:**
1. All `ctest` tests pass (Table used for globals + string interning + module exports)
2. Run `tests/gc_stress.ms` (or any GC-heavy script) — `remove_white()` exercises bulk deletion
3. Verify `count_` accuracy: add a debug assertion `assert(count_ >= 0)` after each remove

---

### O4.2 Cached Capacity Mask

**Status quo:** Every `find_entry()` call computes `entries_.size() - 1`.

**Change:** Add `sz_t mask_{0}` member to `Table`. Update in `adjust_capacity()` and constructor. Replace all `entries_.size() - 1` with `mask_`.

**Files:** Table.hh, Table.cc
**Depends on:** nothing (can combine with O4.1 in one commit)
**Test:**
1. `ctest` — all tests pass
2. Build succeeds on all compilers (MSVC, GCC, Clang)

---

### O4.3 Prefetch on Linear Probe

**Change:** In `find_entry()`, after computing the next probe index, prefetch that cache line:

```cpp
#if defined(__GNUC__) || defined(__clang__)
  __builtin_prefetch(&entries_[(idx + 1) & mask_], 0, 1);
#endif
```

On MSVC, use `_mm_prefetch` with `<intrin.h>`:
```cpp
#if defined(_MSC_VER)
  _mm_prefetch(reinterpret_cast<const char*>(&entries_[(idx + 1) & mask_]), _MM_HINT_T1);
#endif
```

**Files:** Table.cc
**Depends on:** O4.2 (uses `mask_`)
**Test:**
1. `ctest` — all tests pass
2. Optional: micro-benchmark with large string interning set to measure cache-miss reduction

---

### O4.4 Hidden Class / Shape (Future Phase)

**Status quo:** `ObjInstance::fields` is `std::unordered_map<ObjString*, Value>` — per-instance hash overhead.

**Concept:** Instances of the same class share a *Shape* (ordered property-name → slot-index map). Instance storage becomes `std::vector<Value>` indexed by slot.

- `ObjClass` owns a `Shape*` (shared among instances with identical property sets)
- Property access: shape lookup (once per IC miss) → O(1) vector index
- IC integration: cache `(shape_id, slot_offset)` — on shape match, direct vector read

**Benefit:** ~56B/instance hashmap overhead → 24B + N*8B vector. Property access becomes array-indexed.

**Files:** Object.hh/cc, VM.cc
**Depends on:** nothing (but benefits from existing IC in VM.cc)
**Scope:** Large — design as a standalone phase after O4.1–O4.3 are stable.

---

## Progress Tracking

| ID | Description | Files | Depends | Status |
|----|-------------|-------|---------|--------|
| O1.1 | BOM detection & skip | Module.cc | — | [x] |
| O1.2 | Scanner UTF-8 identifier/string pass-through | Scanner.hh/cc | O1.1 | [ ] |
| O2.1 | .msc header: add source_hash field | Serializer.hh/cc | — | [ ] |
| O2.2 | Auto-cache in run_file() | Serializer.hh/cc, main.cc | O2.1 | [ ] |
| O2.3 | Import module cache | VM.cc | O2.2 | [ ] |
| O3.1 | Keyword lookup: switch-trie → static hashmap | Scanner.cc | — | [ ] |
| O4.1 | Backward-shift deletion (remove tombstones) | Table.hh/cc | — | [ ] |
| O4.2 | Cached capacity mask | Table.hh/cc | — | [ ] |
| O4.3 | Prefetch on linear probe | Table.cc | O4.2 | [ ] |
| O4.4 | Hidden Class / Shape | Object.hh/cc, VM.cc | — | [ ] |
