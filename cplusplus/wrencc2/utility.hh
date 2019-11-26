// Copyright (c) 2019 ASMlover. All rights reserved.
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

#include <functional>
#include <vector>
#include "common.hh"
#include "string.hh"

namespace wrencc {

template <typename T>
using CompareFn = std::function<bool (const T&, const T&)>;

template <typename T>
inline bool __compare(const T& x, const T& y) { return x == y; }

template <typename T, typename Function = CompareFn<T>>
class DynamicTable final : private UnCopyable {
  using Iterator = typename std::vector<T>::iterator;
  using CIterator = typename std::vector<T>::const_iterator;

  Function cmp_fn_{};
  std::vector<T> datas_;
public:
  DynamicTable(Function&& cmp_fn = __compare<T>) noexcept
    : cmp_fn_(std::move(cmp_fn)) {
  }

  inline int count() const noexcept { return Xt::as_type<int>(datas_.size()); }
  inline void clear() noexcept { datas_.clear(); }
  inline T& operator[](int i) noexcept { return datas_[i]; }
  inline const T& operator[](int i) const noexcept { return datas_[i]; }
  inline T& at(int i) noexcept { return datas_[i]; }
  inline const T& at(int i) const noexcept { return datas_[i]; }
  inline Iterator begin() noexcept { return datas_.begin(); }
  inline CIterator begin() const noexcept { return datas_.begin(); }
  inline Iterator end() noexcept { return datas_.end(); }
  inline CIterator end() const noexcept { return datas_.end(); }

  inline int append(const T& x) {
    datas_.push_back(x);
    return count() - 1;
  }

  inline int ensure(const T& x) {
    if (int existing = find(x); existing != -1)
      return existing;
    return append(x);
  }

  inline int find(const T& x) {
    for (int i = 0; i < count(); ++i) {
      if (cmp_fn_(datas_[i], x))
        return i;
    }
    return -1;
  }

  inline void iter_values(std::function<void (T&)>&& fn) {
    for (auto& x : datas_)
      fn(x);
  }

  inline void iter_values(std::function<void (const T&)>&& fn) {
    for (auto& x : datas_)
      fn(x);
  }
};

using SymbolTable = DynamicTable<String>;

}
