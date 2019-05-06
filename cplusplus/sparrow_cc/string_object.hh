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

#include "base_object.hh"

namespace sparrow {

class StringObject final : public BaseObject {
  u32_t hash_code_{};
  sz_t length_{};
  char* chars_{};

  StringObject(VM& vm, const char* s, sz_t n, bool deepcopy = true);
  virtual ~StringObject(void);
public:
  inline u32_t hash_code(void) const { return hash_code_; }
  inline sz_t size(void) const { return length_; }
  inline const char* data(void) const { return chars_; }
  inline const char* c_str(void) const { return chars_; }

  virtual bool is_equal(BaseObject* other) const override;

  static StringObject* create(VM& vm, const char* s, sz_t n) {
    return new StringObject(vm, s, n);
  }

  static StringObject* create(VM& vm, const str_t& s) {
    return new StringObject(vm, s.c_str(), s.size());
  }
};

}
