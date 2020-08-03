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
#include <iostream>
#include "vm.hh"

namespace tadpole {

class CallFrame final : public Copyable {
};

VM::VM() noexcept {
}

VM::~VM() {
}

void VM::define_native(const str_t& name, NativeFn&& fn) {
}

void VM::append_object(BaseObject* o) {
  if (objects_.size() >= kGCThreshold)
    collect();

  objects_.push_back(o);
}

void VM::mark_object(BaseObject* o) {
  if (o == nullptr || o->is_marked())
    return;

#if defined(_TADPOLE_DEBUG_GC)
  std::cout << "[" << o << "] mark object: `" << o->stringify() << "`" << std::endl;
#endif

  o->set_marked(true);
  worklist_.push_back(o);
}

void VM::mark_value(const Value& v) {
  if (v.is_object())
    mark_object(v.as_object());
}

void VM::collect() {
}

}
