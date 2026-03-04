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
#include <cstdarg>
#include <fstream>
#include <iostream>
#include <format>
#include "VM.hh"
#include "Compiler.hh"
#include "Memory.hh"
#include "Debug.hh"
#include "Logger.hh"

namespace ms {

// --- Global accessor functions for GC/Memory ---
Object*& vm_objects() noexcept {
  return VM::get_instance().gc_objects();
}

sz_t& vm_bytes_allocated() noexcept {
  return VM::get_instance().gc_bytes_allocated();
}

sz_t& vm_next_gc() noexcept {
  return VM::get_instance().gc_next_gc();
}

std::vector<Object*>& vm_gray_stack() noexcept {
  return VM::get_instance().gc_gray_stack();
}

void vm_mark_roots() noexcept {
  VM::get_instance().mark_roots();
}

// --- Global accessor for Compiler ---
ObjString* vm_copy_string(cstr_t chars, sz_t length) noexcept {
  return VM::get_instance().copy_string(chars, length);
}

// --- allocate_object template (used by Memory.hh) ---
template <typename T, typename... Args>
T* allocate_object(Args&&... args) noexcept {
  return VM::get_instance().allocate<T>(std::forward<Args>(args)...);
}

// Explicit template instantiations
template ObjString* allocate_object<ObjString>(str_t&&, u32_t&&);
template ObjString* allocate_object<ObjString>(str_t&, u32_t&);
template ObjFunction* allocate_object<ObjFunction>();
template ObjNative* allocate_object<ObjNative>(NativeFn&&);
template ObjNative* allocate_object<ObjNative>(NativeFn&);
template ObjClosure* allocate_object<ObjClosure>(ObjFunction*&&);
template ObjClosure* allocate_object<ObjClosure>(ObjFunction*&);
template ObjUpvalue* allocate_object<ObjUpvalue>(Value*&&);
template ObjUpvalue* allocate_object<ObjUpvalue>(Value*&);
template ObjClass* allocate_object<ObjClass>(ObjString*&&);
template ObjClass* allocate_object<ObjClass>(ObjString*&);
template ObjInstance* allocate_object<ObjInstance>(ObjClass*&&);
template ObjInstance* allocate_object<ObjInstance>(ObjClass*&);
template ObjBoundMethod* allocate_object<ObjBoundMethod>(Value&&, ObjClosure*&&);
template ObjBoundMethod* allocate_object<ObjBoundMethod>(Value&, ObjClosure*&);
template ObjModule* allocate_object<ObjModule>(ObjString*&&);
template ObjModule* allocate_object<ObjModule>(ObjString*&);

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
    ObjFunction* function = frame.closure->function_;
    sz_t instruction = static_cast<sz_t>(frame.ip - function->chunk_.code_data() - 1);
    std::cerr << std::format("[line {}] in ",
        function->chunk_.line_at(instruction));
    if (function->name_ == nullptr) {
      std::cerr << "script" << std::endl;
    } else {
      std::cerr << function->name_->value_ << "()" << std::endl;
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
    strings_.remove_white();
    collect_garbage();
  }
#endif

  auto* object = new T(std::forward<Args>(args)...);
  object->next_ = objects_;
  objects_ = object;

#ifdef MAPLE_DEBUG_LOG_GC
  auto& logger = Logger::get_instance();
  logger.debug("GC", "allocate {} for {} ({})",
      sizeof(T), static_cast<void*>(object), object->stringify());
#endif

  return object;
}

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
       upvalue != nullptr; upvalue = upvalue->next_upvalue_) {
    mark_object(upvalue);
  }

  // Mark globals
  globals_.mark_table();

  // Mark compiler roots
  mark_compiler_roots();

  // Mark init string
  mark_object(init_string_);
}

void VM::concatenate() noexcept {
  ObjString* b = as_string(peek(0));
  ObjString* a = as_string(peek(1));

  str_t result = a->value_ + b->value_;
  ObjString* str = take_string(std::move(result));

  pop();
  pop();
  push(Value(static_cast<Object*>(str)));
}

