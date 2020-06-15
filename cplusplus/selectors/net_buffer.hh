#pragma once

#include <array>
#include <vector>
#include "common.hh"

namespace sel::net {

class MutableBuf final : public Copyable {
  void* data_{};
  sz_t size_{};
public:
  MutableBuf() noexcept {}
  MutableBuf(void* buf, sz_t len) noexcept : data_(buf), size_(len) {}

  inline void* data() const noexcept { return data_; }
  inline sz_t size() const noexcept { return size_; }

  MutableBuf& operator+=(sz_t n) noexcept {
    sz_t offset = n < size_ ? n : size_;
    data_ = as_type<char*>(data_) + offset;
    size_ -= offset;
    return *this;
  }
};

class ConstBuf final : public Copyable {
  const void* data_{};
  sz_t size_{};
public:
  ConstBuf() noexcept {}
  ConstBuf(const void* buf, sz_t len) noexcept : data_(buf), size_(len) {}

  ConstBuf(const MutableBuf& b) noexcept
    : data_(b.data()), size_(b.size()) {
  }

  inline const void* data() const noexcept { return data_; }
  inline sz_t size() const noexcept { return size_; }

  ConstBuf& operator+=(sz_t n) noexcept {
    sz_t offset = n < size_ ? n : size_;
    data_ = as_type<const char*>(data_) + offset;
    size_ -= offset;
    return *this;
  }
};

class NullBuf : public Copyable {};

inline MutableBuf buffer(const MutableBuf& b) noexcept {
  return MutableBuf(b);
}

inline ConstBuf buffer(const ConstBuf& b) noexcept {
  return ConstBuf(b);
}

inline MutableBuf buffer(const MutableBuf& b, sz_t max_bytes) noexcept {
  return MutableBuf(b.data(), b.size() < max_bytes ? b.size() : max_bytes);
}

inline ConstBuf buffer(const ConstBuf& b, sz_t max_bytes) noexcept {
  return ConstBuf(b.data(), b.size() < max_bytes ? b.size() : max_bytes);
}

inline MutableBuf buffer(void* buf, sz_t len) noexcept {
  return MutableBuf(buf, len);
}

inline ConstBuf buffer(const void* buf, sz_t len) noexcept {
  return ConstBuf(buf, len);
}

template <typename PodType, sz_t N> inline MutableBuf buffer(PodType (&buf)[N]) noexcept {
  return MutableBuf(buf, N * sizeof(PodType));
}

template <typename PodType, sz_t N> inline ConstBuf buffer(const PodType (&buf)[N]) noexcept {
  return ConstBuf(buf, N * sizeof(PodType));
}

template <typename PodType, sz_t N>
inline MutableBuf buffer(PodType (&buf)[N], sz_t max_bytes) noexcept {
  return MutableBuf(buf, N * sizeof(PodType) < max_bytes ? N * sizeof(PodType) : max_bytes);
}

template <typename PodType, sz_t N>
inline ConstBuf buffer(const PodType (&buf)[N], sz_t max_bytes) noexcept {
  return ConstBuf(buf, N * sizeof(PodType) < max_bytes ? N * sizeof(PodType) : max_bytes);
}

template <typename PodType, sz_t N>
inline MutableBuf buffer(std::array<PodType, N>& b) noexcept {
  return MutableBuf(b.data(), b.size() * sizeof(PodType));
}

template <typename PodType, sz_t N>
inline ConstBuf buffer(const std::array<PodType, N>& b) noexcept {
  return ConstBuf(b.data(), b.size() * sizeof(PodType));
}

template <typename PodType, sz_t N>
inline MutableBuf buffer(std::array<PodType, N>& b, sz_t max_bytes) noexcept {
  return MutableBuf(b.data(),
    b.size() * sizeof(PodType) < max_bytes ? b.size() * sizeof(PodType) : max_bytes);
}

template <typename PodType, sz_t N>
inline ConstBuf buffer(const std::array<PodType, N>& b, sz_t max_bytes) noexcept {
  return ConstBuf(b.data(),
    b.size() * sizeof(PodType) < max_bytes ? b.size() * sizeof(PodType) : max_bytes);
}

template <typename PodType, typename Allocator>
inline MutableBuf buffer(std::vector<PodType, Allocator>& b) noexcept {
  return MutableBuf(b.size() ? &b[0] : nullptr, b.size() * sizeof(PodType));
}

template <typename PodType, typename Allocator>
inline ConstBuf buffer(const std::vector<PodType, Allocator>& b) noexcept {
  return ConstBuf(b.size() ? &b[0] : nullptr, b.size() * sizeof(PodType));
}

template <typename PodType, typename Allocator>
inline MutableBuf buffer(std::vector<PodType, Allocator>& b, sz_t max_bytes) noexcept {
  return MutableBuf(b.size() ? &b[0] : nullptr,
    b.size() * sizeof(PodType) < max_bytes ? b.size() * sizeof(PodType) : max_bytes);
}

template <typename PodType, typename Allocator>
inline ConstBuf buffer(const std::vector<PodType, Allocator>& b, sz_t max_bytes) noexcept {
  return ConstBuf(b.size() ? &b[0] : nullptr,
    b.size() * sizeof(PodType) < max_bytes ? b.size() * sizeof(PodType) : max_bytes);
}

template <typename Elem, typename Traits, typename Allocator>
inline MutableBuf buffer(std::basic_string<Elem, Traits, Allocator>& b) noexcept {
  return MutableBuf(b.size() ? &b[0] : nullptr, b.size() * sizeof(Elem));
}

template <typename Elem, typename Traits, typename Allocator>
inline ConstBuf buffer(const std::basic_string<Elem, Traits, Allocator>& b) noexcept {
  return ConstBuf(b.size() ? &b[0] : nullptr, b.size() * sizeof(Elem));
}

template <typename Elem, typename Traits, typename Allocator>
inline MutableBuf buffer(std::basic_string<Elem, Traits, Allocator>& b, sz_t max_bytes) noexcept {
  return MutableBuf(b.size() ? &b[0] : nullptr,
    b.size() * sizeof(Elem) < max_bytes ? b.size() * sizeof(Elem) : max_bytes);
}

template <typename Elem, typename Traits, typename Allocator>
inline ConstBuf buffer(const std::basic_string<Elem, Traits, Allocator>& b, sz_t max_bytes) noexcept {
  return ConstBuf(b.size() ? &b[0] : nullptr,
    b.size() * sizeof(Elem) < max_bytes ? b.size() * sizeof(Elem) : max_bytes);
}

}