// Copyright (c) 2018 ASMlover. All rights reserved.
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

#include <limits>

namespace ext {

template <typename T>
class TaggedPtr {
  using tag_t = std::size_t;

  T* ptr_{};
  tag_t tag_{};
public:
  TaggedPtr(void) noexcept = default;
  TaggedPtr(const TaggedPtr&) = default;
  TaggedPtr& operator=(const TaggedPtr&) = default;

  explicit TaggedPtr(T* p, tag_t t = 0)
    : ptr_(p)
    , tag_(t) {
  }

  void set(T* p, tag_t t) {
    ptr_ = p;
    tag_ = t;
  }

  T* get_ptr(void) const {
    return ptr_;
  }

  void set_ptr(T* p) {
    ptr_ = p;
  }

  tag_t get_tag(void) const {
    return tag_;
  }

  void set_tag(tag_t t) {
    tag_ = t;
  }

  tag_t get_next_tag(void) const {
    return (get_tag() + 1) & (std::numeric_limits<tag_t>::max)();
  }

  T& operator*(void) const {
    return *ptr_;
  }

  T* operator->(void) const {
    return ptr_;
  }

  operator bool(void) const {
    return ptr_ != nullptr;
  }

  bool operator==(volatile const TaggedPtr& p) const {
    return (ptr_ == p.ptr_) && (tag_ == p.tag_);
  }

  bool operator!=(volatile const TaggedPtr& p) const {
    return !operator==(p);
  }
};

}
