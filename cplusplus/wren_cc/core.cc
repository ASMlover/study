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
#define DEF_NATIVE_FN(fn, i)\
DEF_NATIVE(fn##i) {\
  return call_function(vm, args, i);\
}

static str_t kLibSource =
"class List {\n"
"  toString {\n"
"    var result = \"[\"\n"
"    for (i in 0...this.len) {\n"
"      if (i > 0) result = result + \", \"\n"
"      result = result + this[i].toString\n"
"    }\n"
"    result = result + \"]\"\n"
"    return result\n"
"  }\n"
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

static bool validate_int_value(WrenVM& vm,
    Value* args, double value, const str_t& arg_name) {
  // validates that [value] is an integer, returns true if it is, if not
  // reports an error and returns false

  if (std::trunc(value) == value)
    return true;

  std::stringstream ss;
  ss << "`" << arg_name << "` must be an integer";
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

  return validate_int_value(vm, args, args[index].as_numeric(), arg_name);
}

static int validate_index_value(WrenVM& vm,
    Value* args, int count, double value, const str_t& arg_name) {
  // validates that [value] is an integer within [0, count), also allows
  // negative indices which map backwards from the end, returns the valid
  // positive index value, if invalid, reports an error and return -1

  if (!validate_int_value(vm, args, value, arg_name))
    return -1;

  int index = Xt::as_type<int>(value);
  // negative indices count from the end
  if (index < 0)
    index = count + index;
  // check bounds
  if (index >= 0 && index < count)
    return index;

  std::stringstream ss;
  ss << "`" << arg_name << "` out of bounds";
  args[0] = StringObject::make_string(vm, ss.str());
  return -1;
}

