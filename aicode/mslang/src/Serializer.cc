// Copyright (c) 2026 ASMlover. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//  * Redistributions of source code must retain the above copyright
//    notice, this list ofconditions and the following disclaimer.
//
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in
//    the documentation and/or other materialsprovided with the
//    distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
#include <cstring>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <vector>
#include "Serializer.hh"
#include "VM.hh"

namespace ms {

// -- Writer helpers ----------------------------------------------------------

class ByteWriter {
  std::vector<u8_t> buf_;
public:
  void write_u8(u8_t v) noexcept { buf_.push_back(v); }

  void write_u32(u32_t v) noexcept {
    buf_.push_back(static_cast<u8_t>((v >> 24) & 0xFF));
    buf_.push_back(static_cast<u8_t>((v >> 16) & 0xFF));
    buf_.push_back(static_cast<u8_t>((v >> 8)  & 0xFF));
    buf_.push_back(static_cast<u8_t>( v        & 0xFF));
  }

  void write_i32(i32_t v) noexcept { write_u32(static_cast<u32_t>(v)); }

  void write_f64(double v) noexcept {
    u64_t bits;
    std::memcpy(&bits, &v, sizeof(bits));
    for (int i = 56; i >= 0; i -= 8)
      buf_.push_back(static_cast<u8_t>((bits >> i) & 0xFF));
  }

  void write_bytes(const u8_t* data, sz_t len) noexcept {
    buf_.insert(buf_.end(), data, data + len);
  }

  void write_str(strv_t s) noexcept {
    write_u32(static_cast<u32_t>(s.size()));
    buf_.insert(buf_.end(), s.begin(), s.end());
  }

  bool flush(strv_t path) const noexcept {
    std::ofstream ofs(str_t(path), std::ios::binary);
    if (!ofs) return false;
    ofs.write(reinterpret_cast<const char*>(buf_.data()),
              static_cast<std::streamsize>(buf_.size()));
    return ofs.good();
  }
};

// -- Reader helpers ----------------------------------------------------------

class ByteReader {
  const u8_t* data_;
  sz_t size_;
  sz_t pos_{0};
public:
  ByteReader(const u8_t* data, sz_t size) noexcept : data_(data), size_(size) {}

  bool has(sz_t n) const noexcept { return pos_ + n <= size_; }

  u8_t read_u8() noexcept { return data_[pos_++]; }

  u32_t read_u32() noexcept {
    u32_t v = 0;
    v |= static_cast<u32_t>(data_[pos_++]) << 24;
    v |= static_cast<u32_t>(data_[pos_++]) << 16;
    v |= static_cast<u32_t>(data_[pos_++]) << 8;
    v |= static_cast<u32_t>(data_[pos_++]);
    return v;
  }

  i32_t read_i32() noexcept { return static_cast<i32_t>(read_u32()); }

  double read_f64() noexcept {
    u64_t bits = 0;
    for (int i = 0; i < 8; ++i)
      bits = (bits << 8) | data_[pos_++];
    double v;
    std::memcpy(&v, &bits, sizeof(v));
    return v;
  }

  str_t read_str() noexcept {
    u32_t len = read_u32();
    str_t s(reinterpret_cast<const char*>(data_ + pos_), len);
    pos_ += len;
    return s;
  }

