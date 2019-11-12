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
#include "utils.hh"

namespace wrencc {

int SymbolTable::ensure(const str_t& name) {
  int existing = get(name);
  if (existing != -1)
    return existing;

  symbols_.push_back(name);
  return Xt::as_type<int>(symbols_.size() - 1);
}

int SymbolTable::add(const str_t& name) {
  symbols_.push_back(name);
  return Xt::as_type<int>(symbols_.size() - 1);
}

int SymbolTable::get(const str_t& name) const {
  for (sz_t i = 0; i < symbols_.size(); ++i) {
    if (symbols_[i] == name)
      return Xt::as_type<int>(i);
  }
  return -1;
}

void SymbolTable::clear(void) {
  symbols_.clear();
}

void SymbolTable::truncate(int count) {
  auto n = Xt::as_type<sz_t>(count);
  ASSERT(n <= symbols_.size(), "cannot truncate to larger size");
  symbols_.resize(n);
}

}
