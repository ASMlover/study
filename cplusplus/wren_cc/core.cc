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
static PrimitiveResult _primitive_##fn(WrenVM& vm, FiberObject* fiber, Value* args)
#define RETURN_VAL(val) do {\
  args[0] = val;\
  return PrimitiveResult::VALUE;\
} while (false)
#define RETURN_ERR(msg) do {\
  args[0] = StringObject::make_string(vm, msg);\
  return PrimitiveResult::ERROR;\
} while (false)

static str_t kCoreLib =
"class IO {\n"
"  static print(obj) {\n"
"    IO.writeString(obj.toString)\n"
"    IO.writeString(\"\n\")\n"
"    return obj\n"
"  }\n"
"\n"
"  static write(obj) {\n"
"    IO.writeString(obj.toString)\n"
"    return obj\n"
"  }\n"
"}\n"
"\n"
"class List {\n"
"  toString {\n"
"    var result = \"[\"\n"
"    var i = 0\n"
"    // TODO: use for loop\n"
"    while (i < this.len) {\n"
"      if (i > 0) result = result + \", \"\n"
"      result = result + this[i].toString\n"
"      i = i + 1\n"
"    }\n"
"    result = result + \"]\"\n"
"    return result\n"
"  }\n"
"}\n"
"\n"
"class Range {\n"
"  new(min, max) {\n"
"    _min = min\n"
"    _max = max\n"
"  }\n"
"\n"
"  min { return _min }\n"
"  max { return _max }\n"
"\n"
"  iterate(prev) {\n"
"    if (prev == nil) return _min\n"
"    if (prev == _max) return false\n"
"    return prev + 1\n"
"  }\n"
"\n"
"  iterValue(i) {\n"
"    return i\n"
"  }\n"
"}\n"
"\n"
"class Numeric {\n"
"  .. other { return new Range(this, other) }\n"
"  ... other { return new Range(this, other - 1) }\n"
"}\n";

