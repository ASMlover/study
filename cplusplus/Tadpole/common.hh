#pragma once

#include <cstdint>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>

#if !defined(TADPOLE_UNUSED)
# define TADPOLE_UNUSED(x) ((void)x)
#endif

namespace tadpole {

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

template <typename T, typename S> inline T as_type(S x) noexcept {
  return static_cast<T>(x);
}

template <typename T, typename S> inline T* as_down(S* x) noexcept {
  return dynamic_cast<T*>(x);
}

template <typename T>
inline T* get_rawptr(const std::unique_ptr<T>& p) noexcept {
  return p.get();
}

template <typename T>
inline T* get_rawptr(const std::shared_ptr<T>& p) noexcept {
  return p.get();
}

inline str_t as_string(double d) noexcept {
  ss_t ss;
  ss << d;
  return ss.str();
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
  sz_t sz = std::snprintf(nullptr, 0, fmt.data(), args...) + 1;
  std::unique_ptr<char[]> buf{ new char[sz] };
  std::snprintf(buf.get(), sz, fmt.data(), args...);
  return str_t(buf.get(), buf.get() + sz - 1);
}

}