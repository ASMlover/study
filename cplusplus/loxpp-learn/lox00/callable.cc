// Copyright (c) 2022 ASMlover. All rights reserved.
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
#include "interpreter.hh"
#include "environment.hh"
#include "return.hh"
#include "callable.hh"

namespace loxpp::callable {

value::Value Function::call(const InterpreterPtr& interp, const std::vector<value::Value>& arguments) {
  env::EnvironmentPtr environment = std::make_shared<env::Environment>(closure_);
  for (sz_t i = 0; i < arguments.size(); ++i)
    environment->define(declaration_->params()[i].literal(), arguments[i]);

  try {
    interp->invoke_execute_block(declaration_->body(), environment);
  }
  catch (const except::Return& rv) {
    return rv.value();
  }

  return nullptr;
}

str_t Function::as_string() const {
  ss_t ss;
  ss << "<fn " << declaration_->name().as_string() << ">";
  return ss.str();
}

}
