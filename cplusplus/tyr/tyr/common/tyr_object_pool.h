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
#ifndef __TYR_OBJECT_POOL_HEADER_H__
#define __TYR_OBJECT_POOL_HEADER_H__

namespace tyr {

template <typename T, typename Mutex = SpinlockMutex>
class ObjectPool : private UnCopyable {
  enum {DEFAULT_OBJCOUNT = 16};
  typedef tyr::SmartPtr<T> TSharesPtr;

  uint32_t                chunk_size_;
  Mutex                   mutex_;
  std::queue<T*>          free_queue_;
  std::vector<TSharesPtr> all_objects_;
public:
  explicit ObjectPool(uint32_t chunk_size = DEFAULT_OBJCOUNT)
    throw(std::invalid_argument, std::bad_alloc)
    : chunk_size_(chunk_size) {
    if (0 == chunk_size_)
      throw std::invalid_argument("chunk size must be positive.");

    AllocateChunk();
  }

  T& AcquireObject(void) {
    UniqueLock<Mutex> lock(mutex_);

    if (free_queue_.empty())
      AllocateChunk();

    T* obj = free_queue_.front();
    free_queue_.pop();

    return *obj;
  }

  void ReleaseObject(T& obj) {
    UniqueLock<Mutex> lock(mutex_);

    free_queue_.push(&obj);
  }
private:
  bool AllocateChunk(void) {
    TSharesPtr new_objects(
        new T[chunk_size_], std::default_delete<T []>());
    if (!new_objects)
      return false;

    all_objects_.push_back(new_objects);
    T* chunks = new_objects.get();
    for (uint32_t i = 0; i < chunk_size_; ++i)
      free_queue_.push(&chunks[i]);

    return true;
  }
};

}

#endif  // __TYR_OBJECT_POOL_HEADER_H__
