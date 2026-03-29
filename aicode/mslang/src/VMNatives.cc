// Copyright (c) 2026 ASMlover. All rights reserved.
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
#include <cstring>
#include <fstream>
#include <iostream>
#include <format>
#include <random>
#include <sstream>
#include "VM.hh"

namespace ms {

void VM::register_natives() noexcept {
  define_native("clock", [](VM& vm, int, Value*) -> Value {
    using namespace std::chrono;
    auto now = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(now.time_since_epoch());
    return Value(static_cast<double>(duration.count()) / 1000000.0);
  });

  define_native("type", [](VM& vm, int arg_count, Value* args) -> Value {
    if (arg_count != 1) {
      vm.runtime_error("type() takes exactly 1 argument.");
      return Value();
    }
    Value& val = args[0];
    cstr_t type_str = "nil";
    if (val.is_boolean()) {
      type_str = "bool";
    } else if (val.is_integer()) {
      type_str = "int";
    } else if (val.is_double()) {
      type_str = "float";
    } else if (val.is_object()) {
      switch (val.as_object()->type()) {
      case ObjectType::OBJ_STRING:       type_str = "string"; break;
      case ObjectType::OBJ_FUNCTION:
      case ObjectType::OBJ_CLOSURE:
      case ObjectType::OBJ_NATIVE:
      case ObjectType::OBJ_BOUND_METHOD: type_str = "function"; break;
      case ObjectType::OBJ_CLASS:        type_str = "class"; break;
      case ObjectType::OBJ_INSTANCE:     type_str = "instance"; break;
      case ObjectType::OBJ_LIST:         type_str = "list"; break;
      case ObjectType::OBJ_MAP:              type_str = "map"; break;
      case ObjectType::OBJ_STRING_BUILDER: type_str = "stringbuilder"; break;
      case ObjectType::OBJ_TUPLE:           type_str = "tuple"; break;
      case ObjectType::OBJ_MODULE:          type_str = "module"; break;
      case ObjectType::OBJ_FILE:            type_str = "file"; break;
      case ObjectType::OBJ_COROUTINE:       type_str = "coroutine"; break;
      default:                              type_str = "object"; break;
      }
    }
    return Value(static_cast<Object*>(vm.copy_string(type_str, std::strlen(type_str))));
  });

  define_native("strlen", [](VM& vm, int arg_count, Value* args) -> Value {
    if (arg_count != 1) {
      vm.runtime_error("strlen() takes exactly 1 argument.");
      return Value();
    }
    if (!args[0].is_string()) {
      vm.runtime_error("strlen() argument must be a string.");
      return Value();
    }
    return Value(static_cast<double>(as_string(args[0])->value().length()));
  });

  define_native("str", [](VM& vm, int arg_count, Value* args) -> Value {
    if (arg_count != 1) {
      vm.runtime_error("str() takes exactly 1 argument.");
      return Value();
    }
    if (args[0].is_string()) return args[0];
    str_t s = args[0].stringify();
    return Value(static_cast<Object*>(vm.copy_string(s.data(), s.length())));
  });

  define_native("num", [](VM& vm, int arg_count, Value* args) -> Value {
    if (arg_count != 1) {
      vm.runtime_error("num() takes exactly 1 argument.");
      return Value();
    }
    if (!args[0].is_string()) {
      vm.runtime_error("num() argument must be a string.");
      return Value();
    }
    str_t s{as_string(args[0])->value()};
    sz_t pos = 0;
    double result;
    try {
      result = std::stod(s, &pos);
    } catch (...) {
      vm.runtime_error(std::format("Could not convert '{}' to a number.", s));
      return Value();
    }
    if (pos != s.length()) {
      vm.runtime_error(std::format("Could not convert '{}' to a number.", s));
      return Value();
    }
    return Value(result);
  });

  define_native("input", [](VM& vm, int arg_count, Value* args) -> Value {
    if (arg_count > 1) {
      vm.runtime_error("input() takes 0 or 1 arguments.");
      return Value();
    }
    if (arg_count == 1) {
      if (!args[0].is_string()) {
        vm.runtime_error("input() prompt must be a string.");
        return Value();
      }
      std::cout << as_string(args[0])->value();
      std::cout.flush();
    }
    str_t line;
    if (!std::getline(std::cin, line)) {
      return Value(); // nil on EOF
    }
    return Value(static_cast<Object*>(vm.copy_string(line.data(), line.length())));
  });

  // --- Math natives ---

  define_native("abs", [](VM& vm, int arg_count, Value* args) -> Value {
    if (arg_count != 1 || !args[0].is_number()) {
      vm.runtime_error("abs() takes exactly 1 number argument.");
      return Value();
    }
    return Value(std::abs(args[0].as_number()));
  });

  define_native("ceil", [](VM& vm, int arg_count, Value* args) -> Value {
    if (arg_count != 1 || !args[0].is_number()) {
      vm.runtime_error("ceil() takes exactly 1 number argument.");
      return Value();
    }
    return Value(std::ceil(args[0].as_number()));
  });

  define_native("floor", [](VM& vm, int arg_count, Value* args) -> Value {
    if (arg_count != 1 || !args[0].is_number()) {
      vm.runtime_error("floor() takes exactly 1 number argument.");
      return Value();
    }
    return Value(std::floor(args[0].as_number()));
  });

  define_native("round", [](VM& vm, int arg_count, Value* args) -> Value {
    if (arg_count != 1 || !args[0].is_number()) {
      vm.runtime_error("round() takes exactly 1 number argument.");
      return Value();
    }
    return Value(std::round(args[0].as_number()));
  });

  define_native("sqrt", [](VM& vm, int arg_count, Value* args) -> Value {
    if (arg_count != 1 || !args[0].is_number()) {
      vm.runtime_error("sqrt() takes exactly 1 number argument.");
      return Value();
    }
    double x = args[0].as_number();
    if (x < 0) {
      vm.runtime_error("sqrt() argument must be non-negative.");
      return Value();
    }
    return Value(std::sqrt(x));
  });

  define_native("pow", [](VM& vm, int arg_count, Value* args) -> Value {
    if (arg_count != 2 || !args[0].is_number() || !args[1].is_number()) {
      vm.runtime_error("pow() takes exactly 2 number arguments.");
      return Value();
    }
    return Value(std::pow(args[0].as_number(), args[1].as_number()));
  });

  define_native("min", [](VM& vm, int arg_count, Value* args) -> Value {
    if (arg_count != 2 || !args[0].is_number() || !args[1].is_number()) {
      vm.runtime_error("min() takes exactly 2 number arguments.");
      return Value();
    }
    return Value(std::min(args[0].as_number(), args[1].as_number()));
  });

  define_native("max", [](VM& vm, int arg_count, Value* args) -> Value {
    if (arg_count != 2 || !args[0].is_number() || !args[1].is_number()) {
      vm.runtime_error("max() takes exactly 2 number arguments.");
      return Value();
    }
    return Value(std::max(args[0].as_number(), args[1].as_number()));
  });

  define_native("random", [](VM& vm, int arg_count, Value*) -> Value {
    if (arg_count != 0) return Value();
    static std::mt19937 gen{std::random_device{}()};
    static std::uniform_real_distribution<double> dist(0.0, 1.0);
    return Value(dist(gen));
  });

  // --- Conversion natives ---

  define_native("int", [](VM& vm, int arg_count, Value* args) -> Value {
    if (arg_count != 1) {
      vm.runtime_error("int() takes exactly 1 argument.");
      return Value();
    }
    if (args[0].is_integer()) {
      return args[0]; // already integer
    }
    if (args[0].is_double()) {
      return Value(static_cast<i64_t>(args[0].as_number()));
    }
    if (args[0].is_boolean()) {
      return Value(static_cast<i64_t>(args[0].as_boolean() ? 1 : 0));
    }
    if (args[0].is_string()) {
      str_t s{as_string(args[0])->value()};
      try {
        sz_t pos = 0;
        long long result = std::stoll(s, &pos);
        if (pos != s.length()) {
          vm.runtime_error(std::format("Could not convert '{}' to an integer.", s));
          return Value();
        }
        return Value(static_cast<i64_t>(result));
      } catch (...) {
        vm.runtime_error(std::format("Could not convert '{}' to an integer.", s));
        return Value();
      }
    }
    vm.runtime_error("int() argument must be a number, bool, or string.");
    return Value();
  });

  define_native("bool", [](VM& vm, int arg_count, Value* args) -> Value {
    if (arg_count != 1) {
      vm.runtime_error("bool() takes exactly 1 argument.");
      return Value();
    }
    return Value(args[0].is_truthy());
  });

  // --- I/O natives ---

  define_native("read_file", [](VM& vm, int arg_count, Value* args) -> Value {
    if (arg_count != 1 || !args[0].is_string()) {
      vm.runtime_error("read_file() takes exactly 1 string argument.");
      return Value();
    }
    str_t path{as_string(args[0])->value()};
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file) {
      vm.runtime_error(std::format("Could not open file '{}'.", path));
      return Value();
    }
    std::ostringstream oss;
    oss << file.rdbuf();
    str_t content = oss.str();
    return Value(static_cast<Object*>(vm.copy_string(content.data(), content.length())));
  });

