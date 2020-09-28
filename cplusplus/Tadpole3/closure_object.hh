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

#include "object.hh"
#include "function_object.hh"
#include "upvalue_object.hh"

namespace tadpole {

class ClosureObject final : public BaseObject {
  FunctionObject* fn_{};
  UpvalueObject** upvalues_{};
public:
  ClosureObject(FunctionObject* fn) noexcept;
  virtual ~ClosureObject();

  inline FunctionObject* fn() const noexcept { return fn_; }
  inline sz_t upvalues_count() const noexcept { return fn_->upvalues_count(); }
  inline UpvalueObject* get_upvalue(sz_t i) const noexcept { return upvalues_[i]; }
  inline void set_upvalue(sz_t i, UpvalueObject* u) noexcept { upvalues_[i] = u; }

  virtual str_t stringify() const override;
  virtual void gc_blacken(VM& vm) override;

  static ClosureObject* create(VM& vm, FunctionObject* fn);
};

}
