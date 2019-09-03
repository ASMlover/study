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
#include <tuple>
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
"class Sequence {\n"
"  map(f) {\n"
"    var result = new List\n"
"    for (element in this) {\n"
"      result.add(f.call(element))\n"
"    }\n"
"    return result\n"
"  }\n"
"\n"
"  where(f) {\n"
"    var result = new List\n"
"    for (element in this) {\n"
"      if (f.call(element)) result.add(element)\n"
"    }\n"
"    return result\n"
"  }\n"
"\n"
"  all(f) {\n"
"    for (element in this) {\n"
"      if (!f.call(element)) return false\n"
"    }\n"
"    return true\n"
"  }\n"
"\n"
"  reduce(acc, f) {\n"
"    for (element in this) {\n"
"      acc = f.call(acc, element)\n"
"    }\n"
"    return acc\n"
"  }\n"
"\n"
"\n"
"  reduce(f) {\n"
"    var iter = iterate(nil)\n"
"    if (!iter) Fiber.abort(\"cannot reduce an empty sequence\")\n"
"\n"
"    // seed with the first element\n"
"    var result = iterValue(iter)\n"
"    while (iter = iterate(iter)) {\n"
"      result = f.call(result, iterValue(iter))\n"
"    }\n"
"    return result\n"
"  }\n"
"\n"
"  join { join(\"\") }\n"
"\n"
"  join(sep) {\n"
"    var first = true\n"
"    var result = \"\"\n"
"\n"
"    for (element in this) {\n"
"      if (!first) result = result + sep\n"
"      first = false\n"
"      result = result + element.toString\n"
"    }\n"
"    return result\n"
"  }\n"
"}\n"
"\n"
"class String is Sequence {}\n"
"\n"
"class List is Sequence {\n"
"  addAll(other) {\n"
"    for (element in other) {\n"
"      add(element)\n"
"    }\n"
"    return other\n"
"  }\n"
"\n"
"  toString { \"[\" + join(\", \") + \"]\" }\n"
"\n"
"  +(other) {\n"
"    var result = this[0..-1]\n"
"    for (element in other) {\n"
"      result.add(element)\n"
"    }\n"
"    return result\n"
"  }\n"
"\n"
"  contains(element) {\n"
"    for (item in this) {\n"
"      if (element == item) {\n"
"        return true\n"
"      }\n"
"    }\n"
"    return false\n"
"  }\n"
"}\n"
"\n"
"class Range is Sequence {}\n"
"\n"
"class MapKeySequence is Sequence {\n"
"  new(map) {\n"
"    _map = map\n"
"  }\n"
"\n"
"  iterate(n) { _map.iter(n) }\n"
"  iterValue(iterator) { _map.keyIterValue(iterator) }\n"
"}\n"
"\n"
"class MapValSequence is Sequence {\n"
"  new(map) {\n"
"    _map = map\n"
"  }\n"
"\n"
"  iterate(n) { _map.iter(n) }\n"
"  iterValue(iterator) { _map.valIterValue(iterator) }\n"
"}\n"
"\n"
"class Map {\n"
"  keys { new MapKeySequence(this) }\n"
"  values { new MapValSequence(this) }\n"
"\n"
"  toString {\n"
"    var first = true\n"
"    var result = \"{\"\n"
"\n"
"    for (key in keys) {\n"
"      if (!first) result = result + \", \"\n"
"      first = false\n"
"      result = result + key.toString + \": \" + this[key].toString\n"
"    }\n"
"\n"
"    return result + \"}\"\n"
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

