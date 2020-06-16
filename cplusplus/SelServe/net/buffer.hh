#pragma once

#include <array>
#include <vector>
#include "../common/common.hh"

namespace sser::net {

// holds a buffer that can be modified
class MutableBuffer final : public Copyable {
  void* data_{};
  sz_t size_{};
public:
  // construct an empty buffer
  MutableBuffer() noexcept {}

  // construct a buffer to represent given memory range
  MutableBuffer(void* data, sz_t size) noexcept
    : data_(data), size_(size) {
  }

  inline void* data() const noexcept { return data_; }
  inline sz_t size() const noexcept { return size_; }

  // move the start of the buffer by the specified number of bytes
  MutableBuffer& operator+=(sz_t n) noexcept {
    sz_t offset = n < size_ ? n : size_;
    data_ = as_type<char*>(data_) + offset;
    size_ -= offset;
    return *this;
  }
};

// holds a buffer that cannot be modified
class ConstBuffer final : public Copyable {
  const void* data_{};
  sz_t size_{};
public:
  // construct an empty buffer
  ConstBuffer() noexcept {}

  // construct a buffer to represent give memory range
  ConstBuffer(const void* data, sz_t size) noexcept
    : data_(data), size_(size) {
  }

  // construct a non-modifiable buffer from a modifiable one
  ConstBuffer(const MutableBuffer& b) noexcept
    : data_(b.data()), size_(b.size()) {
  }

  inline const void* data() const noexcept { return data_; }
  inline sz_t size() const noexcept { return size_; }

