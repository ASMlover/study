// Copyright (c) 2017 ASMlover. All rights reserved.
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
#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <vector>
#include <Chaos/UnCopyable.h>
#include <Chaos/Concurrent/Mutex.h>
#include <Chaos/Concurrent/Condition.h>

namespace gc {

class BaseObject;
class Sweeper;

using ForwadingCallback = std::function<void (BaseObject*)>;

class ParallelCopy : private Chaos::UnCopyable {
  using SweeperEntity = std::unique_ptr<Sweeper>;

  int order_{};
  std::atomic<int> tracing_counter_{};
  int sweeper_counter_{};
  std::size_t object_counter_{};
  mutable Chaos::Mutex sweeper_mutex_;
  Chaos::Condition sweeper_cond_;
  std::vector<SweeperEntity> sweepers_;
  static constexpr int kMaxSweepers = 4;

  friend class Sweeper;

  bool is_traced(void) const {
    return tracing_counter_ >= kMaxSweepers;
  }

  ParallelCopy(void);
  ~ParallelCopy(void);

  void startup_sweepers(void);
  void clearup_sweepers(void);
  int put_in_order(void);
  int fetch_out_order(void);
  void generate_work(int sweeper_id,
      BaseObject* from_ref, ForwadingCallback&& cb);
  void notify_traced(int sweeper_id);
  void notify_collected(int sweeper_id, std::size_t alive_count);
public:
  static ParallelCopy& get_instance(void);

  void collect(void);
  BaseObject* put_in(int value);
  BaseObject* put_in(BaseObject* first, BaseObject* second);
  BaseObject* fetch_out(void);
};

}
