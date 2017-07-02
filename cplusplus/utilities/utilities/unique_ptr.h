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
#ifndef __UTIL_UNIQUE_PTR_HEADER_H__
#define __UTIL_UNIQUE_PTR_HEADER_H__


namespace util {


// UniquePtr
// 
// UniquePtr mimics a built-in pointer except that it guaranteed
// deletion of the object pointed to, either on destruction of 
// the UniquePtr or via an explicit Reset().
// UniquePtr it a simple solution for simple needs, use SmartPtr 
// if your needs are more complex.
template <typename T>
class UniquePtr : private UnCopyable {
  T*  ptr_;

  typedef UniquePtr<T>  SelfType;
public:
  explicit UniquePtr(T* p = nullptr) 
    : ptr_(p) {
  }

  ~UniquePtr(void) {
    if (nullptr != ptr_)
      delete ptr_;
  }
public:
  void Reset(T* p = nullptr) {
    SelfType(p).Swap(*this);
  }

  T& operator*(void) const {
    return *ptr_;
  }

  T* operator->(void) const {
    return ptr_;
  }

  T* Get(void) const {
    return ptr_;
  }
private:
  void Swap(UniquePtr& x) {
    std::swap(ptr_, x.ptr_);
  }
};


}


#endif  // __UTIL_UNIQUE_PTR_HEADER_H__
