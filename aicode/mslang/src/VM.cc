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
#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdarg>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <format>
#include <random>
#include <sstream>
#include <unordered_set>
#include "VM.hh"
#include "Compiler.hh"
#include "Memory.hh"
#include "Debug.hh"
#include "Logger.hh"

namespace ms {

// --- VM implementation ---

VM::VM() noexcept {
  reset_stack();
  init_string_ = copy_string("init", 4);

  // Register native functions
  define_native("clock", [](int, Value*) -> Value {
    using namespace std::chrono;
    auto now = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(now.time_since_epoch());
    return Value(static_cast<double>(duration.count()) / 1000000.0);
  });

  define_native("type", [this](int arg_count, Value* args) -> Value {
    if (arg_count != 1) {
      runtime_error("type() takes exactly 1 argument.");
      return Value();
    }
    Value& val = args[0];
    cstr_t type_str = "nil";
    if (val.is_boolean()) {
      type_str = "bool";
    } else if (val.is_number()) {
      type_str = "number";
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
      case ObjectType::OBJ_MAP:          type_str = "map"; break;
      default:                           type_str = "object"; break;
      }
    }
    return Value(static_cast<Object*>(copy_string(type_str, std::strlen(type_str))));
  });

  define_native("strlen", [this](int arg_count, Value* args) -> Value {
    if (arg_count != 1) {
      runtime_error("strlen() takes exactly 1 argument.");
      return Value();
    }
    if (!args[0].is_string()) {
      runtime_error("strlen() argument must be a string.");
      return Value();
    }
    return Value(static_cast<double>(as_string(args[0])->value().length()));
  });

  define_native("str", [this](int arg_count, Value* args) -> Value {
    if (arg_count != 1) {
      runtime_error("str() takes exactly 1 argument.");
      return Value();
    }
    str_t s = args[0].stringify();
    return Value(static_cast<Object*>(copy_string(s.data(), s.length())));
  });

  define_native("num", [this](int arg_count, Value* args) -> Value {
    if (arg_count != 1) {
      runtime_error("num() takes exactly 1 argument.");
      return Value();
    }
    if (!args[0].is_string()) {
      runtime_error("num() argument must be a string.");
      return Value();
    }
    const str_t& s = as_string(args[0])->value();
    sz_t pos = 0;
    double result;
    try {
      result = std::stod(s, &pos);
    } catch (...) {
      runtime_error(std::format("Could not convert '{}' to a number.", s));
      return Value();
    }
    if (pos != s.length()) {
      runtime_error(std::format("Could not convert '{}' to a number.", s));
      return Value();
    }
    return Value(result);
  });

  define_native("input", [this](int arg_count, Value* args) -> Value {
    if (arg_count > 1) {
      runtime_error("input() takes 0 or 1 arguments.");
      return Value();
    }
    if (arg_count == 1) {
      if (!args[0].is_string()) {
        runtime_error("input() prompt must be a string.");
        return Value();
      }
      std::cout << as_string(args[0])->value();
      std::cout.flush();
    }
    str_t line;
    if (!std::getline(std::cin, line)) {
      return Value(); // nil on EOF
    }
    return Value(static_cast<Object*>(copy_string(line.data(), line.length())));
  });

  // --- Math natives ---

  define_native("abs", [this](int arg_count, Value* args) -> Value {
    if (arg_count != 1 || !args[0].is_number()) {
      runtime_error("abs() takes exactly 1 number argument.");
      return Value();
    }
    return Value(std::abs(args[0].as_number()));
  });

  define_native("ceil", [this](int arg_count, Value* args) -> Value {
    if (arg_count != 1 || !args[0].is_number()) {
      runtime_error("ceil() takes exactly 1 number argument.");
      return Value();
    }
    return Value(std::ceil(args[0].as_number()));
  });

  define_native("floor", [this](int arg_count, Value* args) -> Value {
    if (arg_count != 1 || !args[0].is_number()) {
      runtime_error("floor() takes exactly 1 number argument.");
      return Value();
    }
    return Value(std::floor(args[0].as_number()));
  });

  define_native("round", [this](int arg_count, Value* args) -> Value {
    if (arg_count != 1 || !args[0].is_number()) {
      runtime_error("round() takes exactly 1 number argument.");
      return Value();
    }
    return Value(std::round(args[0].as_number()));
  });

  define_native("sqrt", [this](int arg_count, Value* args) -> Value {
    if (arg_count != 1 || !args[0].is_number()) {
      runtime_error("sqrt() takes exactly 1 number argument.");
      return Value();
    }
    double x = args[0].as_number();
    if (x < 0) {
      runtime_error("sqrt() argument must be non-negative.");
      return Value();
    }
    return Value(std::sqrt(x));
  });

  define_native("pow", [this](int arg_count, Value* args) -> Value {
    if (arg_count != 2 || !args[0].is_number() || !args[1].is_number()) {
      runtime_error("pow() takes exactly 2 number arguments.");
      return Value();
    }
    return Value(std::pow(args[0].as_number(), args[1].as_number()));
  });

  define_native("min", [this](int arg_count, Value* args) -> Value {
    if (arg_count != 2 || !args[0].is_number() || !args[1].is_number()) {
      runtime_error("min() takes exactly 2 number arguments.");
      return Value();
    }
    return Value(std::min(args[0].as_number(), args[1].as_number()));
  });

  define_native("max", [this](int arg_count, Value* args) -> Value {
    if (arg_count != 2 || !args[0].is_number() || !args[1].is_number()) {
      runtime_error("max() takes exactly 2 number arguments.");
      return Value();
    }
    return Value(std::max(args[0].as_number(), args[1].as_number()));
  });

  define_native("random", [](int arg_count, Value*) -> Value {
    if (arg_count != 0) return Value();
    static std::mt19937 gen{std::random_device{}()};
    static std::uniform_real_distribution<double> dist(0.0, 1.0);
    return Value(dist(gen));
  });

  // --- Conversion natives ---

  define_native("int", [this](int arg_count, Value* args) -> Value {
    if (arg_count != 1) {
      runtime_error("int() takes exactly 1 argument.");
      return Value();
    }
    if (args[0].is_number()) {
      return Value(static_cast<double>(static_cast<long long>(args[0].as_number())));
    }
    if (args[0].is_boolean()) {
      return Value(args[0].as_boolean() ? 1.0 : 0.0);
    }
    if (args[0].is_string()) {
      const str_t& s = as_string(args[0])->value();
      try {
        sz_t pos = 0;
        long long result = std::stoll(s, &pos);
        if (pos != s.length()) {
          runtime_error(std::format("Could not convert '{}' to an integer.", s));
          return Value();
        }
        return Value(static_cast<double>(result));
      } catch (...) {
        runtime_error(std::format("Could not convert '{}' to an integer.", s));
        return Value();
      }
    }
    runtime_error("int() argument must be a number, bool, or string.");
    return Value();
  });

  define_native("bool", [this](int arg_count, Value* args) -> Value {
    if (arg_count != 1) {
      runtime_error("bool() takes exactly 1 argument.");
      return Value();
    }
    return Value(args[0].is_truthy());
  });

  // --- I/O natives ---

