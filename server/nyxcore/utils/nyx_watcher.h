// Copyright (c) 2018 ASMlover. All rights reserved.
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

#include <string>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <Python.h>

namespace nyx {

#if defined(_NYXCORE_ENABLE_WATCHER)
class WatcherObject {
public:
  WatcherObject(void);
  virtual ~WatcherObject(void);
};
#else
struct WatcherObject {};
#endif

struct _ObjectData {
  const WatcherObject* _ins;
  std::string _bt;

  _ObjectData(const WatcherObject* o, std::string&& bt)
    : _ins(o)
    , _bt(std::move(bt)) {
  }
};

class Watcher {
  using WatcherMap = std::unordered_map<const WatcherObject*, _ObjectData>;

  mutable std::mutex lock_;
  WatcherMap objects_;

  Watcher(const Watcher&) = delete;
  Watcher& operator=(const Watcher&) = delete;

  Watcher(void)
    : lock_() {
  }
public:
  static Watcher& instance(void) {
    static Watcher w;
    return w;
  }

  inline const WatcherMap& get_all_objects(void) const {
    return objects_;
  }

  inline void record_dealloc(const WatcherObject* w) {
    std::unique_lock<std::mutex> g(lock_);
    objects_.erase(w);
  }

  inline void lock(void) {
    lock_.lock();
  }

  inline void unlock(void) {
    lock_.unlock();
  }

  void record_alloc(const WatcherObject* w);
  std::vector<_ObjectData> find_by_typename(const char* name);
  std::vector<_ObjectData> find_by_typeinfo(const std::type_info& ti);
  std::uint32_t count_by_typename(const char* name);
  std::uint32_t count_by_typeinfo(const std::type_info& ti);
};

void nyx_watcher_wrap(PyObject* m);

}
