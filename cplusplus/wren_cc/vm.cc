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
#include <iostream>
#include <sstream>
#include "core.hh"
#include "compiler.hh"
#include "vm.hh"

namespace wrencc {

int SymbolTable::ensure(const str_t& name) {
  int existing = get(name);
  if (existing != -1)
    return existing;

  symbols_.push_back(name);
  return Xt::as_type<int>(symbols_.size() - 1);
}

int SymbolTable::add(const str_t& name) {
  if (get(name) != -1)
    return -1;

  symbols_.push_back(name);
  return Xt::as_type<int>(symbols_.size() - 1);
}

int SymbolTable::get(const str_t& name) const {
  for (sz_t i = 0; i < symbols_.size(); ++i) {
    if (symbols_[i] == name)
      return Xt::as_type<int>(i);
  }
  return -1;
}

void SymbolTable::clear(void) {
  symbols_.clear();
}

void SymbolTable::truncate(int count) {
  auto n = Xt::as_type<sz_t>(count);
  ASSERT(n <= symbols_.size(), "cannot truncate to larger size");
  symbols_.resize(n);
}

struct CallFrame {
  int ip{};
  FunctionObject* fn{};
  int stack_start{};

  CallFrame(int ip_arg, FunctionObject* fn_arg, int stack_start_arg) noexcept
    : ip(ip_arg), fn(fn_arg), stack_start(stack_start_arg) {
  }

  inline u8_t get_code(int i) const { return fn->get_code(i); }
  inline const Value& get_constant(int i) const { return fn->get_constant(i); }
};

class Fiber : private UnCopyable {
  std::vector<Value> stack_;
  std::vector<CallFrame> frames_;
public:
  inline void reset(void) {
    stack_.clear();
    frames_.clear();
  }

  inline Value* values_at(int i) { return &stack_[i]; }
  inline void resize_stack(int n) { stack_.resize(n); }
  inline int stack_size(void) const { return Xt::as_type<int>(stack_.size()); }
  inline int frame_size(void) const { return Xt::as_type<int>(frames_.size()); }
  inline CallFrame& peek_frame(void) { return frames_.back(); }
  inline void pop_frame(void) { frames_.pop_back(); }
  inline bool empty_frame(void) const { return frames_.empty(); }
  inline const Value& get_value(int i) const { return stack_[i]; }
  inline void set_value(int i, const Value& v) { stack_[i] = v; }

  inline const Value& peek_value(int distance = 0) const {
    return stack_[stack_.size() - 1 - distance];
  }

  inline void push(const Value& v) {
    stack_.push_back(v);
  }

  inline Value pop(void) {
    Value v = stack_.back();
    stack_.pop_back();
    return v;
  }

  void call_function(FunctionObject* fn, int argc = 0) {
    frames_.push_back(CallFrame(0, fn, stack_size() - argc));
  }

  void iter_stacks(std::function<void (const Value&)>&& visit) {
    for (auto& v : stack_)
      visit(v);
  }