  const u8_t* read_bytes(sz_t n) noexcept {
    const u8_t* p = data_ + pos_;
    pos_ += n;
    return p;
  }
};

// -- Constant tags -----------------------------------------------------------

enum class ConstTag : u8_t {
  TAG_NIL      = 0,
  TAG_BOOL     = 1,
  TAG_NUMBER   = 2,
  TAG_STRING   = 3,
  TAG_FUNCTION = 4,
  TAG_INTEGER  = 5,
};

// -- Serialization -----------------------------------------------------------

// Collect all ObjFunction* reachable from `root` in DFS post-order.
// Inner functions appear before outer ones.
static void collect_functions(ObjFunction* root,
    std::vector<ObjFunction*>& out,
    std::unordered_map<ObjFunction*, u32_t>& index_map) noexcept {
  if (index_map.count(root)) return;

  // Visit child functions first (constants that are ObjFunction*)
  for (const auto& val : root->chunk().constants()) {
    if (val.is_object() && val.as_object()->type() == ObjectType::OBJ_FUNCTION) {
      collect_functions(as_obj<ObjFunction>(val.as_object()), out, index_map);
    }
  }

  index_map[root] = static_cast<u32_t>(out.size());
  out.push_back(root);
}

static void write_function(ByteWriter& w, ObjFunction* fn,
    const std::unordered_map<ObjFunction*, u32_t>& index_map) noexcept {
  w.write_u32(static_cast<u32_t>(fn->arity()));
  w.write_u32(static_cast<u32_t>(fn->upvalue_count()));

  // Name
  if (fn->name()) {
    w.write_str(fn->name()->value());
  } else {
    w.write_u32(0);
  }

  // Script path
  w.write_str(fn->script_path());

  // Chunk: code
  const auto& code = fn->chunk().code();
  w.write_u32(static_cast<u32_t>(code.size()));
  w.write_bytes(code.data(), code.size());

  // Chunk: constants
  const auto& constants = fn->chunk().constants();
  w.write_u32(static_cast<u32_t>(constants.size()));
  for (const auto& val : constants) {
    if (val.is_nil()) {
      w.write_u8(static_cast<u8_t>(ConstTag::TAG_NIL));
    } else if (val.is_boolean()) {
      w.write_u8(static_cast<u8_t>(ConstTag::TAG_BOOL));
      w.write_u8(val.as_boolean() ? 1 : 0);
    } else if (val.is_integer()) {
      w.write_u8(static_cast<u8_t>(ConstTag::TAG_INTEGER));
      // Write i64 as 8 bytes big-endian
      i64_t ival = val.as_integer();
      u64_t uval = static_cast<u64_t>(ival);
      for (int shift = 56; shift >= 0; shift -= 8)
        w.write_u8(static_cast<u8_t>((uval >> shift) & 0xFF));
    } else if (val.is_double()) {
      w.write_u8(static_cast<u8_t>(ConstTag::TAG_NUMBER));
      w.write_f64(val.as_number());
    } else if (val.is_object()) {
      auto* obj = val.as_object();
      if (obj->type() == ObjectType::OBJ_STRING) {
        w.write_u8(static_cast<u8_t>(ConstTag::TAG_STRING));
        w.write_str(as_obj<ObjString>(obj)->value());
      } else if (obj->type() == ObjectType::OBJ_FUNCTION) {
        w.write_u8(static_cast<u8_t>(ConstTag::TAG_FUNCTION));
        auto it = index_map.find(as_obj<ObjFunction>(obj));
        w.write_u32(it->second);
      }
    }
  }

  // Chunk: line runs
  const auto& lines = fn->chunk().lines();
  w.write_u32(static_cast<u32_t>(lines.size()));
  for (const auto& run : lines) {
    w.write_i32(run.line);
    w.write_i32(run.column);
    w.write_i32(run.token_length);
    w.write_i32(run.count);
  }

  // Inline cache slot count
  w.write_u32(static_cast<u32_t>(fn->ic_count()));
}

bool serialize(ObjFunction* function, strv_t path) noexcept {
  std::vector<ObjFunction*> functions;
  std::unordered_map<ObjFunction*, u32_t> index_map;
  collect_functions(function, functions, index_map);

  ByteWriter w;

  // Header: magic + version + reserved
  w.write_u8('M'); w.write_u8('S'); w.write_u8('C'); w.write_u8('\0');
  w.write_u8(kMSC_VERSION_MAJOR);
  w.write_u8(kMSC_VERSION_MINOR);
  w.write_u8(0); w.write_u8(0); // reserved

  // Function count
  w.write_u32(static_cast<u32_t>(functions.size()));

  // Serialize each function (inner-first order)
  for (auto* fn : functions) {
    write_function(w, fn, index_map);
  }

  return w.flush(path);
}

// -- Deserialization ---------------------------------------------------------

static ObjFunction* read_function(ByteReader& r,
    const std::vector<ObjFunction*>& fn_table) noexcept {
  auto& vm = VM::get_instance();

  u32_t arity = r.read_u32();
  u32_t upvalue_count = r.read_u32();
  str_t name = r.read_str();
  str_t script_path = r.read_str();

  ObjFunction* fn = vm.allocate<ObjFunction>();
  fn->set_arity(static_cast<int>(arity));
  for (u32_t i = 0; i < upvalue_count; ++i) fn->increment_upvalue_count();

  if (!name.empty()) {
    fn->set_name(vm.copy_string(name.c_str(), name.size()));
  }
  fn->set_script_path(script_path);

  // Chunk: code
  u32_t code_size = r.read_u32();
  auto& code = fn->chunk().code();
  code.resize(code_size);
  if (code_size > 0) {
    const u8_t* code_data = r.read_bytes(code_size);
    std::memcpy(code.data(), code_data, code_size);
  }

  // Chunk: constants
  u32_t const_count = r.read_u32();
  for (u32_t i = 0; i < const_count; ++i) {
    auto tag = static_cast<ConstTag>(r.read_u8());
    switch (tag) {
    case ConstTag::TAG_NIL:
      fn->chunk().add_constant(Value());
      break;
    case ConstTag::TAG_BOOL:
      fn->chunk().add_constant(Value(r.read_u8() != 0));
      break;
    case ConstTag::TAG_NUMBER:
      fn->chunk().add_constant(Value(r.read_f64()));
      break;
    case ConstTag::TAG_INTEGER: {
      u64_t uval = 0;
      for (int shift = 56; shift >= 0; shift -= 8)
        uval |= static_cast<u64_t>(r.read_u8()) << shift;
      fn->chunk().add_constant(Value(static_cast<i64_t>(uval)));
      break;
    }
    case ConstTag::TAG_STRING: {
      str_t s = r.read_str();
      ObjString* str = vm.copy_string(s.c_str(), s.size());
      fn->chunk().add_constant(Value(static_cast<Object*>(str)));
      break;
    }
    case ConstTag::TAG_FUNCTION: {
      u32_t fn_index = r.read_u32();
      fn->chunk().add_constant(Value(static_cast<Object*>(fn_table[fn_index])));
      break;
    }
    }
  }

  // Chunk: line runs
  u32_t run_count = r.read_u32();
  auto& lines = fn->chunk().lines();
  lines.reserve(run_count);
  for (u32_t i = 0; i < run_count; ++i) {
    SourceRun run;
    run.line = r.read_i32();
    run.column = r.read_i32();
    run.token_length = r.read_i32();
    run.count = r.read_i32();
    lines.push_back(run);
  }

  // Inline cache slots (allocate empty slots)
  u32_t ic_count = r.read_u32();
  for (u32_t i = 0; i < ic_count; ++i) fn->add_ic();

  return fn;
}

ObjFunction* deserialize(strv_t path) noexcept {
  // Read entire file
  std::ifstream ifs(str_t(path), std::ios::binary | std::ios::ate);
  if (!ifs) {
    std::cerr << "Could not open bytecode file \"" << path << "\"." << std::endl;
    return nullptr;
  }
  auto file_size = ifs.tellg();
  ifs.seekg(0, std::ios::beg);
  std::vector<u8_t> data(static_cast<sz_t>(file_size));
  ifs.read(reinterpret_cast<char*>(data.data()), file_size);
  if (!ifs) {
    std::cerr << "Failed to read bytecode file \"" << path << "\"." << std::endl;
    return nullptr;
  }

  ByteReader r(data.data(), data.size());

  // Validate header
  if (!r.has(8)) {
    std::cerr << "Invalid .msc file: too short." << std::endl;
    return nullptr;
  }
  if (r.read_u8() != 'M' || r.read_u8() != 'S' || r.read_u8() != 'C' || r.read_u8() != '\0') {
    std::cerr << "Invalid .msc file: bad magic." << std::endl;
    return nullptr;
  }
  u8_t major = r.read_u8();
  u8_t minor = r.read_u8();
  if (major != kMSC_VERSION_MAJOR) {
    std::cerr << "Unsupported .msc version " << static_cast<int>(major)
              << "." << static_cast<int>(minor) << "." << std::endl;
    return nullptr;
  }
  r.read_u8(); r.read_u8(); // reserved

  // Read functions
  u32_t fn_count = r.read_u32();
  std::vector<ObjFunction*> fn_table;
  fn_table.reserve(fn_count);

  for (u32_t i = 0; i < fn_count; ++i) {
    ObjFunction* fn = read_function(r, fn_table);
    if (!fn) return nullptr;
    fn_table.push_back(fn);
  }

  // Last function is the top-level script
  return fn_table.empty() ? nullptr : fn_table.back();
}

} // namespace ms
