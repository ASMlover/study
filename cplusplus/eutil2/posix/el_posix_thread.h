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
#ifndef __EL_POSIX_THREAD_HEADER_H__
#define __EL_POSIX_THREAD_HEADER_H__

namespace el {

class Thread : private UnCopyable {
  pthread_t    thread_id_;
  RoutinerType routine_;
  void*        argument_;
public:
  Thread(void) 
    : thread_id_(0)
    , routine_(nullptr) 
    , argument_(nullptr) {
  }

  ~Thread(void) {
    Join();
  }

  void Create(const RoutinerType& routine, void* argument = nullptr) {
    routine_ = routine;
    argument_ = argument;

    EL_ASSERT(0 == pthread_create(
          &thread_id_, nullptr, &Thread::Routine, this));
  }

  void Join(void) {
    if (0 != thread_id_) {
      EL_ASSERT(0 == pthread_join(thread_id_, 0));

      thread_id_ = 0;
    }
  }
private:
  static void* Routine(void* argument) {
    Thread* self = static_cast<Thread*>(argument);
    if (nullptr == self)
      return nullptr;

    if (nullptr != self->routine_)
      self->routine_(self->argument_);

    return nullptr;
  }
};

}

#endif  // __EL_POSIX_THREAD_HEADER_H__