  // move the start of the buffer by the specified number of bytes
  ConstBuffer& operator+=(sz_t n) noexcept {
    sz_t offset = n < size_ ? n : size_;
    data_ = as_type<const char*>(data_) + offset;
    size_ -= offset;
    return *this;
  }
};

// create a new modifibale buffer that is offset from the start of another
inline MutableBuffer operator+(const MutableBuffer& b, sz_t n) noexcept {
  sz_t offset = n < b.size() ? n : b.size();
  char* new_data = as_type<char*>(b.data()) + offset;
  sz_t new_size = b.size() - offset;
  return MutableBuffer(new_data, new_size);
}

inline MutableBuffer operator+(sz_t n, const MutableBuffer& b) noexcept {
  return b + n;
}

// create a new non-modifiable buffer that is offset from the start of another
inline ConstBuffer operator+(const ConstBuffer& b, sz_t n) noexcept {
  sz_t offset = n < b.size() ? n : b.size();
  const char* new_data = as_type<const char*>(b.data()) + offset;
  sz_t new_size = b.size() - offset;
  return ConstBuffer(new_data, new_size);
}

inline ConstBuffer operator+(sz_t n, const ConstBuffer& b) noexcept {
  return b + n;
}

inline sz_t bmin(sz_t sz1, sz_t sz2) noexcept {
  return sz1 < sz2 ? sz1 : sz2;
}

// create a new modifiable buffer from an existing buffer
// @returns: MutableBuffer(b)
inline MutableBuffer buffer(const MutableBuffer& b) noexcept {
  return MutableBuffer(b);
}

// create a new modifiable buffer from an existing buffer
// @returns a MutableBuffer value equivalent to:
//    MutableBuffer(b.data(), min(b.size(), max_bytes))
inline MutableBuffer buffer(const MutableBuffer& b, sz_t max_bytes) noexcept {
  return MutableBuffer(b.data(), bmin(b.size(), max_bytes));
}

// create a new non-modifiable buffer from an existing buffer
// @returns: ConstBuffer(b)
inline ConstBuffer buffer(const ConstBuffer& b) noexcept {
  return ConstBuffer(b);
}

// create a new non-modifiable buffer from an existing buffer
// @returns a ConstBuffer value equivalent to:
//    ConstBuffer(b.data(), min(b.size(), max_bytes))
inline ConstBuffer buffer(const ConstBuffer& b, sz_t max_bytes) noexcept {
  return ConstBuffer(b.data(), bmin(b.size(), max_bytes));
}

// create a new modifiable buffer that represents the given memory range
// @returns: MutableBuff(data, size)
inline MutableBuffer buffer(void* data, sz_t size) noexcept {
  return MutableBuffer(data, size);
}

// create a new non-modifiable buffer that represents the given memory range
// @returns: ConstBuffer(data, size)
inline ConstBuffer buffer(const void* data, sz_t size) noexcept {
  return ConstBuffer(data, size);
}

// create a new modifiable buffer that represents the given POD array
// @returns a MutableBuffer value equivalent to:
//    MutaleBuffer(static_cast<void*>(b), N*sizeof(POD))
template <typename T, sz_t N>
inline MutableBuffer buffer(T (&b)[N]) noexcept {
  return MutableBuffer(b, N * sizeof(T));
}

// create a new modifiable buffer that represents the given POD array
// @returns a MutableBuffer value equivalent to:
//    MutableBuffer(static_cast<void*>(b), min(N*sizeof(POD), max_bytes)
template <typename T, sz_t N>
inline MutableBuffer buffer(T (&b)[N], sz_t max_bytes) noexcept {
  return MutableBuffer(b, bmin(N * sizeof(T), max_bytes));
}

// create a new non-modifiable buffer that represents the given POD array
// @returns a ConstBuffer value equivalent to:
//    ConstBuffer(static_cast<const void*>(b), N*sizeof(POD))
template <typename T, sz_t N>
inline ConstBuffer buffer(T (&b)[N]) noexcept {
  return ConstBuffer(b, N * sizeof(T));
}

// create a new non-modifiable buffer that represents the given POD array
// @returns a ConstBuffer value equivalent to:
//    ConstBuffer(static_cast<const void*>(b), min(N*sizeof(POD), max_bytes))
template <typename T, sz_t N>
inline ConstBuffer buffer(T (&b)[N], sz_t max_byets) noexcept {
  return ConstBuffer(b, bmin(N * sizeof(T), max_byets));
}

// create a new modifiable buffer that represents the given POD array
// @returns a MutableBuffer value equivalent to:
//    MutableBuffer(b.data(), b.size() * sizeof(POD))
template <typename T, sz_t N>
inline MutableBuffer buffer(std::array<T, N>& b) noexcept {
  return MutableBuffer(b.data(), b.size() * sizeof(T));
}

// create a new modifiable buffer that represents the given POD array
// @returns a MutableBuffer value equivalent to:
//    MutableBuffer(b.data(), min(b.size() * sizeof(POD), max_bytes))
template <typename T, sz_t N>
inline MutableBuffer buffer(std::array<T, N>& b, sz_t max_bytes) noexcept {
  return MutableBuffer(b.data(), bmin(b.size() * sizeof(T), max_bytes));
}

// create a new non-modifiable buffer that represents the given POD array
// @returns a ConstBuffer value equivalent to:
//    ConstBuffer(b.data(), b.size() * sizeof(POD))
template <typename T, sz_t N>
inline ConstBuffer buffer(std::array<const T, N>& b) noexcept {
  return ConstBuffer(b.data(), b.size() * sizeof(T));
}

// create a new non-modifiable buffer that represents the given POD array
// @returns a ConstBuffer value equivalent to:
//    ConstBuffer(b.data(), min(b.size() * sizeof(POD), max_bytes))
template <typename T, sz_t N>
inline ConstBuffer buffer(std::array<const T, N>& b, sz_t max_bytes) noexcept {
  return ConstBuffer(b.data(), bmin(b.size() * sizeof(T), max_bytes));
}

// create a new non-modifiable buffer that represents the given POD array
// @returns a ConstBuffer value equivalent to:
//    ConstBuffer(b.data(), b.size() * sizeof(POD))
template <typename T, sz_t N>
inline ConstBuffer buffer(const std::array<T, N>& b) noexcept {
  return ConstBuffer(b.data(), b.size() * sizeof(T));
}

// create a new non-modifiable buffer that represents the given POD array
// @returns a ConstBuffer value equivalent to:
//    ConstBuffer(b.data(), min(b.size() * sizeof(POD), max_bytes))
template <typename T, sz_t N>
inline ConstBuffer buffer(const std::array<T, N>& b, sz_t max_bytes) noexcept {
  return ConstBuffer(b.data(), bmin(b.size() * sizeof(T), max_bytes));
}

// create a new modifiable buffer that represents the given POD vector
// @returns a MutableBuffer value equivalent to:
//    MutableBuffer(b.size() ? &b[0]: 0, b.size() * sizeof(POD))
//
// @note: the buffer is invalidated by any vector operation that would
//    also invalidate iterators
template <typename T, typename Alloc>
inline MutableBuffer buffer(std::vector<T, Alloc>& b) noexcept {
  return MutableBuffer(b.size() ? &b[0] : nullptr, b.size() * sizeof(T));
}

// create a new modifiable buffer that represents the given POD vector
// @returns a MutableBuffer value equivalent to:
//    MutableBuffer(b.size() ? &b[0] : 0, min(b.size() * sizeof(POD), max_bytes))
//
// @note: the buffer is invalidated by any vector operation that would
//    also invalidate iterators
template <typename T, typename Alloc>
inline MutableBuffer buffer(std::vector<T, Alloc>& b, sz_t max_bytes) noexcept {
  return MutableBuffer(b.size() ? &b[0] : nullptr, bmin(b.size() * sizeof(T), max_bytes));
}

// create a new non-modifiable buffer that represents the given POD vector
// @returns a ConstBuffer value equivalent to:
//    ConstBuffer(b.size() ? &b[0] : 0, b.size() * sizeof(POD))
//
// @note: the buffer is invalidated by any vector operation that would
//    also invalidate iterators
template <typename T, typename Alloc>
inline ConstBuffer buffer(const std::vector<T, Alloc>& b) noexcept {
  return ConstBuffer(b.size() ? &b[0] : nullptr, b.size() * sizeof(T));
}

// create a new non-modifiable buffer that represents the given POD vector
// @returns a ConstBuffer value equivalent to:
//    ConstBuffer(b.size() ? &b[0] : 0, min(b.size() * sizeof(POD), max_bytes))
//
// @note: the buffer is invalidated by any vector operation that would
//    also invalidate iterators
template <typename T, typename Alloc>
inline ConstBuffer buffer(const std::vector<T, Alloc>& b, sz_t max_bytes) noexcept {
  return ConstBuffer(b.size() ? &b[0] : nullptr, bmin(b.size() * sizeof(T), max_bytes));
}

// create a new modifiable buffer that represents the given string
// @returns MutableBuffer(b.size() ? &b[0] : 0, b.size() * sizeof(Elem))
//
// @note: the buffer is invalidated by any non-const operation called on
//    the given string object
template <typename T, typename Traits, typename Alloc>
inline MutableBuffer buffer(std::basic_string<T, Traits, Alloc>& b) noexcept {
  return MutableBuffer(b.size() ? &b[0] : nullptr, b.size() * sizeof(T));
}

// create a new modifiable buffer that represents the given string
// @returns a MutableBuffer value equivalent to:
//    MutableBuffer(b.size() ? &b[0] : 0, min(b.size() * sizeof(Elem), max_bytes))
//
// @note: the buffer is invalidated by any non-const operation called on
//    the given string object
template <typename T, typename Traits, typename Alloc>
inline MutableBuffer buffer(std::basic_string<T, Traits, Alloc>& b, sz_t max_bytes) noexcept {
  return MutableBuffer(b.size() ? &b[0] : nullptr, bmin(b.size() * sizeof(T), max_bytes));
}

// create a new non-modifiable buffer that represents the given string
// @returns a ConstBuffer(b.size() ? &b[0] : 0, b.size() * sizeof(Elem))
//
// @note: the buffer is invalidated by any non-const operation called on
//    the given string object
template <typename T, typename Traits, typename Alloc>
inline ConstBuffer buffer(const std::basic_string<T, Traits, Alloc>& b) noexcept {
  return ConstBuffer(b.size() ? &b[0] : nullptr, b.size() * sizeof(T));
}

// create a new non-modifiable buffer that represents the given string
// @returns a ConstBuffer value equivalent to:
//    ConstBuffer(b.size() ? &b[0] : 0, min(b.size() * sizeof(Elem), max_bytes))
//
// @note: the buffer is invalidated by any non-const operation called on
//    the given string object
template <typename T, typename Traits, typename Alloc>
inline ConstBuffer buffer(const std::basic_string<T, Traits, Alloc>& b, sz_t max_bytes) noexcept {
  return ConstBuffer(b.size() ? &b[0] : nullptr, bmin(b.size() * sizeof(T), max_bytes));
}

// create a new non-modifiable buffer that represents the given string_view
// @returns ConstBuffer(b.size() ? &b[0] : 0, b.size() * sizeof(Elem))
template <typename T, typename Traits>
inline ConstBuffer buffer(std::basic_string_view<T, Traits> b) noexcept {
  return ConstBuffer(b.size() ? &b[0] : nullptr ? b.size() * sizeof(T));
}

// create a new non-modifiable buffer that represents the given string_view
// @returns a ConstBuffer value equivalent to:
//    ConstBuffer(b.size() ? &b[0] : 0, min(b.size() * sizeof(Elem), max_bytes))
template <typename T, typename Traits>
inline ConstBuffer buffer(std::basic_string_view<T, Traits> b, sz_t max_bytes) noexcept {
  return ConstBuffer(b.size() ? &b[0] : nullptr, bmin(b.size() * sizeof(T), max_bytes));
}

}