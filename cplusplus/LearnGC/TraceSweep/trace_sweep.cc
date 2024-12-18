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
#include <Chaos/Base/Types.h>
#include "object.h"
#include "trace_sweep.h"

namespace gc {

void TraceSweep::roots_tracing(std::stack<BaseObject*>& trace_objects) {
  for (auto* obj : roots_)
    trace_objects.push(obj);
}

void TraceSweep::scan_tracing(std::stack<BaseObject*>& trace_objects) {
  while (!trace_objects.empty()) {
    auto* obj = trace_objects.top();
    trace_objects.pop();
    if (obj->inc_ref() == 1 && obj->is_pair()) {
      auto append_fn = [&trace_objects](BaseObject* obj) {
        if (obj != nullptr)
          trace_objects.push(obj);
      };

      append_fn(Chaos::down_cast<Pair*>(obj)->first());
      append_fn(Chaos::down_cast<Pair*>(obj)->second());
    }
  }
}

void TraceSweep::sweep_tracing(void) {
  for (auto it = objects_.begin(); it != objects_.end();) {
    if ((*it)->ref() == 0) {
      delete *it;
      objects_.erase(it++);
    }
    else {
      (*it)->set_ref(0);
      ++it;
    }
  }
}

TraceSweep& TraceSweep::get_instance(void) {
  static TraceSweep ins;
  return ins;
}

void TraceSweep::collect_tracing(void) {
  auto old_count = objects_.size();

  std::stack<BaseObject*> trace_objects;
  roots_tracing(trace_objects);
  scan_tracing(trace_objects);
  sweep_tracing();

  std::cout
    << "[" << old_count - objects_.size() << "] objects colleced, "
    << "[" << objects_.size() << "] objects remaining." << std::endl;
}

BaseObject* TraceSweep::put_in(int value) {
  if (objects_.size() >= kMaxObjects)
    collect_tracing();

  auto* obj = new Int();
  obj->set_value(value);

  roots_.push_back(obj);
  objects_.push_back(obj);

  return obj;
}

BaseObject* TraceSweep::put_in(BaseObject* first, BaseObject* second) {
  if (objects_.size() >= kMaxObjects)
    collect_tracing();

  auto* obj = new Pair();
  if (first != nullptr)
    obj->set_first(first);
  if (second != nullptr)
    obj->set_second(second);

  roots_.push_back(obj);
  objects_.push_back(obj);

  return obj;
}

BaseObject* TraceSweep::fetch_out(void) {
  auto* obj = roots_.back();
  roots_.pop_back();
  return obj;
}

}
