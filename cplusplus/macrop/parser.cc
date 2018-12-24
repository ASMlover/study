// Copyright (c) 2018 ASMlover. All rights reserved.
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
#include <cstdlib>
#include <iostream>
#include "parser.h"

void Parser::dump_macros(void) {
  while (!is_eof())
    dump_macro();
}

void Parser::report_error(const std::string& err, const Token& tok) {
  std::cerr << err << " : " << tok.get_lexeme() << std::endl;
  std::abort();
}

bool Parser::is_eof(void) const {
  return pos_ >= tokens_ref_.size();
}

const Token& Parser::peek(void) const {
  if (is_eof())
    return tokens_ref_.back();
  return tokens_ref_[pos_];
}

const Token& Parser::advance(void) {
  return tokens_ref_[pos_++];
}

void Parser::dump_macro(void) {
  auto& tok = advance();
  switch (tok.get_kind()) {
  case TokenKind::PP_IF:
    break;
  }
}

void Parser::resolve_macro_if(void) {
  auto& tok = peek();
  if (tok.get_kind() == TokenKind::IDENTIFILER) {
    auto it = valid_macros_.find(tok.get_lexeme());
    if (it != valid_macros_.end()) {
      auto kind = it->second.get_kind();
    }
    else {
      // TODO: scan to endif
    }
  }
  else {
    report_error("invalid #if macro", tok);
  }
}