bool VM::call(ObjClosure* closure, int arg_count) noexcept {
  if (arg_count != closure->function_->arity_) {
    runtime_error(std::format("Expected {} arguments but got {}.",
        closure->function_->arity_, arg_count));
    return false;
  }

  if (frame_count_ == static_cast<int>(kFRAMES_MAX)) {
    runtime_error("Stack overflow.");
    return false;
  }

  CallFrame& frame = frames_[frame_count_++];
  frame.closure = closure;
  frame.ip = closure->function_->chunk_.code_data();
  frame.slots = stack_top_ - arg_count - 1;
  return true;
}

bool VM::call_value(Value callee, int arg_count) noexcept {
  if (callee.is_object()) {
    switch (callee.as_object()->type()) {
    case ObjectType::OBJ_CLOSURE:
      return call(as_closure(callee), arg_count);

    case ObjectType::OBJ_NATIVE: {
      NativeFn& native = as_native(callee)->function_;
      Value result = native(arg_count, stack_top_ - arg_count);
      stack_top_ -= arg_count + 1;
      push(result);
      return true;
    }

    case ObjectType::OBJ_CLASS: {
      ObjClass* klass = as_class(callee);
      stack_top_[-arg_count - 1] = Value(static_cast<Object*>(allocate<ObjInstance>(klass)));

      // Call initializer if present
      auto it = klass->methods_.find(init_string_);
      if (it != klass->methods_.end()) {
        return call(as_closure(it->second), arg_count);
      } else if (arg_count != 0) {
        runtime_error(std::format("Expected 0 arguments but got {}.", arg_count));
        return false;
      }
      return true;
    }

    case ObjectType::OBJ_BOUND_METHOD: {
      ObjBoundMethod* bound = as_bound_method(callee);
      stack_top_[-arg_count - 1] = bound->receiver_;
      return call(bound->method_, arg_count);
    }

    default:
      break;
    }
  }

  runtime_error("Can only call functions and classes.");
  return false;
}

bool VM::invoke_from_class(ObjClass* klass, ObjString* name, int arg_count) noexcept {
  auto it = klass->methods_.find(name);
  if (it == klass->methods_.end()) {
    runtime_error(std::format("Undefined property '{}'.", name->value_));
    return false;
  }
  return call(as_closure(it->second), arg_count);
}

bool VM::invoke(ObjString* name, int arg_count) noexcept {
  Value receiver = peek(arg_count);

  if (!is_obj_type(receiver, ObjectType::OBJ_INSTANCE)) {
    runtime_error("Only instances have methods.");
    return false;
  }

  ObjInstance* instance = as_instance(receiver);

  // Check for field first (field shadowing method)
  auto it = instance->fields_.find(name);
  if (it != instance->fields_.end()) {
    stack_top_[-arg_count - 1] = it->second;
    return call_value(it->second, arg_count);
  }

  return invoke_from_class(instance->klass_, name, arg_count);
}

void VM::bind_method(ObjClass* klass, ObjString* name) noexcept {
  auto it = klass->methods_.find(name);
  if (it == klass->methods_.end()) {
    runtime_error(std::format("Undefined property '{}'.", name->value_));
    return;
  }

  ObjBoundMethod* bound = allocate<ObjBoundMethod>(peek(0), as_closure(it->second));
  pop();
  push(Value(static_cast<Object*>(bound)));
}

ObjUpvalue* VM::capture_upvalue(Value* local) noexcept {
  ObjUpvalue* prev_upvalue = nullptr;
  ObjUpvalue* upvalue = open_upvalues_;

  while (upvalue != nullptr && upvalue->location_ > local) {
    prev_upvalue = upvalue;
    upvalue = upvalue->next_upvalue_;
  }

  if (upvalue != nullptr && upvalue->location_ == local) {
    return upvalue;
  }

  ObjUpvalue* created_upvalue = allocate<ObjUpvalue>(local);
  created_upvalue->next_upvalue_ = upvalue;

  if (prev_upvalue == nullptr) {
    open_upvalues_ = created_upvalue;
  } else {
    prev_upvalue->next_upvalue_ = created_upvalue;
  }

  return created_upvalue;
}

