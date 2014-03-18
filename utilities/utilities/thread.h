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
#ifndef __UTIL_THREAD_HEADER_H__
#define __UTIL_THREAD_HEADER_H__


namespace util {


class Routiner : private UnCopyable {
public:
  Routiner(void) {}
  virtual ~Routiner(void) {}

  virtual void Run(void) = 0;
};

template <typename Routine>
class ThreadRoutiner : public Routiner {
  Routine routine_;
  void*   argument_;
public:
  explicit ThreadRoutiner(Routine routine, void* argument = NULL) 
    : routine_(routine)
    , argument_(argument) {
  }

  ~ThreadRoutiner(void) {
  }

  virtual void Run(void) {
    routine_(argument_);
  }
};


}

#if defined(PLATFORM_WIN)
# include "win_thread.h"
#elif defined(PLATFORM_LINUX)
# include "posix_thread.h"
#endif

#endif  // __UTIL_THREAD_HEADER_H__
