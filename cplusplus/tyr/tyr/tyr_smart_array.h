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
#ifndef __TYR_SMART_ARRAY_HEADER_H__
#define __TYR_SMART_ARRAY_HEADER_H__

namespace tyr {

class RefArray : private UnCopyable {
public:
  virtual ~RefArray(void) tyr_noexcept {}
  virtual void Destroy(void) = 0;
};

template <typename T>
class RefArrayDelete : public RefArray {
  T* ptr_;
public:
  explicit RefArrayDelete(T* p) tyr_noexcept
    : ptr_(p) {
  }

  virtual void Destroy(void) override {
    if (nullptr != ptr_)
      delete [] ptr_;
  }
};

template <typename T, typename D>
class RefArrayDestructor : public RefArray {
  T* ptr_;
  D  dtor_;
public:
  RefArrayDestructor(T* p, D d) tyr_noexcept
    : ptr_(p)
    , dtor_(d) {
  }

  virtual void Destroy(void) override {
    if (nullptr != ptr_)
      dtor_(ptr_);
  }
};

// SmartArray
template <typename T>
class SmartArray {
  T*             ptr_;
  RefArray*      ra_;
  AtomicCounter* rc_;

  typedef SmartArray<T> SelfType;
public:
  SmartArray(void) tyr_noexcept
    : ptr_(nullptr)
    , ra_(nullptr)
    , rc_(nullptr) {
  }
};

}

#endif  // __TYR_SMART_ARRAY_HEADER_H__
