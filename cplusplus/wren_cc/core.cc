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
#include <chrono>
#include <cmath>
#include <iostream>
#include "value.hh"
#include "vm.hh"
#include "core.hh"

namespace wrencc {

#define DEF_NATIVE(fn)\
static Value _primitive_##fn(WrenVM& vm, Value* args)
#define DEF_FIBER_NATIVE(fn)\
static void _primitive_##fn(WrenVM& vm, Fiber& fiber, Value* args)
#define DEF_FIBER_NATIVE_FN(fn, argc)\
static void _primitive_##fn(WrenVM& vm, Fiber& fiber, Value* args) {\
  vm.call_function(fiber, args[0].as_function(), argc);\
}

static int validate_index(const Value& index, int count) {
  if (!index.is_numeric())
    return -1;

  double index_num = index.as_numeric();
  int i = Xt::as_type<int>(index_num);
  // make sure the index as an integer
  if (index_num != i)
    return -1;

  // negative indices count from the end
  if (i < 0)
    i += count;
  if (i < 0 || i >= count)
    return -1;

  return i;
}

DEF_FIBER_NATIVE_FN(fn_call0, 1)
DEF_FIBER_NATIVE_FN(fn_call1, 2)
DEF_FIBER_NATIVE_FN(fn_call2, 3)
DEF_FIBER_NATIVE_FN(fn_call3, 4)
DEF_FIBER_NATIVE_FN(fn_call4, 5)
DEF_FIBER_NATIVE_FN(fn_call5, 6)
DEF_FIBER_NATIVE_FN(fn_call6, 7)
DEF_FIBER_NATIVE_FN(fn_call7, 8)
DEF_FIBER_NATIVE_FN(fn_call8, 9)

DEF_NATIVE(bool_tostring) {
  if (args[0].as_boolean())
    return StringObject::make_string(vm, "true");
  else
    return StringObject::make_string(vm, "false");
}

DEF_NATIVE(bool_not) {
  return !args[0].as_boolean();
}

DEF_NATIVE(numeric_abs) {
  return std::abs(args[0].as_numeric());
}

DEF_NATIVE(numeric_tostring) {
  return StringObject::make_string(vm, args[0].stringify());
}

DEF_NATIVE(numeric_neg) {
  return -args[0].as_numeric();
}

DEF_NATIVE(numeric_add) {
  if (!args[1].is_numeric())
    return vm.unsupported();

  return args[0].as_numeric() + args[1].as_numeric();
}

DEF_NATIVE(numeric_sub) {
  if (!args[1].is_numeric())
    return vm.unsupported();

  return args[0].as_numeric() - args[1].as_numeric();
}

DEF_NATIVE(numeric_mul) {
  if (!args[1].is_numeric())
    return vm.unsupported();

  return args[0].as_numeric() * args[1].as_numeric();
}

DEF_NATIVE(numeric_div) {
  if (!args[1].is_numeric())
    return vm.unsupported();

  return args[0].as_numeric() / args[1].as_numeric();
}

DEF_NATIVE(numeric_mod) {
  if (!args[1].is_numeric())
    return vm.unsupported();

  return std::fmod(args[0].as_numeric(), args[1].as_numeric());
}

DEF_NATIVE(numeric_gt) {
  if (!args[1].is_numeric())
    return vm.unsupported();
  return args[0].as_numeric() > args[1].as_numeric();
}

DEF_NATIVE(numeric_ge) {
  if (!args[1].is_numeric())
    return vm.unsupported();
  return args[0].as_numeric() >= args[1].as_numeric();
}

DEF_NATIVE(numeric_lt) {
  if (!args[1].is_numeric())
    return vm.unsupported();
  return args[0].as_numeric() < args[1].as_numeric();
}

DEF_NATIVE(numeric_le) {
  if (!args[1].is_numeric())
    return vm.unsupported();
  return args[0].as_numeric() <= args[1].as_numeric();
}

DEF_NATIVE(numeric_eq) {
  if (!args[1].is_numeric())
    return false;

  return args[0].as_numeric() == args[1].as_numeric();
}

DEF_NATIVE(numeric_ne) {
  if (!args[1].is_numeric())
    return true;

  return args[0].as_numeric() != args[1].as_numeric();
}

