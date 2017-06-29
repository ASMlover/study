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
#include <cstdint>

namespace gc {

struct MemoryHeader {
  enum {INVALID, INT, PAIR};
  std::int8_t _type{INVALID};
  bool _marked{};
  std::size_t _size{};

  bool is_invalid(void) const { return _type == INVALID; }
  bool is_int(void) const { return _type == INT; }
  bool is_pair(void) const { return _type == PAIR; }
  void set_type(std::int8_t t) { _type = t; }
  std::int8_t type(void) const { return _type; }
  void set_mark(void) { _marked = true; }
  void unset_mark(void) { _marked = false; }
  bool is_marked(void) const { return _marked; }
  std::size_t set_size(std::size_t n) { return _size = n, _size; }
  std::size_t inc_size(std::size_t n) { return _size += n, _size; }
  std::size_t dec_size(std::size_t n) { return _size -= n, _size; }
  std::size_t size(void) const { return _size; }
};

}
