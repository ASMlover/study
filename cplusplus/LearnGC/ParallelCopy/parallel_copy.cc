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
#include <iostream>
#include <queue>
#include <Chaos/Types.h>
#include <Chaos/Concurrent/Thread.h>
#include "object.h"
#include "parallel_copy.h"

namespace gc {

struct ForwardNode {
  BaseObject* from_ref{};
  ForwadingCallback callback{};

  ForwardNode(void) {}

  ForwardNode(BaseObject* ref, ForwadingCallback&& cb)
    : from_ref(ref)
    , callback(std::move(cb)) {
  }
};

class Sweeper : private Chaos::UnCopyable {
  int id_{};
  bool running_{true};
  bool sweeping_{};
  mutable Chaos::Mutex mutex_;
  Chaos::Condition cond_;
  Chaos::Thread thread_;
  byte_t* heaptr_{};
  byte_t* fromspace_{};
  byte_t* tospace_{};
  byte_t* allocptr_{};
  byte_t* scanptr_{};
  std::size_t object_counter_{};
  std::vector<BaseObject*> roots_;
  mutable Chaos::Mutex forward_mutex_;
  std::queue<ForwardNode> forward_workers_;
  static constexpr std::size_t kSemispaceSize = 100 << 9;

  void worklist_init(void) { scanptr_ = allocptr_; }
  bool worklist_empty(void) const { return scanptr_ == allocptr_; }
  void worklist_put(BaseObject* /*obj*/) {}

  BaseObject* worklist_fetch(void) {
    auto* ref = as_object(scanptr_);
    scanptr_ += ref->get_size();
    return ref;
  }

  BaseObject* copy(BaseObject* from_ref, bool with_worklist = false) {
    auto* p = allocptr_;
    allocptr_ += from_ref->get_size();

    BaseObject* to_ref{};
    if (from_ref->is_int())
      to_ref = new (p) Int(std::move(*Chaos::down_cast<Int*>(from_ref)));
    else if (from_ref->is_pair())
      to_ref = new (p) Pair(std::move(*Chaos::down_cast<Pair*>(from_ref)));
    CHAOS_CHECK(to_ref != nullptr, "copy object failed");
    from_ref->set_forward(to_ref);
    if (with_worklist)
      worklist_put(to_ref);

    return to_ref;
  }

  BaseObject* forward(BaseObject* from_ref) {
    auto* to_ref = from_ref->forward();
    if (to_ref == nullptr) {
      to_ref = copy(from_ref, true);
      ++object_counter_;
    }
    return Chaos::down_cast<BaseObject*>(to_ref);
  }

  void* alloc(std::size_t n) {
    if (allocptr_ + n > tospace_ + kSemispaceSize)
      ParallelCopy::get_instance().collect();
    CHAOS_CHECK(allocptr_ + n <= tospace_ + kSemispaceSize,
        "allocating failed");

    auto* p = allocptr_;
    allocptr_ += n;
    return p;
  }

  void sweeper_closure(void) {
    while (running_) {
      {
        Chaos::ScopedLock<Chaos::Mutex> g(mutex_);
        while (running_ && !sweeping_)
          cond_.wait();
        if (!running_)
          break;
      }

      // flip fromspace and tospace
      std::swap(fromspace_, tospace_);
      allocptr_ = tospace_;

      object_counter_ = 0;
      worklist_init();
      for (auto& obj : roots_)
        obj = forward(obj);

      while (!worklist_empty()) {
        auto* ref = worklist_fetch();
        if (ref->is_pair()) {
          auto* pair = Chaos::down_cast<Pair*>(ref);
          auto* first = pair->first();
          if (first != nullptr) {
            if (!in_sweeper(first)) {
              ParallelCopy::get_instance().generate_work(id_, first,
                  [pair](BaseObject* ref) { pair->set_first(ref); });
            }
            else {
              pair->set_first(forward(first));
            }
          }

          auto* second = pair->second();
          if (second != nullptr) {
            if (!in_sweeper(second)) {
              ParallelCopy::get_instance().generate_work(id_, second,
                  [pair](BaseObject* ref) { pair->set_second(ref); });
            }
            else {
              pair->set_second(forward(second));
            }
          }
        }
      }
      ParallelCopy::get_instance().notify_traced(id_);

      while (true) {
        if (ParallelCopy::get_instance().is_traced()
            && forward_workers_.empty())
          break;

        if (!forward_workers_.empty()) {
          ForwardNode node;
          {
            Chaos::ScopedLock<Chaos::Mutex> g(forward_mutex_);
            node = forward_workers_.front();
            forward_workers_.pop();
          }
          auto* to_ref = forward(node.from_ref);
          node.callback(to_ref);
        }
      }

      if (sweeping_) {
        sweeping_ = false;
        ParallelCopy::get_instance().notify_collected(id_, object_counter_);
      }
    }
  }
public:
  explicit Sweeper(int id)
    : id_(id)
    , mutex_()
    , cond_(mutex_)
    , thread_([this]{ sweeper_closure(); }) {
    thread_.start();

    heaptr_ = new byte_t[kSemispaceSize * 2];
    CHAOS_CHECK(heaptr_ != nullptr, "create semispace failed");

    fromspace_ = heaptr_ + kSemispaceSize;
    tospace_ = heaptr_;
    allocptr_ = tospace_;
  }

