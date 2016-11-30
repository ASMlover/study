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
#ifndef CHAOS_CONCURRENT_BOUNDEDBLOCKINGQUEUE_H
#define CHAOS_CONCURRENT_BOUNDEDBLOCKINGQUEUE_H

#include <Chaos/Types.h>
#include <Chaos/Container/CircularBuffer.h>
#include <Chaos/Concurrent/Mutex.h>
#include <Chaos/Concurrent/Condition.h>

namespace Chaos {

template <typename T>
class BoundedBlockingQueue : private UnCopyable {
  mutable Mutex mtx_;
  Condition non_empty_;
  Condition non_full_;
  CircularBuffer<T> queue_;
public:
  explicit BoundedBlockingQueue(size_t capacity)
    : mtx_()
    , non_empty_(mtx_)
    , non_full_(mtx_)
    , queue_(capacity) {
  }

  bool empty(void) const {
    ScopedLock<Mutex> guard(mtx_);
    return queue_.empty();
  }

  bool full(void) const {
    ScopedLock<Mutex> guard(mtx_);
    return queue_.full();
  }

  size_t size(void) const {
    ScopedLock<Mutex> guard(mtx_);
    return queue_.size();
  }

  size_t capacity(void) const {
    ScopedLock<Mutex> guard(mtx_);
    return queue_.capacity();
  }

  void put_in(const T& x) {
    ScopedLock<Mutex> guard(mtx_);

    while (queue_.full())
      non_full_.wait();
    CHAOS_CHECK(!queue_.full(), "BoundedBlockingQueue::put_in(const T&) - queue should be not full");

    queue_.push_back(x);
    non_empty_.notify_one();
  }

  void put_in(T&& x) {
    ScopedLock<Mutex> guard(mtx_);

    while (queue_.full())
      non_full_.wait();
    CHAOS_CHECK(!queue_.full(), "BoundedBlockingQueue::put_in(T&&) - queue should be not full");

    queue_.push_back(std::forward<T>(x));
    non_empty_.notify_one();
  }

  T fetch_out(void) {
    ScopedLock<Mutex> guard(mtx_);

    while (queue_.empty())
      non_empty_.wait();
    CHAOS_CHECK(!queue_.empty(), "BoundedBlockingQueue::fetch_out - queue should be not empty");

    T front(queue_.front());
    queue_.pop_front();
    non_full_.notify_one();

    return front;
  }
};

}

#endif // CHAOS_CONCURRENT_BOUNDEDBLOCKINGQUEUE_H
