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
#include "mark_sweep.hh"

namespace tadpole::gc {


MarkSweep::MarkSweep() noexcept {
  heapptr_ = new byte_t[kHeapSize];

  freelist_ = new (heapptr_) MemoryHeader(MemoryTag::MEMORY, kHeapSize, nullptr);
}

MarkSweep::~MarkSweep() noexcept {
  delete [] heapptr_;
}

void* MarkSweep::alloc(sz_t n) {
  MemoryHeader* prev = freelist_;

  for (auto* p = prev->next(); ; prev = p, p = p->next()) {
    bool is_first = false;
    if (p == nullptr) {
      p = freelist_;
      is_first = true;
    }

    sz_t freesize = p->size();
    if (freesize >= n) {
      MemoryHeader* r = p;
      if (freesize == n || freesize - n < kHeaderSize) {
        if (is_first)
          p = freelist_ = freelist_->next();
        else
          prev->set_next(p->next());
      }
      else {
        p = as_ptr<MemoryHeader>(as_ptr<byte_t>(p) + n);
        p->set_tag(MemoryTag::MEMORY);
        p->set_size(freesize - n);
        p->set_next(r->next());

        r->set_size(n);
      }

      return r;
    }

    if (p == nullptr)
      break;
  }

  return nullptr;
}

}