  define_native("read_file", [this](int arg_count, Value* args) -> Value {
    if (arg_count != 1 || !args[0].is_string()) {
      runtime_error("read_file() takes exactly 1 string argument.");
      return Value();
    }
    const str_t& path = as_string(args[0])->value();
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file) {
      runtime_error(std::format("Could not open file '{}'.", path));
      return Value();
    }
    std::ostringstream oss;
    oss << file.rdbuf();
    str_t content = oss.str();
    return Value(static_cast<Object*>(copy_string(content.data(), content.length())));
  });

  define_native("write_file", [this](int arg_count, Value* args) -> Value {
    if (arg_count != 2 || !args[0].is_string() || !args[1].is_string()) {
      runtime_error("write_file() takes exactly 2 string arguments (path, data).");
      return Value();
    }
    const str_t& path = as_string(args[0])->value();
    const str_t& data = as_string(args[1])->value();
    std::ofstream file(path, std::ios::out | std::ios::binary);
    if (!file) {
      runtime_error(std::format("Could not open file '{}' for writing.", path));
      return Value();
    }
    file.write(data.data(), static_cast<std::streamsize>(data.length()));
    return Value();
  });

  // --- Assert native ---

  define_native("assert", [this](int arg_count, Value* args) -> Value {
    if (arg_count < 1 || arg_count > 2) {
      runtime_error("assert() takes 1 or 2 arguments.");
      return Value();
    }
    if (!args[0].is_truthy()) {
      if (arg_count == 2 && args[1].is_string()) {
        runtime_error(std::format("Assertion failed: {}.", as_string(args[1])->value()));
      } else {
        runtime_error("Assertion failed.");
      }
      return Value();
    }
    return Value(true);
  });

  // --- range() native ---

  define_native("range", [this](int arg_count, Value* args) -> Value {
    if (arg_count < 1 || arg_count > 3) {
      runtime_error("range() takes 1 to 3 arguments.");
      return Value();
    }
    for (int i = 0; i < arg_count; i++) {
      if (!args[i].is_number()) {
        runtime_error("range() arguments must be numbers.");
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
      runtime_error("range() step argument must not be zero.");
      return Value();
    }

    ObjList* list = allocate<ObjList>();
    // Push to stack immediately so GC can find it
    push(Value(static_cast<Object*>(list)));

    if (step > 0) {
      for (double i = start; i < stop; i += step)
        list->elements().push_back(Value(i));
    } else {
      for (double i = start; i > stop; i += step)
        list->elements().push_back(Value(i));
    }

    pop(); // pop the list we pushed for GC safety
    return Value(static_cast<Object*>(list));
  });
}

VM::~VM() noexcept {
  init_string_ = nullptr;
  free_objects();
}

void VM::reset_stack() noexcept {
  stack_top_ = stack_.data();
  frame_count_ = 0;
  open_upvalues_ = nullptr;
}

void VM::push(Value value) noexcept {
  if (stack_top_ >= stack_.data() + stack_.size()) {
    runtime_error("Stack overflow.");
    std::exit(70);
  }
  *stack_top_ = value;
  stack_top_++;
}

Value VM::pop() noexcept {
  stack_top_--;
  return *stack_top_;
}

Value VM::peek(int distance) const noexcept {
  return stack_top_[-1 - distance];
}

void VM::runtime_error(strv_t message) noexcept {
  std::cerr << message << std::endl;

  for (int i = frame_count_ - 1; i >= 0; i--) {
    CallFrame& frame = frames_[i];
    ObjFunction* function = frame.closure->function();
    sz_t instruction = static_cast<sz_t>(frame.ip - function->chunk().code_data() - 1);
    std::cerr << std::format("[line {}] in ",
        function->chunk().line_at(instruction));
    if (function->name() == nullptr) {
      std::cerr << "script" << std::endl;
    } else {
      std::cerr << function->name()->value() << "()" << std::endl;
    }
  }

  reset_stack();
}

void VM::define_native(strv_t name, NativeFn function) noexcept {
  push(Value(static_cast<Object*>(copy_string(name.data(), name.length()))));
  push(Value(static_cast<Object*>(allocate<ObjNative>(std::move(function)))));
  globals_.set(as_string(stack_[0]), stack_[1]);
  pop();
  pop();
}

ObjString* VM::copy_string(cstr_t chars, sz_t length) noexcept {
  u32_t hash = ObjString::hash_string(chars, length);
  ObjString* interned = strings_.find_string(chars, length, hash);
  if (interned != nullptr) return interned;

  auto* string = allocate<ObjString>(str_t(chars, length), hash);
  push(Value(static_cast<Object*>(string)));
  strings_.set(string, Value());
  pop();
  return string;
}

ObjString* VM::take_string(str_t value) noexcept {
  u32_t hash = ObjString::hash_string(value.c_str(), value.length());
  ObjString* interned = strings_.find_string(value.c_str(), value.length(), hash);
  if (interned != nullptr) return interned;

  auto* string = allocate<ObjString>(std::move(value), hash);
  push(Value(static_cast<Object*>(string)));
  strings_.set(string, Value());
  pop();
  return string;
}

template <typename T, typename... Args>
T* VM::allocate(Args&&... args) noexcept {
  bytes_allocated_ += sizeof(T);

#ifdef MAPLE_DEBUG_STRESS_GC
  collect_garbage();
#else
  if (bytes_allocated_ > next_gc_) {
    collect_garbage();
  }
#endif

  auto* object = new T(std::forward<Args>(args)...);
  object->set_next(objects_);
  objects_ = object;

#ifdef MAPLE_DEBUG_LOG_GC
  auto& logger = Logger::get_instance();
  logger.debug("GC", "allocate {} for {} ({})",
      sizeof(T), static_cast<void*>(object), object->stringify());
#endif

  return object;
}

// Explicit instantiations for types allocated from outside VM.cc (e.g. Compiler.cc)
template ObjFunction* VM::allocate<ObjFunction>();

void VM::mark_roots() noexcept {
  // Mark the stack
  for (Value* slot = stack_.data(); slot < stack_top_; slot++) {
    mark_value(*slot);
  }

  // Mark call frames
  for (int i = 0; i < frame_count_; i++) {
    mark_object(frames_[i].closure);
  }

  // Mark open upvalues
  for (ObjUpvalue* upvalue = open_upvalues_;
       upvalue != nullptr; upvalue = upvalue->next_upvalue()) {
    mark_object(upvalue);
  }

  // Mark globals
  globals_.mark_table();

  // Mark compiler roots
  mark_compiler_roots();

  // Mark init string
  mark_object(init_string_);

  // Mark pending import modules
  for (auto& pending : pending_imports_) {
    mark_object(pending.module);
    for (auto& req : pending.from_imports) {
      mark_object(req.name);
      if (req.alias) mark_object(req.alias);
    }
  }
}

void VM::push_gray(Object* object) noexcept {
  gray_stack_.push_back(object);
}

void VM::trace_references() noexcept {
  while (!gray_stack_.empty()) {
    Object* object = gray_stack_.back();
    gray_stack_.pop_back();

#ifdef MAPLE_DEBUG_LOG_GC
    auto& logger = Logger::get_instance();
    logger.debug("GC", "blacken {} ({})",
        static_cast<void*>(object), object->stringify());
#endif

    object->trace_references();
  }
}

void VM::sweep() noexcept {
  Object* previous = nullptr;
  Object* object = objects_;

  while (object != nullptr) {
    if (object->is_marked()) {
      object->set_marked(false);
      previous = object;
      object = object->next();
    } else {
      Object* unreached = object;
      object = object->next();
      if (previous != nullptr) {
        previous->set_next(object);
      } else {
        objects_ = object;
      }

#ifdef MAPLE_DEBUG_LOG_GC
      auto& logger = Logger::get_instance();
      logger.debug("GC", "free {} ({})",
          static_cast<void*>(unreached), unreached->stringify());
#endif

      bytes_allocated_ -= unreached->size();
      delete unreached;
    }
  }
}