void VM::close_upvalues(Value* last) noexcept {
  while (open_upvalues_ != nullptr &&
         open_upvalues_->location_ >= last) {
    ObjUpvalue* upvalue = open_upvalues_;
    upvalue->closed_ = *upvalue->location_;
    upvalue->location_ = &upvalue->closed_;
    open_upvalues_ = upvalue->next_upvalue_;
  }
}

void VM::import_module(ObjString* path) noexcept {
  str_t path_str = path->value_;

  // Check cache
  auto it = modules_.find(path_str);
  if (it != modules_.end()) {
    return; // already imported
  }

  // Read source file
  std::ifstream file(path_str);
  if (!file.is_open()) {
    runtime_error(std::format("Could not open module '{}'.", path_str));
    return;
  }

  file.seekg(0, std::ios::end);
  auto file_size = file.tellg();
  file.seekg(0, std::ios::beg);
  str_t source;
  source.resize(static_cast<sz_t>(file_size));
  file.read(source.data(), file_size);

  // Compile module
  ObjFunction* function = compile(strv_t(source));
  if (function == nullptr) {
    runtime_error(std::format("Could not compile module '{}'.", path_str));
    return;
  }

  // Execute module
  ObjClosure* closure = allocate<ObjClosure>(function);
  push(Value(static_cast<Object*>(closure)));
  call(closure, 0);

  // Module execution happens in the normal run loop
  // For now, just mark it as imported
  ObjModule* module = allocate<ObjModule>(path);
  modules_[path_str] = module;
}

