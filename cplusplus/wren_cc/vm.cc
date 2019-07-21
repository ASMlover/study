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

/// WrenVM IMPLEMENTATIONS

WrenVM::WrenVM(void) noexcept {
  globals_.resize(kMaxGlobals);
  for (int i = 0; i < kMaxGlobals; ++i)
    globals_[i] = nullptr;

  initialize_core(*this);
}

WrenVM::~WrenVM(void) {
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

void WrenVM::call_foreign(FiberObject* fiber, const Method& method, int argc) {
  foreign_call_slot_ = fiber->values_at(fiber->stack_size() - argc);

  // donot include the receiver
  foreign_call_argc_ = argc - 1;
  method.native()(*this);

  // discard the stack slots for the arguments (but leave one for result)
  if (foreign_call_slot_ != nullptr) {
    *foreign_call_slot_ = nullptr;
    foreign_call_slot_ = nullptr;
  }
}

Value WrenVM::interpret(const Value& function, FiberObject* fiber) {
#define PUSH(v) fiber->push(v)
#define POP()   fiber->pop()
#define PEEK()  fiber->peek_value()
#define RDARG() (*frame->ip++)

  CallFrame* frame{};
  FunctionObject* fn{};
  ClosureObject* co{};

// use this after a CallFrame has been pushed or popped ti refresh the
// local variables
#define LOAD_FRAME()\
  frame = &fiber->peek_frame();\
  if (frame->fn->type() == ObjType::FUNCTION) {\
    fn = Xt::down<FunctionObject>(frame->fn);\
    co = nullptr;\
  }\
  else {\
    fn = Xt::down<ClosureObject>(frame->fn)->fn();\
    co = Xt::down<ClosureObject>(frame->fn);\
  }

#if defined(COMPUTED_GOTOS)
  static void* _dispatch_table[] = {
# undef CODEF
# define CODEF(c) &&__code_##c,
# include "codes_def.hh"
  };
# define INTERPRET_LOOP() DISPATCH();
# define CASE_CODE(name)  __code_##name
# define DISPATCH()       goto *_dispatch_table[Xt::as_type<int>(c = Xt::as_type<Code>(*frame->ip++))]
#else
# define INTERPRET_LOOP() for (;;) switch (c = Xt::as_type<Code>(*frame->ip++))
# define CASE_CODE(name)  case Code::##name
# define DISPATCH()       break
#endif

  LOAD_FRAME();

  Code c;
  INTERPRET_LOOP() {
    CASE_CODE(CONSTANT): PUSH(fn->get_constant(RDARG())); DISPATCH();
    CASE_CODE(NIL): PUSH(nullptr); DISPATCH();
    CASE_CODE(FALSE): PUSH(false); DISPATCH();
    CASE_CODE(TRUE): PUSH(true); DISPATCH();
    CASE_CODE(LOAD_LOCAL):
    {
      PUSH(fiber->get_value(frame->stack_start + RDARG()));

      DISPATCH();
    }
    CASE_CODE(STORE_LOCAL):
    {
      fiber->set_value(frame->stack_start + RDARG(), PEEK());

      DISPATCH();
    }
    CASE_CODE(LOAD_UPVALUE):
    {
      ASSERT(co->has_upvalues(),
          "should not have LOAD_UPVALUE instruction in non-closure");
      PUSH(co->get_upvalue(RDARG())->value_asref());

      DISPATCH();
    }
    CASE_CODE(STORE_UPVALUE):
    {
      ASSERT(co->has_upvalues(),
          "should not have STORE_UPVALUE instruction in non-closure");
      co->get_upvalue(RDARG())->set_value(POP());

      DISPATCH();
    }
    CASE_CODE(LOAD_GLOBAL):
    {
      PUSH(globals_[RDARG()]);

      DISPATCH();
    }
    CASE_CODE(STORE_GLOBAL):
    {
      globals_[RDARG()] = PEEK();

      DISPATCH();
    }
    CASE_CODE(LOAD_FIELD_THIS):
    {
      int field = RDARG();
      const Value& receiver = fiber->get_value(frame->stack_start);
      ASSERT(receiver.is_instance(), "receiver should be instance");
      InstanceObject* inst = receiver.as_instance();
      ASSERT(field < inst->cls()->num_fields(), "out of bounds field");
      PUSH(inst->get_field(field));

      DISPATCH();
    }
    CASE_CODE(STORE_FIELD_THIS):
    {
      int field = RDARG();
      const Value& receiver = fiber->get_value(frame->stack_start);
      ASSERT(receiver.is_instance(), "receiver should be instance");
      InstanceObject* inst = receiver.as_instance();
      ASSERT(field < inst->cls()->num_fields(), "out of bounds field");
      inst->set_field(field, PEEK());

      DISPATCH();
    }
    CASE_CODE(LOAD_FIELD):
    {
      int field = RDARG();
      Value receiver = POP();
      ASSERT(receiver.is_instance(), "receiver should be instance");
      InstanceObject* inst = receiver.as_instance();
      ASSERT(field < inst->cls()->num_fields(), "out of bounds field");
      PUSH(inst->get_field(field));

      DISPATCH();
    }
    CASE_CODE(STORE_FIELD):
    {
      int field = RDARG();
      Value receiver = POP();
      ASSERT(receiver.is_instance(), "receiver should be instance");
      InstanceObject* inst = receiver.as_instance();
      ASSERT(field < inst->cls()->num_fields(), "out of bounds field");
      inst->set_field(field, PEEK());

      DISPATCH();
    }
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
      case MethodType::PRIMITIVE:
        {
          Value* args = fiber->values_at(fiber->stack_size() - argc);
          // argc +1 to include the receiver since that's in the args array
          Value result = method.primitive()(*this, args);

          fiber->set_value(fiber->stack_size() - argc, result);
          // discard the stack slots for the arguments (but leave one for
          // the result)
          fiber->resize_stack(fiber->stack_size() - (argc - 1));
        } break;
      case MethodType::FIBER:
        {
          Value* args = fiber->values_at(fiber->stack_size() - argc);
          method.fiber_primitive()(*this, fiber, args);
          LOAD_FRAME();
        } break;
      case MethodType::FOREIGN:
        call_foreign(fiber, method, argc);
        break;
      case MethodType::BLOCK:
        fiber->call_function(method.fn(), argc);
        LOAD_FRAME();
        break;
      case MethodType::NONE:
        std::cerr
          << "receiver: `" << receiver << "` "
          << "does not implement method `" << methods_.get_name(symbol) << "`"
          << std::endl;
        std::exit(-1); break;
      }

      DISPATCH();
    }
    CASE_CODE(SUPER_0):
    CASE_CODE(SUPER_1):
    CASE_CODE(SUPER_2):
    CASE_CODE(SUPER_3):
    CASE_CODE(SUPER_4):
    CASE_CODE(SUPER_5):
    CASE_CODE(SUPER_6):
    CASE_CODE(SUPER_7):
    CASE_CODE(SUPER_8):
    CASE_CODE(SUPER_9):
    CASE_CODE(SUPER_10):
    CASE_CODE(SUPER_11):
    CASE_CODE(SUPER_12):
    CASE_CODE(SUPER_13):
    CASE_CODE(SUPER_14):
    CASE_CODE(SUPER_15):
    CASE_CODE(SUPER_16):
    {
      // add one for the implicit receiver argument
      int argc = c - Code::SUPER_0 + 1;
      int symbol = RDARG();

      const Value& receiver = fiber->get_value(fiber->stack_size() - argc);
      ClassObject* cls = get_class(receiver);

      // ignore methods defined on the receiver's immediate class
      cls  = cls->superclass();
      auto& method = cls->get_method(symbol);
      switch (method.type) {
      case MethodType::PRIMITIVE:
        {
          Value* args = fiber->values_at(fiber->stack_size() - argc);
          Value result = method.primitive()(*this, args);

          fiber->set_value(fiber->stack_size() - argc, result);

          // discard the stack slots for the arguments
          fiber->resize_stack(fiber->stack_size() - (argc - 1));
        } break;
      case MethodType::FIBER:
        {
          Value* args = fiber->values_at(fiber->stack_size() - argc);
          method.fiber_primitive()(*this, fiber, args);
          LOAD_FRAME();
        } break;
      case MethodType::FOREIGN:
        call_foreign(fiber, method, argc);
        break;
      case MethodType::BLOCK:
        fiber->call_function(method.fn(), argc);
        LOAD_FRAME();
        break;
      case MethodType::NONE:
        std::cerr
          << "receiver: `" << receiver << "` "
          << "does not implement method `" << methods_.get_name(symbol) << "`"
          << std::endl;
        std::exit(-1); break;
      }

      DISPATCH();
    }
    CASE_CODE(JUMP): frame->ip += RDARG(); DISPATCH();
    CASE_CODE(LOOP):
    {
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
    CASE_CODE(CLOSE_UPVALUE):
    {
      fiber->close_upvalue();
      POP();

      DISPATCH();
    }
    CASE_CODE(RETURN):
    {
      Value r = POP();
      fiber->pop_frame();

      if (fiber->empty_frame())
        return r;

      // close any upvalues still in scope
      fiber->close_upvalues(frame->stack_start);

      // store the result of the block in the first slot, which is where the
      // caller expects it
      if (fiber->stack_size() <= frame->stack_start)
        PUSH(r);
      else
        fiber->set_value(frame->stack_start, r);

      // discard the stack slots for the call frame
      fiber->resize_stack(frame->stack_start + 1);
      LOAD_FRAME();

      DISPATCH();
    }
    CASE_CODE(NEW):
    {
      // handle object not being a class
      ClassObject* cls = POP().as_class();
      PUSH(InstanceObject::make_instance(*this, cls));

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
    CASE_CODE(CLOSURE):
    {
      FunctionObject* prototype = fn->get_constant(RDARG()).as_function();
      ASSERT(prototype->num_upvalues() > 0,
          "should not create closure for functions that donot need it");

      // create the closure and push it on the stack before creating upvalues
      // so that it does not get collected
      ClosureObject* closure = ClosureObject::make_closure(*this, prototype);
      PUSH(closure);

      // capture upvalues
      for (int i = 0; i < prototype->num_upvalues(); ++i) {
        int is_local = RDARG();
        int index = RDARG();
        if (is_local) {
          // make an new upvalue to close over the parent's local variable
          closure->set_upvalue(i,
              fiber->capture_upvalue(*this, frame->stack_start + index));
        }
        else {
          // use the same upvalue as the current call frame
          closure->set_upvalue(i, co->get_upvalue(index));
        }
      }

      DISPATCH();
    }
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
      PUSH(cls);

      DISPATCH();
    }
    CASE_CODE(METHOD_INSTANCE):
    CASE_CODE(METHOD_STATIC):
    {
      int symbol = RDARG();
      Value method = POP();
      ClassObject* cls = PEEK().as_class();
      cls->bind_method(symbol, Xt::as_type<int>(c), method);

      DISPATCH();
    }
    CASE_CODE(END):
    {
      // a END should always preceded by a RETURN. if we get here, the compiler
      // generated wrong code
      ASSERT(false, "should not execute past end of bytecode");
    }
  }

  ASSERT(false, "should not reach end of interpret");
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
    case ObjType::UPVALUE:
      ASSERT(false, "upvalues should not be used as first-class objects");
      return nullptr;
    case ObjType::CLOSURE: return fn_class_;
    case ObjType::FIBER: return fiber_class_;
    case ObjType::CLASS: return val.as_class()->meta_class();
    case ObjType::INSTANCE: return val.as_instance()->cls();
    default: ASSERT(false, "unreachable"); return nullptr;
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
    case ObjType::UPVALUE:
      ASSERT(false, "upvalues should not be used as first-class objects");
      return nullptr;
    case ObjType::CLOSURE: return fn_class_;
    case ObjType::FIBER: return fiber_class_;
    case ObjType::CLASS: return val.as_class()->meta_class();
    case ObjType::INSTANCE: return val.as_instance()->cls();
    default: ASSERT(false, "unreachable"); return nullptr;
    }
    break;
  }