void VM::collect_garbage() noexcept {
#ifdef MAPLE_DEBUG_LOG_GC
  auto& logger = Logger::get_instance();
  logger.info("GC", "-- gc begin");
  sz_t before = bytes_allocated_;
#endif

  mark_roots();
  trace_references();

  // Remove interned strings that are unmarked before sweep
  strings_.remove_white();

  sweep();

  next_gc_ = bytes_allocated_ * kGC_HEAP_GROW;

#ifdef MAPLE_DEBUG_LOG_GC
  logger.info("GC", "-- gc end (collected {} bytes, from {} to {}, next at {})",
      before - bytes_allocated_, before, bytes_allocated_, next_gc_);
#endif
}

void VM::free_objects() noexcept {
  Object* object = objects_;
  while (object != nullptr) {
    Object* next = object->next();
    delete object;
    object = next;
  }
  objects_ = nullptr;
}

void VM::concatenate() noexcept {
  ObjString* b = as_string(peek(0));
  ObjString* a = as_string(peek(1));

  str_t result = str_t(a->value()) + str_t(b->value());
  ObjString* str = take_string(std::move(result));

  pop();
  pop();
  push(Value(static_cast<Object*>(str)));
}

bool VM::call(ObjClosure* closure, int arg_count) noexcept {
  if (arg_count != closure->function()->arity()) {
    runtime_error(std::format("Expected {} arguments but got {}.",
        closure->function()->arity(), arg_count));
    return false;
  }

  if (frame_count_ == static_cast<int>(kFRAMES_MAX)) {
    runtime_error("Stack overflow.");
    return false;
  }

  CallFrame& frame = frames_[frame_count_++];
  frame.closure = closure;
  frame.ip = closure->function()->chunk().code_data();
  frame.slots = stack_top_ - arg_count - 1;
  return true;
}

bool VM::call_value(Value callee, int arg_count) noexcept {
  if (callee.is_object()) {
    switch (callee.as_object()->type()) {
    case ObjectType::OBJ_CLOSURE:
      return call(as_closure(callee), arg_count);

    case ObjectType::OBJ_NATIVE: {
      NativeFn& native = as_native(callee)->function();
      Value result = native(arg_count, stack_top_ - arg_count);
      stack_top_ -= arg_count + 1;
      push(result);
      return true;
    }

    case ObjectType::OBJ_CLASS: {
      ObjClass* klass = as_class(callee);
      stack_top_[-arg_count - 1] = Value(static_cast<Object*>(allocate<ObjInstance>(klass)));

      // Call initializer if present
      Value init_method;
      if (klass->methods().get(init_string_, &init_method)) {
        return call(as_closure(init_method), arg_count);
      } else if (arg_count != 0) {
        runtime_error(std::format("Expected 0 arguments but got {}.", arg_count));
        return false;
      }
      return true;
    }

    case ObjectType::OBJ_BOUND_METHOD: {
      ObjBoundMethod* bound = as_bound_method(callee);
      stack_top_[-arg_count - 1] = bound->receiver();
      return call(bound->method(), arg_count);
    }

    default:
      break;
    }
  }

  runtime_error("Can only call functions and classes.");
  return false;
}

bool VM::invoke_from_class(ObjClass* klass, ObjString* name, int arg_count) noexcept {
  Value method;
  if (!klass->methods().get(name, &method)) {
    runtime_error(std::format("Undefined property '{}'.", name->value()));
    return false;
  }
  return call(as_closure(method), arg_count);
}

