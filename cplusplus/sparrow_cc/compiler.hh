// Copyright (c) 2019 ASMlover. All rights reserved.
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

#include <vector>
#include "common.hh"
#include "base_object.hh"

namespace sparrow {

struct Upvalue {
  int index{};
  bool is_local{};

  Upvalue(int arg_index, bool arg_islocal = false)
    : index(arg_index), is_local(arg_islocal) {
  }
};

struct LocalVar {
  str_t name;
  int scope_depth{};
  bool is_upvalue{};

  LocalVar(const str_t& arg_name, int arg_depth, bool arg_upvalue = false)
    : name(arg_name), scope_depth(arg_depth), is_upvalue(arg_upvalue) {
  }
};

enum class SignatureType {
  CTOR,
  METHOD,
  GETTER,
  SETTER,
  SUBSCRIPT, // subscript
  ASS_SUBSCRIPT, // ass_subscript
};

struct Signature {
  SignatureType type{};
  str_t name;
  int argc{};

  Signature(SignatureType arg_type, const str_t& arg_name, int arg_num = 0)
    : type(arg_type), name(arg_name), argc(arg_num) {
  }
};

struct Loop {
  int cond_jump{};
  int body_jump{};
  int exit_jump{};
  int scope_depth{};

  Loop(int cond, int body, int exit, int depth = 0)
    : cond_jump(cond), body_jump(body), exit_jump(exit), scope_depth(depth) {
  }
};

struct ClassInfo {
  StringObject* name{};
  bool is_static{};
  std::vector<Value> fields;
  std::vector<int> methods;
  std::vector<int> static_methods;
  Signature* signature{};
};

class Compiler : private UnCopyable {
public:
  int def_modvar(VM& vm,
      ModuleObject* mod, const str_t& name, const Value& value);
};

}