static int validate_index(WrenVM& vm,
    Value* args, int arg_index, int count, const str_t& arg_name) {
  // validates that argument at [arg_index] is an integer within `[0, count)`,
  // also allows negative indices which map backwards from the end, returns
  // the valid positive index value, if invalid reports an error and return -1

  if (!validate_numeric(vm, args, arg_index, arg_name))
    return -1;

  return validate_index_value(vm,
      args, count, args[arg_index].as_numeric(), arg_name);
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

DEF_NATIVE(class_name) {
  ClassObject* cls = args[0].as_class();
  RETURN_VAL(cls->name());
}

DEF_NATIVE(fiber_create) {
  if (!args[1].is_function() && !args[1].is_closure())
    RETURN_ERR("Argument must be a function or closure");

  FiberObject* new_fiber = FiberObject::make_fiber(vm, args[1].as_object());

  // the compiler expect the first slot of a function to hold the receiver.
  // since a fiber's stack is invoked directly, it does not have one, so
  // put it in here
  new_fiber->push(nullptr);

  RETURN_VAL(new_fiber);
}

DEF_NATIVE(fiber_isdone) {
  FiberObject* run_fiber = args[0].as_fiber();
  RETURN_VAL(run_fiber->empty_frame());
}

DEF_NATIVE(fiber_run) {
  FiberObject* run_fiber = args[0].as_fiber();

  if (run_fiber->empty_frame())
    RETURN_ERR("cannot run a finished fiber");

  // remember who ran it
  run_fiber->set_caller(fiber);
  // if the fiber was yielded, make the yield call return nil
  if (run_fiber->stack_size() > 0)
    run_fiber->set_value(run_fiber->stack_size() - 1, nullptr);

  return PrimitiveResult::RUN_FIBER;
}

DEF_NATIVE(fiber_run1) {
  FiberObject* run_fiber = args[0].as_fiber();

  if (run_fiber->empty_frame())
    RETURN_ERR("cannot run a finished fiber");

  // remember who ran it
  run_fiber->set_caller(fiber);
  // if the fiber was yielded, make the yield call return the value passed
  // to run
  if (run_fiber->stack_size() > 0)
    run_fiber->set_value(run_fiber->stack_size() - 1, args[1]);

  // when the calling fiber resumes, we will store the result of the run call
  // in it's stack, since fiber.run(value) has two arguments (the fiber and
  // the value) and we only need one slot for the result, discard the other
  // slot now
  fiber->pop();

  return PrimitiveResult::RUN_FIBER;
}

DEF_NATIVE(fiber_yield) {
  if (fiber->caller() == nullptr)
    RETURN_ERR("no fiber to yield to");

  // make the caller's run method return nil
  fiber->caller()->set_value(fiber->caller()->stack_size() - 1, nullptr);

  // return the fiber to resume
  args[0] = fiber->caller();
  return PrimitiveResult::RUN_FIBER;
}

DEF_NATIVE(fiber_yield1) {
  if (fiber->caller() == nullptr)
    RETURN_ERR("no fiber to yield to");

  // make the caller's run method return the argument passed to yield
  fiber->caller()->set_value(fiber->caller()->stack_size() - 1, args[1]);

  // when we yielding fiber resumes, we will store the result of the yield
  // call in it's stack, since Fiber.yield(value) has two arguments (the
  // Fiber class and the value) and we only need one slot for the result,
  // discard the other slot now
  fiber->pop();

  // return the fiber to resume
  args[0] = fiber->caller();
  return PrimitiveResult::RUN_FIBER;
}

DEF_NATIVE(numeric_abs) {
  RETURN_VAL(std::abs(args[0].as_numeric()));
}

DEF_NATIVE(numeric_ceil) {
  RETURN_VAL(std::ceil(args[0].as_numeric()));
}

DEF_NATIVE(numeric_cos) {
  RETURN_VAL(std::cos(args[0].as_numeric()));
}

DEF_NATIVE(numeric_floor) {
  RETURN_VAL(std::floor(args[0].as_numeric()));
}

DEF_NATIVE(numeric_isnan) {
  RETURN_VAL(std::isnan(args[0].as_numeric()));
}

DEF_NATIVE(numeric_sin) {
  RETURN_VAL(std::sin(args[0].as_numeric()));
}

DEF_NATIVE(numeric_sqrt) {
  RETURN_VAL(std::sqrt(args[0].as_numeric()));
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
  if (!validate_numeric(vm, args, 1, "Right operand"))
    return PrimitiveResult::ERROR;

  RETURN_VAL(args[0].as_numeric() > args[1].as_numeric());
}

DEF_NATIVE(numeric_ge) {
  if (!validate_numeric(vm, args, 1, "Right operand"))
    return PrimitiveResult::ERROR;

  RETURN_VAL(args[0].as_numeric() >= args[1].as_numeric());
}

DEF_NATIVE(numeric_lt) {
  if (!validate_numeric(vm, args, 1, "Right operand"))
    return PrimitiveResult::ERROR;

  RETURN_VAL(args[0].as_numeric() < args[1].as_numeric());
}

DEF_NATIVE(numeric_le) {
  if (!validate_numeric(vm, args, 1, "Right operand"))
    return PrimitiveResult::ERROR;

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

DEF_NATIVE(numeric_dotdot) {
  if (!validate_numeric(vm, args, 1, "Right hand side of range"))
    return PrimitiveResult::ERROR;

  double from = args[0].as_numeric();
  double to = args[1].as_numeric();
  RETURN_VAL(RangeObject::make_range(vm, from, to, true));
}

DEF_NATIVE(numeric_dotdotdot) {
  if (!validate_numeric(vm, args, 1, "Right hand side of range"))
    return PrimitiveResult::ERROR;

  double from = args[0].as_numeric();
  double to = args[1].as_numeric();
  RETURN_VAL(RangeObject::make_range(vm, from, to, false));
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
  if (args[0].is_class()) {
    RETURN_VAL(args[0].as_class()->name());
  }
  else if (args[0].is_instance()) {
    InstanceObject* inst = args[0].as_instance();
    RETURN_VAL(StringObject::concat_string(
          vm, "instance of ", inst->cls()->name_cstr()));
  }
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

static PrimitiveResult call_function(WrenVM& vm, Value* args, int argc) {
  FunctionObject* fn;
  if (args[0].is_closure())
    fn = args[0].as_closure()->fn();
  else
    fn = args[0].as_function();

  if (argc < fn->num_params())
    RETURN_ERR("function expects more arguments");
  return PrimitiveResult::CALL;
}

DEF_NATIVE_FN(fn_call, 0)
DEF_NATIVE_FN(fn_call, 1)
DEF_NATIVE_FN(fn_call, 2)
DEF_NATIVE_FN(fn_call, 3)
DEF_NATIVE_FN(fn_call, 4)
DEF_NATIVE_FN(fn_call, 5)
DEF_NATIVE_FN(fn_call, 6)
DEF_NATIVE_FN(fn_call, 7)
DEF_NATIVE_FN(fn_call, 8)
DEF_NATIVE_FN(fn_call, 9)
DEF_NATIVE_FN(fn_call, 10)
DEF_NATIVE_FN(fn_call, 11)
DEF_NATIVE_FN(fn_call, 12)
DEF_NATIVE_FN(fn_call, 13)
DEF_NATIVE_FN(fn_call, 14)
DEF_NATIVE_FN(fn_call, 15)
DEF_NATIVE_FN(fn_call, 16)

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

  if (args[1].is_numeric()) {
    int index = validate_index(vm, args, 1, list->count(), "Subscript");
    if (index ==  -1)
      return PrimitiveResult::ERROR;

    RETURN_VAL(list->get_element(index));
  }

  if (!args[1].is_range())
    RETURN_ERR("Subscript must be a numeric or a range");

  RangeObject* range = args[1].as_range();
  int from = validate_index_value(vm,
      args, list->count(), range->from(), "Range start");
  if (from == -1)
    return PrimitiveResult::ERROR;

  int to, count;
  if (range->is_inclusive()) {
    to = validate_index_value(vm,
        args, list->count(), range->to(), "Range end");
    if (to == -1)
      return PrimitiveResult::ERROR;

    count = std::abs(from - to) + 1;
  }
  else {
    if (!validate_int_value(vm, args, range->to(), "Range end"))
      return PrimitiveResult::ERROR;

    // bounds check it manually here since the exclusive range can
    // hang over the edge
    to = Xt::as_type<int>(range->to());
    if (to < 0)
      to = list->count() + to;
    if (to < -1 || to > list->count())
      RETURN_ERR("Reange end out of bounds");

    count = std::abs(from - to);
  }

  int step = from < to ? 1 : -1;
  ListObject* result = ListObject::make_list(vm, count);
  for (int i = 0; i < count; ++i)
    result->set_element(i, list->get_element(from + (i * step)));
  RETURN_VAL(result);
}

DEF_NATIVE(list_subscript_setter) {
  ListObject* list = args[0].as_list();
  int index = validate_index(vm, args, 1, list->count(), "Index");
  if (index == -1)
    return PrimitiveResult::ERROR;

  list->set_element(index, args[2]);
  RETURN_VAL(args[2]);
}

DEF_NATIVE(range_from) {
  RangeObject* range = args[0].as_range();
  RETURN_VAL(range->from());
}

DEF_NATIVE(range_to) {
  RangeObject* range = args[0].as_range();
  RETURN_VAL(range->to());
}

DEF_NATIVE(range_min) {
  RangeObject* range = args[0].as_range();
  RETURN_VAL(std::fmin(range->from(), range->to()));
}

DEF_NATIVE(range_max) {
  RangeObject* range = args[0].as_range();
  RETURN_VAL(std::fmax(range->from(), range->to()));
}

DEF_NATIVE(range_isinclusive) {
  RangeObject* range = args[0].as_range();
  RETURN_VAL(range->is_inclusive());
}

DEF_NATIVE(range_iterate) {
  RangeObject* range = args[0].as_range();

  if (range->from() == range->to() && !range->is_inclusive())
    RETURN_VAL(false);
  if (args[1].is_nil())
    RETURN_VAL(range->from());
  if (!validate_numeric(vm, args, 1, "Iterator"))
    return PrimitiveResult::ERROR;

  double iterator = args[1].as_numeric();
  // iterate towards [to] from [from]
  if (range->from() < range->to()) {
    ++iterator;
    if (iterator > range->to())
      RETURN_VAL(false);
  }
  else {
    --iterator;
    if (iterator < range->to())
      RETURN_VAL(false);
  }
  if (!range->is_inclusive() && iterator == range->to())
    RETURN_VAL(false);

  RETURN_VAL(iterator);
}

DEF_NATIVE(range_itervalue) {
  RETURN_VAL(args[1]);
}

DEF_NATIVE(range_tostring) {
  RangeObject* range = args[0].as_range();

  std::stringstream ss;
  ss << range->from() << (range->is_inclusive() ? ".." : "...") << range->to();
  RETURN_VAL(StringObject::make_string(vm, ss.str()));
}

static ClassObject* define_single_class(WrenVM& vm, const str_t& name) {
  StringObject* name_string = StringObject::make_string(vm, name);

  PinnedGuard guard(vm, name_string);
  ClassObject* cls = ClassObject::make_single_class(vm, name_string);
  vm.set_global(name, cls);
  return cls;
}

static ClassObject* define_class(WrenVM& vm, const str_t& name) {
  StringObject* name_string = StringObject::make_string(vm, name);

  PinnedGuard guard(vm, name_string);
  ClassObject* cls = ClassObject::make_class(vm, vm.obj_cls(), 0, name_string);
  vm.set_global(name, cls);
  return cls;
}

namespace core {
  void initialize(WrenVM& vm) {
    // define the root object class, this has to be done a little specially
    // because it has no superclass and an unusual metaclass (Class)
    vm.set_obj_cls(define_single_class(vm, "Object"));

    vm.set_native(vm.obj_cls(), "== ", _primitive_object_eq);
    vm.set_native(vm.obj_cls(), "!= ", _primitive_object_ne);
    vm.set_native(vm.obj_cls(), "new", _primitive_object_new);
    vm.set_native(vm.obj_cls(), "toString", _primitive_object_tostring);
    vm.set_native(vm.obj_cls(), "type", _primitive_object_type);

    // now we can define Class, which is a subclass of Object, but Object's
    // metclass
    vm.set_class_cls(define_single_class(vm, "Class"));
    vm.set_native(vm.class_cls(), "name", _primitive_class_name);

    // now that Object and Class are defined, we can wrie them up to each other
    vm.class_cls()->bind_superclass(vm.obj_cls());
    vm.obj_cls()->set_meta_class(vm.class_cls());
    vm.class_cls()->set_meta_class(vm.class_cls());

    vm.set_bool_cls(define_class(vm, "Bool"));
    vm.set_native(vm.bool_cls(), "toString", _primitive_bool_tostring);
    vm.set_native(vm.bool_cls(), "!", _primitive_bool_not);

    vm.set_fiber_cls(define_class(vm, "Fiber"));
    vm.set_native(vm.fiber_cls()->meta_class(), "create ", _primitive_fiber_create);
    vm.set_native(vm.fiber_cls()->meta_class(), "yield", _primitive_fiber_yield);
    vm.set_native(vm.fiber_cls()->meta_class(), "yield ", _primitive_fiber_yield1);
    vm.set_native(vm.fiber_cls(), "isDone", _primitive_fiber_isdone);
    vm.set_native(vm.fiber_cls(), "run", _primitive_fiber_run);
    vm.set_native(vm.fiber_cls(), "run ", _primitive_fiber_run1);

    vm.set_fn_cls(define_class(vm, "Function"));
    vm.set_native(vm.fn_cls(), "call", _primitive_fn_call0);
    vm.set_native(vm.fn_cls(), "call ", _primitive_fn_call1);
    vm.set_native(vm.fn_cls(), "call  ", _primitive_fn_call2);
    vm.set_native(vm.fn_cls(), "call   ", _primitive_fn_call3);
    vm.set_native(vm.fn_cls(), "call    ", _primitive_fn_call4);
    vm.set_native(vm.fn_cls(), "call     ", _primitive_fn_call5);
    vm.set_native(vm.fn_cls(), "call      ", _primitive_fn_call6);
    vm.set_native(vm.fn_cls(), "call       ", _primitive_fn_call7);
    vm.set_native(vm.fn_cls(), "call        ", _primitive_fn_call8);
    vm.set_native(vm.fn_cls(), "call         ", _primitive_fn_call9);
    vm.set_native(vm.fn_cls(), "call          ", _primitive_fn_call10);
    vm.set_native(vm.fn_cls(), "call           ", _primitive_fn_call11);
    vm.set_native(vm.fn_cls(), "call            ", _primitive_fn_call12);
    vm.set_native(vm.fn_cls(), "call             ", _primitive_fn_call13);
    vm.set_native(vm.fn_cls(), "call              ", _primitive_fn_call14);
    vm.set_native(vm.fn_cls(), "call               ", _primitive_fn_call15);
    vm.set_native(vm.fn_cls(), "call                ", _primitive_fn_call16);

    vm.set_nil_cls(define_class(vm, "Nil"));
    vm.set_native(vm.nil_cls(), "toString", _primitive_nil_tostring);

    vm.set_num_cls(define_class(vm, "Numeric"));
    vm.set_native(vm.num_cls(), "abs", _primitive_numeric_abs);
    vm.set_native(vm.num_cls(), "ceil", _primitive_numeric_ceil);
    vm.set_native(vm.num_cls(), "cos", _primitive_numeric_cos);
    vm.set_native(vm.num_cls(), "floor", _primitive_numeric_floor);
    vm.set_native(vm.num_cls(), "isNan", _primitive_numeric_isnan);
    vm.set_native(vm.num_cls(), "sin", _primitive_numeric_sin);
    vm.set_native(vm.num_cls(), "sqrt", _primitive_numeric_sqrt);
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
    vm.set_native(vm.num_cls(), ".. ", _primitive_numeric_dotdot);
    vm.set_native(vm.num_cls(), "... ", _primitive_numeric_dotdotdot);

    vm.set_range_cls(define_class(vm, "Range"));
    vm.set_native(vm.range_cls(), "from", _primitive_range_from);
    vm.set_native(vm.range_cls(), "to", _primitive_range_to);
    vm.set_native(vm.range_cls(), "min", _primitive_range_min);
    vm.set_native(vm.range_cls(), "max", _primitive_range_max);
    vm.set_native(vm.range_cls(), "isInclusive", _primitive_range_isinclusive);
    vm.set_native(vm.range_cls(), "iterate ", _primitive_range_iterate);
    vm.set_native(vm.range_cls(), "iterValue ", _primitive_range_itervalue);
    vm.set_native(vm.range_cls(), "toString", _primitive_range_tostring);

    // vm.set_obj_cls(vm.get_global("Object").as_class());

    vm.set_str_cls(define_class(vm, "String"));
    vm.set_native(vm.str_cls(), "len", _primitive_string_len);
    vm.set_native(vm.str_cls(), "contains ", _primitive_string_contains);
    vm.set_native(vm.str_cls(), "toString", _primitive_string_tostring);
    vm.set_native(vm.str_cls(), "+ ", _primitive_string_add);
    vm.set_native(vm.str_cls(), "== ", _primitive_string_eq);
    vm.set_native(vm.str_cls(), "!= ", _primitive_string_ne);
    vm.set_native(vm.str_cls(), "[ ]", _primitive_string_subscript);

    /// from core library source
    vm.interpret("Wren core library", kLibSource);

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
  }
}

}
