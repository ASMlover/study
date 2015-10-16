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
#ifndef __TYR_LOCKER_HEADER_H__
#define __TYR_LOCKER_HEADER_H__

namespace tyr {

template <typename _Mutex>
class LockerGuard : private UnCopyable {
  _Mutex& mutex_;
public:
  explicit LockerGuard(_Mutex& mutex)
    : mutex_(mutex) {
    mutex_.Lock();
  }

  ~LockerGuard(void) tyr_noexcept {
    mutex_.Unlock();
  }

  _Mutex* Mutex(void) const tyr_noexcept {
    return &mutex_;
  }
};

template <typename _Mutex>
class UniqueLock : private UnCopyable {
  _Mutex* mutex_;
  bool    owned_;
public:
  UniqueLock(void) tyr_noexcept
    : mutex_(nullptr)
    , owned_(false) {
  }

  explicit UniqueLock(_Mutex& mutex)
    : mutex_(&mutex)
    , owned_(false) {
    mutex_->Lock();
    owned_ = true;
  }

  ~UniqueLock(void) tyr_noexcept {
    if (owned_)
      mutex_->Unlock();
  }

  UniqueLock(UniqueLock&& other) tyr_noexcept
    : mutex_(other.mutex_)
    , owned_(other.owned_) {
    other.mutex_ = nullptr;
    other.owned_ = false;
  }

  UniqueLock& operator=(UniqueLock&& other) {
    if (&other != this) {
      if (owned_)
        mutex_->Unlock();
      mutex_ = other.mutex_;
      owned_ = other.owned_;
      other.mutex_ = nullptr;
      other.owned_ = false;
    }

    return *this;
  }

  void Lock(void) {
    if (nullptr != mutex_ && !owned_) {
      mutex_->Lock();
      owned_ = true;
    }
  }

  void Unlock(void) {
    if (nullptr != mutex_ && owned_) {
      mutex_->Unlock();
      owned_ = false;
    }
  }

  void Swap(UniqueLock& other) tyr_noexcept {
    std::swap(mutex_, other.mutex_);
    std::swap(owned_, other.owned_);
  }

  _Mutex* Release(void) tyr_noexcept {
    _Mutex* mutex = mutex_;
    mutex_ = nullptr;
    owned_ = false;
    return mutex;
  }

  bool OwnedMutex(void) const tyr_noexcept {
    return owned_;
  }

  explicit operator bool(void) const tyr_noexcept {
    return owned_;
  }

  _Mutex* Mutex(void) const tyr_noexcept {
    return mutex_;
  }
};

class DummyMutex : private UnCopyable {
public:
  void Lock(void) {}
  void Unlock(void) {}
};

}

#if defined(TYR_CPP0X)
# include "tyr_mutex.h"
#else
# if defined(TYR_OS_WIN)
#   include "win/tyr_win_mutex.h"
# elif defined(TYR_OS_LINUX)
#   include "posix/tyr_posix_mutex.h"
# elif defined(TYR_OS_MAC)
#   include "mac/tyr_mac_mutex.h"
# endif
#endif

#endif  // __TYR_LOCKER_HEADER_H__
