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
#ifndef __TYR_THREADLOCAL_HEADER_H__
#define __TYR_THREADLOCAL_HEADER_H__

#include <pthread.h>
#include "TUnCopyable.h"
#include "TMutex.h"

namespace tyr {

template <typename T>
class ThreadLocal : private UnCopyable {
  pthread_key_t pkey_;

  static void destructor(void* x) {
    T* obj = static_cast<T*>(x);
    if (nullptr != obj)
      delete obj;
  }
public:
  ThreadLocal(void) {
    pthread_key_create(&pkey_, &ThreadLocal::destructor);
  }

  ~ThreadLocal(void) {
    pthread_key_delete(pkey_);
  }

  T& value(void) {
    T* val = static_cast<T*>(pthread_getspecific(pkey_));
    if (!val) {
      T* new_obj = new T();
      pthread_setspecific(pkey_, new_obj);
      val = new_obj;
    }
    return *val;
  }
};

}

#endif // __TYR_THREADLOCAL_HEADER_H__
