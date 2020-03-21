#pragma once

#include <WinSock2.h>
#include <thread>
#include <future>
#include <iostream>

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
  UniqueSocket(SOCKET fd) noexcept : sockfd_(fd) {}
  ~UniqueSocket() noexcept;

  inline operator SOCKET() const { return sockfd_; }
  inline operator bool() const { return sockfd_ != INVALID_SOCKET; }
};

template <typename Fn, typename... Args>
inline auto async(Fn&& fn, Args&&... args) {
  return std::async(std::launch::async, std::forward<Fn>(fn), std::forward<Args>(args)...);
}

}