DEF_NATIVE(object_eq) {
  return args[0] == args[1];
}

DEF_NATIVE(object_ne) {
  return args[0] != args[1];
}

DEF_NATIVE(object_type) {
  return vm.get_class(args[0]);
}

DEF_NATIVE(string_len) {
  return args[0].as_string()->size();
}

DEF_NATIVE(string_contains) {
  StringObject* orig = args[0].as_string();
  StringObject* subs = args[1].as_string();

  if (orig->size() == 0 && subs->size() == 0)
    return true;
  return strstr(orig->cstr(), subs->cstr()) != nullptr;
}

DEF_NATIVE(string_tostring) {
  return args[0];
}

DEF_NATIVE(string_add) {
  if (!args[1].is_string())
    return vm.unsupported();

  StringObject* lhs = args[0].as_string();
  StringObject* rhs = args[1].as_string();
  return StringObject::make_string(vm, lhs, rhs);
}

DEF_NATIVE(string_eq) {
  if (!args[1].is_string())
    return false;

  return strcmp(args[0].as_cstring(), args[1].as_cstring()) == 0;
}

DEF_NATIVE(string_ne) {
  if (!args[1].is_string())
    return true;

  return strcmp(args[0].as_cstring(), args[1].as_cstring()) != 0;
}

DEF_NATIVE(string_subscript) {
  if (!args[1].is_numeric())
    return nullptr;

  double index_num = args[1].as_numeric();
  int index = Xt::as_type<int>(index_num);
  // make sure the index is an integer
  if (index_num != index)
    return nullptr;

  StringObject* s = args[0].as_string();
  if (index < 0)
    index = index + s->size();
  if (index < 0 || index >= s->size())
    return nullptr;

  return StringObject::make_string(vm, (*s)[index]);
}

DEF_NATIVE(list_add) {
  ListObject* list = args[0].as_list();
  list->add_element(args[1]);
  return args[1];
}

DEF_NATIVE(list_clear) {
  ListObject* list = args[0].as_list();
  list->clear();
  return nullptr;
}

DEF_NATIVE(list_len) {
  return args[0].as_list()->count();
}

DEF_NATIVE(list_insert) {
  ListObject* list = args[0].as_list();
  int index = validate_index(args[1], list->count() + 1);
  if (index == -1)
    return nullptr;

  list->insert(index, args[2]);
  return args[2];
}

DEF_NATIVE(list_remove) {
  ListObject* list = args[0].as_list();
  int index = validate_index(args[1], list->count());
  if (index == -1)
    return nullptr;

  Value removed = list->get_element(index);
  list->remove(index);

  return removed;
}

DEF_NATIVE(list_subscript) {
  ListObject* list = args[0].as_list();
  int index = validate_index(args[1], list->count());
  if (index == -1)
    return nullptr;

  return list->get_element(index);
}

DEF_NATIVE(io_write) {
  std::cout << args[1] << std::endl;
  return args[1];
}

DEF_NATIVE(os_clock) {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now().time_since_epoch()).count() / 1000.0;
}

static ClassObject* define_class(
    WrenVM& vm, const str_t& name, ClassObject* superclass) {
  ClassObject* cls = ClassObject::make_class(vm, superclass, 0);
  vm.set_global(name, cls);
  return cls;
}

