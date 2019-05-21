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
#include "interpret_errors.hh"
#include "interpret_environment.hh"
#include "interpret_interpreter.hh"
#include "interpret_callable.hh"

namespace loxcc::interpret {

Value Function::call(
    const InterpreterPtr& interp, const std::vector<Value>& args) {
  auto envp = std::make_shared<Environment>(closure_);
  const auto& params = decl_->params();
  for (sz_t i = 0; i < params.size(); ++i)
    envp->define(params[i], args[i]);

  try {
    interp->invoke_evaluate(decl_->body(), envp);
  }
  catch (const Return& r) {
    return is_ctor_ ? closure_->get_at(0, "this") : r.value();
  }

  return is_ctor_ ? closure_->get_at(0, "this") : nullptr;
}

sz_t Function::arity(void) const {
  return decl_->params().size();
}

str_t Function::stringify(void) const {
  std::stringstream ss;

  ss << "<function fun `" + decl_->name().as_string() << "` "
    << "at `" << this << "`>";
  return ss.str();
}

FunctionPtr Function::bind(const InstancePtr& inst) {
  auto envp = std::make_shared<Environment>(closure_);
  envp->define("this", inst);
  return std::make_shared<Function>(decl_, envp, is_ctor_);
}

Value Class::call(
    const InterpreterPtr& interp, const std::vector<Value>& args) {
  auto inst = std::make_shared<Instance>(shared_from_this());
  if (auto ctor = get_method(inst, "ctor"); ctor)
    return ctor->call(interp, args);
  return inst;
}

sz_t Class::arity(void) const {
  if (auto meth_iter = methods_.find("ctor"); meth_iter != methods_.end())
    return meth_iter->second->arity();
  return 0;
}

str_t Class::stringify(void) const {
  return "<class `" + name_ + "`>";
}

FunctionPtr Class::get_method(const InstancePtr& inst, const str_t& name) {
  if (auto meth_iter = methods_.find(name); meth_iter != methods_.end())
    return meth_iter->second->bind(inst);
  if (superclass_)
    return superclass_->get_method(inst, name);
  return nullptr;
}

FunctionPtr Class::get_method(const InstancePtr& inst, const Token& name) {
  return get_method(inst, name.as_string());
}

str_t Instance::stringify(void) const {
  std::stringstream ss;

  ss << "<`" << cls_->name() << "` object at `" << this << "`>";
  return ss.str();
}

void Instance::set_attr(const Token& name, const Value& value) {
  attrs_[name.as_string()] = value;
}

Value Instance::get_attr(const Token& name) {
  str_t attr_name = name.as_string();
  if (auto attr_iter = attrs_.find(attr_name); attr_iter != attrs_.end())
    return attr_iter->second;

  CallablePtr method = cls_->get_method(shared_from_this(), attr_name);
  if (method)
    return method;

  throw RuntimeError(name, "undefined attribute `" + attr_name + "`");
}

}
