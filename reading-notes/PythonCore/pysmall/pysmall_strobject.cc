// Copyright (c) 2018 ASMlover. All rights reserved.
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
#include <iostream>
#include "pysmall_strobject.h"

namespace pysmall {

StrObject::StrObject(const char* v)
  : Object()
  , value_(v) {
  type = set_objecttype();
}

TypeObject* StrObject::set_objecttype(void) {
  static TypeObject _strtype(0, "str");
  _strtype.set_printfunc([this](Object* o) {
        auto* so = dynamic_cast<StrObject*>(o);
        std::cout << so->value_ << std::endl;
      });
  _strtype.set_plusfunc([this](Object* x, Object* y) -> Object* {
        auto* lo = dynamic_cast<StrObject*>(x);
        auto* ro = dynamic_cast<StrObject*>(y);
        auto* r = new StrObject();
        if (r == nullptr) {
          std::cerr << "have no enough memory !!!" << std::endl;
          std::abort();
        }
        else {
          r->value_ = lo->value_ + ro->value_;
        }
        return r;
      });
  _strtype.set_hashfunc([this](Object* o) -> long {
        auto* so = dynamic_cast<StrObject*>(o);
        if (so->hash_value_ != -1)
          return so->hash_value_;

        auto n = static_cast<int>(so->value_.size());
        auto* s = reinterpret_cast<const unsigned char*>(so->value_.c_str());
        auto r = *s << 7;
        while (--n >= 0)
          r = (1000003 * r) ^ *s++;
        r ^= so->value_.size();
        if (r == -1)
          r = -2;
        so->hash_value_ = r;
        return r;
      });
  return &_strtype;
}

}
