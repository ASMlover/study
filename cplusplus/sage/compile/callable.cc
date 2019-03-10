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
#include "../common/errors.hh"
#include "environment.hh"
#include "interpret_helper.hh"
#include "interpreter.hh"
#include "callable.hh"

namespace sage {

Function::Function(
    const FunctionStmtPtr& decl, const EnvironmentPtr& closure, bool is_ctor)
  : decl_(decl)
  , closure_(closure)
  , is_ctor_(is_ctor) {
}

std::string Function::name(void) const {
  return decl_->name().get_literal();
}

Value Function::call(
    const InterpreterPtr& interp, const std::vector<Value>& arguments) {
  auto envp = std::make_shared<Environment>(closure_);
  auto& params = decl_->params();
  std::size_t n{params.size()};
  for (std::size_t i = 0; i < n; ++i)
    envp->define(params[i].get_literal(), arguments[i]);

  try {
    interp->invoke_evaluate_block(decl_->body(), envp);
  }
  catch (const Return& r) {
    if (is_ctor_)
      return closure_->get_at(0, "self");
    return r.value();
  }
  if (is_ctor_)
    return closure_->get_at(0, "self");
  return nullptr;
}

std::size_t Function::arity(void) const {
  return decl_->params().size();
}

std::string Function::to_string(void) const {
  return "<script fn `" + decl_->name().get_literal() + "`>";
}

FunctionPtr Function::bind(const InstancePtr& inst) {
  auto envp = std::make_shared<Environment>(closure_);
  envp->define("self", Value(inst));
  return std::make_shared<Function>(decl_, envp, is_ctor_);
}

Class::Class(const std::string& name,
    const ClassPtr& superclass,
    const std::unordered_map<std::string, FunctionPtr>& methods)
  : name_(name)
  , superclass_(superclass)
  , methods_(methods) {
}

std::string Class::name(void) const {
  return name_;
}

Value Class::call(
    const InterpreterPtr& interp, const std::vector<Value>& arguments) {
  auto inst = std::make_shared<Instance>(shared_from_this());
  auto ctor = get_method(inst, "ctor");
  if (ctor)
    ctor->call(interp, arguments);
  return inst;
}

std::size_t Class::arity(void) const {
  auto method_iter = methods_.find("ctor");
  if (method_iter != methods_.end())
    return method_iter->second->arity();
  return 0;
}

std::string Class::to_string(void) const {
  return "<script class `" + name_ + "`>";
}

FunctionPtr Class::get_method(
    const InstancePtr& inst, const std::string& name) {
  auto method_iter = methods_.find(name);
  if (method_iter != methods_.end())
    return method_iter->second->bind(inst);
  if (superclass_)
    return superclass_->get_method(inst, name);
  return nullptr;
}

FunctionPtr Class::get_method(const InstancePtr& inst, const Token& name) {
  return get_method(inst, name.get_literal());
}

Instance::Instance(const ClassPtr& cls)
  : class_(cls) {
}

bool Instance::is_truthy(void) const {
  // TODO: default is true
  return true;
}

std::string Instance::to_string(void) const {
  std::stringstream ss;
  ss << "<`" << class_->name() << "` instance at " << this << ">";
  return ss.str();
}

void Instance::set_attribute(const Token& name, const Value& value) {
  attributes_[name.get_literal()] = value;
}

Value Instance::get_attribute(const Token& name) {
  auto attr_iter = attributes_.find(name.get_literal());
  if (attr_iter != attributes_.end())
    return attr_iter->second;

  auto method = class_->get_method(shared_from_this(), name.get_literal());
  if (method)
    return method;

  throw RuntimeError(name, "undefined attribute `" + name.get_literal() + "`");
}

}
