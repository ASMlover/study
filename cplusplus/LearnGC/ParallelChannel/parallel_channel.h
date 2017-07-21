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

#include <memory>
#include <vector>
#include <Chaos/UnCopyable.h>
#include <Chaos/Concurrent/Mutex.h>
#include <Chaos/Concurrent/Condition.h>

namespace gc {

class BaseObject;
class Sweeper;

class ParallelChannel : private Chaos::UnCopyable {
  using SweeperEntity = std::unique_ptr<Sweeper>;

  static constexpr int kMaxSweepers = 4;
  static constexpr std::size_t kMaxObjects = 4096;
  static constexpr std::size_t kChannelObjects = kMaxObjects / kMaxSweepers;

  int order_{};
  int sweeper_counter_{};
  std::size_t object_counter_{};
  mutable Chaos::Mutex sweeper_mutex_;
  Chaos::Condition sweeper_cond_;
  std::vector<SweeperEntity> sweepers_;
  std::vector<BaseObject*> channels_[kMaxSweepers][kMaxSweepers];

  friend class Sweeper;

  ParallelChannel(void);
  ~ParallelChannel(void);

  void startup_sweepers(void);
  void clearup_sweepers(void);
  int put_in_order(void);
  int fetch_out_order(void);
  void put_into_sweeper(BaseObject* obj);
  void init_marked_objects(int sweeper_id, std::vector<BaseObject*>& objects);
  void acquire_work(int sweeper_id, std::vector<BaseObject*>& objects);
  bool generate_work(int sweeper_id, BaseObject* obj);
  void notify_sweeping(int id, std::size_t remain_count);
public:
  static ParallelChannel& get_instance(void);

  void collect(void);
  BaseObject* put_in(int value);
  BaseObject* put_in(BaseObject* first = nullptr, BaseObject* second = nullptr);
  BaseObject* fetch_out(void);
};

}
