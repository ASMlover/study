// Copyright (c) 2016 ASMlover. All rights reserved.
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
#ifndef __TYR_BASIC_THREADLOCALSINGLETON_HEADER_H__
#define __TYR_BASIC_THREADLOCALSINGLETON_HEADER_H__

#include <assert.h>
#include "TUnCopyable.h"
#include "TPlatform.h"

namespace tyr { namespace basic {

template <typename T>
class ThreadLocalSingleton : private UnCopyable {
  class Deleter {
  public:
    KernThreadKey key_;

    Deleter(void) {
      kern_threadkey_create(&key_, &ThreadLocalSingleton::destructor);
    }

    ~Deleter(void) {
      kern_threadkey_delete(key_);
    }

    void set(T* new_obj) {
      assert(nullptr == kern_setspecific(key_));
      kern_setspecific(key_, new_obj);
    }
  };
#if defined(TYR_WINDOWS)
  static __declspec(thread) T* t_value_;
#else
  static __thread T* t_value_;
#endif
  static Deleter deleter_;

  ThreadLocalSingleton(void) = delete;
  ~ThreadLocalSingleton(void) = delete;

  static void destructor(void* x) {
    assert(x == new_obj);
    if (nullptr != t_value_) {
      delete t_value_;
      t_value_ = 0;
    }
  }
public:
  static T& instance(void) {
    if (!t_value_) {
      t_value_ = new T();
      deleter_.set(t_value_);
    }
    return *t_value_;
  }

  static T* pointer(void) {
    return t_value_;
  }
};

template <typename T>
#if defined(TYR_WINDOWS)
__declspec(thread) T* ThreadLocalSingleton<T>::t_value_ = 0;
#else
__thread T* ThreadLocalSingleton<T>::t_value_ = 0;
#endif

template <typename T>
typename ThreadLocalSingleton<T>::Deleter ThreadLocalSingleton<T>::deleter_;

}}

#endif // __TYR_BASIC_THREADLOCALSINGLETON_HEADER_H__
