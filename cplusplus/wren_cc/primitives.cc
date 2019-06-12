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
#include <cmath>
#include <iostream>
#include <sstream>
#include "vm.hh"
#include "compiler.hh"
#include "primitives.hh"

namespace wrencc {

#define DEF_PRIMITIVE(fn)\
static Value _primitive_##fn(VM& vm, Fiber& fiber, Value* args)

DEF_PRIMITIVE(fn_call) {
  vm.call_function(fiber, args[0]->as_function(), 1);
  return nullptr;
}

DEF_PRIMITIVE(bool_eq) {
  if (!args[1]->is_boolean())
    return BooleanObject::make_boolean(false);

  return BooleanObject::make_boolean(
      args[0]->as_boolean() == args[1]->as_boolean());
}

DEF_PRIMITIVE(bool_ne) {
  if (!args[1]->is_boolean())
    return BooleanObject::make_boolean(true);

  return BooleanObject::make_boolean(
      args[0]->as_boolean() != args[1]->as_boolean());
}

DEF_PRIMITIVE(bool_tostring) {
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

DEF_PRIMITIVE(numeric_abs) {
  double d = args[0]->as_numeric();
  if (d < 0)
    d = -d;
  else if (d == 0)
    d = 0;
  return NumericObject::make_numeric(d);
}

DEF_PRIMITIVE(numeric_tostring) {
  std::stringstream ss;
  ss << args[0]->as_numeric();
  str_t temp(ss.str());

  char* s = new char [temp.size() + 1];
  memcpy(s, temp.data(), temp.size());
  s[temp.size()] = 0;

  return StringObject::make_string(s);
}

DEF_PRIMITIVE(numeric_add) {
  if (!args[1]->is_numeric())
    return vm.unsupported();

  return NumericObject::make_numeric(
      args[0]->as_numeric() + args[1]->as_numeric());
}

DEF_PRIMITIVE(numeric_sub) {
  if (!args[1]->is_numeric())
    return vm.unsupported();

  return NumericObject::make_numeric(
      args[0]->as_numeric() - args[1]->as_numeric());
}

DEF_PRIMITIVE(numeric_mul) {
  if (!args[1]->is_numeric())
    return vm.unsupported();

  return NumericObject::make_numeric(
      args[0]->as_numeric() * args[1]->as_numeric());
}

DEF_PRIMITIVE(numeric_div) {
  if (!args[1]->is_numeric())
    return vm.unsupported();

  return NumericObject::make_numeric(
      args[0]->as_numeric() / args[1]->as_numeric());
}

DEF_PRIMITIVE(numeric_mod) {
  if (!args[1]->is_numeric())
    return vm.unsupported();

  return NumericObject::make_numeric(
      std::fmod(args[0]->as_numeric(), args[1]->as_numeric()));
}

DEF_PRIMITIVE(numeric_gt) {
  if (!args[1]->is_numeric())
    return vm.unsupported();
  return BooleanObject::make_boolean(
      args[0]->as_numeric() > args[1]->as_numeric());
}

DEF_PRIMITIVE(numeric_ge) {
  if (!args[1]->is_numeric())
    return vm.unsupported();
  return BooleanObject::make_boolean(
      args[0]->as_numeric() >= args[1]->as_numeric());
}

DEF_PRIMITIVE(numeric_lt) {
  if (!args[1]->is_numeric())
    return vm.unsupported();
  return BooleanObject::make_boolean(
      args[0]->as_numeric() < args[1]->as_numeric());
}

DEF_PRIMITIVE(numeric_le) {
  if (!args[1]->is_numeric())
    return vm.unsupported();
  return BooleanObject::make_boolean(
      args[0]->as_numeric() <= args[1]->as_numeric());
}

DEF_PRIMITIVE(numeric_eq) {
  if (!args[1]->is_numeric())
    return BooleanObject::make_boolean(false);

  return BooleanObject::make_boolean(
      args[0]->as_numeric() == args[1]->as_numeric());
}

DEF_PRIMITIVE(numeric_ne) {
  if (!args[1]->is_numeric())
    return BooleanObject::make_boolean(true);

  return BooleanObject::make_boolean(
      args[0]->as_numeric() != args[1]->as_numeric());
}

DEF_PRIMITIVE(string_len) {
  return NumericObject::make_numeric(args[0]->as_string()->size());
}

DEF_PRIMITIVE(string_contains) {
  StringObject* orig = args[0]->as_string();
  StringObject* subs = args[1]->as_string();

  if (orig->size() == 0 && subs->size() == 0)
    return NumericObject::make_numeric(1);
  return NumericObject::make_numeric(strstr(orig->cstr(), subs->cstr()) != 0);
}

DEF_PRIMITIVE(string_tostring) {
  return args[0];
}

DEF_PRIMITIVE(string_add) {
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

DEF_PRIMITIVE(string_eq) {
  if (!args[1]->is_string())
    return BooleanObject::make_boolean(false);

  auto r = strcmp(args[0]->as_cstring(), args[1]->as_cstring()) == 0;
  return BooleanObject::make_boolean(r);
}

DEF_PRIMITIVE(string_ne) {
  if (!args[1]->is_string())
    return BooleanObject::make_boolean(true);

  auto r = strcmp(args[0]->as_cstring(), args[1]->as_cstring()) != 0;
  return BooleanObject::make_boolean(r);
}

DEF_PRIMITIVE(fn_eq) {
  if (!args[1]->is_function())
    return BooleanObject::make_boolean(false);

  return BooleanObject::make_boolean(args[0] == args[1]);
}

DEF_PRIMITIVE(fn_ne) {
  if (!args[1]->is_function())
    return BooleanObject::make_boolean(true);

  return BooleanObject::make_boolean(args[0] != args[1]);
}

DEF_PRIMITIVE(io_write) {
  std::cout << args[1] << std::endl;
  return args[1];
}

static constexpr const char* kCoreLib =
"class Nil {}\n"
"class Bool {}\n"
"class Numeric {}\n"
"class String {}\n"
"class Function {}\n"
"class Class {}\n"
"class IO {}\n"
"var io = IO.new\n";

void load_core(VM& vm) {
  vm.interpret(kCoreLib);

  vm.set_bool_cls(vm.get_global("Bool")->as_class());
  vm.set_primitive(vm.bool_cls(), "toString", _primitive_bool_tostring);
  vm.set_primitive(vm.bool_cls(), "== ", _primitive_bool_eq);
  vm.set_primitive(vm.bool_cls(), "!= ", _primitive_bool_ne);

  vm.set_class_cls(vm.get_global("Class")->as_class());

  vm.set_fn_cls(vm.get_global("Function")->as_class());
  vm.set_primitive(vm.fn_cls(), "call", _primitive_fn_call);
  vm.set_primitive(vm.fn_cls(), "== ", _primitive_fn_eq);
  vm.set_primitive(vm.fn_cls(), "!= ", _primitive_fn_ne);

  vm.set_nil_cls(vm.get_global("Nil")->as_class());

  vm.set_num_cls(vm.get_global("Numeric")->as_class());
  vm.set_primitive(vm.num_cls(), "abs", _primitive_numeric_abs);
  vm.set_primitive(vm.num_cls(), "toString", _primitive_numeric_tostring);
  vm.set_primitive(vm.num_cls(), "+ ", _primitive_numeric_add);
  vm.set_primitive(vm.num_cls(), "- ", _primitive_numeric_sub);
  vm.set_primitive(vm.num_cls(), "* ", _primitive_numeric_mul);
  vm.set_primitive(vm.num_cls(), "/ ", _primitive_numeric_div);
  vm.set_primitive(vm.num_cls(), "% ", _primitive_numeric_mod);
  vm.set_primitive(vm.num_cls(), "> ", _primitive_numeric_gt);
  vm.set_primitive(vm.num_cls(), ">= ", _primitive_numeric_ge);
  vm.set_primitive(vm.num_cls(), "< ", _primitive_numeric_lt);
  vm.set_primitive(vm.num_cls(), "<= ", _primitive_numeric_le);
  vm.set_primitive(vm.num_cls(), "== ", _primitive_numeric_eq);
  vm.set_primitive(vm.num_cls(), "!= ", _primitive_numeric_ne);

  vm.set_str_cls(vm.get_global("String")->as_class());
  vm.set_primitive(vm.str_cls(), "len", _primitive_string_len);
  vm.set_primitive(vm.str_cls(), "contains ", _primitive_string_contains);
  vm.set_primitive(vm.str_cls(), "toString", _primitive_string_tostring);
  vm.set_primitive(vm.str_cls(), "+ ", _primitive_string_add);
  vm.set_primitive(vm.str_cls(), "== ", _primitive_string_eq);
  vm.set_primitive(vm.str_cls(), "!= ", _primitive_string_ne);

  ClassObject* io_cls = vm.get_global("IO")->as_class();
  vm.set_primitive(io_cls, "write ", _primitive_io_write);

  ClassObject* unsupported_cls = ClassObject::make_class();
  vm.set_unsupported(InstanceObject::make_instance(unsupported_cls));
}

}
