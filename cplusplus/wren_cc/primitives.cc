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
#include <sstream>
#include "vm.hh"
#include "primitives.hh"

namespace wrencc {

static Value _primitive_bool_tostring(VM& vm, int argc, Value* args) {
  if (args[0]->as_boolean()) {
    char* s = new char[5];
    memcpy(s, "true", 4);
    s[4] = 0;
    return StringObject::make_string(s);
  }
  else {
    char* s = new char[6];
    memcpy(s, "false", 5);
    s[5] = 0;
    return StringObject::make_string(s);
  }
}

static Value _primitive_numeric_abs(VM& vm, int argc, Value* args) {
  double d = args[0]->as_numeric();
  if (d < 0)
    d = -d;
  else if (d == 0)
    d = 0;
  return NumericObject::make_numeric(d);
}

static Value _primitive_numeric_tostring(VM& vm, int argc, Value* args) {
  std::stringstream ss;
  ss << args[0]->as_numeric();
  str_t temp(ss.str());

  char* s = new char [temp.size() + 1];
  memcpy(s, temp.data(), temp.size());
  s[temp.size()] = 0;

  return StringObject::make_string(s);
}

static Value _primitive_numeric_add(VM& vm, int argc, Value* args) {
  if (!args[1]->is_numeric())
    return vm.unsupported();

  return NumericObject::make_numeric(
      args[0]->as_numeric() + args[1]->as_numeric());
}

static Value _primitive_numeric_sub(VM& vm, int argc, Value* args) {
  if (!args[1]->is_numeric())
    return vm.unsupported();

  return NumericObject::make_numeric(
      args[0]->as_numeric() - args[1]->as_numeric());
}

static Value _primitive_numeric_mul(VM& vm, int argc, Value* args) {
  if (!args[1]->is_numeric())
    return vm.unsupported();

  return NumericObject::make_numeric(
      args[0]->as_numeric() * args[1]->as_numeric());
}

static Value _primitive_numeric_div(VM& vm, int argc, Value* args) {
  if (!args[1]->is_numeric())
    return vm.unsupported();

  return NumericObject::make_numeric(
      args[0]->as_numeric() / args[1]->as_numeric());
}

static Value _primitive_numeric_gt(VM& vm, int argc, Value* args) {
  if (!args[1]->is_numeric())
    return vm.unsupported();
  return BooleanObject::make_boolean(
      args[0]->as_numeric() > args[1]->as_numeric());
}

static Value _primitive_numeric_ge(VM& vm, int argc, Value* args) {
  if (!args[1]->is_numeric())
    return vm.unsupported();
  return BooleanObject::make_boolean(
      args[0]->as_numeric() >= args[1]->as_numeric());
}

static Value _primitive_numeric_lt(VM& vm, int argc, Value* args) {
  if (!args[1]->is_numeric())
    return vm.unsupported();
  return BooleanObject::make_boolean(
      args[0]->as_numeric() < args[1]->as_numeric());
}

static Value _primitive_numeric_le(VM& vm, int argc, Value* args) {
  if (!args[1]->is_numeric())
    return vm.unsupported();
  return BooleanObject::make_boolean(
      args[0]->as_numeric() <= args[1]->as_numeric());
}

static Value _primitive_string_len(VM& vm, int argc, Value* args) {
  return NumericObject::make_numeric(args[0]->as_string()->size());
}

static Value _primitive_string_contains(VM& vm, int argc, Value* args) {
  StringObject* orig = args[0]->as_string();
  StringObject* subs = args[1]->as_string();

  if (orig->size() == 0 && subs->size() == 0)
    return NumericObject::make_numeric(1);
  return NumericObject::make_numeric(strstr(orig->cstr(), subs->cstr()) != 0);
}

static Value _primitive_string_tostring(VM& vm, int argc, Value* args) {
  return args[0];
}

static Value _primitive_string_add(VM& vm, int argc, Value* args) {
  if (!args[1]->is_string())
    return vm.unsupported();

  StringObject* lhs = args[0]->as_string();
  StringObject* rhs = args[1]->as_string();

  int n = lhs->size() + rhs->size();
  char* s = new char[Xt::as_type<sz_t>(n) + 1];
  memcpy(s, lhs->cstr(), lhs->size());
  memcpy(s + lhs->size(), rhs->cstr(), rhs->size());
  s[n] = 0;

  return StringObject::make_string(s);
}

static Value _primitive_io_write(VM& vm, int argc, Value* args) {
  std::cout << args[1] << std::endl;
  return args[1];
}

void register_primitives(VM& vm) {
  vm.set_bool_cls(ClassObject::make_class());
  vm.set_primitive(vm.bool_cls(), "toString", _primitive_bool_tostring);

  vm.set_block_cls(ClassObject::make_class());
  vm.set_class_cls(ClassObject::make_class());

  vm.set_num_cls(ClassObject::make_class());
  vm.set_primitive(vm.num_cls(), "abs", _primitive_numeric_abs);
  vm.set_primitive(vm.num_cls(), "toString", _primitive_numeric_tostring);
  vm.set_primitive(vm.num_cls(), "+ ", _primitive_numeric_add);
  vm.set_primitive(vm.num_cls(), "- ", _primitive_numeric_sub);
  vm.set_primitive(vm.num_cls(), "* ", _primitive_numeric_mul);
  vm.set_primitive(vm.num_cls(), "/ ", _primitive_numeric_div);
  vm.set_primitive(vm.num_cls(), "> ", _primitive_numeric_gt);
  vm.set_primitive(vm.num_cls(), ">= ", _primitive_numeric_ge);
  vm.set_primitive(vm.num_cls(), "< ", _primitive_numeric_lt);
  vm.set_primitive(vm.num_cls(), "<= ", _primitive_numeric_le);

  vm.set_str_cls(ClassObject::make_class());
  vm.set_primitive(vm.str_cls(), "len", _primitive_string_len);
  vm.set_primitive(vm.str_cls(), "contains ", _primitive_string_contains);
  vm.set_primitive(vm.str_cls(), "toString", _primitive_string_tostring);
  vm.set_primitive(vm.str_cls(), "+ ", _primitive_string_add);

  ClassObject* io_cls = ClassObject::make_class();
  vm.set_primitive(io_cls, "write ", _primitive_io_write);
  vm.set_global(io_cls, "io");

  ClassObject* unsupported_cls = ClassObject::make_class();
  vm.set_unsupported(InstanceObject::make_instance(unsupported_cls));
}

}
