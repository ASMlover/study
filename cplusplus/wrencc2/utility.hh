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
#include "common.hh"
#include "./container/string.hh"
#include "./container/array_list.hh"

namespace wrencc {

template <typename T>
using CompareFn = std::function<bool (const T&, const T&)>;

template <typename T>
inline bool __compare(const T& x, const T& y) { return x == y; }

template <typename T, typename Function = CompareFn<T>>
class DynamicTable final : private UnCopyable {
  Function compare_{};
  ArrayList<T> datas_;
public:
  DynamicTable(Function&& fn = __compare<T>) noexcept
    : compare_(std::move(fn)) {
  }

  inline int count() const noexcept { return Xt::as_type<int>(datas_.size()); }
  inline void clear() noexcept { datas_.clear(); }
  inline T& operator[](int i) noexcept { return datas_[i]; }
  inline const T& operator[](int i) const noexcept { return datas_[i]; }
  inline T& at(int i) noexcept { return datas_[i]; }
  inline const T& at(int i) const noexcept { return datas_[i]; }

  inline int append(const T& x) {
    datas_.append(x);
    return count() - 1;
  }

  inline int ensure(const T& x) {
    if (int existing = find(x); existing != -1)
      return existing;
    return append(x);
  }

  inline int find(const T& x) {
    for (int i = 0; i < count(); ++i) {
      if (compare_(datas_[i], x))
        return i;
    }
    return -1;
  }

  template <typename Function>
  inline void for_each(Function&& fn) { datas_.for_each(fn); }
};

using SymbolTable = DynamicTable<String>;

}
