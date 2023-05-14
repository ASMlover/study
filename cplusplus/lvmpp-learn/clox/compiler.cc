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
#include "common.hh"
#include "vm.hh"
#include "token.hh"
#include "scanner.hh"
#include "compiler.hh"

namespace clox {

class Parser final : private UnCopyable {
  VM& vm_;
  Scanenr& scanner_;
  Token previous_;
  Token current_;
  bool had_error_{};
  bool panic_mode_{};

  void advance() {
    previous_ = current_;

    for (;;) {
      current_ = scanner_.scan_token();
      if (current_.type() != TokenType::TOKEN_ERROR)
        break;

      error_at_current(current_.as_string());
    }
  }

  void consume(TokenType type, const str_t& message) {
    if (current_.type() == type) {
      advance();
      return;
    }

    error_at_current(message);
  }

  inline void error_at_current(const str_t& message) noexcept { error_at(current_, message); }
  inline void error(const str_t& message) noexcept { error_at(previous_, message); }

  void error_at(const Token& token, const str_t& message) noexcept {
    if (panic_mode_)
      return;
    panic_mode_ = true;

    std::cerr << "[line " << token.lineno() << "] Error";
    if (token.type() == TokenType::TOKEN_EOF) {
      std::cerr << " at end";
    }
    else if (token.type() == TokenType::TOKEN_ERROR) {
    }
    else {
      std::cerr << " at `" << token.as_string() << "`";
    }
    std::cerr << ": " << message << std::endl;

    had_error_ = true;
  }
public:
  Parser(VM& vm, Scanenr& scanner) noexcept : vm_{vm}, scanner_{scanner} {}

  bool compile() {
    advance();
    // TODO: expression();
    // consume(...)

    return !had_error_;
  }
};

void Compiler::compile(VM& vm, const str_t& source) noexcept {
  Scanenr scanner(source);

  if (parser_ = new Parser{vm, scanner}; parser_ != nullptr) {
    parser_->compile();
    delete parser_;
    parser_ = nullptr;
  }
}

}
