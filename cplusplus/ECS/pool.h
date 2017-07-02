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

#include <cassert>
#include <vector>

namespace ecs {

class BasePool {
public:
  virtual ~BasePool(void) {}
  virtual void clear(void) = 0;
};

template <typename T>
class Pool : public BasePool {
  std::vector<T> data_;
public:
  Pool(std::size_t n = 128) { resize(n); }
  virtual ~Pool(void) {}
  bool is_empty(void) const { return data_.empty(); }
  std::size_t get_size(void) const { return data_.size(); }
  void resize(std::size_t n) { data_.resize(n); }
  void append(const T& obj) { data_.push_back(obj); }
  T& operator[](std::size_t index) { return data_[index]; }
  const T& operator[](std::size_t index) const { return data_[index]; }
  std::vector<T>& get_data(void) { return data_; }

  bool set(std::size_t index, T obj) {
    assert(index < get_size());
    return data_[index] = obj, true;
  }

  T& get(std::size_t index) {
    assert(index < get_size());
    return static_cast<T&>(data_[index]);
  }
};

}
