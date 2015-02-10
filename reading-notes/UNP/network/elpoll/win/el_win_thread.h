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
#ifndef __EL_WIN_THREAD_HEADER_H__
#define __EL_WIN_THREAD_HEADER_H__

#ifndef _MT
# error "requires /MTd, /MT, /MDd or /MD compiler options ..."
#endif

namespace el {

class Thread : private UnCopyable {
  HANDLE       start_event_;
  HANDLE       thread_;
  RoutinerType routiner_;
  void*        argument_;
public:
  Thread(void)
    : start_event_(nullptr)
    , thread_(nullptr)
    , routiner_(nullptr)
    , argument_(nullptr) {
  }

  ~Thread(void) {
    Join();
  }

  void Create(const RoutinerType& routiner, void* argument = nullptr) {
    routiner_ = routiner;
    argument_ = argument;

    start_event_ = CreateEvent(nullptr, TRUE, FALSE, nullptr);
    EL_ASSERT(nullptr != start_event_, "create event failed ...");

    thread_ = reinterpret_cast<HANDLE>(_beginthreadex(
          nullptr, 0, &Thread::Routiner, this, 0, nullptr));
    EL_ASSERT(nullptr != thread_, "create thread failed ...");

    WaitForSingleObject(start_event_, INFINITE);
    CloseHandle(start_event_);
  }

  void Join(void) {
    if (nullptr != thread_) {
      WaitForSingleObject(thread_, INFINITE);

      CloseHandle(thread_);
      thread_ = nullptr;
    }
  }
private:
  static UINT WINAPI Routiner(void* argument) {
    Thread* self = static_cast<Thread*>(argument);
    if (nullptr == self)
      return 0;

    SetEvent(self->start_event_);

    if (nullptr != self->routiner_)
      self->routiner_(self->argument_);

    return 0;
  }
};

}

#endif  // __EL_WIN_THREAD_HEADER_H__
