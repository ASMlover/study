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
#include <cstdarg>
#include <iostream>
#include "bytecc_chunk.hh"
#include "bytecc_vm.hh"

namespace loxcc::bytecc {

class CallFrame final : public Copyable {
  ClosureObject* closure_{};
  const u8_t* ip_{};
  int begpos_{};
public:
  CallFrame(ClosureObject* closure, const u8_t* ip, int begpos = 0) noexcept
    : closure_(closure), ip_(ip), begpos_(begpos) {
  }

  inline ClosureObject* closure(void) const { return closure_; }
  inline const u8_t* ip(void) const { return ip_; }
  inline int begpos(void) const { return begpos_; }

  inline void set_ip(const u8_t* ip) { ip_ = ip; }
  inline u8_t get_ip(int i) const { return ip_[i]; }
  inline u8_t inc_ip(void) { return *ip_++; }
  inline u8_t dec_ip(void) { return *ip_--; }
  inline void add_ip(int offset) { ip_ += offset; }
  inline void sub_ip(int offset) { ip_ -= offset; }

  inline FunctionObject* frame_fn(void) const { return closure_->fn(); }
  inline Chunk* frame_chunk(void) const { return closure_->fn()->chunk(); }
};

VM::VM(void) noexcept {
  // TODO:
}

VM::~VM(void) {
  // TODO:
}

void VM::runtime_error(const char* format, ...) {
  va_list ap;
  va_start(ap, format);
  vfprintf(stderr, format, ap);
  va_end(ap);
  fprintf(stderr, "\n");

  int i = Xt::as_type<int>(frames_.size()) - 1;
  for (; i >= 0; --i) {
    auto& frame = frames_[i];
    FunctionObject* fn = frame.frame_fn();
    Chunk* chunk = frame.frame_chunk();
    int ins = frame.ip() - chunk->codes() - 1;

    std::cerr << "[LINE: " << chunk->get_line(ins) << "] at "
      << "`" << fn->name_astr() << "()`" << std::endl;
  }
  reset();
}

void VM::reset(void) {
  // TODO:
}

}
