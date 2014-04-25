// Copyright (c) 2014 ASMlover. All rights reserved.
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

// AutoArray　
//
// AutoArray extends AutoPtr to arrays. Deletion of the array 
// pointed to is guaranteed, either on destruction of the 
// AutoArray or via on explicit Reset(). Use SmartArray if 
// your needs are more complex.
template <typename T>
class AutoArray : private UnCopyable {
  T* ptr_;

  typedef AutoArray<T> SelfType;
public:
  explicit AutoArray(T* p = nullptr) 
    : ptr_(p) {
  }

  ~AutoArray(void) {
    if (nullptr != ptr_)
      delete [] ptr_;
  }
public:
  inline void Reset(T* p = nullptr) {
    SelfType(p).Swap(*this);
  }

  inline T* Get(void) const {
    return ptr_;
  }

  inline T& operator[](uint32_t i) const {
    return ptr_[i];
  }
private:
  void Swap(AutoArray& x) {
    std::swap(ptr_, x.ptr_);
  }
};

}


#endif  // __EL_AUTO_ARRAY_HEADER_H__
