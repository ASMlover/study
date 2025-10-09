// Copyright (c) 2025 ASMlover. All rights reserved.
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
#include "Callable.hh"

namespace ms {

// ========= Function ========
FunctionPtr Function::bind(InstancePtr instance) noexcept {
  auto env = std::make_shared<Environment>(closure_);
  env->define("this", instance);
  return std::make_shared<Function>(declaration_, env, is_initializer_);
}

Value Function::call(const InterpreterPtr& interp, const std::vector<Value>& arguments) {
  // TODO:
  return nullptr;
}

str_t Function::as_string() const {
  ss_t ss;
  ss << "<fn " << declaration_->name().as_string() << ">";
  return ss.str();
}

// ========= Class =========
Value Class::call(const InterpreterPtr& interp, const std::vector<Value>& arguments) {
  // TODO:
  return nullptr;
}

sz_t Class::arity() const {
  auto initializer = find_method("init");
  return initializer ? initializer->arity() : 0;
}

// ========= Instance =========
str_t Instance::as_string() const noexcept {
  ss_t ss;
  ss << "<" << klass_->name() << " instance at " << this << ">";
  return ss.str();
}

}