static int validate_index(const Value& index, int count) {
  // validates that [index] is an integer within `[0, count)` also allows
  // negative indices which map backwards from the end, returns the valid
  // positive index value, or -1 if the index was not valid (not a number,
  // not an int, out of bounds)

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

static bool validate_numeric(WrenVM& vm,
    Value* args, int index, const str_t& arg_name) {
  // validates that the given argument in [args] is a Numeric, returns true
  // if it is, if not reports an error and returns false

  if (args[index].is_numeric())
    return true;

  std::stringstream ss;
  ss << "`" << arg_name << "` must be a numeric";
  args[0] = StringObject::make_string(vm, ss.str());
  return false;
}

static bool validate_int(WrenVM& vm,
    Value* args, int index, const str_t& arg_name) {
  // validates that the given argument in [args] is an integer, returns true
  // if it is, if not reports error and returns false

  // make sure it's a Numeric first
  if (!validate_numeric(vm, args, index, arg_name))
    return false;

  double value = args[index].as_numeric();
  if (std::trunc(value) == value)
    return true;

  std::stringstream ss;
  ss << "`" << arg_name << "` must be an integer";
  args[0] = StringObject::make_string(vm, ss.str());
  return false;
}

static int validate_index(WrenVM& vm,
    Value* args, int arg_index, int count, const str_t& arg_name) {
  // validates that [arg_index] is an integer within `[0, count)`, also allows
  // negative indices which map backwards from the end, returns the valid
  // positive index value, if invalid reports an error and return -1

  if (!validate_int(vm, args, arg_index, arg_name))
    return -1;

  int index = Xt::as_type<int>(args[arg_index].as_numeric());
  // negative indices count from the end
  if (index < 0)
    index += count;
  // check bounds
  if (index >= 0 && index < count)
    return index;

  std::stringstream ss;
  ss << "`" << arg_name << "` out of bounds";
  args[0] = StringObject::make_string(vm, ss.str());
  return -1;
}

static bool validate_string(WrenVM& vm,
    Value* args, int index, const str_t& arg_name) {
  // validates that the given argument in [args] is a String, returns true
  // if it is, if not reports an error and returns false

  if (args[index].is_string())
    return true;

  std::stringstream ss;
  ss << "`" << arg_name << "` must be a string";
  args[0] = StringObject::make_string(vm, ss.str());
  return false;
}

DEF_NATIVE(fn_call) {
  return PrimitiveResult::CALL;
}

DEF_NATIVE(nil_tostring) {
  RETURN_VAL(StringObject::make_string(vm, "nil"));
}

DEF_NATIVE(bool_tostring) {
  if (args[0].as_boolean())
    RETURN_VAL(StringObject::make_string(vm, "true"));
  else
    RETURN_VAL(StringObject::make_string(vm, "false"));
}

DEF_NATIVE(bool_not) {
  RETURN_VAL(!args[0].as_boolean());
}

DEF_NATIVE(numeric_abs) {
  RETURN_VAL(std::abs(args[0].as_numeric()));
}

DEF_NATIVE(numeric_tostring) {
  RETURN_VAL(StringObject::make_string(vm, args[0].stringify()));
}

DEF_NATIVE(numeric_neg) {
  RETURN_VAL(-args[0].as_numeric());
}

DEF_NATIVE(numeric_add) {
  if (!validate_numeric(vm, args, 1, "Right operand"))
    return PrimitiveResult::ERROR;

  RETURN_VAL(args[0].as_numeric() + args[1].as_numeric());
}

DEF_NATIVE(numeric_sub) {
  if (!validate_numeric(vm, args, 1, "Right operand"))
    return PrimitiveResult::ERROR;

  RETURN_VAL(args[0].as_numeric() - args[1].as_numeric());
}

DEF_NATIVE(numeric_mul) {
  if (!validate_numeric(vm, args, 1, "Right operand"))
    return PrimitiveResult::ERROR;

  RETURN_VAL(args[0].as_numeric() * args[1].as_numeric());
}

DEF_NATIVE(numeric_div) {
  if (!validate_numeric(vm, args, 1, "Right operand"))
    return PrimitiveResult::ERROR;

  RETURN_VAL(args[0].as_numeric() / args[1].as_numeric());
}

DEF_NATIVE(numeric_mod) {
  if (!validate_numeric(vm, args, 1, "Right operand"))
    return PrimitiveResult::ERROR;

  RETURN_VAL(std::fmod(args[0].as_numeric(), args[1].as_numeric()));
}

DEF_NATIVE(numeric_gt) {
  if (!args[1].is_numeric())
    RETURN_VAL(nullptr);
  RETURN_VAL(args[0].as_numeric() > args[1].as_numeric());
}

DEF_NATIVE(numeric_ge) {
  if (!args[1].is_numeric())
    RETURN_VAL(nullptr);
  RETURN_VAL(args[0].as_numeric() >= args[1].as_numeric());
}

DEF_NATIVE(numeric_lt) {
  if (!args[1].is_numeric())
    RETURN_VAL(nullptr);
  RETURN_VAL(args[0].as_numeric() < args[1].as_numeric());
}

DEF_NATIVE(numeric_le) {
  if (!args[1].is_numeric())
    RETURN_VAL(nullptr);
  RETURN_VAL(args[0].as_numeric() <= args[1].as_numeric());
}

DEF_NATIVE(numeric_eq) {
  if (!args[1].is_numeric())
    RETURN_VAL(false);

  RETURN_VAL(args[0].as_numeric() == args[1].as_numeric());
}

DEF_NATIVE(numeric_ne) {
  if (!args[1].is_numeric())
    RETURN_VAL(true);

  RETURN_VAL(args[0].as_numeric() != args[1].as_numeric());
}

DEF_NATIVE(numeric_bitnot) {
  // bitwise operators always work on 32-bit unsigned int
  u32_t val = Xt::as_type<u32_t>(args[0].as_numeric());
  RETURN_VAL(~val);
}

DEF_NATIVE(object_eq) {
  RETURN_VAL(args[0] == args[1]);
}

DEF_NATIVE(object_ne) {
  RETURN_VAL(args[0] != args[1]);
}

DEF_NATIVE(object_new) {
  // this is the default argument-less constructor that all objects
  // inherit. it just returns `this`
  RETURN_VAL(args[0]);
}

DEF_NATIVE(object_tostring) {
  RETURN_VAL(StringObject::make_string(vm, "<object>"));
}

DEF_NATIVE(object_type) {
  RETURN_VAL(vm.get_class(args[0]));
}

DEF_NATIVE(string_len) {
  RETURN_VAL(args[0].as_string()->size());
}

DEF_NATIVE(string_contains) {
  if (!validate_string(vm, args, 1, "Argument"))
    return PrimitiveResult::ERROR;

  StringObject* orig = args[0].as_string();
  StringObject* subs = args[1].as_string();

  if (orig->size() == 0 && subs->size() == 0)
    RETURN_VAL(true);
  RETURN_VAL(strstr(orig->cstr(), subs->cstr()) != nullptr);
}

DEF_NATIVE(string_tostring) {
  RETURN_VAL(args[0]);
}

DEF_NATIVE(string_add) {
  if (!args[1].is_string())
    RETURN_VAL(nullptr);

  StringObject* lhs = args[0].as_string();
  StringObject* rhs = args[1].as_string();
  RETURN_VAL(StringObject::make_string(vm, lhs, rhs));
}

DEF_NATIVE(string_eq) {
  if (!args[1].is_string())
    RETURN_VAL(false);

  RETURN_VAL(strcmp(args[0].as_cstring(), args[1].as_cstring()) == 0);
}

DEF_NATIVE(string_ne) {
  if (!args[1].is_string())
    RETURN_VAL(true);

  RETURN_VAL(strcmp(args[0].as_cstring(), args[1].as_cstring()) != 0);
}

DEF_NATIVE(string_subscript) {
  StringObject* s = args[0].as_string();
  int index = validate_index(vm, args, 1, s->size(), "Subscript");
  if (index == -1)
    return PrimitiveResult::ERROR;

  RETURN_VAL(StringObject::make_string(vm, (*s)[index]));
}

DEF_NATIVE(list_add) {
  ListObject* list = args[0].as_list();
  list->add_element(args[1]);
  RETURN_VAL(args[1]);
}

DEF_NATIVE(list_clear) {
  ListObject* list = args[0].as_list();
  list->clear();
  RETURN_VAL(nullptr);
}

DEF_NATIVE(list_len) {
  RETURN_VAL(args[0].as_list()->count());
}

DEF_NATIVE(list_insert) {
  ListObject* list = args[0].as_list();
  int index = validate_index(vm, args, 1, list->count() + 1, "Index");
  if (index == -1)
    return PrimitiveResult::ERROR;

  list->insert(index, args[2]);
  RETURN_VAL(args[2]);
}

DEF_NATIVE(list_remove) {
  ListObject* list = args[0].as_list();
  int index = validate_index(vm, args, 1, list->count(), "Index");
  if (index == -1)
    return PrimitiveResult::ERROR;

  RETURN_VAL(list->remove(index));
}

DEF_NATIVE(list_iterate) {
  // if we are starting the interation, return the first index
  if (args[1].is_nil())
    RETURN_VAL(0);

  if (!validate_int(vm, args, 1, "Iterator"))
    return PrimitiveResult::ERROR;

  ListObject* list = args[0].as_list();
  int index = Xt::as_type<int>(args[1].as_numeric());

  // stop if we are out out bounds
  if (index < 0 || index >= list->count() - 1)
    RETURN_VAL(false);

  RETURN_VAL(index + 1);
}

DEF_NATIVE(list_itervalue) {
  ListObject* list = args[0].as_list();
  int index = validate_index(vm, args, 1, list->count(), "Iterator");
  if (index == -1)
    return PrimitiveResult::ERROR;

  RETURN_VAL(list->get_element(index));
}

DEF_NATIVE(list_subscript) {
  ListObject* list = args[0].as_list();
  int index = validate_index(vm, args, 1, list->count(), "Index");
  if (index == -1)
    return PrimitiveResult::ERROR;

  RETURN_VAL(list->get_element(index));
}

DEF_NATIVE(list_subscript_setter) {
  ListObject* list = args[0].as_list();
  int index = validate_index(vm, args, 1, list->count(), "Index");
  if (index == -1)
    return PrimitiveResult::ERROR;

  list->set_element(index, args[2]);
  RETURN_VAL(args[2]);
}

DEF_NATIVE(io_write) {
  if (!validate_string(vm, args, 1, "Argument"))
    return PrimitiveResult::ERROR;

  std::cout << args[1];
  RETURN_VAL(nullptr);
}

DEF_NATIVE(os_clock) {
  RETURN_VAL(std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now().time_since_epoch()).count() / 1000.0);
}

