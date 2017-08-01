// Copyright (c) 2017 ASMlover. All rights reserved.
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
#pragma once

#include <atomic>
#include <vector>
#include <Chaos/Types.h>
#include <Chaos/Concurrent/Mutex.h>
#include <Chaos/Concurrent/Condition.h>
#include <Chaos/Concurrent/Thread.h>

namespace gc {

class BaseObject;

class ConcurrencyCopy {
  byte_t* heaptr_{};
  byte_t* fromspace_{};
  byte_t* tospace_{};
  byte_t* allocptr_{};
  bool running_{true};
  bool copying_{};
  std::atomic<std::size_t> object_counter_{};
  mutable Chaos::Mutex mutex_;
  mutable Chaos::Mutex copy_mutex_;
  Chaos::Condition copy_cond_;
  std::atomic<bool> collecting_{};
  mutable Chaos::Mutex collect_mutex_;
  Chaos::Condition collect_cond_;
  Chaos::Thread thread_;
  std::vector<BaseObject*> roots_;
  std::vector<BaseObject*> worklist_;
  static constexpr std::size_t kSemispaceSize = 512 << 9;

  ConcurrencyCopy(void);
  ~ConcurrencyCopy(void);

  void* alloc(std::size_t n);
  void concurrency_closure(void);
  BaseObject* forward(BaseObject* from_ref);
  BaseObject* copy(BaseObject* from_ref);
  void collecting(void);
public:
  static ConcurrencyCopy& get_instance(void);

  void collect(void);
  BaseObject* put_in(int value);
  BaseObject* put_in(BaseObject* first, BaseObject* second);
  BaseObject* fetch_out(void);
};

}
