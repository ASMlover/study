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
#ifndef __UTIL_POSIX_THREAD_HEADER_H__
#define __UTIL_POSIX_THREAD_HEADER_H__


namespace util {


class Thread : private UnCopyable {
  pthread_t           thread_id_;
  SmartPtr<Routiner>  routine_;
public:
  Thread(void) 
    : thread_id_(0)
    , routine_(static_cast<Routiner*>(NULL)) {
  }

  ~Thread(void) {
    Join();
  }

  template <typename R>
  void Create(R routine, void* argument = NULL) {
    routine_ = SmartPtr<Routiner>(new ThreadRoutiner<R>(routine, argument));
    if (NULL == routine_.Get())
      return;

    UTIL_ASSERT(0 == 
        pthread_create(&thread_id_, 0, &Thread::Routine, this));
  }

  void Join(void) {
    if (0 != thread_id_) {
      pthread_join(thread_id_, 0);
      thread_id_ = 0;
    }
  }

  uint32_t GetID(void) const {
    return pthread_self();
  }

  void Kill(void) {
    if (0 != thread_id_) {
      pthread_cancel(thread_id_);
      thread_id_ = 0;
    }
  }
private:
  static void* Routine(void* arg) {
    Thread* self = static_cast<Thread*>(arg);
    if (NULL == self)
      return NULL;

    self->routine_->Run();

    return NULL;
  }
};


}

#endif  // __UTIL_POSIX_THREAD_HEADER_H__
