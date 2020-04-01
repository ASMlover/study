#pragma once

#include <condition_variable>
#include <deque>
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

namespace coext {

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
using ss_t    = std::stringstream;

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

template <typename Obj> class Singleton : private UnCopyable {
public:
  static Obj& get_instance() {
    static Obj _ins;
    return _ins;
  }
};

template <typename Fn, typename... Args>
inline auto async_call(Fn&& fn, Args&&... args) noexcept {
  return std::async(std::launch::async, std::forward<Fn>(fn), std::forward<Args>(args)...);
}

template <typename Key, typename Val>
class LRUCache final : private UnCopyable {
  using CacheItem = std::pair<Key, Val>;
  using CahceIter = typename std::list<CacheItem>::iterator;

  static constexpr sz_t kMaxCapacity = 256;

  sz_t capacity_{ kMaxCapacity };
  std::list<CacheItem> caches_;
  std::unordered_map<Key, CahceIter> caches_map_;
public:
  std::optional<Val> get(const Key& key) {
    auto it = caches_map_.find(key);
    if (it == caches_map_.end())
      return {};

    caches_.splice(caches_.begin(), caches_, it->second);
    return { it->second->second };
  }

  void put(const Key& key, const Val& val) {
    auto it = caches_map_.find(key);
    if (it != caches_map_.end()) {
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

class ThreadPool final : private UnCopyable {
  using TaskItem    = std::function<void ()>;
  using ThreadItem  = std::unique_ptr<std::thread>;

  bool running_{ true };
  mutable std::mutex mtx_;
  std::condition_variable non_empty_;
  std::vector<ThreadItem> threads_;
  std::deque<TaskItem> tasks_;

  TaskItem fetch_task() {
    std::unique_lock<std::mutex> lock(mtx_);

    while (running_ && tasks_.empty())
      non_empty_.wait(lock);

    TaskItem t;
    if (!tasks_.empty()) {
      t = std::move(tasks_.front());
      tasks_.pop_front();
    }
    return t;
  }
public:
  ThreadPool(int thread_num = 4) noexcept {
    threads_.reserve(thread_num);
    for (int i = 0; i < thread_num; ++i) {
      threads_.emplace_back(new std::thread([this] {
        while (running_) {
          auto t = fetch_task();
          if (t)
            t();
        }
      }));
    }
  }

  ~ThreadPool() noexcept {
    if (running_) {
      std::unique_lock<std::mutex> lock(mtx_);
      running_ = false;
      non_empty_.notify_all();
    }

    for (auto& t : threads_)
      t->join();
  }

  template <typename Fn, typename... Args>
  void run_task(Fn&& fn, Args&&... args) {
    if (threads_.empty()) {
      fn(std::forward<Args>(args)...);
    }
    else {
      using ReturnType = typename std::invoke_result<Fn, Args...>::type;
      auto task = std::make_shared<std::packaged_task<ReturnType()>>(
        std::bind(std::forward<Fn>(fn), std::forward<Args>(args)...));

      {
        std::unique_lock<std::mutex> lock(mtx_);
        tasks_.emplace_back([task] { (*task)(); });
      }
      non_empty_.notify_one();
    }
  }
};

}