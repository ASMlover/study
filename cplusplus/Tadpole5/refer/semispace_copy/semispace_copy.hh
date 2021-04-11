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
#pragma once

#include <vector>
#include "object.hh"

namespace tadpole::gc {

class SemispaceCopy final : public Singleton<SemispaceCopy> {
  static constexpr sz_t kSmispaceSize = 512 << 9;

  byte_t* heapptr_{};
  byte_t* freeptr_{};
  byte_t* scanptr_{};
  byte_t* fromspace_{};
  byte_t* tospace_{};
  std::vector<BaseObject*> roots_;
  sz_t objcount_{};

  void alloc_fail();
  void* alloc(sz_t n);
  void flip();
  BaseObject* copy(BaseObject* from_ref);
public:
  SemispaceCopy() noexcept;
  ~SemispaceCopy() noexcept;

  void collect();

  template <typename Object, typename... Args> inline Object* create_object(Args&&... args) {
    void* p = alloc(sizeof(Object));
    Object* o = new (p) Object(std::forward<Args>(args)...);

    ++objcount_;
    return o;
  }
};

}
