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
#include "base_object.hh"
#include "module_object.hh"

namespace sparrow {

class Value;

struct FunctionDebug {
  char* name{};
  std::vector<int> lines;
};

class FunctionObject final : public BaseObject {
  std::vector<u8_t> codes_;
  std::vector<Value> constants_;
  ModuleObject* module_{};

  u32_t stack_slot_{};
  u32_t upvalues_count_{};
  u8_t argc_{};

  FunctionDebug debug_{};

  FunctionObject(VM& vm, ModuleObject* module, u32_t stack_slot);
  virtual ~FunctionObject(void);
public:
  inline const u8_t* codes(void) const { return codes_.data(); }
  inline const Value* constants(void) const { return constants_.data(); }
  inline ModuleObject* module(void) const { return module_; }
  inline u32_t stack_slot(void) const { return stack_slot_; }
  inline u32_t upvalues_count(void) const { return upvalues_count_; }
  inline u8_t argc(void) const { return argc_; }

  static FunctionObject* create(VM& vm, ModuleObject* module, u32_t slot) {
    return new FunctionObject(vm, module, slot);
  }
};

}
