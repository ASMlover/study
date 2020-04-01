#pragma once

#include <WinSock2.h>
#include <deque>
#include <functional>
#include <memory>
#include <thread>
#include <future>
#include <condition_variable>
#include <iostream>
#include <list>
#include <optional>
#include <unordered_map>

namespace common {

class Copyable {
protected:
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

template <typename T> class Singleton : private UnCopyable {
public:
  static T& get_instance() noexcept {
    static T _ins;
    return _ins;
  }
};

class WSGuarder final : private UnCopyable {
public:
  WSGuarder() noexcept;
  ~WSGuarder() noexcept;
};

class UniqueSocket final : private UnCopyable {
  SOCKET sockfd_{};
public:
  UniqueSocket(SOCKET fd = INVALID_SOCKET) noexcept : sockfd_(fd) {}
  ~UniqueSocket() noexcept;

  inline void reset(SOCKET fd) noexcept { sockfd_ = fd; }
  inline SOCKET get() const noexcept { return sockfd_; }
  inline operator SOCKET() const noexcept { return sockfd_; }
  inline operator bool() const noexcept { return sockfd_ != INVALID_SOCKET; }
};

template <typename Fn, typename... Args>
inline auto async(Fn&& fn, Args&&... args) {
  return std::async(std::launch::async, std::forward<Fn>(fn), std::forward<Args>(args)...);
}

template <typename T>
class LRUCache final : private UnCopyable {
  using CacheItem = std::pair<int, T>;
  using CacheIter = typename std::list<CacheItem>::iterator;

  static constexpr int kMaxCapacity = 512;

  int capacity_{kMaxCapacity};
  std::list<CacheItem> cache_;
  std::unordered_map<int, CacheIter> cache_map_;
public:
  std::optional<T> get(int key) {
    auto it = cache_map_.find(key);
    if (it == cache_map_.end())
      return {};

    cache_.splice(cache_.begin(), cache_, it->second);
    return {it->second->second};
  }

  void put(int key, const T& val) {
    auto it = cache_map_.find(key);
    if (it != cache_map_.end()) {
      it->second->second = value;
      cache_.splice(cache_.begin(), cache_, it->second);
    }
    else {
      if (cache_.size() >= capacity_) {
        cache_map_.erase(cache_.back().first);
        cache_.pop_back();
      }

      cache_.emplace_front(std::make_pair(key, value));
      cache_map_[key] = cache_.begin();
    }
  }
};

class ThreadPool final : private UnCopyable {
  using TaskEntity = std::function<void ()>;
  using ThreadEntity = std::unique_ptr<std::thread>;

  bool running_{true};
  mutable std::mutex mtx_;
  std::condition_variable non_empty_;
  std::vector<ThreadEntity> threads_;
  std::deque<TaskEntity> tasks_;

  TaskEntity fetch_task() {
    std::unique_lock<std::mutex> guard(mtx_);
    while (tasks_.empty() && running_)
      non_empty_.wait(guard);

    TaskEntity t;
    if (!tasks_.empty()) {
      t = tasks_.front();
      tasks_.pop_front();
    }
    return t;
  }
public:
  ThreadPool(int thread_num = 4) noexcept {
    threads_.reserve(thread_num);
    for (int i = 0; i < thread_num; ++i)
      threads_.emplace_back(new std::thread([this] {
          while (running_) {
            auto t = fetch_task();
            if (t)
              t();
          }
        }));
  }

  ~ThreadPool() noexcept {
    if (running_) {
      {
        std::unique_lock<std::mutex> guard(mtx_);
        running_ = false;
        non_empty_.notify_all();
      }

      for (auto& t : threads_)
        t->join();
    }
  }

  template <typename Fn, typename... Args>
  void run_task(Fn&& fn, Args&&... args) {
    if (threads_.empty()) {
      fn(std::forward<Args>(args)...);
    }
    else {
      using ReturnType = typename std::invoke_result<Fn, Args...>::type;
      auto task = std::make_shared<std::packaged_task<ReturnType ()>>(
        std::bind(std::forward<Fn>(fn), std::forward<Args>(args)...));

      {
        std::unique_lock<std::mutex> guard(mtx_);
        tasks_.emplace_back([task] { (*task)(); });
      }
      non_empty_.notify_one();
    }
  }
};

}
