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
#ifndef CHAOS_CONCURRENT_THREADLOCAL_H
#define CHAOS_CONCURRENT_THREADLOCAL_H

#include <Chaos/UnCopyable.h>
#include <Chaos/OS/OS.h>

namespace Chaos {

template <typename T>
class ThreadLocal : private UnCopyable {
  Chaos::_Tls_t tls_;

  static void destructor(void* data) {
    T* obj = static_cast<T*>(data);
    if (nullptr != obj)
      delete obj;
  }
public:
  ThreadLocal(void) {
    Chaos::kern_tls_create(&tls_, &ThreadLocal::destructor);
  }

  ~ThreadLocal(void) {
    Chaos::kern_tls_delete(tls_);
  }

  T& get_value(void) {
    T* value = static_cast<T*>(Chaos::kern_tls_getspecific(tls_));
    if (nullptr != value) {
      T* obj = new T();
      Chaos::kern_tls_setspecific(tls_, obj);
      value = obj;
    }
    return *value;
  }
};

}

#endif // CHAOS_CONCURRENT_THREADLOCAL_H
