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
#include "vm.hh"
#include "primitive.hh"

namespace wrencc {

static int validate_index_value(
    WrenVM& vm, int count, double value, const str_t& arg_name) {
  // validates that [value] is an integer within [0, count), also allows
  // negative indices which map backwards from the end, returns the valid
  // positive index value, if invalid, reports an error and return -1

  if (!validate_int_value(vm, value, arg_name))
    return -1;

  // negative indices count from the end
  if (value < 0)
    value = count + value;
  // check bounds
  if (value >= 0 && value < count)
    return Xt::as_type<int>(value);

  vm.fiber()->set_error(StringObject::format(
        vm, "`$` out of bounds", arg_name.c_str()));
  return -1;
}

int validate_index(const Value& index, int count) {
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

bool validate_function(WrenVM& vm, const Value& arg, const str_t& arg_name) {
  // validates that the given [arg] is a function, returns true if it is,
  // if not reports an error and returns false

  if (arg.is_closure() || arg.is_closure())
    return true;

  vm.fiber()->set_error(
      StringObject::format(vm, "`$` must be a function", arg_name.c_str()));
  return false;
}

bool validate_numeric(WrenVM& vm, const Value& arg, const str_t& arg_name) {
  // validates that the given [arg] is a Numeric, returns true if it is,
  // if not reports an error and returns false

  if (arg.is_numeric())
    return true;

  RETURN_FERR("`$` must be a numeric", arg_name.c_str());
}

bool validate_int_value(WrenVM& vm, double value, const str_t& arg_name) {
  // validates that [value] is an integer, returns true if it is, if not
  // reports an error and returns false

  if (std::trunc(value) == value)
    return true;

  RETURN_FERR("`$` must be an integer", arg_name.c_str());
}

bool validate_int(WrenVM& vm, const Value& arg, const str_t& arg_name) {
  // validates that the given [arg] is an integer, returns true if it is,
  // if not reports error and returns false

  // make sure it's a Numeric first
  if (!validate_numeric(vm, arg, arg_name))
    return false;

  return validate_int_value(vm, arg.as_numeric(), arg_name);
}

int validate_index(
    WrenVM& vm, const Value& arg, int count, const str_t& arg_name) {
  // validates that argument at [arg_index] is an integer within `[0, count)`,
  // also allows negative indices which map backwards from the end, returns
  // the valid positive index value, if invalid reports an error and return -1

  if (!validate_numeric(vm, arg, arg_name))
    return -1;

  return validate_index_value(vm, count, arg.as_numeric(), arg_name);
}

bool validate_string(WrenVM& vm, const Value& arg, const str_t& arg_name) {
  // validates that the given argument in [args] is a String, returns true
  // if it is, if not reports an error and returns false

  if (arg.is_string())
    return true;

  RETURN_FERR("`$` must be a string", arg_name.c_str());
}

bool validate_key(WrenVM& vm, const Value& arg) {
  // validates that [arg] is a valid object for use as a map key, returns
  // true if it is, if not, reports an error and returns false

  if (arg.is_boolean() || arg.is_class() || arg.is_nil() ||
      arg.is_numeric() || arg.is_range() || arg.is_string())
    return true;

  RETURN_ERR("key must be a value type");
}

std::tuple<int, int, int> calculate_range(
    WrenVM& vm, RangeObject* range, int length) {
  // given a [range] and the [length] of the object being operated on determine
  // if the range is valid and return a tuple<start, step, legth>

  // edge case: an empty range is allowed at the end of a sequence, this way,
  // list[0..-1] and list[0...list.count] can be used to copy a list even
  // when empty
  if (range->from() == length &&
      range->to() == (range->is_inclusive() ? -1.0 : length))
    return std::make_tuple(0, 0, 0);

  int from = validate_index_value(vm, length, range->from(), "Range start");
  if (from == -1)
    return std::make_tuple(-1, 0, length);

  // bounds check the end manually to handle exclusive ranges
  double value = range->to();
  if (!validate_int_value(vm, value, "Range end"))
    return std::make_tuple(-1, 0, length);

  // negative indices count from the end
  if (value < 0)
    value = length + value;

  // convert the exclusive range to an inclusive one
  if (!range->is_inclusive()) {
    if (value == from)
      return std::make_tuple(from, 0, 0);

    // shift the endpoint to make it inclusive, handling both increasing and
    // decreasing ranges
    value += value >= from ? -1 : 1;
  }

  // check bounds
  if (value < 0 || value >= length) {
    vm.fiber()->set_error(
        StringObject::make_string(vm, "`Range end` out of bounds"));
    return std::make_tuple(-1, 0, length);
  }

  int to = Xt::as_type<int>(value);
  int count = std::abs(from - to) + 1;
  return std::make_tuple(from, (from < to ? 1 : -1), count);
}

}