  ~Sweeper(void) {
    stop();
    thread_.join();

    delete [] heaptr_;
  }

  int get_id(void) const { return id_; }
  void stop(void) { running_ = false; cond_.notify_one(); }
  void sweeping(void) { sweeping_ = true; cond_.notify_one(); }

  bool in_sweeper(BaseObject* obj) {
    auto* p = as_pointer(obj);
    return p >= heaptr_ && p < (heaptr_ + kSemispaceSize * 2);
  }

  bool need_moving(BaseObject* obj) {
    auto* to_ref = obj->forward();
    auto* p = as_pointer(obj);
    if (to_ref == nullptr && (p < tospace_ || p > tospace_ + kSemispaceSize))
      return true;
    return false;
  }

  void put_in_forwarding(BaseObject* from_ref, ForwadingCallback&& cb) {
    Chaos::ScopedLock<Chaos::Mutex> g(forward_mutex_);
    if (need_moving(from_ref) && !from_ref->is_generated()) {
      forward_workers_.push(ForwardNode(from_ref, std::move(cb)));
      from_ref->set_generated();
    }
  }

  BaseObject* put_in(int value) {
    auto* obj = new (alloc(sizeof(Int))) Int();
    obj->set_value(value);

    roots_.push_back(obj);
    ++object_counter_;

    return obj;
  }

  BaseObject* put_in(BaseObject* first, BaseObject* second) {
    auto* obj = new (alloc(sizeof(Pair))) Pair();
    if (first != nullptr)
      obj->set_first(first);
    if (second != nullptr)
      obj->set_second(second);

    roots_.push_back(obj);
    ++object_counter_;

    return obj;
  }

  BaseObject* fetch_out(void) {
    auto* obj = roots_.back();
    roots_.pop_back();
    return obj;
  }
};

ParallelCopy::ParallelCopy(void)
  : sweeper_mutex_()
  , sweeper_cond_(sweeper_mutex_) {
  startup_sweepers();
}

ParallelCopy::~ParallelCopy(void) {
  clearup_sweepers();
}

void ParallelCopy::startup_sweepers(void) {
  for (auto i = 0; i < kMaxSweepers; ++i)
    sweepers_.emplace_back(new Sweeper(i));
}

void ParallelCopy::clearup_sweepers(void) {
  for (auto& s : sweepers_)
    s->stop();
}

int ParallelCopy::put_in_order(void) {
  int r = order_;
  order_ = (order_ + 1) % kMaxSweepers;
  return r;
}

int ParallelCopy::fetch_out_order(void) {
  order_ = (order_ - 1 + kMaxSweepers) % kMaxSweepers;
  return order_;
}

void ParallelCopy::generate_work(int sweeper_id,
      BaseObject* from_ref, ForwadingCallback&& cb) {
  for (auto& s : sweepers_) {
    if (s->get_id() == sweeper_id)
      continue;

    if (s->in_sweeper(from_ref))
      s->put_in_forwarding(from_ref, std::move(cb));
  }
}

void ParallelCopy::notify_traced(int /*sweeper_id*/) {
  ++tracing_counter_;
}

void ParallelCopy::notify_collected(
    int /*sweeper_id*/, std::size_t alive_count) {
  {
    Chaos::ScopedLock<Chaos::Mutex> g(sweeper_mutex_);
    ++sweeper_counter_;
    object_counter_ += alive_count;
  }
  sweeper_cond_.notify_one();
}

ParallelCopy& ParallelCopy::get_instance(void) {
  static ParallelCopy ins;
  return ins;
}

void ParallelCopy::collect(void) {
  auto old_count = object_counter_;

  tracing_counter_ = 0;
  sweeper_counter_ = 0;
  object_counter_ = 0;
  for (auto& s : sweepers_)
    s->sweeping();

  {
    Chaos::ScopedLock<Chaos::Mutex> g(sweeper_mutex_);
    while (sweeper_counter_ < kMaxSweepers)
      sweeper_cond_.wait();
  }

  std::cout
    << "[" << old_count - object_counter_ << "] objects collected, "
    << "[" << object_counter_ << "] objects remaining." << std::endl;
}

BaseObject* ParallelCopy::put_in(int value) {
  ++object_counter_;
  return sweepers_[put_in_order()]->put_in(value);
}

BaseObject* ParallelCopy::put_in(BaseObject* first, BaseObject* second) {
  ++object_counter_;
  return sweepers_[put_in_order()]->put_in(first, second);
}

BaseObject* ParallelCopy::fetch_out(void) {
  return sweepers_[fetch_out_order()]->fetch_out();
}

}