  void iter_frames(std::function<void (const CallFrame&)>&& visit) {
    for (auto& c : frames_)
      visit(c);
  }
};

/// WrenVM IMPLEMENTATIONS

WrenVM::WrenVM(void) noexcept {
  fiber_ = new Fiber();

  globals_.resize(kMaxGlobals);
  for (int i = 0; i < kMaxGlobals; ++i)
    globals_[i] = nullptr;

  initialize_core(*this);
}

WrenVM::~WrenVM(void) {
  delete fiber_;
}

void WrenVM::set_native(ClassObject* cls, const str_t& name, PrimitiveFn fn) {
  int symbol = methods_.ensure(name);
  cls->set_method(symbol, fn);
}

void WrenVM::set_native(
    ClassObject* cls, const str_t& name, FiberPrimitiveFn fn) {
  int symbol = methods_.ensure(name);
  cls->set_method(symbol, fn);
}

void WrenVM::set_global(const str_t& name, const Value& value) {
  int symbol = global_symbols_.add(name);
  globals_[symbol] = value;
}

const Value& WrenVM::get_global(const str_t& name) const {
  int symbol = global_symbols_.get(name);
  return globals_[symbol];
}

void WrenVM::pin_object(BaseObject* obj, Pinned* pinned) {
  pinned->obj = obj;
  pinned->prev = pinned_;
  pinned_ = pinned;
}

void WrenVM::unpin_object(void) {
  pinned_ = pinned_->prev;
}

Value WrenVM::interpret(FunctionObject* fn) {
  Fiber* fiber = fiber_;
  fiber->call_function(fn, 0);

#define PUSH(v) fiber->push(v)
#define POP()   fiber->pop()
#define PEEK()  fiber->peek_value()
#define RDARG() frame->get_code(frame->ip++)

  auto* frame = &fiber->peek_frame();

// use this after a CallFrame has been pushed or popped ti refresh the
// local variables
#define LOAD_FRAME()  frame = &fiber->peek_frame()

#if defined(COMPUTED_GOTOS)
  static void* _dispatch_table[] = {
# undef CODEF
# define CODEF(c) &&__code_##c,
# include "codes_def.hh"
  };
# define INTERPRET_LOOP() DISPATCH();
# define CASE_CODE(name)  __code_##name
# define DISPATCH()       goto *_dispatch_table[Xt::as_type<int>(c = Xt::as_type<Code>(frame->get_code(frame->ip++)))]
#else
# define INTERPRET_LOOP() for (;;) switch (c = Xt::as_type<Code>(frame->get_code(frame->ip++)))
# define CASE_CODE(name)  case Code::##name
# define DISPATCH()       break
#endif

  Code c;
  INTERPRET_LOOP() {
    CASE_CODE(CONSTANT): PUSH(frame->get_constant(RDARG())); DISPATCH();
    CASE_CODE(NIL): PUSH(nullptr); DISPATCH();
    CASE_CODE(FALSE): PUSH(false); DISPATCH();
    CASE_CODE(TRUE): PUSH(true); DISPATCH();
    CASE_CODE(CLASS):
    CASE_CODE(SUBCLASS):
    {
      bool is_subclass = c == Code::SUBCLASS;
      int num_fields = RDARG();

      ClassObject* superclass;
      if (is_subclass)
        superclass = POP().as_class();
      else
        superclass = obj_class_;

      ClassObject* cls = ClassObject::make_class(*this, superclass, num_fields);

      // assume the first class being defined is Object
      if (obj_class_ == nullptr)
        obj_class_ = cls;

      // define `new` method on the metaclass.
      int new_symbol = methods_.ensure("new");
      cls->meta_class()->set_method(new_symbol, MethodType::CTOR, nullptr);
      PUSH(cls);

      DISPATCH();
    }
    CASE_CODE(METHOD_INSTANCE):
    CASE_CODE(METHOD_STATIC):
    CASE_CODE(METHOD_CTOR):
    {
      Code type = c;
      int symbol = RDARG();
      Value method = POP();
      ClassObject* cls = PEEK().as_class();

      FunctionObject* body_fn = method.as_function();
      switch (type) {
      case Code::METHOD_INSTANCE:
        cls->set_method(symbol, MethodType::BLOCK, body_fn); break;
      case Code::METHOD_STATIC:
        cls->meta_class()->set_method(symbol, MethodType::BLOCK, body_fn); break;
      case Code::METHOD_CTOR:
        cls->meta_class()->set_method(symbol, MethodType::CTOR, body_fn); break;
      }

      DISPATCH();
    }
    CASE_CODE(LIST):
    {
      int num_elements = RDARG();
      ListObject* list = ListObject::make_list(*this, num_elements);
      for (int i = 0; i < num_elements; ++i) {
        list->set_element(i,
            fiber->get_value(fiber->stack_size() - num_elements + i));
      }
      // discard the elements
      fiber->resize_stack(fiber->stack_size() - num_elements);

      PUSH(list);

      DISPATCH();
    }
    CASE_CODE(LOAD_LOCAL):
    {
      int local = RDARG();
      PUSH(fiber->get_value(frame->stack_start + local));

      DISPATCH();
    }
    CASE_CODE(STORE_LOCAL):
    {
      int local = RDARG();
      fiber->set_value(frame->stack_start + local, PEEK());

      DISPATCH();
    }
    CASE_CODE(LOAD_GLOBAL):
    {
      int global = RDARG();
      PUSH(globals_[global]);

      DISPATCH();
    }
    CASE_CODE(STORE_GLOBAL):
    {
      int global = RDARG();
      globals_[global] = PEEK();

      DISPATCH();
    }
    CASE_CODE(LOAD_FIELD):
    {
      int field = RDARG();
      const Value& receiver = fiber->get_value(frame->stack_start);
      ASSERT(receiver.is_instance(), "receiver should be instance");
      InstanceObject* inst = receiver.as_instance();
      ASSERT(field < inst->cls()->num_fields(), "out of bounds field");
      PUSH(inst->get_field(field));

      DISPATCH();
    }
    CASE_CODE(STORE_FIELD):
    {
      int field = RDARG();
      const Value& receiver = fiber->get_value(frame->stack_start);
      ASSERT(receiver.is_instance(), "receiver should be instance");
      InstanceObject* inst = receiver.as_instance();
      ASSERT(field < inst->cls()->num_fields(), "out of bounds field");
      inst->set_field(field, PEEK());

      DISPATCH();
    }
    CASE_CODE(DUP): PUSH(PEEK()); DISPATCH();
    CASE_CODE(POP): POP(); DISPATCH();
    CASE_CODE(CALL_0):
    CASE_CODE(CALL_1):
    CASE_CODE(CALL_2):
    CASE_CODE(CALL_3):
    CASE_CODE(CALL_4):
    CASE_CODE(CALL_5):
    CASE_CODE(CALL_6):
    CASE_CODE(CALL_7):
    CASE_CODE(CALL_8):
    CASE_CODE(CALL_9):
    CASE_CODE(CALL_10):
    CASE_CODE(CALL_11):
    CASE_CODE(CALL_12):
    CASE_CODE(CALL_13):
    CASE_CODE(CALL_14):
    CASE_CODE(CALL_15):
    CASE_CODE(CALL_16):
    {
      int argc = c - Code::CALL_0 + 1;
      int symbol = RDARG();
      const Value& receiver = fiber->get_value(fiber->stack_size() - argc);

      ClassObject* cls = get_class(receiver);
      auto& method = cls->get_method(symbol);
      switch (method.type) {
      case MethodType::NONE:
        std::cerr
          << "receiver: `" << receiver << "` "
          << "does not implement method `" << methods_.get_name(symbol) << "`"
          << std::endl;
        std::exit(-1); break;
      case MethodType::PRIMITIVE:
        {
          Value* args = fiber->values_at(fiber->stack_size() - argc);
          // argc +1 to include the receiver since that's in the args array
          Value result = method.primitive(*this, args);

          fiber->set_value(fiber->stack_size() - argc, result);
          // discard the stack slots for the arguments (but leave one for
          // the result)
          fiber->resize_stack(fiber->stack_size() - (argc - 1));
        } break;
      case MethodType::FIBER:
        {
          Value* args = fiber->values_at(fiber->stack_size() - argc);
          method.fiber_primitive(*this, *fiber, args);
          LOAD_FRAME();
        } break;
      case MethodType::BLOCK:
        fiber->call_function(method.fn, argc);
        LOAD_FRAME();
        break;
      case MethodType::CTOR:
        {
          Value instance =
            InstanceObject::make_instance(*this, receiver.as_class());

          // store the new instance in the receiver slot so that it can
          // be `this` in the body of the constructor and returned by it
          fiber->set_value(fiber->stack_size() - argc, instance);
          if (method.fn == nullptr) {
            // default constructor, so no body to call, just discard the
            // stack slots for the arguments (but leave one for the instance)
            fiber->resize_stack(fiber->stack_size() - (argc - 1));
          }
          else {
            // invoke the constructor body
            fiber->call_function(method.fn, argc);
            LOAD_FRAME();
          }
        } break;
      }

      DISPATCH();
    }
    CASE_CODE(JUMP): frame->ip += RDARG(); DISPATCH();
    CASE_CODE(LOOP):
    {
      // the loop body's result is on the top of the stack, since we are
      // looping and running the body again, discard it
      POP();

      // jump back to the top of the loop
      frame->ip -= RDARG();

      DISPATCH();
    }
    CASE_CODE(JUMP_IF):
    {
      int offset = RDARG();
      Value cond = POP();

      if (cond.is_falsely())
        frame->ip += offset;

      DISPATCH();
    }
    CASE_CODE(AND):
    {
      int offset = RDARG();
      const Value& cond = PEEK();

      // false and nil is falsely value
      if (!cond.is_falsely())
        POP(); // discard the condition and evaluate the right hand side
      else
        frame->ip += offset; // short-circuit the right hand side

      DISPATCH();
    }
    CASE_CODE(OR):
    {
      int offset = RDARG();
      const Value& cond = PEEK();

      // false and nil is falsely value
      if (cond.is_falsely())
        POP(); // discard the condition and evaluate the right hand side
      else
        frame->ip += offset; // short-circuit the right hand side

      DISPATCH();
    }
    CASE_CODE(IS):
    {
      ClassObject* expected = POP().as_class();
      Value obj = POP();

      ClassObject* actual = get_class(obj);
      bool is_instance = false;

      // walk the superclass chain looking for the class
      while (actual != nullptr) {
        if (actual == expected) {
          is_instance = true;
          break;
        }
        actual = actual->superclass();
      }
      PUSH(is_instance);

      DISPATCH();
    }
    CASE_CODE(END):
    {
      Value r = POP();
      fiber->pop_frame();

      if (fiber->empty_frame())
        return r;

      if (fiber->stack_size() <= frame->stack_start)
        PUSH(r);
      else
        fiber->set_value(frame->stack_start, r);
      fiber->resize_stack(frame->stack_start + 1);

      LOAD_FRAME();

      DISPATCH();
    }
  }
  return nullptr;
}

ClassObject* WrenVM::get_class(const Value& val) const {
#ifdef NAN_TAGGING
  if (val.is_numeric())
    return num_class_;
  if (val.is_object()) {
    switch (val.objtype()) {
    case ObjType::STRING: return str_class_;
    case ObjType::LIST: return list_class_;
    case ObjType::FUNCTION: return fn_class_;
    case ObjType::CLASS: return val.as_class()->meta_class();
    case ObjType::INSTANCE: return val.as_instance()->cls();
    }
  }
  switch (val.tag()) {
  case Tag::NaN: return num_class_;
  case Tag::NIL: return nil_class_;
  case Tag::TRUE:
  case Tag::FALSE: return bool_class_;
  }
#else
  switch (val.type()) {
  case ValueType::NIL: return nil_class_;
  case ValueType::TRUE:
  case ValueType::FALSE: return bool_class_;
  case ValueType::NUMERIC: return num_class_;
  case ValueType::OBJECT:
    switch (val.objtype()) {
    case ObjType::STRING: return str_class_;
    case ObjType::LIST: return list_class_;
    case ObjType::FUNCTION: return fn_class_;
    case ObjType::CLASS: return val.as_class()->meta_class();
    case ObjType::INSTANCE: return val.as_instance()->cls();
    }
    break;
  }
#endif
  return nullptr;
}

void WrenVM::interpret(const str_t& source_bytes) {
  auto* fn = compile(*this, source_bytes);

  if (fn != nullptr)
    interpret(fn);
}

void WrenVM::call_function(Fiber& fiber, FunctionObject* fn, int argc) {
  fiber.call_function(fn, argc);
}

void WrenVM::collect(void) {
  // global variables
  for (int i = 0; i < global_symbols_.count(); ++i)
    mark_value(globals_[i]);
  // pinned objects
  for (auto* p = pinned_; p != nullptr; p = p->prev)
    mark_object(p->obj);
  // stack functions
  fiber_->iter_frames([this](const CallFrame& f) { mark_object(f.fn); });
  // stack variables
  fiber_->iter_stacks([this](const Value& v) { mark_value(v); });

  // collect any unmarked objects
  for (auto it = objects_.begin(); it != objects_.end();) {
    if (!((*it)->flag() & ObjFlag::MARKED)) {
      free_object(*it);
      objects_.erase(it++);
    }
    else {
      ObjFlag f = (*it)->flag();
      (*it)->set_flag(f & ~ObjFlag::MARKED);
      ++it;
    }
  }
  total_allocated_ = objects_.size();
}

void WrenVM::free_object(BaseObject* obj) {
  std::cout
    << "`" << Xt::cast<void>(obj) << "` free object "
    << "`" << obj->stringify() << "`" << std::endl;

  delete obj;
}

void WrenVM::append_object(BaseObject* obj) {
  if (objects_.size() > next_gc_) {
    collect();
    next_gc_ = total_allocated_ * 3 / 2;
  }

  objects_.push_back(obj);
}

void WrenVM::mark_object(BaseObject* obj) {
  if (obj == nullptr)
    return;
  if (obj->flag() & ObjFlag::MARKED)
    return;

  obj->set_flag(obj->flag() | ObjFlag::MARKED);

  obj->gc_mark(*this);
}

void WrenVM::mark_value(const Value& val) {
  if (val.is_object())
    mark_object(val.as_object());
}

}
