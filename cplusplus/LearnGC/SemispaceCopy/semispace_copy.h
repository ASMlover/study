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

#include <vector>
#include <Chaos/Types.h>

namespace gc {

class BaseObject;

class SemispaceCopy : private Chaos::UnCopyable {
  byte_t* heaptr_{};
  byte_t* fromspace_{};
  byte_t* tospace_{};
  byte_t* allocptr_{};
  byte_t* scanptr_{};
  std::vector<BaseObject*> roots_;
  std::size_t obj_count_{};
  static constexpr std::size_t kSemispaceSize = 512 << 9;

  SemispaceCopy(void);
  ~SemispaceCopy(void);
  void* alloc(std::size_t n);
  void worklist_init(void) { scanptr_ = allocptr_; }
  bool worklist_empty(void) const { return scanptr_ == allocptr_; }
  void worklist_put(BaseObject* /*ob*/) {}
  BaseObject* worklist_fetch(void);
  BaseObject* forward(BaseObject* from_ref);
  BaseObject* copy(BaseObject* from_ref);
public:
  static SemispaceCopy& get_instance(void);

  void collect(void);
  BaseObject* put_in(int value);
  BaseObject* put_in(BaseObject* first = nullptr, BaseObject* second = nullptr);
  BaseObject* fetch_out(void);
};

}
