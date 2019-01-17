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
#include <sstream>
#include "errors.h"
#include "return.h"
#include "token.h"
#include "environment.h"
#include "interpreter.h"
#include "callable.h"

namespace lox {

Value Function::call(
    const InterpreterPtr& interp, const std::vector<Value>& arguments) {
  auto envp = std::make_shared<Environment>(closure_);
  for (auto i = 0u; i < arguments.size(); ++i)
    envp->define(decl_->params()[i], arguments[i]);

  try {
    interp->invoke_evaluate_block(decl_->body(), envp);
  }
  catch (const Return& r) {
    return r.value();
  }
  return nullptr;
}

std::size_t Function::arity(void) const {
  return decl_->params().size();
}

std::string Function::to_string(void) const {
  return "<fn `" + decl_->name().get_literal() + "`>";
}

FunctionPtr Function::bind(const InstancePtr& inst) {
  auto envp = std::make_shared<Environment>(closure_);
  envp->define("self", Value(inst));
  return std::make_shared<Function>(decl_, envp);
}

Value Class::call(
    const InterpreterPtr& interp, const std::vector<Value>& arguments) {
  auto inst = std::make_shared<Instance>(shared_from_this());
  return inst;
}

std::size_t Class::arity(void) const {
  return 0;
}

std::string Class::to_string(void) const {
  return "<class " + name_ + ">";
}

FunctionPtr Class::get_method(
    const InstancePtr& inst, const std::string& name) {
  auto meth_iter = methods_.find(name);
  if (meth_iter != methods_.end())
    return meth_iter->second->bind(inst);
  return nullptr;
}

std::string Instance::to_string(void) const {
  std::stringstream ss;
  ss << "<" << class_->name() << " instance at " << this << ">";
  return ss.str();
}

void Instance::set_property(const Token& name, const Value& value) {
  properties_[name.get_literal()] = value;
}

Value Instance::get_property(const Token& name) {
  auto prop_iter = properties_.find(name.get_literal());
  if (prop_iter != properties_.end())
    return prop_iter->second;

  auto method = class_->get_method(shared_from_this(), name.get_literal());
  if (method)
    return method;

  throw RuntimeError(name,
      "undefined property `" + name.get_literal() + "` ...");
}

}
