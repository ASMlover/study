#pragma once

#include <memory>
#include <sstream>
#include <string>
#include <string_view>

#define TADPOLE_UNUSED(x) ((void)x)

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
using ssz_t   = std::int64_t;
using str_t   = std::string;
using strv_t  = std::string_view;
using ss_t    = std::stringstream;

template <typename T, typename S> inline T as_type(S x) noexcept {
  return static_cast<T>(x);
}

template <typename T, typename S> inline T* as_down(S* x) noexcept {
  return dynamic_cast<T*>(x);
}

template <typename T, typename PTR> inline T* get_rawptr(const PTR& p) noexcept {
  return p.get();
}

template <typename T, typename... Args>
inline str_t as_string(T&& x, Args&&... args) noexcept {
  ss_t ss;

  ss << std::forward<T>(x);
  ((ss << std::forward<Args>(args)), ...);

  return ss.str();
}

template <typename... Args> inline str_t from_fmt(strv_t fmt, const Args&... args) {
  int sz = std::snprintf(nullptr, 0, fmt.data(), args...);
  std::unique_ptr<char[]> buf{ new char[sz + 1] };
  std::snprintf(buf.get(), sz, fmt.data(), args...);
  return str_t(buf.get(), buf.get() + sz);
}

}