bool VM::invoke(ObjString* name, int arg_count) noexcept {
  Value receiver = peek(arg_count);

  // Handle module property access (e.g., math.add(1, 2))
  if (is_obj_type(receiver, ObjectType::OBJ_MODULE)) {
    ObjModule* module = as_module(receiver);
    Value export_val;
    if (!module->exports().get(name, &export_val)) {
      runtime_error(std::format("Undefined export '{}'.", name->value()));
      return false;
    }
    stack_top_[-arg_count - 1] = export_val;
    return call_value(export_val, arg_count);
  }

  // Handle static method calls (e.g., Math.max(3, 5))
  if (is_obj_type(receiver, ObjectType::OBJ_CLASS)) {
    ObjClass* klass = as_class(receiver);
    Value method;
    if (!klass->static_methods().get(name, &method)) {
      runtime_error(std::format("Undefined static method '{}'.", name->value()));
      return false;
    }
    stack_top_[-arg_count - 1] = method;
    return call_value(method, arg_count);
  }

  // Handle string method calls (e.g., s.len(), s.upper(), s.split(","))
  if (is_obj_type(receiver, ObjectType::OBJ_STRING)) {
    ObjString* str = as_string(receiver);
    const str_t& sv = str->value();
    strv_t method_name = name->value();

    if (method_name == "len") {
      if (arg_count != 0) {
        runtime_error("len() takes no arguments.");
        return false;
      }
      stack_top_[-1] = Value(static_cast<double>(sv.length()));
      return true;
    } else if (method_name == "slice") {
      if (arg_count != 2) {
        runtime_error("slice() takes exactly 2 arguments (start, end).");
        return false;
      }
      if (!peek(1).is_number() || !peek(0).is_number()) {
        runtime_error("slice() arguments must be numbers.");
        return false;
      }
      auto len = static_cast<i32_t>(sv.length());
      auto start = static_cast<i32_t>(peek(1).as_number());
      auto end = static_cast<i32_t>(peek(0).as_number());
      if (start < 0) start += len;
      if (end < 0) end += len;
      start = std::clamp(start, 0, len);
      end = std::clamp(end, start, len);
      str_t result = sv.substr(start, end - start);
      ObjString* res = copy_string(result.data(), result.length());
      stack_top_[-arg_count - 1] = Value(static_cast<Object*>(res));
      stack_top_ -= arg_count;
      return true;
    } else if (method_name == "find") {
      if (arg_count != 1) {
        runtime_error("find() takes exactly 1 argument.");
        return false;
      }
      if (!peek(0).is_string()) {
        runtime_error("find() argument must be a string.");
        return false;
      }
      const str_t& needle = as_string(peek(0))->value();
      auto pos = sv.find(needle);
      double result = (pos == str_t::npos) ? -1.0 : static_cast<double>(pos);
      stack_top_[-arg_count - 1] = Value(result);
      stack_top_ -= arg_count;
      return true;
    } else if (method_name == "replace") {
      if (arg_count != 2) {
        runtime_error("replace() takes exactly 2 arguments (old, new).");
        return false;
      }
      if (!peek(1).is_string() || !peek(0).is_string()) {
        runtime_error("replace() arguments must be strings.");
        return false;
      }
      const str_t& old_str = as_string(peek(1))->value();
      const str_t& new_str = as_string(peek(0))->value();
      str_t result = sv;
      auto pos = result.find(old_str);
      if (pos != str_t::npos) {
        result.replace(pos, old_str.length(), new_str);
      }
      ObjString* res = copy_string(result.data(), result.length());
      stack_top_[-arg_count - 1] = Value(static_cast<Object*>(res));
      stack_top_ -= arg_count;
      return true;
    } else if (method_name == "split") {
      if (arg_count != 1) {
        runtime_error("split() takes exactly 1 argument.");
        return false;
      }
      if (!peek(0).is_string()) {
        runtime_error("split() argument must be a string.");
        return false;
      }
      const str_t& delim = as_string(peek(0))->value();
      ObjList* list = allocate<ObjList>();
      // Push list to protect from GC during string allocations
      push(Value(static_cast<Object*>(list)));
      if (delim.empty()) {
        // Split into individual characters
        for (sz_t i = 0; i < sv.length(); ++i) {
          ObjString* ch = copy_string(sv.data() + i, 1);
          list->elements().push_back(Value(static_cast<Object*>(ch)));
        }
      } else {
        sz_t start = 0;
        sz_t pos;
        while ((pos = sv.find(delim, start)) != str_t::npos) {
          str_t part = sv.substr(start, pos - start);
          ObjString* s = copy_string(part.data(), part.length());
          list->elements().push_back(Value(static_cast<Object*>(s)));
          start = pos + delim.length();
        }
        str_t part = sv.substr(start);
        ObjString* s = copy_string(part.data(), part.length());
        list->elements().push_back(Value(static_cast<Object*>(s)));
      }
      pop(); // pop GC guard
      // Replace receiver + arg with result
      stack_top_[-arg_count - 1] = Value(static_cast<Object*>(list));
      stack_top_ -= arg_count;
      return true;
    } else if (method_name == "upper") {
      if (arg_count != 0) {
        runtime_error("upper() takes no arguments.");
        return false;
      }
      str_t result = sv;
      std::transform(result.begin(), result.end(), result.begin(),
        [](unsigned char c) { return std::toupper(c); });
      ObjString* res = copy_string(result.data(), result.length());
      stack_top_[-1] = Value(static_cast<Object*>(res));
      return true;
    } else if (method_name == "lower") {
      if (arg_count != 0) {
        runtime_error("lower() takes no arguments.");
        return false;
      }
      str_t result = sv;
      std::transform(result.begin(), result.end(), result.begin(),
        [](unsigned char c) { return std::tolower(c); });
      ObjString* res = copy_string(result.data(), result.length());
      stack_top_[-1] = Value(static_cast<Object*>(res));
      return true;
    } else if (method_name == "trim") {
      if (arg_count != 0) {
        runtime_error("trim() takes no arguments.");
        return false;
      }
      auto first = sv.find_first_not_of(" \t\n\r\f\v");
      if (first == str_t::npos) {
        ObjString* res = copy_string("", 0);
        stack_top_[-1] = Value(static_cast<Object*>(res));
      } else {
        auto last = sv.find_last_not_of(" \t\n\r\f\v");
        str_t result = sv.substr(first, last - first + 1);
        ObjString* res = copy_string(result.data(), result.length());
        stack_top_[-1] = Value(static_cast<Object*>(res));
      }
      return true;
    } else if (method_name == "starts_with") {
      if (arg_count != 1) {
        runtime_error("starts_with() takes exactly 1 argument.");
        return false;
      }
      if (!peek(0).is_string()) {
        runtime_error("starts_with() argument must be a string.");
        return false;
      }
      const str_t& prefix = as_string(peek(0))->value();
      bool result = sv.starts_with(prefix);
      stack_top_[-arg_count - 1] = Value(result);
      stack_top_ -= arg_count;
      return true;
    } else if (method_name == "ends_with") {
      if (arg_count != 1) {
        runtime_error("ends_with() takes exactly 1 argument.");
        return false;
      }
      if (!peek(0).is_string()) {
        runtime_error("ends_with() argument must be a string.");
        return false;
      }
      const str_t& suffix = as_string(peek(0))->value();
      bool result = sv.ends_with(suffix);
      stack_top_[-arg_count - 1] = Value(result);
      stack_top_ -= arg_count;
      return true;
    } else if (method_name == "contains") {
      if (arg_count != 1) {
        runtime_error("contains() takes exactly 1 argument.");
        return false;
      }
      if (!peek(0).is_string()) {
        runtime_error("contains() argument must be a string.");
        return false;
      }
      const str_t& needle = as_string(peek(0))->value();
      bool result = sv.find(needle) != str_t::npos;
      stack_top_[-arg_count - 1] = Value(result);
      stack_top_ -= arg_count;
      return true;
    }
    runtime_error(std::format("Undefined string method '{}'.", method_name));
    return false;
  }

  if (is_obj_type(receiver, ObjectType::OBJ_LIST)) {
    ObjList* list = as_list(receiver);
    strv_t method_name = name->value();
    if (method_name == "len") {
      if (arg_count != 0) {
        runtime_error("len() takes no arguments.");
        return false;
      }
      stack_top_[-1] = Value(static_cast<double>(list->len()));
      return true;
    } else if (method_name == "push") {
      if (arg_count != 1) {
        runtime_error("push() takes exactly 1 argument.");
        return false;
      }
      Value val = stack_top_[-1];
      list->elements().push_back(val);
      stack_top_ -= 2; // pop arg and receiver
      push(Value());   // return nil
      return true;
    } else if (method_name == "pop") {
      if (arg_count != 0) {
        runtime_error("pop() takes no arguments.");
        return false;
      }
      if (list->elements().empty()) {
        runtime_error("Cannot pop from an empty list.");
        return false;
      }
      Value val = list->elements().back();
      list->elements().pop_back();
      stack_top_[-1] = val;
      return true;
    }
    runtime_error(std::format("Undefined list method '{}'.", method_name));
    return false;
  }

  if (is_obj_type(receiver, ObjectType::OBJ_MAP)) {
    ObjMap* map = as_map(receiver);
    strv_t method_name = name->value();
    if (method_name == "len") {
      if (arg_count != 0) {
        runtime_error("len() takes no arguments.");
        return false;
      }
      stack_top_[-1] = Value(static_cast<double>(map->len()));
      return true;
    } else if (method_name == "keys") {
      if (arg_count != 0) {
        runtime_error("keys() takes no arguments.");
        return false;
      }
      ObjList* keys = allocate<ObjList>();
      for (auto& [k, v] : map->entries()) {
        keys->elements().push_back(k);
      }
      stack_top_[-1] = Value(static_cast<Object*>(keys));
      return true;
    } else if (method_name == "values") {
      if (arg_count != 0) {
        runtime_error("values() takes no arguments.");
        return false;
      }
      ObjList* vals = allocate<ObjList>();
      for (auto& [k, v] : map->entries()) {
        vals->elements().push_back(v);
      }
      stack_top_[-1] = Value(static_cast<Object*>(vals));
      return true;
    } else if (method_name == "has") {
      if (arg_count != 1) {
        runtime_error("has() takes exactly 1 argument.");
        return false;
      }
      Value key = stack_top_[-1];
      bool found = map->entries().find(key) != map->entries().end();
      stack_top_ -= 2; // pop arg and receiver
      push(Value(found));
      return true;
    } else if (method_name == "remove") {
      if (arg_count != 1) {
        runtime_error("remove() takes exactly 1 argument.");
        return false;
      }
      Value key = stack_top_[-1];
      map->entries().erase(key);
      stack_top_ -= 2; // pop arg and receiver
      push(Value()); // return nil
      return true;
    }
    runtime_error(std::format("Undefined map method '{}'.", method_name));
    return false;
  }

  if (!is_obj_type(receiver, ObjectType::OBJ_INSTANCE)) {
    runtime_error("Only instances have methods.");
    return false;
  }

  ObjInstance* instance = as_instance(receiver);

  // Check for field first (field shadowing method)
  Value field_val;
  if (instance->fields().get(name, &field_val)) {
    stack_top_[-arg_count - 1] = field_val;
    return call_value(field_val, arg_count);
  }

  return invoke_from_class(instance->klass(), name, arg_count);
}

