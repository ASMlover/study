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
#include <Tadpole/Chunk.hh>

namespace _mevo::tadpole {

void Chunk::dis(const str_t& s) {
  std::cout << "========= [" << s << "] =========" << std::endl;
  for (sz_t i = 0; i < codes_.size();)
    i = dis_code(i);
}

sz_t Chunk::dis_code(sz_t i) {
  auto const_code = [this](const char* s, sz_t i) -> sz_t {
    auto c = codes_[i + 1];
    fprintf(stdout, "%-16s %4d ", s, c);
    std::cout << "`" << get_constant(c) << "`" << std::endl;
    return i + 2;
  };
  auto simple_code = [](const char* s, sz_t i) -> sz_t {
    std::cout << s << std::endl;
    return i + 1;
  };
  auto simple_codeN = [](const char* s, sz_t i, sz_t n) -> sz_t {
    std::cout << s << n << std::endl;
    return i + 1;
  };
  auto code_code = [this](const char* s, sz_t i) -> sz_t {
    fprintf(stdout, "%-16s %4d\n", s, codes_[i + 1]);
    return i + 2;
  };

  fprintf(stdout, "%04d ", as_type<int>(i));
  if (i > 1 && lines_[i] == lines_[i - 1])
    std::cout << "   | ";
  else
    fprintf(stdout, "%4d ", lines_[i]);

  switch (auto c = as_type<Code>(codes_[i])) {
  case Code::CONSTANT: return const_code("CONSTANT", i);
  case Code::NIL: return simple_code("NIL", i);
  case Code::FALSE: return simple_code("FALSE", i);
  case Code::TRUE: return simple_code("TRUE", i);
  case Code::POP: return simple_code("NIL", i);
  case Code::DEF_GLOBAL: return const_code("DEF_GLOBAL", i);
  case Code::GET_GLOBAL: return const_code("GET_GLOBAL", i);
  case Code::SET_GLOBAL: return const_code("SET_GLOBAL", i);
  case Code::GET_LOCAL: return code_code("GET_LOCAL", i);
  case Code::SET_LOCAL: return code_code("SET_LOCAL", i);
  case Code::GET_UPVALUE: return code_code("GET_UPVALUE", i);
  case Code::SET_UPVALUE: return code_code("SET_UPVALUE", i);
  case Code::ADD: return simple_code("ADD", i);
  case Code::SUB: return simple_code("SUB", i);
  case Code::MUL: return simple_code("MUL", i);
  case Code::DIV: return simple_code("DIV", i);
  case Code::CALL_0:
  case Code::CALL_1:
  case Code::CALL_2:
  case Code::CALL_3:
  case Code::CALL_4:
  case Code::CALL_5:
  case Code::CALL_6:
  case Code::CALL_7:
  case Code::CALL_8: return simple_codeN("CALL_", i, c - Code::CALL_0);
  case Code::CLOSURE: return const_code("CLOSURE", i);
  case Code::CLOSE_UPVALUE: return simple_code("CLOSE_UPVALUE", i);
  case Code::RETURN: return simple_code("RETURN", i);
  default: std::cerr << "<Unknown Code>" << std::endl; break;
  }
  return i + 1;
}

}
