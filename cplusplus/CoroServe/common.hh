// Copyright (c) 2020 ASMlover. All rights reserved.
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

#include <condition_variable>
#include <functional>
#include <future>
#include <iostream>
#include <list>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>
#include <vector>
#include <unordered_map>

namespace coro {

using nil_t   = std::nullptr_t;
using byte_t  = std::uint8_t;
using i8_t    = std::int8_t;
using u8_t    = std::uint8_t;
using i16_t   = std::int16_t;
using u16_t   = std::uint16_t;
using i32_t   = std::int32_t;
using u32_t   = std::uint32_t;
using i64_t   = std::int64_t;
using u64_t   = std::uint64_t;
using sz_t    = std::size_t;
using str_t   = std::string;
using strv_t  = std::string_view;
using oss_t   = std::ostringstream;

class Copyable {
public:
  Copyable() noexcept = default;
  ~Copyable() noexcept = default;
  Copyable(const Copyable&) noexcept = default;
  Copyable(Copyable&&) noexcept = default;
  Copyable& operator=(const Copyable&) noexcept = default;
  Copyable& operator=(Copyable&&) noexcept = default;
};

class UnCopyable {
  UnCopyable(const UnCopyable&) noexcept = delete;
  UnCopyable(UnCopyable&&) noexcept = delete;
  UnCopyable& operator=(const UnCopyable&) noexcept = delete;
  UnCopyable& operator=(UnCopyable&&) noexcept = delete;
protected:
  UnCopyable() noexcept = default;
  ~UnCopyable() noexcept = default;
};

template <typename Object> class Singleton : private UnCopyable {
public:
  static Object& get_instance() {
    static Object _ins;
    return _ins;
  }
};

template <typename Fn, typename... Args>
inline auto async_wrap(Fn&& fn, Args&&... args) noexcept {
  return std::async(std::launch::async,
      std::forward<Fn>(fn), std::forward<Args>(args)...);
}

template <typename Key, typename Val>
class LRUCache final : private UnCopyable {
  using CacheItem = std::pair<Key, Val>;
  using CacheIter = typename std::list<CacheItem>::iterator;

  static constexpr sz_t kMaxCapacity = 256;

  sz_t capacity_{};
  std::list<CacheItem> caches_;
  std::unordered_map<Key, CacheIter> caches_map_;
public:
  LRUCache(sz_t capacity = kMaxCapacity) noexcept
    : capacity_(capacity) {
  }

  std::optional<Val> get(const Key& key) {
    if (auto it = caches_map_.find(key); it != caches_map_.end()) {
      caches_.splice(caches_.begin(), caches_, it->second);
      return {it->second->second};
    }
    return {};
  }

  void put(const Key& key, const Val& val) {
    if (auto it = caches_map_.find(key); it != caches_map_.end()) {
      it->second->second = val;
      caches_.splice(caches_.begin(), caches_, it->second);
    }
    else {
      if (caches_.size() >= capacity_) {
        caches_map_.erase(caches_.back().first);
        caches_.pop_back();
      }

      caches_.emplace_front(std::make_pair(key, val));
      caches_map_[key] = caches_.begin();
    }
  }
};

}
