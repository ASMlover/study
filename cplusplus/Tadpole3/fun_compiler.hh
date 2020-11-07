// Copyright (c) 2020 ASMlover. All rights reserved.
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

#include <functional>
#include <vector>
#include "common.hh"
#include "parser.hh"

namespace tadpole {

enum class FunType {
  FUNCTION,
  TOPLEVEL,
};

class FunctionObject;

class FunCompiler final : private UnCopyable {
  using ErrorFn = std::function<void (const str_t&)>;

  FunCompiler* enclosing_{};
  FunctionObject* fn_{};
  FunType fn_type_{};

  int scope_depth_{};

  std::vector<LocalVar> locals_;
  std::vector<Upvalue> upvalues_;
public:
  FunCompiler() noexcept {}

  FunCompiler(FunCompiler* enclosing,
      FunctionObject* fn, FunType fn_type, int scope_depth = 0) noexcept
    : enclosing_(enclosing), fn_(fn), fn_type_(fn_type), scope_depth_(scope_depth) {
  }

  inline void set_compiler(FunCompiler* enclosing,
      FunctionObject* fn, FunType fn_type, int scope_depth = 0) noexcept {
    enclosing_ = enclosing;
    fn_ = fn;
    fn_type_ = fn_type;
    scope_depth_ = scope_depth;
  }

  inline FunCompiler* enclosing() const noexcept { return enclosing_;}
  inline FunctionObject* fn() const noexcept { return fn_; }
  inline FunType fn_type() const noexcept { return fn_type_; }
  inline int scope_depth() const noexcept { return scope_depth_; }
  inline void set_scope_depth(int depth) noexcept { scope_depth_ = depth; }
  inline int locals_count() const noexcept { return as_type<int>(locals_.size()); }
  inline LocalVar& get_local(sz_t i) noexcept { return locals_[i]; }
  inline const LocalVar& get_local(sz_t i) const noexcept { return locals_[i]; }
  inline LocalVar& peek_local() noexcept { return locals_.back(); }
  inline const LocalVar& peek_local() const noexcept { return locals_.back(); }
  inline void append_local(const LocalVar& var) noexcept { locals_.push_back(var); }
  inline Upvalue& get_upvalue(sz_t i) noexcept { return upvalues_[i]; }
  inline const Upvalue& get_upvalue(sz_t i) const noexcept { return upvalues_[i]; }
  inline void append_upvalue(const Upvalue& u) noexcept { upvalues_.push_back(u); }
};

}
