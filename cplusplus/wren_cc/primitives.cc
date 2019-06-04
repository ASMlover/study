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
#include <iostream>
#include "vm.hh"
#include "primitives.hh"

namespace wrencc {

static Value _primitive_numeric_abs(VM& vm, int argc, Value* args) {
  double d = Xt::down<NumericObject>(args[0])->value();
  if (d < 0)
    d = -d;
  else if (d == 0)
    d = 0;
  return NumericObject::make_numeric(d);
}

static Value _primitive_numeric_add(VM& vm, int argc, Value* args) {
  if (!args[1]->is_numeric())
    return vm.unsupported();

  return NumericObject::make_numeric(
      args[0]->as_numeric()->value() + args[1]->as_numeric()->value());
}

static Value _primitive_numeric_sub(VM& vm, int argc, Value* args) {
  if (!args[1]->is_numeric())
    return vm.unsupported();

  return NumericObject::make_numeric(
      args[0]->as_numeric()->value() - args[1]->as_numeric()->value());
}

static Value _primitive_numeric_mul(VM& vm, int argc, Value* args) {
  if (!args[1]->is_numeric())
    return vm.unsupported();

  return NumericObject::make_numeric(
      args[0]->as_numeric()->value() * args[1]->as_numeric()->value());
}

static Value _primitive_numeric_div(VM& vm, int argc, Value* args) {
  if (!args[1]->is_numeric())
    return vm.unsupported();

  return NumericObject::make_numeric(
      args[0]->as_numeric()->value() / args[1]->as_numeric()->value());
}

static Value _primitive_string_len(VM& vm, int argc, Value* args) {
  int len = Xt::as_type<int>(strlen(args[0]->as_string()->cstr()));
  return NumericObject::make_numeric(len);
}

static Value _primitive_string_contains(VM& vm, int argc, Value* args) {
  StringObject* orig = args[0]->as_string();
  StringObject* subs = args[1]->as_string();

  if (orig->size() == 0 && subs->size() == 0)
    return NumericObject::make_numeric(1);
  return NumericObject::make_numeric(strstr(orig->cstr(), subs->cstr()) != 0);
}

static Value _primitive_string_add(VM& vm, int argc, Value* args) {
  if (!args[1]->is_string())
    return vm.unsupported();

  StringObject* lhs = args[0]->as_string();
  StringObject* rhs = args[1]->as_string();

  int n = lhs->size() + rhs->size();
  char* s = new char[n + 1];
  memcpy(s, lhs->cstr(), lhs->size());
  memcpy(s + lhs->size(), rhs->cstr(), rhs->size());
  s[n] = 0;

  return StringObject::make_string(s);
}

static Value _primitive_io_write(VM& vm, int argc, Value* args) {
  std::cout << args[1] << std::endl;
  return args[1];
}

void reigister_primitives(VM& vm) {
  vm.set_primitive(vm.num_cls(), "abs", _primitive_numeric_abs);
  vm.set_primitive(vm.num_cls(), "+ ", _primitive_numeric_add);
  vm.set_primitive(vm.num_cls(), "- ", _primitive_numeric_sub);
  vm.set_primitive(vm.num_cls(), "* ", _primitive_numeric_mul);
  vm.set_primitive(vm.num_cls(), "/ ", _primitive_numeric_div);

  vm.set_primitive(vm.str_cls(), "len", _primitive_string_len);
  vm.set_primitive(vm.str_cls(), "contains ", _primitive_string_contains);
  vm.set_primitive(vm.str_cls(), "+ ", _primitive_string_add);

  ClassObject* io_cls = ClassObject::make_class();
  vm.set_primitive(io_cls, "write ", _primitive_io_write);
  vm.set_global(io_cls, "io");

  ClassObject* unsupported_cls = ClassObject::make_class();
  vm.set_unsupported(InstanceObject::make_instance(unsupported_cls));
}

}
