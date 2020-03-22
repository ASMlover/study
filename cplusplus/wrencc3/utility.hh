// Copyright (c) 2020 ASMlover. All rights reserved.
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

namespace wrencc {

template <typename T>
using CompareFn = std::function<bool (const T&, const T&)>;

template <typename T>
inline bool __compare(const T& x, const T& y) { return x == y; }

template <typename T, typename Compare = CompareFn<T>>
class DynamicTable final : private UnCopyable {
  Compare compare_{};
  std::vector<T> datas_;
public:
  DynamicTable(Compare&& cmp = __compare<T>) noexcept
    : compare_(cmp) {
  }

  inline sz_t size() const noexcept { return datas_.size(); }
  inline int count() const noexcept { return Xt::as_type<int>(size()); }
  inline void clear() noexcept { datas_.clear(); }
  inline T& operator[](sz_t i) noexcept { return datas_[i]; }
  inline const T& operator[](sz_t i) const noexcept { return datas_[i]; }
  inline T& at(sz_t i) noexcept { return datas_[i]; }
  inline const T& at(sz_t i) const noexcept { return datas_[i]; }

  inline int append(const T& x) {
    datas_.push_back(x);
    return count() - 1;
  }

  inline int append(T&& x) {
    datas_.push_back(std::move(x));
    return count() - 1;
  }

  template <typename... Args> inline int append(Args&&... args) {
    datas_.push_back(std::forward<Args>(args)...);
    return count() - 1;
  }

  int ensure(const T& x) {
    if (int existing = find(x); existing != -1)
      return existing;
    return append(x);
  }

  int find(const T& x) const noexcept {
    for (int i = 0; i < count(); ++i) {
      if (compare_(datas_[i], x))
        return i;
    }
    return -1;
  }

  template <typename Visitor> inline void for_each(Visitor&& visitor) {
    for (auto& v : datas_)
      visitor(v);
  }
};

using SymbolTable = DynamicTable<str_t>;

}