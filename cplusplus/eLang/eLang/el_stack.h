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
#ifndef __EL_STACK_HEADER_H__
#define __EL_STACK_HEADER_H__

namespace el {

template <typename _Tp> class Stack : private UnCopyable {
  Array<_Tp> items_;
public:
  Stack(void)
    : items_() {
  }

  inline void Clear(void) {
    items_.Clear();
  }

  inline bool IsEmpty(void) const {
    return items_.IsEmpty();
  }

  inline int Count(void) const {
    return items_.Count();
  }

  inline int Capacity(void) const {
    return items_.Capacity();
  }

  inline void Push(const _Tp& value) {
    items_.Append(value);
  }

  inline _Tp Pop(void) {
    EL_ASSERT(!IsEmpty(), "Cannot pop an empty stack.");

    _Tp poped = items_[-1];
    items_.RemoveAt(-1);

    return poped;
  }

  inline _Tp& Peek(void) {
    EL_ASSERT(!IsEmpty(), "Cannot peek an empty stack.");

    return items_[-1];
  }

  inline _Tp& operator[](int index) {
    EL_ASSERT_RANGE(index, Count());
    return items_[-1 - index];
  }

  inline const _Tp& operator[](int index) const {
    EL_ASSERT_RANGE(index, Count());
    return items_[-1 - index];
  }
};

}

#endif  // __EL_STACK_HEADER_H__
