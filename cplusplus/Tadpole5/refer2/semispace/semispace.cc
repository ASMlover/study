// Copyright (c) 2021 ASMlover. All rights reserved.
//
//  ______             __                  ___           ____    ____
// /\__  _\           /\ \                /\_ \         /\  _`\ /\  _`\
// \/_/\ \/    __     \_\ \  _____     ___\//\ \      __\ \ \L\_\ \ \/\_\
//    \ \ \  /'__`\   /'_` \/\ '__`\  / __`\\ \ \   /'__`\ \ \L_L\ \ \/_/_
//     \ \ \/\ \L\.\_/\ \L\ \ \ \L\ \/\ \L\ \\_\ \_/\  __/\ \ \/, \ \ \L\ \
//      \ \_\ \__/.\_\ \___,_\ \ ,__/\ \____//\____\ \____\\ \____/\ \____/
//       \/_/\/__/\/_/\/__,_ /\ \ \/  \/___/ \/____/\/____/ \/___/  \/___/
//                             \ \_\
//                              \/_/
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
#include <exception>
#include "semispace.hh"

namespace tadpole::gc {

Semispace::Semispace() noexcept {
  heapptr_ = new byte_t[kSemispaceSize * 2];

  fromspace_ = heapptr_ + kSemispaceSize;
  tospace_ = heapptr_;
  freeptr_ = tospace_;
}

Semispace::~Semispace() noexcept {
  delete [] heapptr_;
}

void* Semispace::alloc(sz_t n) {
  if (freeptr_ + n > tospace_ + kSemispaceSize) {
    collect();

    if (freeptr_ + n > tospace_ + kSemispaceSize)
      throw std::runtime_error("[Semispace] out of memory ...");
  }

  auto* p = freeptr_;
  freeptr_ += n;
  return p;
}

void Semispace::flip() {
  std::swap(fromspace_, tospace_);
  freeptr_ = tospace_;
}

void Semispace::collect() {
}

}
