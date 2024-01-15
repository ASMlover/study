// Copyright (c) 2023 ASMlover. All rights reserved.
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
#include <cstdarg>
#include <chrono>
#include <iostream>
#include "chunk.hh"
#include "compiler.hh"
#include "object.hh"
#include "vm.hh"

namespace clox {

static VM* _vm{};

void init_vm() noexcept {
  _vm = new VM();
}

void free_vm() noexcept {
  if (_vm) {
    delete _vm;
    _vm = nullptr;
  }
}

VM& get_vm() noexcept {
  if (!_vm)
    init_vm();
  return *_vm;
}

VM::VM() noexcept {
  reset_stack();

  define_native("time", [](int arg_count, Value* args) -> Value {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count() / 1000.0;
      });
  define_native("clock", [](int arg_count, Value* args) -> Value {
        return std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
      });
}

VM::~VM() noexcept {
  globals_.clear();
}

bool VM::call(ObjClosure* closure, int arg_count) noexcept {
  if (arg_count != closure->function()->arity()) {
    runtime_error("expect %d arguments but got %d.", closure->function()->arity(), arg_count);
    return false;
  }

  if (frame_count_ >= kFramesMax) {
    runtime_error("stack overflow");
    return false;
  }

  CallFrame* frame = &frames_[frame_count_++];
  frame->set_callframe(
      closure,
      const_cast<u8_t*>(closure->function()->chunk()->codes()),
      stack_top_ - arg_count - 1);
  return true;
}

bool VM::call_value(const Value& callee, int arg_count) noexcept {
  if (callee.is_obj()) {
    switch (callee.as_obj()->type()) {
    case ObjType::OBJ_CLOSURE: return call(callee.as_closure(), arg_count);
    case ObjType::OBJ_NATIVE:
      {
        NativeFn native = callee.as_native()->function();
        Value result = native(arg_count, stack_top_ - arg_count);
        stack_top_ -= arg_count + 1;
        push(result);
      } break;
    default: break; // Non-callable object type.
    }
  }

  runtime_error("can only call functions and classes");
  return false;
}

ObjUpvalue* VM::capture_upvalue(Value* local) noexcept {
  ObjUpvalue* prev_upvalue = nullptr;
  ObjUpvalue* upvalue = open_upvalues_;
  while (upvalue != nullptr && upvalue->location() > local) {
    prev_upvalue = upvalue;
    upvalue = upvalue->next();
  }

  if (upvalue != nullptr && upvalue->location() == local)
    return upvalue;

  ObjUpvalue* created_upvalue = ObjUpvalue::create(local);
  return created_upvalue;
}

void VM::runtime_error(const char* format, ...) noexcept {
  va_list args;
  va_start(args, format);
  std::vfprintf(stderr, format, args);
  va_end(args);
  std::cerr << std::endl;

  for (int i = frame_count_ - 1; i >= 0; --i) {
    CallFrame* frame = &frames_[i];
    ObjFunction* function = frame->closure->function();
    sz_t instruction = function->chunk()->offset_from(frame->ip) - 1; // frame->ip - function->chunk()->codes() - 1
    std::cerr << "[Line " << function->chunk()->get_line(instruction) << "] in ";
    if (function->name() == nullptr) {
      std::cerr << "script" << std::endl;
    }
    else {
      std::cerr << function->name_ascstr() << "()" << std::endl;
    }
  }

  reset_stack();
}

void VM::define_native(const str_t& name, NativeFn&& function) noexcept {
  push(ObjString::create(name));
  push(ObjNative::create(std::move(function)));
  globals_[name] = stack_[1];
  pop();
  pop();
}

void VM::free_object(Obj* o) noexcept {
  delete o;
}

InterpretResult VM::interpret(const str_t& source) noexcept {
  GlobalCompiler compiler;

  ObjFunction* function = compiler.compile(*this, source);
  if (function == nullptr)
    return InterpretResult::INTERPRET_COMPILE_ERROR;

  push(function);
  ObjClosure* closure = ObjClosure::create(function);
  pop();
  push(closure);
  call(closure, 0);

  return run();
}

void VM::append_object(Obj* o) noexcept {
  objects_.push_back(o);
}

void VM::free_objects() noexcept {
  for (auto it = objects_.begin(); it != objects_.end();) {
    free_object(*it);
    objects_.erase(it++);
  }
}

InterpretResult VM::run() noexcept {
  CallFrame* frame = &frames_[frame_count_ - 1];

#define READ_BYTE()     (*frame->ip++)
#define READ_SHORT()    (frame->ip += 2, as_type<u16_t>((frame->ip[-2] << 8) | frame->ip[-1]))
#define READ_CONSTANT() (frame->closure->function()->chunk()->get_constant(READ_BYTE()))
#define READ_STRING()   READ_CONSTANT().as_string()
#define READ_CSTRING()  READ_CONSTANT().as_cstring()
#define BINARY_OP(op)\
  do {\
    if (!peek(0).is_number() || !peek(1).is_number()) {\
      runtime_error("Operands must be numbers.");\
      return InterpretResult::INTERPRET_RUNTIME_ERROR;\
    }\
    double b = pop().as_number();\
    double a = pop().as_number();\
    push(a op b);\
  } while (false)

  for (;;) {
#if defined(_CLOX_DEBUG_TRACE_EXECUTION)
    std::cout << "          ";
    for (Value* slot = stack_; slot < stack_top_; ++slot) {
      std::cout << "[ " << *slot << " ]";
    }
    std::cout << std::endl;
    frame->closure->function()->chunk()->dis_code(
        as_type<sz_t>(frame->ip - frame->closure->function()->chunk()->codes()));
#endif

    switch (OpCode instruction = as_type<OpCode>(READ_BYTE())) {
    case OpCode::OP_CONSTANT:
      {
        Value constant = READ_CONSTANT();
        push(constant);
      } break;
    case OpCode::OP_NIL: push(nullptr); break;
    case OpCode::OP_TRUE: push(true); break;
    case OpCode::OP_FALSE: push(false); break;
    case OpCode::OP_POP: pop(); break;
    case OpCode::OP_GET_LOCAL:
      {
        u8_t slot = READ_BYTE();
        push(frame->slots[slot]);
      } break;
    case OpCode::OP_SET_LOCAL:
      {
        u8_t slot = READ_BYTE();
        frame->slots[slot] = peek();
      } break;
    case OpCode::OP_GET_GLOBAL:
      {
        cstr_t name = READ_CSTRING();
        if (auto it = globals_.find(name); it != globals_.end()) {
          push(it->second);
        }
        else {
          runtime_error("`%s` is not defined", name);
          return InterpretResult::INTERPRET_RUNTIME_ERROR;
        }
      } break;
    case OpCode::OP_DEFINE_GLOBAL:
      {
        cstr_t name = READ_CSTRING();
        if (auto it = globals_.find(name); it != globals_.end()) {
          runtime_error("name `%s` is redefined", name);
          return InterpretResult::INTERPRET_RUNTIME_ERROR;
        }
        else {
          globals_[name] = pop();
        }
      } break;
    case OpCode::OP_SET_GLOBAL:
      {
        cstr_t name = READ_CSTRING();
        if (auto it = globals_.find(name); it != globals_.end()) {
          globals_[name] = peek();
        }
        else {
          runtime_error("name `%s` is not defined", name);
          return InterpretResult::INTERPRET_RUNTIME_ERROR;
        }
      } break;
    case OpCode::OP_GET_UPVALUE:
      {
        u8_t slot = READ_BYTE();
        push(*frame->closure->get_upvalue(slot)->location());
      } break;
    case OpCode::OP_SET_UPVALUE:
      {
        u8_t slot = READ_BYTE();
        frame->closure->get_upvalue(slot)->set_location(peek(0));
      } break;
    case OpCode::OP_EQUAL:
      {
        Value b = pop();
        Value a = pop();
        push(a == b);
      } break;
    case OpCode::OP_GREATER: BINARY_OP(>); break;
    case OpCode::OP_LESS: BINARY_OP(<); break;
    case OpCode::OP_ADD:
      if (peek(0).is_string() && peek(1).is_string()) {
        ObjString* b = peek(0).as_string();
        ObjString* a = peek(1).as_string();
        ObjString* s = ObjString::concat(a, b);
        pop();
        pop();
        push(s);
      }
      else if (peek(0).is_number() && peek(1).is_number()) {
        double b = pop().as_number();
        double a = pop().as_number();
        push(a + b);
      }
      else {
        runtime_error("Operands must be two numbers or two strings.");
        return InterpretResult::INTERPRET_RUNTIME_ERROR;
      }
      break;
    case OpCode::OP_SUBTRACT: BINARY_OP(-); break;
    case OpCode::OP_MULTIPLY: BINARY_OP(*); break;
    case OpCode::OP_DIVIDE: BINARY_OP(/); break;
    case OpCode::OP_NOT: push(pop().is_falsey()); break;
    case OpCode::OP_NEGATE:
      {
        if (!peek().is_number()) {
          runtime_error("Operand must be a number.");
          return InterpretResult::INTERPRET_RUNTIME_ERROR;
        }
        push(-pop().as_number());
      } break;
    case OpCode::OP_PRINT:
      {
        std::cout << pop() << std::endl;
      } break;
    case OpCode::OP_JUMP:
      {
        u16_t offset = READ_SHORT();
        frame->ip += offset;
      } break;
    case OpCode::OP_JUMP_IF_FALSE:
      {
        u16_t offset = READ_SHORT();
        if (peek().is_falsey())
          frame->ip += offset;
      } break;
    case OpCode::OP_LOOP:
      {
        u16_t offset = READ_SHORT();
        frame->ip -= offset;
      } break;
    case OpCode::OP_CALL:
      {
        int arg_count = READ_BYTE();
        if (!call_value(peek(arg_count), arg_count)) {
          return InterpretResult::INTERPRET_RUNTIME_ERROR;
        }
        frame = &frames_[frame_count_ - 1];
      } break;
    case OpCode::OP_CLOSURE:
      {
        ObjFunction* function = READ_CONSTANT().as_function();
        ObjClosure* closure = ObjClosure::create(function);
        push(closure);

        for (int i = 0; i < closure->upvalue_count(); ++i) {
          u8_t is_local = READ_BYTE();
          u8_t index = READ_BYTE();
          if (is_local) {
            closure->set_upvalue(i, capture_upvalue(frame->slots + index));
          }
          else {
            closure->set_upvalue(i, frame->closure->get_upvalue(index));
          }
        }
      } break;
    case OpCode::OP_RETURN:
      {
        Value result = pop();
        --frame_count_;
        if (frame_count_ == 0) {
          pop();
          return InterpretResult::INTERPRET_OK;
        }

        stack_top_ = frame->slots;
        push(result);
        frame = &frames_[frame_count_ - 1];
      } break;
    }
  }

#undef BINARY_OP
#undef READ_CSTRING
#undef READ_STRING
#undef READ_CONSTANT
#undef READ_SHORT
#undef READ_BYTE
  return InterpretResult::INTERPRET_OK;
}

}