void VM::bind_method(ObjClass* klass, ObjString* name) noexcept {
  Value method;
  if (!klass->methods().get(name, &method)) {
    runtime_error(std::format("Undefined property '{}'.", name->value()));
    return;
  }

  ObjBoundMethod* bound = allocate<ObjBoundMethod>(peek(0), as_closure(method));
  pop();
  push(Value(static_cast<Object*>(bound)));
}

ObjUpvalue* VM::capture_upvalue(Value* local) noexcept {
  ObjUpvalue* prev_upvalue = nullptr;
  ObjUpvalue* upvalue = open_upvalues_;

  while (upvalue != nullptr && upvalue->location() > local) {
    prev_upvalue = upvalue;
    upvalue = upvalue->next_upvalue();
  }

  if (upvalue != nullptr && upvalue->location() == local) {
    return upvalue;
  }

  ObjUpvalue* created_upvalue = allocate<ObjUpvalue>(local);
  created_upvalue->set_next_upvalue(upvalue);

  if (prev_upvalue == nullptr) {
    open_upvalues_ = created_upvalue;
  } else {
    prev_upvalue->set_next_upvalue(created_upvalue);
  }

  return created_upvalue;
}

void VM::close_upvalues(Value* last) noexcept {
  while (open_upvalues_ != nullptr &&
         open_upvalues_->location() >= last) {
    ObjUpvalue* upvalue = open_upvalues_;
    upvalue->closed() = *upvalue->location();
    upvalue->set_location(&upvalue->closed());
    open_upvalues_ = upvalue->next_upvalue();
  }
}

void VM::import_module(ObjString* path) noexcept {
  // Resolve path relative to current script
  str_t resolved = ModuleLoader::resolve_path(path->value(), current_script_path_);

  // Check cache
  auto it = modules_.find(resolved);
  if (it != modules_.end()) {
    return; // already imported
  }

  // Read source file via ModuleLoader
  auto source_opt = ModuleLoader::read_source(resolved);
  if (!source_opt.has_value()) {
    runtime_error(std::format("Could not open module '{}'.", resolved));
    return;
  }

  // Compile module
  ObjFunction* function = compile(strv_t(*source_opt));
  if (function == nullptr) {
    runtime_error(std::format("Could not compile module '{}'.", resolved));
    return;
  }

  // Snapshot current global keys before module execution
  std::vector<ObjString*> pre_keys;
  for (auto& entry : globals_.entries()) {
    if (entry.key != nullptr) {
      pre_keys.push_back(entry.key);
    }
  }

  // Create module object and cache it (use resolved path as key)
  ObjString* resolved_str = copy_string(resolved.c_str(), resolved.size());
  ObjModule* module = allocate<ObjModule>(resolved_str);
  modules_[resolved] = module;

  // Save current script path and switch to module path
  str_t previous_path = current_script_path_;
  current_script_path_ = resolved;

  // Execute module
  ObjClosure* closure = allocate<ObjClosure>(function);
  push(Value(static_cast<Object*>(closure)));
  call(closure, 0);

  // Track this import so OP_RETURN can collect exports
  pending_imports_.push_back({frame_count_ - 1, module, std::move(pre_keys), {}, std::move(previous_path)});
}

InterpretResult VM::interpret(strv_t source) noexcept {
  return interpret(source, "");
}

InterpretResult VM::interpret(strv_t source, strv_t script_path) noexcept {
  current_script_path_ = str_t(script_path);

  ObjFunction* function = compile(source);
  if (function == nullptr) return InterpretResult::INTERPRET_COMPILE_ERROR;

  push(Value(static_cast<Object*>(function)));
  ObjClosure* closure = allocate<ObjClosure>(function);
  pop();
  push(Value(static_cast<Object*>(closure)));
  call(closure, 0);

  return run();
}

