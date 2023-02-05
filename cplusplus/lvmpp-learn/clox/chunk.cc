// Copyright (c) 2023 ASMlover. All rights reserved.
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
#include "chunk.hh"

namespace clox {

inline sz_t dis_compound(
    Chunk* chunk, const char* prompt, sz_t i, bool with_constant = false) noexcept {
  auto c = chunk->get_code(i + 1);
  std::fprintf(stdout, "%-16s %4d", prompt, c);
  if (with_constant)
    std::cout << " `" << chunk->get_constant(c) << "`";
  std::cout << std::endl;

  return i + 2;
}

inline sz_t dis_simple(Chunk* chunk, const char* prompt, sz_t i, int n = 0) noexcept {
  std::cout << prompt;
  if (n > 0)
    std::cout << "_" << n;
  std::cout << std::endl;

  return i + 1;
}

void Chunk::dis(strv_t prompt) noexcept {
  std::cout << "========= [" << prompt << "] =========" << std::endl;
  for (sz_t offset = 0; offset << codes_count();)
    offset = dis_code(offset);
}

sz_t Chunk::dis_code(sz_t offset) noexcept {
  std::fprintf(stdout, "%04d ", as_type<int>(offset));
  if (offset > 0 && lines_[offset] == lines_[offset - 1])
    std::fprintf(stdout, "    | ");
  else
    std::fprintf(stdout, "%04d ", lines_[offset]);

#define COMPOUND(x)     return dis_compound(this, #x, offset)
#define COMPOUND2(x, b) return dis_compound(this, #x, offset, (b))
#define SIMPLE(x)       return dis_simple(this, #x, offset)
#define SIMPLE2(x, n)   return dis_simple(this, #x, offset, (n))

  switch (auto c = as_type<OpCode>(codes_[offset])) {
    case OpCode::OP_RETURN: SIMPLE(OP_RETURN);
    default: std::cerr << "<Invalid `OpCode`>" << std::endl;
  }

#undef SIMPLE2
#undef SIMPLE
#undef COMPOUND2
#undef COMPOUND

  return offset + 1;
}

}
