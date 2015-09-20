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
#ifndef __EL_QUEUE_HEADER_H__
#define __EL_QUEUE_HEADER_H__

namespace el {

template <typename _Tp, int _N = 1>
class Queue : private UnCopyable {
  int head_;
  int count_;
  _Tp items_[_N];
public:
  Queue(void)
    : head_(0)
    , count_(0) {
    EL_ASSERT(_N > 0, "Queue capacity must large than zero.");
  }

  inline void Clear(void) {
    head_ = 0;
    count_ = 0;
  }

  inline int Count(void) const {
    return count_;
  }

  inline int Capacity(void) const {
    return _N;
  }

  inline bool IsEmpty(void) const {
    return (0 == count_);
  }

  inline void Enqueue(const _Tp& value) {
    EL_ASSERT(count_ < _N, "Cannot enqueue a full queue.");

    items_[head_] = value;
    head_ = Wrap(head_ + 1);
    ++count_;
  }

  inline _Tp Dequeue(void) {
    EL_ASSERT(count_ > 0, "Cannot dequeue an empty queue.");

    int tail = Wrap(head_ - count_);
    _Tp dequeued = items_[tail];
    items_[tail] = _Tp();

    --count_;
    return dequeued;
  }

  inline _Tp& operator[](int index) {
    EL_ASSERT_RANGE(index, count_);

    return items_[Wrap(head_ - count_ + index)];
  }

  inline const _Tp& operator[](int index) const {
    EL_ASSERT_RANGE(index, count_);

    return items_[Wrap(head_ - count_ + index)];
  }
private:
  inline int Wrap(int index) const {
    return (index + _N) % _N;
  }
};

}

#endif  // __EL_QUEUE_HEADER_H__
