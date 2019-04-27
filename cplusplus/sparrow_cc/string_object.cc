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
#include "string_object.hh"
#include "vm.hh"

namespace sparrow {

// fnv-1a hashing
inline u32_t hash_string(const char* s, sz_t n) {
  u32_t hash_code = 2166136261;
  for (sz_t i = 0; i < n; ++i) {
    hash_code ^= s[i];
    hash_code *= 16777619;
  }
  return hash_code;
}

StringObject::StringObject(VM& vm, const char* s, sz_t n, bool deepcopy)
  : BaseObject(vm, ObjType::STRING, vm.strcls())
  , length_(n) {
  if (deepcopy) {
    chars_ = new char[length_ + 1];
    memcpy(chars_, s, length_);
    chars_[length_] = 0;
  }
  else {
    chars_ = const_cast<char*>(s);
  }
  hash_code_ = hash_string(s, n);
}

StringObject::~StringObject(void) {
  if (chars_ != nullptr)
    delete [] chars_;
}

}