void initialize_core(WrenVM& vm) {
  vm.set_obj_cls(define_class(vm, "Object", nullptr));
  vm.set_native(vm.obj_cls(), "== ", _primitive_object_eq);
  vm.set_native(vm.obj_cls(), "!= ", _primitive_object_ne);
  vm.set_native(vm.obj_cls(), "type", _primitive_object_type);

  // the "Class" class is the superclass of all metaclasses
  vm.set_class_cls(define_class(vm, "Class", vm.obj_cls()));

  vm.set_bool_cls(define_class(vm, "Bool", vm.obj_cls()));
  vm.set_native(vm.bool_cls(), "toString", _primitive_bool_tostring);
  vm.set_native(vm.bool_cls(), "!", _primitive_bool_not);

  vm.set_fn_cls(define_class(vm, "Function", vm.obj_cls()));
  vm.set_native(vm.fn_cls(), "call", _primitive_fn_call0);
  vm.set_native(vm.fn_cls(), "call ", _primitive_fn_call1);
  vm.set_native(vm.fn_cls(), "call  ", _primitive_fn_call2);
  vm.set_native(vm.fn_cls(), "call   ", _primitive_fn_call3);
  vm.set_native(vm.fn_cls(), "call    ", _primitive_fn_call4);
  vm.set_native(vm.fn_cls(), "call     ", _primitive_fn_call5);
  vm.set_native(vm.fn_cls(), "call      ", _primitive_fn_call6);
  vm.set_native(vm.fn_cls(), "call       ", _primitive_fn_call7);
  vm.set_native(vm.fn_cls(), "call        ", _primitive_fn_call8);

  vm.set_list_cls(define_class(vm, "List", vm.obj_cls()));
  vm.set_native(vm.list_cls(), "add ", _primitive_list_add);
  vm.set_native(vm.list_cls(), "clear", _primitive_list_clear);
  vm.set_native(vm.list_cls(), "len", _primitive_list_len);
  vm.set_native(vm.list_cls(), "insert  ", _primitive_list_insert);
  vm.set_native(vm.list_cls(), "remove ", _primitive_list_remove);
  vm.set_native(vm.list_cls(), "[ ]", _primitive_list_subscript);

  vm.set_nil_cls(define_class(vm, "Nil", vm.obj_cls()));

  vm.set_num_cls(define_class(vm, "Numeric", vm.obj_cls()));
  vm.set_native(vm.num_cls(), "abs", _primitive_numeric_abs);
  vm.set_native(vm.num_cls(), "toString", _primitive_numeric_tostring);
  vm.set_native(vm.num_cls(), "-", _primitive_numeric_neg);
  vm.set_native(vm.num_cls(), "+ ", _primitive_numeric_add);
  vm.set_native(vm.num_cls(), "- ", _primitive_numeric_sub);
  vm.set_native(vm.num_cls(), "* ", _primitive_numeric_mul);
  vm.set_native(vm.num_cls(), "/ ", _primitive_numeric_div);
  vm.set_native(vm.num_cls(), "% ", _primitive_numeric_mod);
  vm.set_native(vm.num_cls(), "> ", _primitive_numeric_gt);
  vm.set_native(vm.num_cls(), ">= ", _primitive_numeric_ge);
  vm.set_native(vm.num_cls(), "< ", _primitive_numeric_lt);
  vm.set_native(vm.num_cls(), "<= ", _primitive_numeric_le);
  vm.set_native(vm.num_cls(), "== ", _primitive_numeric_eq);
  vm.set_native(vm.num_cls(), "!= ", _primitive_numeric_ne);

  // vm.set_obj_cls(vm.get_global("Object").as_class());

  vm.set_str_cls(define_class(vm, "String", vm.obj_cls()));
  vm.set_native(vm.str_cls(), "len", _primitive_string_len);
  vm.set_native(vm.str_cls(), "contains ", _primitive_string_contains);
  vm.set_native(vm.str_cls(), "toString", _primitive_string_tostring);
  vm.set_native(vm.str_cls(), "+ ", _primitive_string_add);
  vm.set_native(vm.str_cls(), "== ", _primitive_string_eq);
  vm.set_native(vm.str_cls(), "!= ", _primitive_string_ne);
  vm.set_native(vm.str_cls(), "[ ]", _primitive_string_subscript);

  ClassObject* io_cls = define_class(vm, "IO", vm.obj_cls());
  vm.set_native(io_cls, "write ", _primitive_io_write);

  // making this an instance is lame, the only reason we are doing it
  // is because "IO.write()" looks ugly, maybe just get used to that ?
  vm.set_global("io", InstanceObject::make_instance(vm, io_cls));

  ClassObject* os_cls = define_class(vm, "OS", vm.obj_cls());
  vm.set_native(os_cls->meta_class(), "clock", _primitive_os_clock);

  ClassObject* unsupported_cls = ClassObject::make_class(vm, vm.obj_cls(), 0);
  vm.set_unsupported(InstanceObject::make_instance(vm, unsupported_cls));
}

}
