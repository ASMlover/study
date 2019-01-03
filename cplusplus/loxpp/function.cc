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
#include "return.h"
#include "environment.h"
#include "interpreter.h"
#include "function.h"

Value LoxFunction::call(
    const InterpreterPtr& interp, const std::vector<Value>& args) {
  auto env = std::make_shared<Environment>(interp->get_globals());
  auto& params = declaration_->function_->params_;
  std::size_t n = params.size();
  for (std::size_t i = 0; i < n; ++i)
    env->define_var(params[i].get_lexeme(), args[i]);

  try {
    interp->invoke_evaluate_block(declaration_->function_->body_, env);
  }
  catch (const Return& r) {
    return r.value_;
  }

  return Value();
}

int LoxFunction::arity(void) const {
  return static_cast<int>(declaration_->function_->params_.size());
}

std::string LoxFunction::to_string(void) const {
  return "<fn " + declaration_->name_.get_lexeme() + ">";
}
