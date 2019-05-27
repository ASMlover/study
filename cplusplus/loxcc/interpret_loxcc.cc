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
#include "lexer.hh"
#include "interpret_parser.hh"
#include "interpret_resolver.hh"
#include "interpret_interpreter.hh"
#include "interpret_loxcc.hh"

namespace loxcc::interpret {

Loxcc::Loxcc(void) noexcept
  : err_report_()
  , interp_(new Interpreter(err_report_)) {
}

void Loxcc::eval_impl(const str_t& source_bytes) {
  Lexer lex(source_bytes);
  Parser p(err_report_, lex);

  auto stmts = p.parse();
  if (err_report_.had_error())
    std::exit(-1);

  auto resolver = std::make_shared<Resolver>(err_report_, interp_);
  resolver->invoke_resolve(stmts);
  if (err_report_.had_error())
    std::exit(-2);

  interp_->interpret(stmts);
  if (err_report_.had_error())
    std::exit(-3);
}

}
