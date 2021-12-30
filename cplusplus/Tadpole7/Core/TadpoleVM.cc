// Copyright (c) 2021 ASMlover. All rights reserved.
//
//  ______             __                  ___
// /\__  _\           /\ \                /\_ \
// \/_/\ \/    __     \_\ \  _____     ___\//\ \      __
//    \ \ \  /'__`\   /'_` \/\ '__`\  / __`\\ \ \   /'__`\
//     \ \ \/\ \L\.\_/\ \L\ \ \ \L\ \/\ \L\ \\_\ \_/\  __/
//      \ \_\ \__/.\_\ \___,_\ \ ,__/\ \____//\____\ \____\
//       \/_/\/__/\/_/\/__,_ /\ \ \/  \/___/ \/____/\/____/
//                             \ \_\
//                              \/_/
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
#include <iostream>
#include <Common/Colorful.hh>
#include <Core/Chunk.hh>
#include <GC/GC.hh>
#include <Object/NativeObject.hh>
#include <Core/CallFrame.hh>
#include <Compiler/Compiler.hh>
#include <Builtin/Builtins.hh>
#include <Core/TadpoleVM.hh>

namespace Tadpole::Core {

TadpoleVM::TadpoleVM() noexcept {
  gcompiler_ = new Compiler::GlobalCompiler();
  stack_.reserve(kDefaultCap);

  Builtin::register_builtins(*this);
  GC::GC::get_instance().append_roots("TadpoleVM", this);
}

TadpoleVM::~TadpoleVM() {
  delete gcompiler_;
  globals_.clear();
}

void TadpoleVM::define_native(const str_t& name, Value::TadpoleCFun&& fn) {
  globals_[name] = Object::NativeObject::create(std::move(fn));
}

InterpretRet TadpoleVM::interpret(const str_t& source_bytes) {
  Object::FunctionObject* fn = gcompiler_->compile(*this, source_bytes);
  if (fn == nullptr)
    return InterpretRet::ECOMPILE;

  // TODO:
  return InterpretRet::OK;
}

void TadpoleVM::iter_objects(Object::ObjectVisitor&& visitor) {
  // iterate Tadpole objects roots
  gcompiler_->iter_objects(std::move(visitor));
  for (auto& v : stack_)
    visitor(v.as_object(safe_t()));
  for (auto& f : frames_)
    visitor(f.closure());
  for (auto& g : globals_)
    visitor(g.second.as_object(safe_t()));
  for (auto* u = open_upvalues_; u != nullptr; u = u->next())
    visitor(u);
}

void TadpoleVM::reset() {
  stack_.clear();
  frames_.clear();
  open_upvalues_ = nullptr;
}

void TadpoleVM::runtime_error(const char* format, ...) {
  std::cerr << Common::Colorful::fg::red << "Traceback (most recent call last):" << std::endl;
  for (auto it = frames_.rbegin(); it != frames_.rend(); ++it) {
    auto& frame = *it;

    sz_t i = frame.frame_chunk()->offset_with(frame.ip()) - 1;
    std::cerr
      << "  [LINE: " << frame.frame_chunk()->get_line(i) << "] in "
      << "`" << frame.frame_fn()->name_asstr() << "()`"
      << std::endl;
  }

  va_list ap;
  va_start(ap, format);
  std::vfprintf(stderr, format, ap);
  va_end(ap);
  std::cerr << Common::Colorful::reset << std::endl;

  reset();
}

void TadpoleVM::push(const Value::Value& value) noexcept {
  stack_.push_back(value);
}

Value::Value TadpoleVM::pop() noexcept {
  Value::Value value = stack_.back();
  stack_.pop_back();
  return value;
}

const Value::Value& TadpoleVM::peek(sz_t distance) const noexcept {
  return stack_[stack_.size() - 1 - distance];
}

bool TadpoleVM::call(Object::ClosureObject* closure, sz_t nargs) {
  Object::FunctionObject* fn = closure->fn();
  if (fn->arity() != nargs) {
    runtime_error("%s() takes exactly %u arguments (%u given)",
        fn->name_asstr(), fn->arity(), nargs);
    return false;
  }

  frames_.push_back({closure, fn->chunk()->codes(), stack_.size() - nargs - 1});
  return true;
}

bool TadpoleVM::call(const Value::Value& callee, sz_t nargs) {
  if (callee.is_object()) {
    switch (callee.objtype()) {
    case Object::ObjType::NATIVE:
      {
        Value::Value* args{};
        if (nargs > 0 && stack_.size() > nargs)
          args = &stack_[stack_.size() - nargs];

        Value::Value result = callee.as_native()->call(nargs, args);
        stack_.resize(stack_.size() - nargs - 1);
        push(result);

        return true;
      }
    case Object::ObjType::CLOSURE: return call(callee.as_closure(), nargs);
    }
  }

  runtime_error("can only call function");
  return false;
}

Object::UpvalueObject* TadpoleVM::capture_upvalue(Value::Value* local) {
  if (open_upvalues_ == nullptr) {
    open_upvalues_ = Object::UpvalueObject::create(local);
    return open_upvalues_;
  }

  Object::UpvalueObject* upvalue = open_upvalues_;
  Object::UpvalueObject* prev_upvalue = nullptr;
  while (upvalue != nullptr && upvalue->value() > local) {
    prev_upvalue = upvalue;
    upvalue = upvalue->next();
  }
  if (upvalue != nullptr && upvalue->value() == local)
    return upvalue;

  Object::UpvalueObject* new_upvate = Object::UpvalueObject::create(local, upvalue);
  if (prev_upvalue == nullptr)
    open_upvalues_ = new_upvate;
  else
    prev_upvalue->set_next(new_upvate);
  return new_upvate;
}

void TadpoleVM::close_upvalues(Value::Value* last) {
  while (open_upvalues_ != nullptr && open_upvalues_->value() >= last) {
    Object::UpvalueObject* upvalue = open_upvalues_;
    upvalue->set_closed(upvalue->value_asref());
    upvalue->set_value(upvalue->closed_asptr());

    open_upvalues_ = upvalue->next();
  }
}

InterpretRet TadpoleVM::run() {
  CallFrame* frame =  &frames_.back();

#define _RDBYTE()   frame->inc_ip()
#define _RDCONST()  frame->frame_chunk()->get_constant(_RDBYTE())
#define _RDSTR()    _RDCONST().as_string()
#define _RDCSTR()   _RDCONST().as_cstring()
#define _BINOP(op)  do {\
    if (!peek(0).is_numeric() || !peek(1).is_numeric()) {\
      runtime_error("operands must be two numerics");\
      return InterpretRet::ERUNTIME;\
    }\
    double b = pop().as_numeric();\
    double a = pop().as_numeric();\
    push(a op b);\
  } while (false)

