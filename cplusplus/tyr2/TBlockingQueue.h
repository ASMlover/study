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
#ifndef __TYR_BLOCKINGQUEUE_HEADER_H__
#define __TYR_BLOCKINGQUEUE_HEADER_H__

#include <assert.h>
#include <deque>
#include "TUnCopyable.h"
#include "TMutex.h"
#include "TCondition.h"

namespace tyr {

template <typename T>
class BlockingQueue : private UnCopyable {
  mutable Mutex mtx_;
  Condition not_empty_;
  std::deque<T> queue_;
public:
  BlockingQueue(void)
    : mtx_()
    , not_empty_(mtx_)
    , queue_() {
  }

  size_t size(void) const {
    MutexGuard guard(mtx_);
    return queue_.size();
  }

  void put(T&& x) {
    MutexGuard guard(mtx_);
    queue_.push_back(std::move(x));
    not_empty_.notify();
  }

  T take(void) {
    MutexGuard guard(mtx_);

    while (queue_.empty())
      not_empty_.wait();
    assert(!queue_.empty());

    T front(std::move(queue_.front()));
    queue_.pop_front();

    return front;
  }
};

}

#endif // __TYR_BLOCKINGQUEUE_HEADER_H__
