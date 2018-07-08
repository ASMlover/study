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
#include "pysmall_intobject.h"

namespace pysmall {

IntObject::IntObject(int v)
  : Object()
  , value_(v) {
}

TypeObject* IntObject::set_objecttype(void) {
  static TypeObject _inttype(0, "int");
  _inttype.set_printfunc([this](Object* o) {
        auto io = dynamic_cast<IntObject*>(o);
        std::cout << io->value_ << std::endl;
      });
  _inttype.set_plusfunc([this](Object* x, Object* y) -> Object* {
        auto* lo = dynamic_cast<IntObject*>(x);
        auto* ro = dynamic_cast<IntObject*>(y);
        auto* r = new IntObject();
        if (r == nullptr) {
          std::cerr << "have no enough memory !!!" << std::endl;
          std::abort();
        }
        else {
          r->value_ = lo->value_ + ro->value_;
        }

        return r;
      });
  _inttype.set_hashfunc([this](Object* o) -> long {
        return static_cast<long>(dynamic_cast<IntObject*>(o)->value_);
      });
  return &_inttype;
}

}