InterpretResult VM::run() noexcept {
  CallFrame* frame = &frames_[frame_count_ - 1];

#define READ_BYTE() (*frame->ip++)
#define READ_SHORT() \
    (frame->ip += 2, \
     static_cast<u16_t>((frame->ip[-2] << 8) | frame->ip[-1]))
#define READ_CONSTANT() \
    (frame->closure->function()->chunk().constant_at(READ_BYTE()))
#define READ_STRING() as_string(READ_CONSTANT())
#define BINARY_OP(value_type, op) \
    do { \
      if (!peek(0).is_number() || !peek(1).is_number()) { \
        runtime_error("Operands must be numbers."); \
        return InterpretResult::INTERPRET_RUNTIME_ERROR; \
      } \
      double b = pop().as_number(); \
      double a = pop().as_number(); \
      push(value_type(a op b)); \
    } while (false)

  using u16_t = std::uint16_t;

#ifdef MAPLE_DEBUG_TRACE
#define TRACE_INSTRUCTION() \
    do { \
      std::cout << "          "; \
      for (Value* slot = frame->slots; slot < stack_top_; slot++) { \
        std::cout << "[ " << slot->stringify() << " ]"; \
      } \
      std::cout << std::endl; \
      disassemble_instruction(frame->closure->function()->chunk(), \
          static_cast<sz_t>(frame->ip - frame->closure->function()->chunk().code_data())); \
    } while (false)
#else
#define TRACE_INSTRUCTION() ((void)0)
#endif

#ifdef MAPLE_GNUC
  // Computed goto (threaded dispatch) — GCC/Clang only.
  // Each opcode jumps directly to the next handler via a label address table,
  // eliminating the overhead of the switch indirect branch.
  static void* dispatch_table[] = {
    &&op_OP_CONSTANT,      &&op_OP_CONSTANT_LONG,
    &&op_OP_NIL,           &&op_OP_TRUE,          &&op_OP_FALSE,
    &&op_OP_POP,
    &&op_OP_GET_LOCAL,     &&op_OP_SET_LOCAL,
    &&op_OP_GET_GLOBAL,    &&op_OP_DEFINE_GLOBAL, &&op_OP_SET_GLOBAL,
    &&op_OP_GET_UPVALUE,   &&op_OP_SET_UPVALUE,
    &&op_OP_GET_PROPERTY,  &&op_OP_SET_PROPERTY,
    &&op_OP_GET_SUPER,
    &&op_OP_EQUAL,         &&op_OP_GREATER,       &&op_OP_LESS,
    &&op_OP_ADD,           &&op_OP_SUBTRACT,
    &&op_OP_MULTIPLY,      &&op_OP_DIVIDE,        &&op_OP_MODULO,
    &&op_OP_NOT,           &&op_OP_NEGATE,        &&op_OP_STR,
    &&op_OP_PRINT,
    &&op_OP_JUMP,          &&op_OP_JUMP_IF_FALSE, &&op_OP_LOOP,
    &&op_OP_CALL,          &&op_OP_INVOKE,        &&op_OP_SUPER_INVOKE,
    &&op_OP_CLOSURE,       &&op_OP_CLOSE_UPVALUE, &&op_OP_RETURN,
    &&op_OP_CLASS,         &&op_OP_INHERIT,       &&op_OP_METHOD,
    &&op_OP_BUILD_LIST,    &&op_OP_BUILD_MAP,
    &&op_OP_INDEX_GET,     &&op_OP_INDEX_SET,
    &&op_OP_IMPORT,        &&op_OP_IMPORT_FROM,   &&op_OP_IMPORT_ALIAS,
    &&op_OP_FOR_ITER,
  };

#define VM_DISPATCH() do { TRACE_INSTRUCTION(); goto *dispatch_table[READ_BYTE()]; } while (false)
#define VM_CASE(name) op_##name:
#else // !MAPLE_GNUC — MSVC fallback: standard switch dispatch
#define VM_DISPATCH() continue
#define VM_CASE(name) case OpCode::name:
#endif

#ifdef MAPLE_GNUC
  VM_DISPATCH();
#else
  for (;;) {
    TRACE_INSTRUCTION();
    switch (static_cast<OpCode>(READ_BYTE())) {
#endif

    VM_CASE(OP_CONSTANT) {
      Value constant = READ_CONSTANT();
      push(constant);
      VM_DISPATCH();
    }

    VM_CASE(OP_CONSTANT_LONG) {
      u32_t index = READ_BYTE();
      index |= static_cast<u32_t>(READ_BYTE()) << 8;
      index |= static_cast<u32_t>(READ_BYTE()) << 16;
      push(frame->closure->function()->chunk().constant_at(index));
      VM_DISPATCH();
    }

    VM_CASE(OP_NIL)   { push(Value()); VM_DISPATCH(); }
    VM_CASE(OP_TRUE)  { push(Value(true)); VM_DISPATCH(); }
    VM_CASE(OP_FALSE) { push(Value(false)); VM_DISPATCH(); }

    VM_CASE(OP_POP) { pop(); VM_DISPATCH(); }

    VM_CASE(OP_GET_LOCAL) {
      u8_t slot = READ_BYTE();
      push(frame->slots[slot]);
      VM_DISPATCH();
    }

    VM_CASE(OP_SET_LOCAL) {
      u8_t slot = READ_BYTE();
      frame->slots[slot] = peek(0);
      VM_DISPATCH();
    }

    VM_CASE(OP_GET_GLOBAL) {
      ObjString* name = READ_STRING();
      Value value;
      if (!globals_.get(name, &value)) {
        runtime_error(std::format("Undefined variable '{}'.", name->value()));
        return InterpretResult::INTERPRET_RUNTIME_ERROR;
      }
      push(value);
      VM_DISPATCH();
    }

    VM_CASE(OP_DEFINE_GLOBAL) {
      ObjString* name = READ_STRING();
      globals_.set(name, peek(0));
      pop();
      VM_DISPATCH();
    }

    VM_CASE(OP_SET_GLOBAL) {
      ObjString* name = READ_STRING();
      if (globals_.set(name, peek(0))) {
        globals_.remove(name);
        runtime_error(std::format("Undefined variable '{}'.", name->value()));
        return InterpretResult::INTERPRET_RUNTIME_ERROR;
      }
      VM_DISPATCH();
    }

    VM_CASE(OP_GET_UPVALUE) {
      u8_t slot = READ_BYTE();
      push(*frame->closure->upvalue_at(slot)->location());
      VM_DISPATCH();
    }

    VM_CASE(OP_SET_UPVALUE) {
      u8_t slot = READ_BYTE();
      *frame->closure->upvalue_at(slot)->location() = peek(0);
      VM_DISPATCH();
    }

    VM_CASE(OP_GET_PROPERTY) {
      if (is_obj_type(peek(0), ObjectType::OBJ_MODULE)) {
        ObjModule* module = as_module(peek(0));
        ObjString* name = READ_STRING();
        Value export_val;
        if (module->exports().get(name, &export_val)) {
          pop(); // Module
          push(export_val);
          VM_DISPATCH();
        }
        runtime_error(std::format("Undefined export '{}'.", name->value()));
        return InterpretResult::INTERPRET_RUNTIME_ERROR;
      }

      if (is_obj_type(peek(0), ObjectType::OBJ_CLASS)) {
        ObjClass* klass = as_class(peek(0));
        ObjString* name = READ_STRING();
        Value method;
        if (klass->static_methods().get(name, &method)) {
          pop(); // Class
          push(method);
          VM_DISPATCH();
        }
        runtime_error(std::format("Undefined static method '{}'.", name->value()));
        return InterpretResult::INTERPRET_RUNTIME_ERROR;
      }

      if (!is_obj_type(peek(0), ObjectType::OBJ_INSTANCE)) {
        runtime_error("Only instances have properties.");
        return InterpretResult::INTERPRET_RUNTIME_ERROR;
      }

      ObjInstance* instance = as_instance(peek(0));
      ObjString* name = READ_STRING();

      Value field_val;
      if (instance->fields().get(name, &field_val)) {
        pop(); // Instance
        push(field_val);
        VM_DISPATCH();
      }

      bind_method(instance->klass(), name);
      VM_DISPATCH();
    }

    VM_CASE(OP_SET_PROPERTY) {
      if (!is_obj_type(peek(1), ObjectType::OBJ_INSTANCE)) {
        runtime_error("Only instances have fields.");
        return InterpretResult::INTERPRET_RUNTIME_ERROR;
      }

      ObjInstance* instance = as_instance(peek(1));
      ObjString* name = READ_STRING();
      instance->fields().set(name, peek(0));
      Value value = pop();
      pop();
      push(value);
      VM_DISPATCH();
    }

    VM_CASE(OP_GET_SUPER) {
      ObjString* name = READ_STRING();
      ObjClass* superclass = as_class(pop());
      bind_method(superclass, name);
      VM_DISPATCH();
    }

    VM_CASE(OP_EQUAL) {
      Value b = pop();
      Value a = pop();
      push(Value(a.is_equal(b)));
      VM_DISPATCH();
    }

    VM_CASE(OP_GREATER) { BINARY_OP(Value, >); VM_DISPATCH(); }
    VM_CASE(OP_LESS)    { BINARY_OP(Value, <); VM_DISPATCH(); }

    VM_CASE(OP_ADD) {
      if (is_obj_type(peek(0), ObjectType::OBJ_STRING) &&
          is_obj_type(peek(1), ObjectType::OBJ_STRING)) {
        concatenate();
      } else if (peek(0).is_number() && peek(1).is_number()) {
        double b = pop().as_number();
        double a = pop().as_number();
        push(Value(a + b));
      } else {
        runtime_error("Operands must be two numbers or two strings.");
        return InterpretResult::INTERPRET_RUNTIME_ERROR;
      }
      VM_DISPATCH();
    }

    VM_CASE(OP_SUBTRACT) { BINARY_OP(Value, -); VM_DISPATCH(); }
    VM_CASE(OP_MULTIPLY) { BINARY_OP(Value, *); VM_DISPATCH(); }
    VM_CASE(OP_DIVIDE)   { BINARY_OP(Value, /); VM_DISPATCH(); }

    VM_CASE(OP_MODULO) {
      if (!peek(0).is_number() || !peek(1).is_number()) {
        runtime_error("Operands must be numbers.");
        return InterpretResult::INTERPRET_RUNTIME_ERROR;
      }
      double b = pop().as_number();
      double a = pop().as_number();
      push(Value(std::fmod(a, b)));
      VM_DISPATCH();
    }

    VM_CASE(OP_NOT) {
      push(Value(!pop().is_truthy()));
      VM_DISPATCH();
    }

    VM_CASE(OP_NEGATE) {
      if (!peek(0).is_number()) {
        runtime_error("Operand must be a number.");
        return InterpretResult::INTERPRET_RUNTIME_ERROR;
      }
      push(Value(-pop().as_number()));
      VM_DISPATCH();
    }

    VM_CASE(OP_STR) {
      Value val = pop();
      if (val.is_string()) {
        push(val);
      } else {
        str_t s = val.stringify();
        push(Value(static_cast<Object*>(copy_string(s.data(), s.length()))));
      }
      VM_DISPATCH();
    }

    VM_CASE(OP_PRINT) {
      std::cout << pop().stringify() << std::endl;
      VM_DISPATCH();
    }

    VM_CASE(OP_JUMP) {
      u16_t offset = READ_SHORT();
      frame->ip += offset;
      VM_DISPATCH();
    }

    VM_CASE(OP_JUMP_IF_FALSE) {
      u16_t offset = READ_SHORT();
      if (!peek(0).is_truthy()) frame->ip += offset;
      VM_DISPATCH();
    }

    VM_CASE(OP_LOOP) {
      u16_t offset = READ_SHORT();
      frame->ip -= offset;
      VM_DISPATCH();
    }

    VM_CASE(OP_CALL) {
      int arg_count = READ_BYTE();
      if (!call_value(peek(arg_count), arg_count)) {
        return InterpretResult::INTERPRET_RUNTIME_ERROR;
      }
      frame = &frames_[frame_count_ - 1];
      VM_DISPATCH();
    }

    VM_CASE(OP_INVOKE) {
      ObjString* method = READ_STRING();
      int arg_count = READ_BYTE();
      if (!invoke(method, arg_count)) {
        return InterpretResult::INTERPRET_RUNTIME_ERROR;
      }
      frame = &frames_[frame_count_ - 1];
      VM_DISPATCH();
    }

    VM_CASE(OP_SUPER_INVOKE) {
      ObjString* method = READ_STRING();
      int arg_count = READ_BYTE();
      ObjClass* superclass = as_class(pop());
      if (!invoke_from_class(superclass, method, arg_count)) {
        return InterpretResult::INTERPRET_RUNTIME_ERROR;
      }
      frame = &frames_[frame_count_ - 1];
      VM_DISPATCH();
    }

    VM_CASE(OP_CLOSURE) {
      ObjFunction* function = as_function(READ_CONSTANT());
      ObjClosure* closure = allocate<ObjClosure>(function);
      push(Value(static_cast<Object*>(closure)));

      for (int i = 0; i < closure->upvalue_count(); i++) {
        u8_t is_local = READ_BYTE();
        u8_t index = READ_BYTE();
        if (is_local) {
          closure->set_upvalue_at(static_cast<sz_t>(i), capture_upvalue(frame->slots + index));
        } else {
          closure->set_upvalue_at(static_cast<sz_t>(i), frame->closure->upvalue_at(index));
        }
      }
      VM_DISPATCH();
    }

    VM_CASE(OP_CLOSE_UPVALUE) {
      close_upvalues(stack_top_ - 1);
      pop();
      VM_DISPATCH();
    }

    VM_CASE(OP_RETURN) {
      Value result = pop();
      close_upvalues(frame->slots);
      frame_count_--;
      if (frame_count_ == 0) {
        pop();
        return InterpretResult::INTERPRET_OK;
      }

      stack_top_ = frame->slots;
      push(result);
      frame = &frames_[frame_count_ - 1];

      // Check if we just returned from a module frame
      if (!pending_imports_.empty() && pending_imports_.back().frame_index == frame_count_) {
        auto pending = std::move(pending_imports_.back());
        pending_imports_.pop_back();

        // Restore previous script path
        current_script_path_ = std::move(pending.previous_script_path);

        // Collect new globals as module exports
        std::unordered_set<ObjString*> pre_set(
            pending.pre_global_keys.begin(), pending.pre_global_keys.end());
        for (auto& entry : globals_.entries()) {
          if (entry.key != nullptr && pre_set.find(entry.key) == pre_set.end()) {
            pending.module->exports().set(entry.key, entry.value);
          }
        }

        // Define module as global variable using filename without extension
        str_t mod_path = str_t(pending.module->name()->value());
        auto slash = mod_path.find_last_of("/\\");
        str_t filename = (slash != str_t::npos) ? mod_path.substr(slash + 1) : mod_path;
        auto dot = filename.find_last_of('.');
        str_t mod_name = (dot != str_t::npos) ? filename.substr(0, dot) : filename;
        ObjString* name_str = copy_string(mod_name.c_str(), mod_name.size());
        globals_.set(name_str, Value(static_cast<Object*>(pending.module)));

        // Process deferred from-import requests
        for (auto& req : pending.from_imports) {
          Value exp_val;
          if (pending.module->exports().get(req.name, &exp_val)) {
            ObjString* target = req.alias ? req.alias : req.name;
            globals_.set(target, exp_val);
          }
        }
      }
      VM_DISPATCH();
    }

    VM_CASE(OP_CLASS) {
      push(Value(static_cast<Object*>(allocate<ObjClass>(READ_STRING()))));
      VM_DISPATCH();
    }

    VM_CASE(OP_INHERIT) {
      Value superclass = peek(1);
      if (!is_obj_type(superclass, ObjectType::OBJ_CLASS)) {
        runtime_error("Superclass must be a class.");
        return InterpretResult::INTERPRET_RUNTIME_ERROR;
      }

      ObjClass* subclass = as_class(peek(0));
      ObjClass* super = as_class(superclass);
      // Copy methods from superclass
      subclass->methods().add_all(super->methods());
      subclass->static_methods().add_all(super->static_methods());
      pop(); // Subclass
      VM_DISPATCH();
    }

    VM_CASE(OP_METHOD) {
      ObjString* name = READ_STRING();
      Value method = peek(0);
      ObjClass* klass = as_class(peek(1));
      klass->methods().set(name, method);
      pop();
      VM_DISPATCH();
    }

    VM_CASE(OP_STATIC_METHOD) {
      ObjString* name = READ_STRING();
      Value method = peek(0);
      ObjClass* klass = as_class(peek(1));
      klass->static_methods().set(name, method);
      pop();
      VM_DISPATCH();
    }

    VM_CASE(OP_BUILD_LIST) {
      u8_t count = READ_BYTE();
      ObjList* list = allocate<ObjList>();
      list->elements().resize(count);
      for (int i = count - 1; i >= 0; i--) {
        list->elements()[static_cast<sz_t>(i)] = pop();
      }
      push(Value(static_cast<Object*>(list)));
      VM_DISPATCH();
    }

    VM_CASE(OP_BUILD_MAP) {
      u8_t count = READ_BYTE();
      ObjMap* map = allocate<ObjMap>();
      for (int i = count - 1; i >= 0; i--) {
        Value val = pop();
        Value key = pop();
        map->entries()[key] = val;
      }
      push(Value(static_cast<Object*>(map)));
      VM_DISPATCH();
    }

    VM_CASE(OP_INDEX_GET) {
      Value index_val = pop();
      Value receiver = pop();
      if (is_obj_type(receiver, ObjectType::OBJ_LIST)) {
        if (!index_val.is_number()) {
          runtime_error("List index must be a number.");
          return InterpretResult::INTERPRET_RUNTIME_ERROR;
        }
        ObjList* list = as_list(receiver);
        int index = static_cast<int>(index_val.as_number());
        if (index < 0 || index >= static_cast<int>(list->len())) {
          runtime_error("List index out of bounds.");
          return InterpretResult::INTERPRET_RUNTIME_ERROR;
        }
        push(list->elements()[static_cast<sz_t>(index)]);
      } else if (is_obj_type(receiver, ObjectType::OBJ_MAP)) {
        ObjMap* map = as_map(receiver);
        auto it = map->entries().find(index_val);
        if (it == map->entries().end()) {
          runtime_error("Key not found in map.");
          return InterpretResult::INTERPRET_RUNTIME_ERROR;
        }
        push(it->second);
      } else if (is_obj_type(receiver, ObjectType::OBJ_STRING)) {
        if (!index_val.is_number()) {
          runtime_error("String index must be a number.");
          return InterpretResult::INTERPRET_RUNTIME_ERROR;
        }
        ObjString* str = as_string(receiver);
        int index = static_cast<int>(index_val.as_number());
        if (index < 0 || index >= static_cast<int>(str->value().length())) {
          runtime_error("String index out of bounds.");
          return InterpretResult::INTERPRET_RUNTIME_ERROR;
        }
        push(Value(static_cast<Object*>(copy_string(&str->value()[static_cast<sz_t>(index)], 1))));
      } else {
        runtime_error("Only lists and strings can be indexed.");
        return InterpretResult::INTERPRET_RUNTIME_ERROR;
      }
      VM_DISPATCH();
    }

    VM_CASE(OP_INDEX_SET) {
      Value value = pop();
      Value index_val = pop();
      Value receiver = pop();
      if (is_obj_type(receiver, ObjectType::OBJ_LIST)) {
        if (!index_val.is_number()) {
          runtime_error("List index must be a number.");
          return InterpretResult::INTERPRET_RUNTIME_ERROR;
        }
        ObjList* list = as_list(receiver);
        int index = static_cast<int>(index_val.as_number());
        if (index < 0 || index >= static_cast<int>(list->len())) {
          runtime_error("List index out of bounds.");
          return InterpretResult::INTERPRET_RUNTIME_ERROR;
        }
        list->elements()[static_cast<sz_t>(index)] = value;
      } else if (is_obj_type(receiver, ObjectType::OBJ_MAP)) {
        ObjMap* map = as_map(receiver);
        map->entries()[index_val] = value;
      } else {
        runtime_error("Only lists and maps support index assignment.");
        return InterpretResult::INTERPRET_RUNTIME_ERROR;
      }
      push(value);
      VM_DISPATCH();
    }

    VM_CASE(OP_IMPORT) {
      Value path_val = pop();
      if (!is_obj_type(path_val, ObjectType::OBJ_STRING)) {
        runtime_error("Import path must be a string.");
        return InterpretResult::INTERPRET_RUNTIME_ERROR;
      }
      import_module(as_string(path_val));
      frame = &frames_[frame_count_ - 1];
      VM_DISPATCH();
    }

    VM_CASE(OP_IMPORT_FROM) {
      // Stack: [path, name]
      Value name_val = pop();
      Value path_val = pop();
      if (!is_obj_type(path_val, ObjectType::OBJ_STRING)) {
        runtime_error("Import path must be a string.");
        return InterpretResult::INTERPRET_RUNTIME_ERROR;
      }
      ObjString* path = as_string(path_val);
      ObjString* name = as_string(name_val);
      str_t resolved = ModuleLoader::resolve_path(path->value(), current_script_path_);
      bool was_cached = modules_.find(resolved) != modules_.end();
      import_module(path);
      frame = &frames_[frame_count_ - 1];

      if (was_cached) {
        // Module already executed, exports available
        auto mod_it = modules_.find(resolved);
        if (mod_it != modules_.end()) {
          Value exp_val;
          if (mod_it->second->exports().get(name, &exp_val)) {
            globals_.set(name, exp_val);
          }
        }
      } else if (!pending_imports_.empty()) {
        // Module not yet executed, defer lookup
        pending_imports_.back().from_imports.push_back({name, nullptr});
      }
      VM_DISPATCH();
    }

    VM_CASE(OP_IMPORT_ALIAS) {
      // Stack: [path, name, alias]
      Value alias_val = pop();
      Value name_val = pop();
      Value path_val = pop();
      if (!is_obj_type(path_val, ObjectType::OBJ_STRING)) {
        runtime_error("Import path must be a string.");
        return InterpretResult::INTERPRET_RUNTIME_ERROR;
      }
      ObjString* path = as_string(path_val);
      ObjString* name = as_string(name_val);
      ObjString* alias = as_string(alias_val);
      str_t resolved = ModuleLoader::resolve_path(path->value(), current_script_path_);
      bool was_cached = modules_.find(resolved) != modules_.end();
      import_module(path);
      frame = &frames_[frame_count_ - 1];

      if (was_cached) {
        auto mod_it = modules_.find(resolved);
        if (mod_it != modules_.end()) {
          Value exp_val;
          if (mod_it->second->exports().get(name, &exp_val)) {
            globals_.set(alias, exp_val);
          }
        }
      } else if (!pending_imports_.empty()) {
        pending_imports_.back().from_imports.push_back({name, alias});
      }
      VM_DISPATCH();
    }

    VM_CASE(OP_FOR_ITER) {
      // Format: OP_FOR_ITER slot jump_hi jump_lo
      // slot   = stack slot of the iterable (index is at slot+1)
      // jump   = forward offset to exit the loop when iteration ends
      u8_t slot = READ_BYTE();
      u16_t offset = READ_SHORT();
      Value& iterable = frame->slots[slot];
      Value& index_val = frame->slots[slot + 1];
      int idx = static_cast<int>(index_val.as_number());

      if (is_obj_type(iterable, ObjectType::OBJ_LIST)) {
        ObjList* list = as_list(iterable);
        if (idx >= static_cast<int>(list->len())) {
          frame->ip += offset; // jump past loop body
        } else {
          push(list->elements()[static_cast<sz_t>(idx)]);
          index_val = Value(static_cast<double>(idx + 1));
        }
      } else if (is_obj_type(iterable, ObjectType::OBJ_STRING)) {
        ObjString* str = as_string(iterable);
        if (idx >= static_cast<int>(str->value().length())) {
          frame->ip += offset;
        } else {
          push(Value(static_cast<Object*>(
              copy_string(&str->value()[static_cast<sz_t>(idx)], 1))));
          index_val = Value(static_cast<double>(idx + 1));
        }
      } else if (is_obj_type(iterable, ObjectType::OBJ_MAP)) {
        ObjMap* map = as_map(iterable);
        auto& entries = map->entries();
        if (idx >= static_cast<int>(entries.size())) {
          frame->ip += offset;
        } else {
          auto it = entries.begin();
          std::advance(it, idx);
          push(it->first); // iterate over keys
          index_val = Value(static_cast<double>(idx + 1));
        }
      } else {
        runtime_error("Can only iterate over lists, strings, and maps.");
        return InterpretResult::INTERPRET_RUNTIME_ERROR;
      }
      VM_DISPATCH();
    }

#ifndef MAPLE_GNUC
    } // switch
  } // for
#endif

#undef VM_CASE
#undef VM_DISPATCH
#undef TRACE_INSTRUCTION
#undef READ_BYTE
#undef READ_SHORT
#undef READ_CONSTANT
#undef READ_STRING
#undef BINARY_OP
}

} // namespace ms