  define_native("write_file", [](VM& vm, int arg_count, Value* args) -> Value {
    if (arg_count != 2 || !args[0].is_string() || !args[1].is_string()) {
      vm.runtime_error("write_file() takes exactly 2 string arguments (path, data).");
      return Value();
    }
    str_t path{as_string(args[0])->value()};
    strv_t data = as_string(args[1])->value();
    std::ofstream file(path, std::ios::out | std::ios::binary);
    if (!file) {
      vm.runtime_error(std::format("Could not open file '{}' for writing.", path));
      return Value();
    }
    file.write(data.data(), static_cast<std::streamsize>(data.length()));
    return Value();
  });

  // --- Assert native ---

  define_native("assert", [](VM& vm, int arg_count, Value* args) -> Value {
    if (arg_count < 1 || arg_count > 2) {
      vm.runtime_error("assert() takes 1 or 2 arguments.");
      return Value();
    }
    if (!args[0].is_truthy()) {
      if (arg_count == 2 && args[1].is_string()) {
        vm.runtime_error(std::format("Assertion failed: {}.", as_string(args[1])->value()));
      } else {
        vm.runtime_error("Assertion failed.");
      }
      return Value();
    }
    return Value(true);
  });

  // --- range() native ---

  define_native("range", [](VM& vm, int arg_count, Value* args) -> Value {
    if (arg_count < 1 || arg_count > 3) {
      vm.runtime_error("range() takes 1 to 3 arguments.");
      return Value();
    }
    for (int i = 0; i < arg_count; i++) {
      if (!args[i].is_number()) {
        vm.runtime_error("range() arguments must be numbers.");
        return Value();
      }
    }

    double start = 0, stop = 0, step = 1;
    if (arg_count == 1) {
      stop = args[0].as_number();
    } else if (arg_count == 2) {
      start = args[0].as_number();
      stop = args[1].as_number();
    } else {
      start = args[0].as_number();
      stop = args[1].as_number();
      step = args[2].as_number();
    }

    if (step == 0) {
      vm.runtime_error("range() step argument must not be zero.");
      return Value();
    }

    ObjList* list = vm.allocate<ObjList>();
    // Push to stack immediately so GC can find it
    vm.push(Value(static_cast<Object*>(list)));

    if (step > 0) {
      for (double i = start; i < stop; i += step)
        list->elements().push_back(Value(i));
    } else {
      for (double i = start; i > stop; i += step)
        list->elements().push_back(Value(i));
    }

    vm.pop(); // pop the list we pushed for GC safety
    return Value(static_cast<Object*>(list));
  });

  // --- StringBuilder native ---

  define_native("weak_ref", [](VM& vm, int arg_count, Value* args) -> Value {
    if (arg_count != 1) {
      vm.runtime_error("weak_ref() takes exactly 1 argument.");
      return Value();
    }
    if (!args[0].is_object()) {
      vm.runtime_error("weak_ref() argument must be an object.");
      return Value();
    }
    ObjWeakRef* ref = vm.allocate<ObjWeakRef>(args[0].as_object());
    vm.weak_refs().push_back(ref);
    return Value(static_cast<Object*>(ref));
  });

  define_native("StringBuilder", [](VM& vm, int arg_count, Value* args) -> Value {
    ObjStringBuilder* sb = vm.allocate<ObjStringBuilder>();
    if (arg_count == 1) {
      str_t s = args[0].stringify();
      sb->append(s);
    } else if (arg_count > 1) {
      vm.runtime_error("StringBuilder() takes 0 or 1 arguments.");
      return Value();
    }
    return Value(static_cast<Object*>(sb));
  });
}

} // namespace ms
