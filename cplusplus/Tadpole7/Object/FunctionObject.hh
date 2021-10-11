// Copyright (c) 2021 ASMlover. All rights reserved.
//
//  ______             __                  ___
// /\__  _\           /\ \                /\_ \
// \/_/\ \/    __     \_\ \  _____     ___\//\ \      __
//    \ \ \  /'__`\   /'_` \/\ '__`\  / __`\\ \ \   /'__`\
//     \ \ \/\ \L\.\_/\ \L\ \ \ \L\ \/\ \L\ \\_\ \_/\  __/
//      \ \_\ \__/.\_\ \___,_\ \ ,__/\ \____//\____\ \____\
//       \/_/\/__/\/_/\/__,_ /\ \ \/  \/___/ \/____/\/____/
//                             \ \_\
//                              \/_/
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

#include <Object/Object.hh>
#include <Object/StringObject.hh>

namespace Tadpole::Core {

class Chunk;

}

namespace Tadpole::Object {

class FunctionObject final : public BaseObject {
  StringObject* name_{};
  sz_t arity_{};
  sz_t upvalues_count_{};
  Core::Chunk* chunk_{};
public:
  FunctionObject(StringObject* name = nullptr) noexcept;
  virtual ~FunctionObject();

  inline StringObject* name() const noexcept { return name_; }
  inline const char* name_asstr() const noexcept { return name_ ? name_->cstr() : "<tadpole>"; }
  inline void set_name(StringObject* name) noexcept { name_ = name; }
  inline sz_t arity() const noexcept { return arity_; }
  inline sz_t inc_arity() noexcept { return arity_++; }
  inline sz_t upvalues_count() const noexcept { return upvalues_count_; }
  inline sz_t inc_upvalues_count() noexcept { return upvalues_count_++; }
  inline Core::Chunk* chunk() const noexcept { return chunk_; }

  virtual str_t stringify() const override;
  virtual void iter_children(ObjectVisitor&& visitor) override;

  static FunctionObject* create(StringObject* name = nullptr);
};

}
