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
#ifndef __EL_LOCKER_HEADER_H__
#define __EL_LOCKER_HEADER_H__

namespace el {

// Helper class to Lock and Unlock a Locker automatically.
template <typename Locker>
class LockerGuard : private UnCopyable {
  Locker& locker_;
public:
  explicit LockerGuard(Locker& locker)
    : locker_(locker) {
    locker_.Lock();
  }

  ~LockerGuard(void) {
    locker_.Unlock();
  }
};

class DummyLock : private UnCopyable {
public:
  DummyLock(void) {}
  ~DummyLock(void) {}

  inline void Lock(void) {}
  inline void Unlock(void) {}
};

}

#if defined(EUTIL_WIN)
# include "./win/el_win_locker.h"
#elif defined(EUTIL_LINUX)
# include "./posix/el_posix_locker.h"
#endif

#endif  // __EL_LOCKER_HEADER_H__
