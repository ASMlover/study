// Copyright (c) 2026 ASMlover. All rights reserved.
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

#include <type_traits>
#include <sstream>
#include "Types.hh"
#include "Consts.hh"

namespace ms {

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
protected:
  UnCopyable() noexcept = default;
  ~UnCopyable() noexcept = default;
  UnCopyable(const UnCopyable&) = delete;
  UnCopyable(UnCopyable&&) = delete;
  UnCopyable& operator=(const UnCopyable&) = delete;
  UnCopyable& operator=(UnCopyable&&) = delete;
};

template <typename T>
class Singleton : private UnCopyable {
public:
  static T& get_instance() noexcept {
    static T instance;
    return instance;
  }
};

template <typename T, typename U>
inline T as_type(U x) noexcept {
  return static_cast<T>(x);
}

template <typename T, typename U>
inline T* as_down(U* p) noexcept {
  return static_cast<T*>(p);
}

template <typename T>
inline T* as_ptr(T& ref) noexcept {
  return &ref;
}

template <typename T>
inline str_t as_string(T&& val) {
  std::stringstream ss;
  ss << std::forward<T>(val);
  return ss.str();
}

} // namespace ms
