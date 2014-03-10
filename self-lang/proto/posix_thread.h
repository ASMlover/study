//! Copyright (c) 2014 ASMlover. All rights reserved.
//!
//! Redistribution and use in source and binary forms, with or without
//! modification, are permitted provided that the following conditions
//! are met:
//!
//!  * Redistributions of source code must retain the above copyright
//!    notice, this list ofconditions and the following disclaimer.
//!
//!  * Redistributions in binary form must reproduce the above copyright
//!    notice, this list of conditions and the following disclaimer in
//!    the documentation and/or other materialsprovided with the
//!    distribution.
//!
//! THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//! "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//! LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
//! FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
//! COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//! INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
//! BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//! LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//! CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
//! LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
//! ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//! POSSIBILITY OF SUCH DAMAGE.
#ifndef __POSIX_THREAD_HEADER_H__
#define __POSIX_THREAD_HEADER_H__

class Thread : private UnCopyable {
  pthread_t          thread_id_;
  SmartPtr<Routiner> routiner_;
public:
  explicit Thread(void)
    : thread_id_(0)
    , routiner_(static_cast<Routiner*>(NULL))
  {
  }

  ~Thread(void)
  {
    Stop();
  }

  template <typename R>
  inline void Start(R r, void* arg = NULL)
  {
    routiner_ = SmartPtr<Routiner>(new ThreadRoutine<R, void*>(r, arg));
    PROTO_ASSERT(NULL != routiner_.Get());

    PROTO_ASSERT(0 == 
        pthread_create(&thread_id_, NULL, &Thread::Routine, this));
  }

  inline void Stop(void) 
  {
    if (0 != thread_id_) {
      PROTO_ASSERT(0 == pthread_join(thread_id_, NULL));
      thread_id_ = 0;
    }
  }
private:
  static void* Routine(void* arg)
  {
    Thread* self = static_cast<Thread*>(arg);
    if (NULL == self)
      return NULL;

    self->routiner_->Call();

    return NULL;
  }
};

#endif  //! __POSIX_THREAD_HEADER_H__