static bool validate_function(WrenVM& vm,
    Value* args, int index, const str_t& arg_name) {
  // validates that the given argument in [args] is a function, returns true
  // if it is, if not reports an error and returns false

  if (args[index].is_function() || args[index].is_closure())
    return true;

  std::stringstream ss;
  ss << "`" << arg_name << "` must be a function";
  args[0] = StringObject::make_string(vm, ss.str());
  return false;
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

static bool validate_key(WrenVM& vm, Value* args, int index) {
  // validates that [key] is a valid object for use as a map key, returns
  // true if it is, if not, reports an error and returns false

  const Value& arg = args[index];
  if (arg.is_boolean() || arg.is_class() || arg.is_nil() ||
      arg.is_numeric() || arg.is_range() || arg.is_string())
    return true;

  args[0] = StringObject::make_string(vm, "key must be a value type");
  return false;
}

static std::tuple<int, int, int> calculate_range(
    WrenVM& vm, Value* args, RangeObject* range, int length) {
  // given a [range] and the [length] of the object being operated on determine
  // if the range is valid and return a tuple<start, step, legth>

  // corner case: an empty range at zero is allowed on an empty sequence, this
  // way, list[0..-1] and list[0...list.count] can be used to copy a list even
  // when empty
  if (length == 0 && range->from() == 0 &&
      range->to() == (range->is_inclusive() ? -1 : 0))
    return std::make_tuple(0, 0, length);

  int from = validate_index_value(
      vm, args, length, range->from(), "Range start");
  if (from == -1)
    return std::make_tuple(-1, 0, length);

  int to, count;
  if (range->is_inclusive()) {
    to = validate_index_value(vm, args, length, range->to(), "Range end");
    if (to == -1)
      return std::make_tuple(-1, 0, length);

    count = std::abs(from - to) + 1;
  }
  else {
    if (!validate_int_value(vm, args, range->to(), "Range end"))
      return std::make_tuple(-1, 0, length);

    // bounds check it manually here since the excusive range can hang over
    // the edge
    to = Xt::as_type<int>(range->to());
    if (to < 0)
      to = length + to;

    if (to < -1 || to > length) {
      args[0] = StringObject::make_string(vm, "`Range end` out of bounds");
      return std::make_tuple(-1, 0, length);
    }

    count = std::abs(from - to);
  }

  return std::make_tuple(from, (from < to ? 1 : -1), count);
}

DEF_NATIVE(nil_not) {
  RETURN_VAL(true);
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

DEF_NATIVE(class_instantiate) {
  ClassObject* cls = args[0].as_class();
  RETURN_VAL(InstanceObject::make_instance(vm, cls));
}

DEF_NATIVE(class_name) {
  ClassObject* cls = args[0].as_class();
  RETURN_VAL(cls->name());
}

DEF_NATIVE(fiber_instantiate) {
  // return the Fiber class itself, when we then call `new` on it, it will
  // create the fiber

  RETURN_VAL(args[0]);
}

DEF_NATIVE(fiber_new) {
  if (!validate_function(vm, args, 1, "Argument"))
    return PrimitiveResult::ERROR;

  FiberObject* new_fiber = FiberObject::make_fiber(vm, args[1].as_object());

  // the compiler expect the first slot of a function to hold the receiver.
  // since a fiber's stack is invoked directly, it does not have one, so
  // put it in here
  new_fiber->push(nullptr);

  RETURN_VAL(new_fiber);
}

DEF_NATIVE(fiber_abort) {
  if (!validate_string(vm, args, 1, "Error message"))
    return PrimitiveResult::ERROR;

  // move the error message to the return position
  args[0] = args[1];
  return PrimitiveResult::ERROR;
}

DEF_NATIVE(fiber_call) {
  FiberObject* run_fiber = args[0].as_fiber();

  if (run_fiber->empty_frame())
    RETURN_ERR("cannot call a finished fiber");
  if (run_fiber->caller() != nullptr)
    RETURN_ERR("fiber has already been called");

  // remember who ran it
  run_fiber->set_caller(fiber);
  // if the fiber was yielded, make the yield call return nil
  if (run_fiber->stack_size() > 0)
    run_fiber->set_value(run_fiber->stack_size() - 1, nullptr);

  return PrimitiveResult::RUN_FIBER;
}

DEF_NATIVE(fiber_call1) {
  FiberObject* run_fiber = args[0].as_fiber();

  if (run_fiber->empty_frame())
    RETURN_ERR("cannot call a finished fiber");
  if (run_fiber->caller() != nullptr)
    RETURN_ERR("fiber has already been called");

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

DEF_NATIVE(fiber_error) {
  FiberObject* run_fiber = args[0].as_fiber();
  if (run_fiber->error() == nullptr)
    RETURN_VAL(nullptr);
  RETURN_VAL(run_fiber->error());
}

DEF_NATIVE(fiber_isdone) {
  FiberObject* run_fiber = args[0].as_fiber();
  RETURN_VAL(run_fiber->empty_frame() || run_fiber->error() != nullptr);
}

DEF_NATIVE(fiber_run) {
  FiberObject* run_fiber = args[0].as_fiber();

  if (run_fiber->empty_frame())
    RETURN_ERR("cannot run a finished fiber");

  // if the fiber was yield, make the yield call return nil
  if (run_fiber->caller() == nullptr && run_fiber->stack_size() > 0)
    run_fiber->set_value(run_fiber->stack_size() - 1, nullptr);

  // unlike call, this does not remeber the calling fiber, instead, it remeber
  // is *that* fiber's caller, you can think of it like tail call elimination
  // the switched-from fiber is discarded and when the switched to fiber
  // completes or yields, control passes to the switched-from fiber's caller.
  run_fiber->set_caller(fiber->caller());

  return PrimitiveResult::RUN_FIBER;
}

DEF_NATIVE(fiber_run1) {
  FiberObject* run_fiber = args[0].as_fiber();

  if (run_fiber->empty_frame())
    RETURN_ERR("cannot run a finished fiber");

  // if the fiber was yield, make the yield call return the value passed to run
  if (run_fiber->caller() == nullptr && run_fiber->stack_size() > 0)
    run_fiber->set_value(run_fiber->stack_size() - 1, args[1]);

  // unlike call, this does not remeber the calling fiber, instead, it remeber
  // is *that* fiber's caller, you can think of it like tail call elimination
  // the switched-from fiber is discard and when the switched to fiber
  // completes or yields, control passes to the switched-from fiber's caller
  run_fiber->set_caller(fiber->caller());

  return PrimitiveResult::RUN_FIBER;
}

DEF_NATIVE(fiber_try) {
  FiberObject* run_fiber = args[0].as_fiber();

  if (run_fiber->empty_frame())
    RETURN_ERR("cannot try a finished fiber");
  if (run_fiber->caller() != nullptr)
    RETURN_ERR("fiber has already benn called");

  // remeber who ran it
  run_fiber->set_caller(fiber);
  run_fiber->set_caller_is_trying(true);

  // if the fiber was yielded, make the yield call return nil
  if (run_fiber->stack_size() > 0)
    run_fiber->set_value(run_fiber->stack_size() - 1, nullptr);

  return PrimitiveResult::RUN_FIBER;
}

DEF_NATIVE(fiber_yield) {
  if (fiber->caller() == nullptr)
    RETURN_ERR("no fiber to yield to");

  FiberObject* caller = fiber->caller();
  fiber->set_caller(nullptr);
  fiber->set_caller_is_trying(false);

  // make the caller's run method return nil
  caller->set_value(caller->stack_size() - 1, nullptr);

  // return the fiber to resume
  args[0] = caller;
  return PrimitiveResult::RUN_FIBER;
}

DEF_NATIVE(fiber_yield1) {
  if (fiber->caller() == nullptr)
    RETURN_ERR("no fiber to yield to");

  FiberObject* caller = fiber->caller();
  fiber->set_caller(nullptr);
  fiber->set_caller_is_trying(false);

  // make the caller's run method return the argument passed to yield
  caller->set_value(caller->stack_size() - 1, args[1]);

  // when we yielding fiber resumes, we will store the result of the yield
  // call in it's stack, since Fiber.yield(value) has two arguments (the
  // Fiber class and the value) and we only need one slot for the result,
  // discard the other slot now
  fiber->pop();

  // return the fiber to resume
  args[0] = caller;
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
  double v = args[0].as_numeric();
  if (v != v)
    RETURN_VAL(StringObject::make_string(vm, "nan"));

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

DEF_NATIVE(numeric_bitand) {
  if (!validate_numeric(vm, args, 1, "Right operand"))
    return PrimitiveResult::ERROR;

  // bitwise operators always work on 32-bit unsigned int
  u32_t lhs = Xt::as_type<u32_t>(args[0].as_numeric());
  u32_t rhs = Xt::as_type<u32_t>(args[1].as_numeric());
  RETURN_VAL(lhs & rhs);
}

DEF_NATIVE(numeric_bitor) {
  if (!validate_numeric(vm, args, 1, "Right operand"))
    return PrimitiveResult::ERROR;

  // bitwise operators always work on 32-bit unsigned int
  u32_t lhs = Xt::as_type<u32_t>(args[0].as_numeric());
  u32_t rhs = Xt::as_type<u32_t>(args[1].as_numeric());
  RETURN_VAL(lhs | rhs);
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

DEF_NATIVE(object_not) {
  RETURN_VAL(false);
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

DEF_NATIVE(object_instantiate) {
  RETURN_ERR("must provide a class to `new` to construct");
}

DEF_NATIVE(string_len) {
  RETURN_VAL(args[0].as_string()->size());
}

DEF_NATIVE(string_endswith) {
  if (!validate_string(vm, args, 1, "Argument"))
    return PrimitiveResult::ERROR;

  StringObject* string = args[0].as_string();
  StringObject* search = args[1].as_string();

  // corner case, if the search string is longer than return false right away
  if (search->size() > string->size())
    RETURN_VAL(false);

  int r = memcmp(
      string->cstr() + string->size() - search->size(),
      search->cstr(), search->size());
  RETURN_VAL(r == 0);
}

DEF_NATIVE(string_indexof) {
  if (!validate_string(vm, args, 1, "Argument"))
    return PrimitiveResult::ERROR;

  StringObject* string = args[0].as_string();
  StringObject* search = args[1].as_string();

  const char* first_occur = strstr(string->cstr(), search->cstr());
  RETURN_VAL(first_occur != nullptr ? (first_occur - string->cstr()) : -1);
}

DEF_NATIVE(string_iterate) {
  StringObject* s = args[0].as_string();

  // if we are starting the interation, return the first index
  if (args[1].is_nil()) {
    if (s->size() == 0)
      RETURN_VAL(false);
    RETURN_VAL(0);
  }

  if (!validate_int(vm, args, 1, "Iterator"))
    return PrimitiveResult::ERROR;

  int index = Xt::as_type<int>(args[1].as_numeric());
  if (index < 0 || index >= s->size() - 1)
    RETURN_VAL(false);

  RETURN_VAL(index + 1);
}

DEF_NATIVE(string_itervalue) {
  StringObject* s = args[0].as_string();
  int index = validate_index(vm, args, 1, s->size(), "Iterator");
  if (index == -1)
    return PrimitiveResult::ERROR;

  RETURN_VAL(StringObject::make_string(vm, (*s)[index]));
}

DEF_NATIVE(string_startswith) {
  if (!validate_string(vm, args, 1, "Argument"))
    return PrimitiveResult::ERROR;

  StringObject* string = args[0].as_string();
  StringObject* search = args[1].as_string();

  // corner case, if the search string is longer than return false right away
  if (search->size() > string->size())
    RETURN_VAL(false);

  RETURN_VAL(memcmp(string->cstr(), search->cstr(), search->size()) == 0);
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
  if (!validate_string(vm, args, 1, "Right operand"))
    return PrimitiveResult::ERROR;

  StringObject* lhs = args[0].as_string();
  StringObject* rhs = args[1].as_string();
  RETURN_VAL(StringObject::make_string(vm, lhs, rhs));
}

DEF_NATIVE(string_subscript) {
  StringObject* s = args[0].as_string();

  if (args[1].is_numeric()) {
    int index = validate_index(vm, args, 1, s->size(), "Subscript");
    if (index == -1)
      return PrimitiveResult::ERROR;

    RETURN_VAL(StringObject::make_string(vm, (*s)[index]));
  }

  if (!args[1].is_range())
    RETURN_ERR("subscript must be a numeric or a range");

  auto [start, step, count] = calculate_range(
      vm, args, args[1].as_range(), s->size());
  if (start == -1)
    return PrimitiveResult::ERROR;

  str_t text(count, 0);
  const char* raw_str = s->cstr();
  for (int i = 0; i < count; ++i)
    text[i] = raw_str[start + (i * step)];
  RETURN_VAL(StringObject::make_string(vm, text));
}

static PrimitiveResult call_function(WrenVM& vm, Value* args, int argc) {
  FunctionObject* fn;
  if (args[0].is_closure())
    fn = args[0].as_closure()->fn();
  else
    fn = args[0].as_function();

  if (argc < fn->arity())
    RETURN_ERR("function expects more arguments");
  return PrimitiveResult::CALL;
}

DEF_NATIVE(fn_instantiate) {
  // return the Function class itself, when we then call `new` on it,
  // it will return the block

  RETURN_VAL(args[0]);
}

DEF_NATIVE(fn_new) {
  if (!validate_function(vm, args, 1, "Argument"))
    return PrimitiveResult::ERROR;

  // the block argument is already a function, so just return it
  RETURN_VAL(args[1]);
}

DEF_NATIVE(fn_arity) {
  RETURN_VAL(args[0].as_function()->arity());
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

DEF_NATIVE(fn_tostring) {
  RETURN_VAL(StringObject::make_string(vm, "<fn>"));
}

DEF_NATIVE(list_instantiate) {
  RETURN_VAL(ListObject::make_list(vm, 0));
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
  ListObject* list = args[0].as_list();

  // if we are starting the interation, return the first index
  if (args[1].is_nil()) {
    if (list->count() == 0)
      RETURN_VAL(false);
    RETURN_VAL(0);
  }

  if (!validate_int(vm, args, 1, "Iterator"))
    return PrimitiveResult::ERROR;

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

  auto [start, step, count] = calculate_range(
      vm, args, args[1].as_range(), list->count());
  if (start == -1)
    return PrimitiveResult::ERROR;

  ListObject* result = ListObject::make_list(vm, count);
  for (int i = 0; i < count; ++i)
    result->set_element(i, list->get_element(start + (i * step)));
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

DEF_NATIVE(map_instantiate) {
  RETURN_VAL(MapObject::make_map(vm));
}

DEF_NATIVE(map_subscript) {
  if (!validate_key(vm, args, 1))
    return PrimitiveResult::ERROR;

  if (auto val = args[0].as_map()->get(args[1]); val)
    RETURN_VAL(*val);
  RETURN_VAL(nullptr);
}

DEF_NATIVE(map_subscript_setter) {
  if (!validate_key(vm, args, 1))
    return PrimitiveResult::ERROR;

  args[0].as_map()->set(args[1], args[2]);
  RETURN_VAL(args[2]);
}

DEF_NATIVE(map_clear) {
  args[0].as_map()->clear();
  RETURN_VAL(nullptr);
}

DEF_NATIVE(map_contains) {
  if (!validate_key(vm, args, 1))
    return PrimitiveResult::ERROR;

  RETURN_VAL(args[0].as_map()->find(args[1]) != -1);
}

DEF_NATIVE(map_len) {
  RETURN_VAL(args[0].as_map()->count());
}

DEF_NATIVE(map_remove) {
  if (!validate_key(vm, args, 1))
    return PrimitiveResult::ERROR;

  RETURN_VAL(args[0].as_map()->remove(args[1]));
}

DEF_NATIVE(map_iterate) {
  MapObject* map = args[0].as_map();

  if (map->count() == 0)
    RETURN_VAL(false);

  // if we are starting the iteration, return the first entry
  int index = -1;
  if (!args[1].is_nil()) {
    if (!validate_int(vm, args, 1, "Iterator"))
      return PrimitiveResult::ERROR;

    index = Xt::as_type<int>(args[1].as_numeric());
    if (index < 0 || index >= map->capacity())
      RETURN_VAL(false);
  }

  // find the index used entry if any
  for (++index; index < map->capacity(); ++index) {
    if (!(*map)[index].first.is_undefined())
      RETURN_VAL(index);
  }

  // if we get here, walked all of the entries
  RETURN_VAL(false);
}

DEF_NATIVE(map_iterkey) {
  MapObject* map = args[0].as_map();
  int index = validate_index(vm, args, 1, map->capacity(), "Iterator");
  if (index == -1)
    return PrimitiveResult::ERROR;

  const auto& entry = (*map)[index];
  if (entry.first.is_undefined())
    RETURN_ERR("invalid map iterator value");
  RETURN_VAL(entry.first);
}

DEF_NATIVE(map_itervalue) {
  MapObject* map = args[0].as_map();
  int index = validate_index(vm, args, 1, map->capacity(), "Iterator");
  if (index == -1)
    return PrimitiveResult::ERROR;

  const auto& entry = (*map)[index];
  if (entry.first.is_undefined())
    RETURN_ERR("invalid map iterator value");
  RETURN_VAL(entry.second);
}

static ClassObject* define_single_class(WrenVM& vm, const str_t& name) {
  StringObject* name_string = StringObject::make_string(vm, name);

  PinnedGuard guard(vm, name_string);
  ClassObject* cls = ClassObject::make_single_class(vm, name_string);
  vm.define_variable(vm.mmodule(), name, cls);
  return cls;
}

static ClassObject* define_class(WrenVM& vm, const str_t& name) {
  StringObject* name_string = StringObject::make_string(vm, name);

  PinnedGuard guard(vm, name_string);
  ClassObject* cls = ClassObject::make_class(vm, vm.obj_cls(), 0, name_string);
  vm.define_variable(vm.mmodule(), name, cls);
  return cls;
}

namespace core {
  void initialize(WrenVM& vm) {
    // define the root object class, this has to be done a little specially
    // because it has no superclass and an unusual metaclass (Class)
    vm.set_obj_cls(define_single_class(vm, "Object"));
    vm.set_native(vm.obj_cls(), "!", _primitive_object_not);
    vm.set_native(vm.obj_cls(), "== ", _primitive_object_eq);
    vm.set_native(vm.obj_cls(), "!= ", _primitive_object_ne);
    vm.set_native(vm.obj_cls(), "new", _primitive_object_new);
    vm.set_native(vm.obj_cls(), "toString", _primitive_object_tostring);
    vm.set_native(vm.obj_cls(), "type", _primitive_object_type);
    vm.set_native(vm.obj_cls(), " instantiate", _primitive_object_instantiate);

    // now we can define Class, which is a subclass of Object, but Object's
    // metclass
    vm.set_class_cls(define_single_class(vm, "Class"));

    // now that Object and Class are defined, we can wrie them up to each other
    vm.class_cls()->bind_superclass(vm.obj_cls());
    vm.obj_cls()->set_cls(vm.class_cls());
    vm.class_cls()->set_cls(vm.class_cls());

    // define the methods specific to Class after wiring up its superclass
    // to prevent the inherited ones from overwriting them
    vm.set_native(vm.class_cls(), " instantiate", _primitive_class_instantiate);
    vm.set_native(vm.class_cls(), "name", _primitive_class_name);

    vm.set_bool_cls(define_class(vm, "Bool"));
    vm.set_native(vm.bool_cls(), "toString", _primitive_bool_tostring);
    vm.set_native(vm.bool_cls(), "!", _primitive_bool_not);

    vm.set_fiber_cls(define_class(vm, "Fiber"));
    vm.set_native(vm.fiber_cls()->cls(), " instantiate", _primitive_fiber_instantiate);
    vm.set_native(vm.fiber_cls()->cls(), "new ", _primitive_fiber_new);
    vm.set_native(vm.fiber_cls()->cls(), "abort ", _primitive_fiber_abort);
    vm.set_native(vm.fiber_cls()->cls(), "yield", _primitive_fiber_yield);
    vm.set_native(vm.fiber_cls()->cls(), "yield ", _primitive_fiber_yield1);
    vm.set_native(vm.fiber_cls(), "call", _primitive_fiber_call);
    vm.set_native(vm.fiber_cls(), "call ", _primitive_fiber_call1);
    vm.set_native(vm.fiber_cls(), "error", _primitive_fiber_error);
    vm.set_native(vm.fiber_cls(), "isDone", _primitive_fiber_isdone);
    vm.set_native(vm.fiber_cls(), "run", _primitive_fiber_run);
    vm.set_native(vm.fiber_cls(), "run ", _primitive_fiber_run1);
    vm.set_native(vm.fiber_cls(), "try", _primitive_fiber_try);

    vm.set_fn_cls(define_class(vm, "Function"));
    vm.set_native(vm.fn_cls()->cls(), " instantiate", _primitive_fn_instantiate);
    vm.set_native(vm.fn_cls()->cls(), "new ", _primitive_fn_new);
    vm.set_native(vm.fn_cls(), "arity", _primitive_fn_arity);
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
    vm.set_native(vm.fn_cls(), "toString", _primitive_fn_tostring);

    vm.set_nil_cls(define_class(vm, "Nil"));
    vm.set_native(vm.nil_cls(), "!", _primitive_nil_not);
    vm.set_native(vm.nil_cls(), "toString", _primitive_nil_tostring);

    vm.set_num_cls(define_class(vm, "Numeric"));
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
    vm.set_native(vm.num_cls(), "& ", _primitive_numeric_bitand);
    vm.set_native(vm.num_cls(), "| ", _primitive_numeric_bitor);
    vm.set_native(vm.num_cls(), ".. ", _primitive_numeric_dotdot);
    vm.set_native(vm.num_cls(), "... ", _primitive_numeric_dotdotdot);
    vm.set_native(vm.num_cls(), "abs", _primitive_numeric_abs);
    vm.set_native(vm.num_cls(), "ceil", _primitive_numeric_ceil);
    vm.set_native(vm.num_cls(), "cos", _primitive_numeric_cos);
    vm.set_native(vm.num_cls(), "floor", _primitive_numeric_floor);
    vm.set_native(vm.num_cls(), "isNan", _primitive_numeric_isnan);
    vm.set_native(vm.num_cls(), "sin", _primitive_numeric_sin);
    vm.set_native(vm.num_cls(), "sqrt", _primitive_numeric_sqrt);
    vm.set_native(vm.num_cls(), "toString", _primitive_numeric_tostring);

    // vm.set_obj_cls(vm.find_variable("Object").as_class());

    /// from core library source
    vm.interpret("", kLibSource);

    vm.set_str_cls(vm.find_variable("String").as_class());
    vm.set_native(vm.str_cls(), "+ ", _primitive_string_add);
    vm.set_native(vm.str_cls(), "[ ]", _primitive_string_subscript);
    vm.set_native(vm.str_cls(), "len", _primitive_string_len);
    vm.set_native(vm.str_cls(), "contains ", _primitive_string_contains);
    vm.set_native(vm.str_cls(), "endsWith ", _primitive_string_endswith);
    vm.set_native(vm.str_cls(), "indexOf ", _primitive_string_indexof);
    vm.set_native(vm.str_cls(), "iterate ", _primitive_string_iterate);
    vm.set_native(vm.str_cls(), "iterValue ", _primitive_string_itervalue);
    vm.set_native(vm.str_cls(), "startsWith ", _primitive_string_startswith);
    vm.set_native(vm.str_cls(), "toString", _primitive_string_tostring);

    vm.set_list_cls(vm.find_variable("List").as_class());
    vm.set_native(vm.list_cls()->cls(), " instantiate", _primitive_list_instantiate);
    vm.set_native(vm.list_cls(), "[ ]", _primitive_list_subscript);
    vm.set_native(vm.list_cls(), "[ ]=", _primitive_list_subscript_setter);
    vm.set_native(vm.list_cls(), "add ", _primitive_list_add);
    vm.set_native(vm.list_cls(), "clear", _primitive_list_clear);
    vm.set_native(vm.list_cls(), "len", _primitive_list_len);
    vm.set_native(vm.list_cls(), "insert  ", _primitive_list_insert);
    vm.set_native(vm.list_cls(), "remove ", _primitive_list_remove);
    vm.set_native(vm.list_cls(), "iterate ", _primitive_list_iterate);
    vm.set_native(vm.list_cls(), "iterValue ", _primitive_list_itervalue);

    vm.set_range_cls(vm.find_variable("Range").as_class());
    vm.set_native(vm.range_cls(), "from", _primitive_range_from);
    vm.set_native(vm.range_cls(), "to", _primitive_range_to);
    vm.set_native(vm.range_cls(), "min", _primitive_range_min);
    vm.set_native(vm.range_cls(), "max", _primitive_range_max);
    vm.set_native(vm.range_cls(), "isInclusive", _primitive_range_isinclusive);
    vm.set_native(vm.range_cls(), "iterate ", _primitive_range_iterate);
    vm.set_native(vm.range_cls(), "iterValue ", _primitive_range_itervalue);
    vm.set_native(vm.range_cls(), "toString", _primitive_range_tostring);

    vm.set_map_cls(vm.find_variable("Map").as_class());
    vm.set_native(vm.map_cls()->cls(), " instantiate", _primitive_map_instantiate);
    vm.set_native(vm.map_cls(), "[ ]", _primitive_map_subscript);
    vm.set_native(vm.map_cls(), "[ ]=", _primitive_map_subscript_setter);
    vm.set_native(vm.map_cls(), "clear", _primitive_map_clear);
    vm.set_native(vm.map_cls(), "containsKey ", _primitive_map_contains);
    vm.set_native(vm.map_cls(), "len", _primitive_map_len);
    vm.set_native(vm.map_cls(), "remove ", _primitive_map_remove);
    vm.set_native(vm.map_cls(), "iter ", _primitive_map_iterate);
    vm.set_native(vm.map_cls(), "keyIterValue ", _primitive_map_iterkey);
    vm.set_native(vm.map_cls(), "valIterValue ", _primitive_map_itervalue);

    // while bootstrapping the core types and running the core library, a number
    // string objects have benn created, many of which were instantiated before
    // `str_class_` was stored in the VM, some of them *must* be created first:
    // the ClassObject from string itself has a reference to the StringObject
    // for its name.
    //
    // these all currently a `nullptr` `class_obj_` pointer, so go back and
    // assign them now that the string class is known
    vm.set_metaclasses();
  }
}

}
