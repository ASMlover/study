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

namespace tadpole {

class StringObject final : public BaseObject {
  sz_t size_{};
  char* data_{};
  u32_t hash_{};
public:
  StringObject(const char* s, sz_t n, u32_t h, bool move_owner = false) noexcept;
  virtual ~StringObject();

  inline sz_t size() const noexcept { return size_; }
  inline const char* data() const noexcept { return data_; }
  inline const char* cstr() const noexcept { return data_; }

  virtual bool is_truthy() const override;
  virtual str_t stringify() const override;

  static StringObject* create(VM& vm, const char* s, sz_t n);
  static StringObject* concat(VM& vm, StringObject* s1, StringObject* s2);

  template <typename N> static StringObject* create(VM& vm, const char* s, N n) {
    return create(vm, s, as_type<sz_t>(n));
  }

  static StringObject* create(VM& vm, const str_t& s) {
    return create(vm, s.data(), s.size());
  }

  static StringObject* create(VM& vm, strv_t s) {
    return create(vm, s.data(), s.size());
  }
};

}
