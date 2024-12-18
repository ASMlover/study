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

FunctionPtr Function::bind(InstancePtr instance) noexcept {
  env::EnvironmentPtr environment = std::make_shared<env::Environment>(closure_);
  environment->define("this", instance);
  return std::make_shared<Function>(declaration_, environment, is_initializer_);
}

value::Value Function::call(const InterpreterPtr& interp, const std::vector<value::Value>& arguments) {
  env::EnvironmentPtr environment = std::make_shared<env::Environment>(closure_);
  for (sz_t i = 0; i < arguments.size(); ++i)
    environment->define(declaration_->params()[i].literal(), arguments[i]);

  try {
    interp->invoke_execute_block(declaration_->body(), environment);
  }
  catch (const except::Return& rv) {
    if (is_initializer_)
      return closure_->get_at(0, "this");

    return rv.value();
  }

  if (is_initializer_)
    return closure_->get_at(0, "this");

  return nullptr;
}

str_t Function::as_string() const {
  ss_t ss;
  ss << "<fn " << declaration_->name().as_string() << ">";
  return ss.str();
}

value::Value Class::call(const InterpreterPtr& interp, const std::vector<value::Value>& arguments) {
  auto instance = std::make_shared<Instance>(shared_from_this());
  FunctionPtr initializer = find_method("init");
  if (initializer)
    initializer->bind(instance)->call(interp, arguments);
  return instance;
}

sz_t Class::arity() const {
  FunctionPtr initializer = find_method("init");
  return initializer ? initializer->arity() : 0;
}

str_t Instance::as_string() const noexcept {
  ss_t ss;
  ss << "<" << klass_->name() << " instance at " << this << ">";
  return ss.str();
}

}