static ClassObject* define_class(WrenVM& vm, const str_t& name) {
  ClassObject* cls = ClassObject::make_class(vm, vm.obj_cls(), 0);
  vm.set_global(name, cls);
  return cls;
}

void initialize_core(WrenVM& vm) {
  // define the root object class, this has to be done a little specially
  // because it has no superclass and an unusual metaclass (Class)
  ClassObject* obj_cls = ClassObject::make_single_class(vm);
  vm.set_global("Object", obj_cls);
  vm.set_obj_cls(obj_cls);

  vm.set_native(vm.obj_cls(), "== ", _primitive_object_eq);
  vm.set_native(vm.obj_cls(), "!= ", _primitive_object_ne);
  vm.set_native(vm.obj_cls(), "new", _primitive_object_new);
  vm.set_native(vm.obj_cls(), "toString", _primitive_object_tostring);
  vm.set_native(vm.obj_cls(), "type", _primitive_object_type);

  // now we can define Class, which is a subclass of Object, but Object's
  // metclass
  ClassObject* class_cls = ClassObject::make_single_class(vm);
  vm.set_global("Class", class_cls);
  vm.set_class_cls(class_cls);

  // now that Object and Class are defined, we can wrie them up to each other
  vm.class_cls()->bind_superclass(vm.obj_cls());
  vm.obj_cls()->set_meta_class(vm.class_cls());
  vm.class_cls()->set_meta_class(vm.class_cls());

  vm.set_bool_cls(define_class(vm, "Bool"));
  vm.set_native(vm.bool_cls(), "toString", _primitive_bool_tostring);
  vm.set_native(vm.bool_cls(), "!", _primitive_bool_not);

  vm.set_fiber_cls(define_class(vm, "Fiber"));

  vm.set_fn_cls(define_class(vm, "Function"));
  vm.set_native(vm.fn_cls(), "call", _primitive_fn_call);
  vm.set_native(vm.fn_cls(), "call ", _primitive_fn_call);
  vm.set_native(vm.fn_cls(), "call  ", _primitive_fn_call);
  vm.set_native(vm.fn_cls(), "call   ", _primitive_fn_call);
  vm.set_native(vm.fn_cls(), "call    ", _primitive_fn_call);
  vm.set_native(vm.fn_cls(), "call     ", _primitive_fn_call);
  vm.set_native(vm.fn_cls(), "call      ", _primitive_fn_call);
  vm.set_native(vm.fn_cls(), "call       ", _primitive_fn_call);
  vm.set_native(vm.fn_cls(), "call        ", _primitive_fn_call);
  vm.set_native(vm.fn_cls(), "call         ", _primitive_fn_call);
  vm.set_native(vm.fn_cls(), "call          ", _primitive_fn_call);
  vm.set_native(vm.fn_cls(), "call           ", _primitive_fn_call);
  vm.set_native(vm.fn_cls(), "call            ", _primitive_fn_call);
  vm.set_native(vm.fn_cls(), "call             ", _primitive_fn_call);
  vm.set_native(vm.fn_cls(), "call              ", _primitive_fn_call);
  vm.set_native(vm.fn_cls(), "call               ", _primitive_fn_call);
  vm.set_native(vm.fn_cls(), "call                ", _primitive_fn_call);

  vm.set_nil_cls(define_class(vm, "Nil"));
  vm.set_native(vm.nil_cls(), "toString", _primitive_nil_tostring);

  // vm.set_obj_cls(vm.get_global("Object").as_class());

  vm.set_str_cls(define_class(vm, "String"));
  vm.set_native(vm.str_cls(), "len", _primitive_string_len);
  vm.set_native(vm.str_cls(), "contains ", _primitive_string_contains);
  vm.set_native(vm.str_cls(), "toString", _primitive_string_tostring);
  vm.set_native(vm.str_cls(), "+ ", _primitive_string_add);
  vm.set_native(vm.str_cls(), "== ", _primitive_string_eq);
  vm.set_native(vm.str_cls(), "!= ", _primitive_string_ne);
  vm.set_native(vm.str_cls(), "[ ]", _primitive_string_subscript);

  /// // making this an instance is lame, the only reason we are doing it
  /// // is because "IO.write()" looks ugly, maybe just get used to that ?

  ClassObject* os_cls = define_class(vm, "OS");
  vm.set_native(os_cls->meta_class(), "clock", _primitive_os_clock);

  /// from core library source
  vm.interpret("Wren core library", kCoreLib);

  vm.set_list_cls(vm.get_global("List").as_class());
  vm.set_native(vm.list_cls(), "add ", _primitive_list_add);
  vm.set_native(vm.list_cls(), "clear", _primitive_list_clear);
  vm.set_native(vm.list_cls(), "len", _primitive_list_len);
  vm.set_native(vm.list_cls(), "insert  ", _primitive_list_insert);
  vm.set_native(vm.list_cls(), "remove ", _primitive_list_remove);
  vm.set_native(vm.list_cls(), "iterate ", _primitive_list_iterate);
  vm.set_native(vm.list_cls(), "iterValue ", _primitive_list_itervalue);
  vm.set_native(vm.list_cls(), "[ ]", _primitive_list_subscript);
  vm.set_native(vm.list_cls(), "[ ]=", _primitive_list_subscript_setter);

  vm.set_num_cls(vm.get_global("Numeric").as_class());
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
  vm.set_native(vm.num_cls(), "~", _primitive_numeric_bitnot);

  ClassObject* io_cls = vm.get_global("IO").as_class();
  vm.set_native(io_cls->meta_class(), "writeString ", _primitive_io_write);

}

}
