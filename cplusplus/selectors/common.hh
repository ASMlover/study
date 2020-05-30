#pragma once

#include <cstdint>
#include <future>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>

#if !defined(SEL_UNUSED)
# define SEL_UNUSED(x) ((void)x)
#endif

#if defined(__x86_64) || defined(__x86_64__) ||\
  defined(__amd64__) || defined(__amd64) || defined(_M_X64)
# define SEL_ARCH64
#else
# define SEL_ARCH32
#endif

namespace sel {

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
#if defined(SEL_ARCH64)
using ssz_t   = std::int64_t;
#else
using ssz_t   = std::int32_t;
#endif
using str_t   = std::string;
using strv_t  = std::string_view;
using ss_t    = std::stringstream;

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

template <typename Object> class Singleton : private UnCopyable {
public:
  static Object& get_instance() {
    static Object _ins;
    return _ins;
  }
};

template <typename T, typename S> inline T as_type(S x) noexcept {
  return static_cast<T>(x);
}

template <typename T> inline T* as_rawptr(const std::unique_ptr<T>& p) noexcept {
  return p.get();
}

template <typename T> inline T* as_rawptr(const std::shared_ptr<T>& p) noexcept {
  return p.get();
}

template <typename Fn, typename... Args>
inline auto async_wrap(Fn&& fn, Args&&... args) noexcept {
  return std::async(std::launch::async,
    std::forward<Fn>(fn), std::forward<Args>(args)...);
}

template <typename T, typename... Args>
inline str_t as_string(T&& x, Args&&... args) noexcept {
  ss_t ss;

  ss << std::forward<T>(x);
  ((ss << std::forward<Args>(args)), ...);

  return ss.str();
}

template <typename... Args>
inline str_t string_format(strv_t fmt, const Args&... args) {
  int sz = std::snprintf(nullptr, 0, fmt.data(), args...);
  std::unique_ptr<char[]> buf{ new char[sz + 1] };
  std::snprintf(buf.get(), sz, fmt.data(), args...);
  return str_t(buf.get(), buf.get() + sz);
}

}