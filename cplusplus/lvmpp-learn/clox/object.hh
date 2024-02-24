// Copyright (c) 2023 ASMlover. All rights reserved.
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
#pragma once

#include "common.hh"
#include "object_helper.hh"
#include "value.hh"

namespace clox {

class Chunk;

class Obj : private UnCopyable {
  ObjType type_;
  bool is_marked_{};

public:
  Obj(ObjType type) noexcept : type_{type} {}
  virtual ~Obj() {}

  inline ObjType type() const noexcept { return type_; }
  inline bool is_type(ObjType type) const noexcept { return type_ == type; }
  inline bool is_marked() const noexcept { return is_marked_; }
  inline void set_marked(bool is_marked = true) noexcept { is_marked_ = is_marked; }

  virtual bool is_equal(Obj* x) const { return this == x; }
  virtual str_t stringify() const { return "<object>"; }
  virtual void blacken() {}

  ObjString* as_string() noexcept;
  cstr_t as_cstring() noexcept;
  ObjFunction* as_function() noexcept;
  ObjNative* as_native() noexcept;
  ObjClosure* as_closure() noexcept;
  ObjUpvalue* as_upvalue() noexcept;
};

template <typename T, typename U>
inline bool is_same_type(T* x, U* y) noexcept { return x->type() == y->type(); }

class ObjString final : public Obj {
  int length_{};
  char* chars_{};
  u32_t hash_{};
public:
  ObjString(const char* chars, int length, u32_t hash) noexcept;
  virtual ~ObjString();

  inline int length() const noexcept { return length_; }
  inline const char* data() const noexcept { return chars_; }
  inline const char* cstr() const noexcept { return chars_; }

  inline bool is_equal_to(ObjString* s) const noexcept { return this == s || is_equal_to(s->chars_); }
  inline bool is_equal_to(const str_t& s) const noexcept { return s.compare(chars_) == 0; }
  inline bool is_equal_to(strv_t s) const noexcept { return s.compare(chars_) == 0; }
  inline bool is_equal_to(const char* s) const noexcept { return std::memcmp(chars_, s, length_) == 0; }

  virtual bool is_equal(Obj* r) const override;
  virtual str_t stringify() const override;

  static ObjString* create(const char* chars, int length);
  static ObjString* concat(ObjString* a, ObjString* b);

  template <typename N> static ObjString* create(const char* chars, N length) {
    return create(chars, as_type<int>(length));
  }

  static ObjString* create(const str_t& s) { return create(s.data(), s.size()); }
  static ObjString* create(strv_t s) { return create(s.data(), s.size()); }
};

class ObjFunction final : public Obj {
  int arity_{};
  int upvalue_count_{};
  Chunk* chunk_{};
  ObjString* name_{};
public:
  ObjFunction(int upvalue_count = 0) noexcept;
  virtual ~ObjFunction();

  inline int arity() const noexcept { return arity_; }
  inline int inc_arity() noexcept { return arity_++; }
  inline int upvalue_count() const noexcept { return upvalue_count_; }
  inline int inc_upvalue_count() noexcept { return upvalue_count_++; }
  inline Chunk* chunk() const noexcept { return chunk_; }
  inline ObjString* name() const noexcept { return name_; }
  inline const char* name_ascstr() const noexcept { return name_ ? name_->cstr() : ""; }
  inline void set_name(ObjString* name) noexcept { name_ = name; }

  virtual str_t stringify() const override;
  virtual void blacken() override;

  static ObjFunction* create(ObjString* name = nullptr);
};

using NativeFn = std::function<Value (int arg_count, Value* args)>;
class ObjNative final : public Obj {
  NativeFn function_{};
public:
  ObjNative(NativeFn&& function) noexcept;
  virtual ~ObjNative();

  inline const NativeFn& function() const noexcept { return function_; }

  virtual str_t stringify() const override;

  static ObjNative* create(NativeFn&& function);
};

class ObjClosure final : public Obj {
  ObjFunction* function_{};
  ObjUpvalue** upvalues_{};
  int upvalue_count_{};
public:
  ObjClosure(ObjFunction* function) noexcept;
  virtual ~ObjClosure();

  inline ObjFunction* function() const noexcept { return function_; }
  inline int upvalue_count() const noexcept { return upvalue_count_; }
  inline ObjUpvalue* get_upvalue(int i) const noexcept { return upvalues_[i]; }
  inline void set_upvalue(int i, ObjUpvalue* upvalue) noexcept { upvalues_[i] = upvalue; }

  virtual str_t stringify() const override;

  static ObjClosure* create(ObjFunction* function);
};

class ObjUpvalue final : public Obj {
  Value* location_{};
  Value closed_{};
  ObjUpvalue* next_{};
public:
  ObjUpvalue(Value* slot) noexcept;
  virtual ~ObjUpvalue();

  inline Value* location() const noexcept { return location_; }
  inline void set_location(const Value& location) noexcept { *location_ = location; }
  inline Value& closed() noexcept { return closed_; }
  inline const Value& closed() const noexcept { return closed_; }
  inline Value* closed_asptr() const noexcept { return const_cast<Value*>(&closed_); }
  inline Value& closed_ref() noexcept { return closed_; }
  inline const Value& closed_ref() const noexcept { return closed_; }
  inline void set_closed(const Value& closed) noexcept { closed_ = closed; }
  inline void set_closed(Value* closed) noexcept { closed_ = *closed; }
  inline ObjUpvalue* next() const noexcept { return next_; }
  inline void set_next(ObjUpvalue* upvalue) noexcept { next_ = upvalue; }

  virtual str_t stringify() const override;

  static ObjUpvalue* create(Value* slot);
};

}
