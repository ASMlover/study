// Copyright (c) 2017 ASMlover. All rights reserved.
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

#include <cstddef>
#include <string>
#include <array>
#include <vector>

namespace netpp {

class MutableBuffer {
  void* data_{};
  std::size_t size_{};
public:
  MutableBuffer(void) noexcept {}

  MutableBuffer(void* buf, std::size_t len) noexcept
    : data_(buf)
    , size_(len) {
  }

  void* data(void) const noexcept {
    return data_;
  }

  std::size_t size(void) const noexcept {
    return size_;
  }

  MutableBuffer& operator+=(std::size_t n) noexcept {
    std::size_t offset = n < size_ ? n : size_;
    data_ = static_cast<char*>(data_) + offset;
    size_ -= offset;
    return *this;
  }
};

class ConstBuffer {
  const void* data_{};
  std::size_t size_{};
public:
  ConstBuffer(void) noexcept {}

  ConstBuffer(const void* buf, std::size_t len) noexcept
    : data_(buf)
    , size_(len) {
  }

  ConstBuffer(const MutableBuffer& b) noexcept
    : data_(b.data())
    , size_(b.size()) {
  }

  const void* data(void) const noexcept {
    return data_;
  }

  std::size_t size(void) const noexcept {
    return size_;
  }

  ConstBuffer& operator+=(std::size_t n) noexcept {
    std::size_t offset = n < size_ ? n : size_;
    data_ = static_cast<const char*>(data_) + offset;
    size_ -= offset;
    return *this;
  }
};

inline MutableBuffer buffer(const MutableBuffer& b) noexcept {
  return MutableBuffer(b);
}

inline MutableBuffer buffer(
    const MutableBuffer& b, std::size_t max_bytes) noexcept {
  return MutableBuffer(b.data(), b.size() < max_bytes ? b.size() : max_bytes);
}

inline ConstBuffer buffer(const ConstBuffer& b) noexcept {
  return ConstBuffer(b);
}

inline ConstBuffer buffer(
    const ConstBuffer& b, std::size_t max_bytes) noexcept {
  return ConstBuffer(b.data(), b.size() < max_bytes ? b.size() : max_bytes);
}

inline MutableBuffer buffer(void* buf, std::size_t len) noexcept {
  return MutableBuffer(buf, len);
}

inline ConstBuffer buffer(const void* buf, std::size_t len) noexcept {
  return ConstBuffer(buf, len);
}

template <typename PodType, std::size_t N>
inline MutableBuffer buffer(PodType (&buf)[N]) noexcept {
  return MutableBuffer(buf, N * sizeof(PodType));
}

template <typename PodType, std::size_t N>
inline MutableBuffer buffer(PodType (&buf)[N], std::size_t max_bytes) noexcept {
  return MutableBuffer(buf,
      N * sizeof(PodType) < max_bytes ? N * sizeof(PodType) : max_bytes);
}

template <typename PodType, std::size_t N>
inline ConstBuffer buffer(PodType (&buf)[N]) noexcept {
  return ConstBuffer(buf, N * sizeof(PodType));
}

template <typename PodType, std::size_t N>
inline ConstBuffer buffer(PodType (&buf)[N], std::size_t max_bytes) noexcept {
  return ConstBuffer(buf,
      N * sizeof(PodType) < max_bytes ? N * sizeof(PodType) : max_bytes);
}

template <typename PodType, std::size_t N>
inline MutableBuffer buffer(std::array<PodType, N>& b) noexcept {
  return MutableBuffer(b.data(), b.size() * sizeof(PodType));
}

template <typename PodType, std::size_t N>
inline MutableBuffer buffer(
    std::array<PodType, N>& b, std::size_t max_bytes) noexcept {
  return MutableBuffer(b.data(),
      b.size() * sizeof(PodType) < max_bytes
      ? b.size() * sizeof(PodType) : max_bytes);
}

template <typename PodType, std::size_t N>
inline ConstBuffer buffer(std::array<PodType, N>& b) noexcept {
  return ConstBuffer(b.data(), b.size() * sizeof(PodType));
}

template <typename PodType, std::size_t N>
inline ConstBuffer buffer(
    std::array<PodType, N>& b, std::size_t max_bytes) noexcept {
  return ConstBuffer(b.data(),
      b.size() * sizeof(PodType) < max_bytes
      ? b.size() * sizeof(PodType) : max_bytes);
}

template <typename PodType, std::size_t N>
inline ConstBuffer buffer(const std::array<PodType, N>& b) noexcept {
  return ConstBuffer(b.data(), b.size() * sizeof(PodType));
}

template <typename PodType, std::size_t N>
inline ConstBuffer buffer(
    const std::array<PodType, N>& b, std::size_t max_bytes) noexcept {
  return ConstBuffer(b.data(),
      b.size() * sizeof(PodType) < max_bytes
      ? b.size() * sizeof(PodType) : max_bytes);
}

template <typename PodType, typename Allocator>
inline MutableBuffer buffer(std::vector<PodType, Allocator>& b) noexcept {
  return MutableBuffer(b.size() ? &b[0] : nullptr, b.size() * sizeof(PodType));
}

template <typename PodType, typename Allocator>
inline MutableBuffer buffer(
    std::vector<PodType, Allocator>& b, std::size_t max_bytes) noexcept {
  return MutableBuffer(b.size() ? &b[0] : nullptr,
      b.size() * sizeof(PodType) < max_bytes
      ? b.size() * sizeof(PodType) : max_bytes);
}

template <typename PodType, typename Allocator>
inline ConstBuffer buffer(const std::vector<PodType, Allocator>& b) noexcept {
  return ConstBuffer(b.size() ? &b[0] : nullptr, b.size() * sizeof(PodType));
}

template <typename PodType, typename Allocator>
inline ConstBuffer buffer(
    const std::vector<PodType, Allocator>& b, std::size_t max_bytes) noexcept {
  return ConstBuffer(b.size() ? &b[0] : nullptr,
      b.size() * sizeof(PodType) < max_bytes
      ? b.size() * sizeof(PodType) : max_bytes);
}

template <typename Elem, typename Traits, typename Allocator>
inline MutableBuffer buffer(
    std::basic_string<Elem, Traits, Allocator>& b) noexcept {
  return MutableBuffer(b.size() ? &b[0] : nullptr, b.size() * sizeof(Elem));
}

template <typename Elem, typename Traits, typename Allocator>
inline MutableBuffer buffer(
    std::basic_string<Elem, Traits, Allocator>& b,
    std::size_t max_bytes) noexcept {
  return MutableBuffer(b.size() ? &b[0] : nullptr,
      b.size() * sizeof(Elem) < max_bytes
      ? b.size() * sizeof(Elem) : max_bytes);
}

template <typename Elem, typename Traits, typename Allocator>
inline ConstBuffer buffer(
    const std::basic_string<Elem, Traits, Allocator>& b) noexcept {
  return ConstBuffer(b.size() ? &b[0] : nullptr, b.size() * sizeof(Elem));
}

template <typename Elem, typename Traits, typename Allocator>
inline ConstBuffer buffer(
    const std::basic_string<Elem, Traits, Allocator>& b,
    std::size_t max_bytes) noexcept {
  return ConstBuffer(b.size() ? &b[0] : nullptr,
      b.size() * sizeof(Elem) < max_bytes
      ? b.size() * sizeof(Elem) : max_bytes);
}

}