#endif
  return nullptr;
}

void WrenVM::interpret(const str_t& source_bytes) {
  FiberObject* fiber = FiberObject::make_fiber(*this);
  Pinned pinned;
  pin_object(fiber, &pinned);

  FunctionObject* fn = compile(*this, source_bytes);
  if (fn != nullptr) {
    fiber->call_function(fn, 0);
    interpret(fn, fiber);
  }

  unpin_object();
}

void WrenVM::call_function(FiberObject* fiber, BaseObject* fn, int argc) {
  fiber->call_function(fn, argc);
}

void WrenVM::collect(void) {
  // global variables
  for (int i = 0; i < global_symbols_.count(); ++i)
    mark_value(globals_[i]);
  // pinned objects
  for (auto* p = pinned_; p != nullptr; p = p->prev)
    mark_object(p->obj);

  // any object the compiler is using
  if (compiler_ != nullptr)
    mark_compiler(*this, compiler_);

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

void WrenVM::define_method(
    const str_t& class_name, const str_t& method_name,
    int num_params, const WrenNativeFn& method) {
  ASSERT(!class_name.empty(), "must provide class name");
  ASSERT(!method_name.empty(), "must provide method name");
  ASSERT(method_name.size() < MAX_METHOD_NAME, "method name too long");
  ASSERT(num_params >= 0, "num_params cannot be negative");
  ASSERT(num_params <= MAX_PARAMETERS, "too many parameters");

  // find or create the class to bind the method to
  int class_symbol = global_symbols_.get(class_name);
  ClassObject* cls;
  if (class_symbol != -1) {
    cls = globals_[class_symbol].as_class();
  }
  else {
    cls = ClassObject::make_class(*this, obj_class_, 0);
    class_symbol = global_symbols_.add(class_name);
    globals_[class_symbol] = cls;
  }

  // create a name for the method, including its arity
  str_t name(method_name);
  for (int i = 0; i < num_params; ++i)
    name.push_back(' ');

  // bind the method
  int method_symbol = methods_.ensure(name);
  cls->set_method(method_symbol, method);
}

double WrenVM::get_argument_double(int index) const {
  ASSERT(foreign_call_slot_ != nullptr, "must be in foreign call");
  ASSERT(index >= 0, "index cannot be negative");
  ASSERT(index < foreign_call_argc_, "not that many arguments");

  return (*(foreign_call_slot_ + index + 1)).as_numeric();
}

void WrenVM::return_double(double value) {
  ASSERT(foreign_call_slot_ != nullptr, "must be in foreign call");

  *foreign_call_slot_ = value;
  foreign_call_slot_ = nullptr;
}

void wrenDefineMethod(WrenVM& vm,
    const str_t& class_name, const str_t& method_name,
    int num_params, const WrenNativeFn& method) {
  vm.define_method(class_name, method_name, num_params, method);
}

double wrenGetArgumentDouble(WrenVM& vm, int index) {
  return vm.get_argument_double(index);
}

void wrenReturnDouble(WrenVM& vm, double value) {
  vm.return_double(value);
}

}