InterpretResult VM::interpret(strv_t source) noexcept {
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
    (frame->closure->function_->chunk_.constant_at(READ_BYTE()))
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

  for (;;) {
#ifdef MAPLE_DEBUG_TRACE
    // Print stack
    std::cout << "          ";
    for (Value* slot = frame->slots; slot < stack_top_; slot++) {
      std::cout << "[ " << slot->stringify() << " ]";
    }
    std::cout << std::endl;
    disassemble_instruction(frame->closure->function_->chunk_,
        static_cast<sz_t>(frame->ip - frame->closure->function_->chunk_.code_data()));
#endif

    u8_t instruction = READ_BYTE();
    switch (static_cast<OpCode>(instruction)) {
    case OpCode::OP_CONSTANT: {
      Value constant = READ_CONSTANT();
      push(constant);
      break;
    }

    case OpCode::OP_NIL:   push(Value()); break;
    case OpCode::OP_TRUE:  push(Value(true)); break;
    case OpCode::OP_FALSE: push(Value(false)); break;

    case OpCode::OP_POP: pop(); break;

    case OpCode::OP_GET_LOCAL: {
      u8_t slot = READ_BYTE();
      push(frame->slots[slot]);
      break;
    }

    case OpCode::OP_SET_LOCAL: {
      u8_t slot = READ_BYTE();
      frame->slots[slot] = peek(0);
      break;
    }

    case OpCode::OP_GET_GLOBAL: {
      ObjString* name = READ_STRING();
      Value value;
      if (!globals_.get(name, &value)) {
        runtime_error(std::format("Undefined variable '{}'.", name->value_));
        return InterpretResult::INTERPRET_RUNTIME_ERROR;
      }
      push(value);
      break;
    }

    case OpCode::OP_DEFINE_GLOBAL: {
      ObjString* name = READ_STRING();
      globals_.set(name, peek(0));
      pop();
      break;
    }

    case OpCode::OP_SET_GLOBAL: {
      ObjString* name = READ_STRING();
      if (globals_.set(name, peek(0))) {
        globals_.remove(name);
        runtime_error(std::format("Undefined variable '{}'.", name->value_));
        return InterpretResult::INTERPRET_RUNTIME_ERROR;
      }
      break;
    }

    case OpCode::OP_GET_UPVALUE: {
      u8_t slot = READ_BYTE();
      push(*frame->closure->upvalues_[slot]->location_);
      break;
    }

    case OpCode::OP_SET_UPVALUE: {
      u8_t slot = READ_BYTE();
      *frame->closure->upvalues_[slot]->location_ = peek(0);
      break;
    }

    case OpCode::OP_GET_PROPERTY: {
      if (!is_obj_type(peek(0), ObjectType::OBJ_INSTANCE)) {
        runtime_error("Only instances have properties.");
        return InterpretResult::INTERPRET_RUNTIME_ERROR;
      }

      ObjInstance* instance = as_instance(peek(0));
      ObjString* name = READ_STRING();

      auto it = instance->fields_.find(name);
      if (it != instance->fields_.end()) {
        pop(); // Instance
        push(it->second);
        break;
      }

      bind_method(instance->klass_, name);
      break;
    }

    case OpCode::OP_SET_PROPERTY: {
      if (!is_obj_type(peek(1), ObjectType::OBJ_INSTANCE)) {
        runtime_error("Only instances have fields.");
        return InterpretResult::INTERPRET_RUNTIME_ERROR;
      }

      ObjInstance* instance = as_instance(peek(1));
      ObjString* name = READ_STRING();
      instance->fields_[name] = peek(0);
      Value value = pop();
      pop();
      push(value);
      break;
    }

    case OpCode::OP_GET_SUPER: {
      ObjString* name = READ_STRING();
      ObjClass* superclass = as_class(pop());
      bind_method(superclass, name);
      break;
    }

    case OpCode::OP_EQUAL: {
      Value b = pop();
      Value a = pop();
      push(Value(a.is_equal(b)));
      break;
    }

    case OpCode::OP_GREATER: BINARY_OP(Value, >); break;
    case OpCode::OP_LESS:    BINARY_OP(Value, <); break;

    case OpCode::OP_ADD: {
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
      break;
    }

    case OpCode::OP_SUBTRACT: BINARY_OP(Value, -); break;
    case OpCode::OP_MULTIPLY: BINARY_OP(Value, *); break;
    case OpCode::OP_DIVIDE:   BINARY_OP(Value, /); break;

    case OpCode::OP_NOT:
      push(Value(!pop().is_truthy()));
      break;

    case OpCode::OP_NEGATE: {
      if (!peek(0).is_number()) {
        runtime_error("Operand must be a number.");
        return InterpretResult::INTERPRET_RUNTIME_ERROR;
      }
      push(Value(-pop().as_number()));
      break;
    }

    case OpCode::OP_PRINT: {
      std::cout << pop().stringify() << std::endl;
      break;
    }

    case OpCode::OP_JUMP: {
      u16_t offset = READ_SHORT();
      frame->ip += offset;
      break;
    }

    case OpCode::OP_JUMP_IF_FALSE: {
      u16_t offset = READ_SHORT();
      if (!peek(0).is_truthy()) frame->ip += offset;
      break;
    }

    case OpCode::OP_LOOP: {
      u16_t offset = READ_SHORT();
      frame->ip -= offset;
      break;
    }

    case OpCode::OP_CALL: {
      int arg_count = READ_BYTE();
      if (!call_value(peek(arg_count), arg_count)) {
        return InterpretResult::INTERPRET_RUNTIME_ERROR;
      }
      frame = &frames_[frame_count_ - 1];
      break;
    }

    case OpCode::OP_INVOKE: {
      ObjString* method = READ_STRING();
      int arg_count = READ_BYTE();
      if (!invoke(method, arg_count)) {
        return InterpretResult::INTERPRET_RUNTIME_ERROR;
      }
      frame = &frames_[frame_count_ - 1];
      break;
    }

    case OpCode::OP_SUPER_INVOKE: {
      ObjString* method = READ_STRING();
      int arg_count = READ_BYTE();
      ObjClass* superclass = as_class(pop());
      if (!invoke_from_class(superclass, method, arg_count)) {
        return InterpretResult::INTERPRET_RUNTIME_ERROR;
      }
      frame = &frames_[frame_count_ - 1];
      break;
    }

    case OpCode::OP_CLOSURE: {
      ObjFunction* function = as_function(READ_CONSTANT());
      ObjClosure* closure = allocate<ObjClosure>(function);
      push(Value(static_cast<Object*>(closure)));

      for (int i = 0; i < closure->upvalue_count_; i++) {
        u8_t is_local = READ_BYTE();
        u8_t index = READ_BYTE();
        if (is_local) {
          closure->upvalues_[static_cast<sz_t>(i)] = capture_upvalue(frame->slots + index);
        } else {
          closure->upvalues_[static_cast<sz_t>(i)] = frame->closure->upvalues_[index];
        }
      }
      break;
    }

    case OpCode::OP_CLOSE_UPVALUE:
      close_upvalues(stack_top_ - 1);
      pop();
      break;

    case OpCode::OP_RETURN: {
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
      break;
    }

    case OpCode::OP_CLASS:
      push(Value(static_cast<Object*>(allocate<ObjClass>(READ_STRING()))));
      break;

    case OpCode::OP_INHERIT: {
      Value superclass = peek(1);
      if (!is_obj_type(superclass, ObjectType::OBJ_CLASS)) {
        runtime_error("Superclass must be a class.");
        return InterpretResult::INTERPRET_RUNTIME_ERROR;
      }

      ObjClass* subclass = as_class(peek(0));
      // Copy methods from superclass
      for (auto& [key, val] : as_class(superclass)->methods_) {
        subclass->methods_[key] = val;
      }
      pop(); // Subclass
      break;
    }

    case OpCode::OP_METHOD: {
      ObjString* name = READ_STRING();
      Value method = peek(0);
      ObjClass* klass = as_class(peek(1));
      klass->methods_[name] = method;
      pop();
      break;
    }

    case OpCode::OP_IMPORT: {
      Value path_val = pop();
      if (!is_obj_type(path_val, ObjectType::OBJ_STRING)) {
        runtime_error("Import path must be a string.");
        return InterpretResult::INTERPRET_RUNTIME_ERROR;
      }
      import_module(as_string(path_val));
      break;
    }

    case OpCode::OP_IMPORT_FROM: {
      // Stack: [path, name]
      Value name_val = pop();
      Value path_val = pop();
      if (!is_obj_type(path_val, ObjectType::OBJ_STRING)) {
        runtime_error("Import path must be a string.");
        return InterpretResult::INTERPRET_RUNTIME_ERROR;
      }
      ObjString* path = as_string(path_val);
      import_module(path);

      // Look up the exported name
      auto mod_it = modules_.find(path->value_);
      if (mod_it != modules_.end()) {
        ObjString* name = as_string(name_val);
        auto exp_it = mod_it->second->exports_.find(name);
        if (exp_it != mod_it->second->exports_.end()) {
          globals_.set(name, exp_it->second);
        }
      }
      break;
    }

    case OpCode::OP_IMPORT_ALIAS: {
      // Stack: [path, name, alias]
      Value alias_val = pop();
      Value name_val = pop();
      Value path_val = pop();
      if (!is_obj_type(path_val, ObjectType::OBJ_STRING)) {
        runtime_error("Import path must be a string.");
        return InterpretResult::INTERPRET_RUNTIME_ERROR;
      }
      ObjString* path = as_string(path_val);
      import_module(path);

      auto mod_it = modules_.find(path->value_);
      if (mod_it != modules_.end()) {
        ObjString* name = as_string(name_val);
        ObjString* alias = as_string(alias_val);
        auto exp_it = mod_it->second->exports_.find(name);
        if (exp_it != mod_it->second->exports_.end()) {
          globals_.set(alias, exp_it->second);
        }
      }
      break;
    }

    } // switch
  } // for

#undef READ_BYTE
#undef READ_SHORT
#undef READ_CONSTANT
#undef READ_STRING
#undef BINARY_OP
}

} // namespace ms
