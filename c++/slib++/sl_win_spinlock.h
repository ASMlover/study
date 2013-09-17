//! Copyright (c) 2013 ASMlover. All rights reserved.
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
#ifndef __SL_WIN_SPINLOCK_HEADER_H__
#define __SL_WIN_SPINLOCK_HEADER_H__

#include <windows.h>
#include "sl_noncopyable.h"


namespace sl {

class spinlock_t : noncopyable {
  CRITICAL_SECTION spinlock_;
public:
  spinlock_t(void)
  {
    InitializeCriticalSectionAndSpinCount(&spinlock_, 4000);
  }

  ~spinlock_t(void)
  {
    DeleteCriticalSection(&spinlock_);
  }

  void 
  lock(void)
  {
    if ((DWORD)spinlock_.OwningThread == GetCurrentThreadId())
      return;

    EnterCriticalSection(&spinlock_);
  }

  int 
  trylock(void)
  {
    if ((DWORD)spinlock_.OwningThread == GetCurrentThreadId())
      return 0;

    if (TryEnterCriticalSection(&spinlock_))
      return 0;
    else 
      return -1;
  }

  void 
  unlock(void)
  {
    LeaveCriticalSection(&spinlock_);
  }
};

}

#endif  //! __SL_WIN_SPINLOCK_HEADER_H__