  for (;;) {
#if defined(_TADPOLE_DEBUG_VM)
    auto* frame_chunk = frame->frame_chunk();
    std::cout << "          ";
    for (auto& v : stack_)
      std::cout << "[" << Common::Colorful::fg::magenta << v << Common::Colorful::reset << "]";
    std::cout << std::endl;
    frame_chunk->dis_code(frame_chunk->offset_with(frame->ip()));
#endif

    switch (Code c = Common::as_type<Code>(_RDBYTE())) {
    case Code::CONSTANT: push(_RDCONST()); break;
    case Code::NIL: push(nullptr); break;
    case Code::FALSE: push(false); break;
    case Code::TRUE: push(true); break;
    case Code::POP: pop(); break;
    case Code::DEF_GLOBAL:
      {
        const char* name = _RDCSTR();
        if (auto it = globals_.find(name); it != globals_.end()) {
          runtime_error("name `%s` is redefined", name);
          return InterpretRet::ERUNTIME;
        }
        else {
          globals_[name] = pop();
        }
      } break;
    case Code::GET_GLOBAL:
      {
        const char* name = _RDCSTR();
        if (auto it = globals_.find(name); it != globals_.end()) {
          push(it->second);
        }
        else {
          runtime_error("name `%s` is not defined", name);
          return InterpretRet::ERUNTIME;
        }
      } break;
    case Code::SET_GLOBAL:
      {
        const char* name = _RDCSTR();
        if (auto it = globals_.find(name); it != globals_.end()) {
          it->second = peek();
        }
        else {
          runtime_error("name `%s` is not defined", name);
          return InterpretRet::ERUNTIME;
        }
      } break;
    case Code::GET_LOCAL: push(stack_[frame->stack_begpos() + _RDBYTE()]); break;
    case Code::SET_LOCAL: stack_[frame->stack_begpos() + _RDBYTE()] = peek(); break;
    case Code::GET_UPVALUE:
      {
        u8_t slot = _RDBYTE();
        push(frame->closure()->get_upvalue(slot)->value_asref());
      } break;
    case Code::SET_UPVALUE:
      {
        u8_t slot = _RDBYTE();
        frame->closure()->get_upvalue(slot)->set_value(peek());
      } break;
    case Code::ADD:
      {
        if (peek(0).is_string() && peek(1).is_string()) {
          Object::StringObject* b = peek(0).as_string();
          Object::StringObject* a = peek(1).as_string();
          Object::StringObject* s = Object::StringObject::concat(a, b);
          pop(); // pop b
          pop(); // pop a
          push(s);
        }
        else if (peek(0).is_numeric() && peek(1).is_numeric()) {
          double b = pop().as_numeric();
          double a = pop().as_numeric();
          push(a + b);
        }
        else {
          runtime_error("operands must be two strings or two numerics");
          return InterpretRet::ERUNTIME;
        }
      } break;
    case Code::SUB: _BINOP(-); break;
    case Code::MUL: _BINOP(*); break;
    case Code::DIV: _BINOP(/); break;
    default: break;
    }
  }

#undef _BINOP
#undef _RDCSTR
#undef _RDSTR
#undef _RDCONST
#undef _RDBYTE

  return InterpretRet::OK;
}

}
