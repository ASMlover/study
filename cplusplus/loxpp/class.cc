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
#include "function.h"
#include "instance.h"
#include "class.h"

Value LoxClass::call(
    const InterpreterPtr& interp, const std::vector<Value>& args) {
  auto inst = std::make_shared<LoxInstance>(shared_from_this());
  auto ctor = get_method(inst, "ctor");
  if (ctor)
    ctor->call(interp, args);
  return Value(inst);
}

int LoxClass::arity(void) const {
  if (methods_.count("ctor"))
    return methods_.at("ctor")->arity();
  return 0;
}

std::string LoxClass::to_string(void) const {
  return "<class " + name_ + ">";
}


std::shared_ptr<LoxFunction> LoxClass::get_method(
    const std::shared_ptr<LoxInstance>& inst, const std::string& name) {
  if (methods_.count(name))
    return methods_[name]->bind(inst);

  if (super_class_)
    return super_class_->get_method(inst, name);

  return nullptr;
}
