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
#ifndef __EL_AUTO_ARRAY_HEADER_H__
#define __EL_AUTO_ARRAY_HEADER_H__

namespace el {

template <typename T>
class AutoArray {
  T* array_;
public:
  explicit AutoArray(T* array)
    : array_(array) {
  }

  ~AutoArray(void) {
    Release();
  }

  inline void Release(void) {
    delete [] array_;
    array_ = nullptr;
  }

  inline T* Get(void) const {
    return array_;
  }

  inline T& operator[](int index) {
    return array_[index];
  }

  inline const T& operator[](int index) const {
    return array_[index];
  }

  AutoArray<T>& operator=(const AutoArray<T>& other) {
    if (&other != this)
      Reset(other.array_);

    return *this;
  }

  inline void Reset(T* array) {
    Release();
    array_ = array;
  }
};

}

#endif  // __EL_AUTO_ARRAY_HEADER_H__
