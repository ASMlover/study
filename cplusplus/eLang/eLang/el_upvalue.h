// Copyright (c) 2015 ASMlover. All rights reserved.
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
#ifndef __EL_UPVALUE_HEADER_H__
#define __EL_UPVALUE_HEADER_H__

#include "el_object.h"

namespace el {

class Upvalue {
  int          stack_index_;
  Value        value_;
  Ref<Upvalue> next_;
public:
  Upvalue(void)
    : stack_index_(-1) {
  }

  explicit Upvalue(int stack_index)
    : stack_index_(stack_index) {
  }

  Value Get(Array<Value>& stack) const;
  void Set(Array<Value>& stack, const Value& value);
  void Close(Array<Value>& stack);

  inline int Index(void) const {
    return stack_index_;
  }

  inline bool IsOpen(void) const {
    return (-1 != stack_index_);
  }

  inline Ref<Upvalue> Next(void) const {
    return next_;
  }

  inline void SetNext(Ref<Upvalue> upvalue) {
    next_ = upvalue;
  }
};

}

#endif  // __EL_UPVALUE_HEADER_H__
