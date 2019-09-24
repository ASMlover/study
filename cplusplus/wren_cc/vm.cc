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
#include <cstdarg>
#include <iostream>
#include <sstream>
#include "core.hh"
#include "io.hh"
#include "meta.hh"
#include "vm.hh"

namespace wrencc {

/// WrenVM IMPLEMENTATIONS

WrenVM::WrenVM(void) noexcept {
  StringObject* name = StringObject::make_string(*this, "core");
  {
    PinnedGuard name_guard(*this, name);

    // implicitly create a `core` module for the built-in libraries
    ModuleObject* core_module = ModuleObject::make_module(*this, name);
    {
      PinnedGuard guard(*this, core_module);

      modules_ = MapObject::make_map(*this);
      modules_->set(nullptr, core_module);
    }
  }

  core::initialize(*this);
  io::load_library(*this);
  meta::load_library(*this);
}

WrenVM::~WrenVM(void) {
  while (!objects_.empty()) {
    BaseObject* obj = objects_.back();
    objects_.pop_back();

    free_object(obj);
  }

  // tell the user if they didn't free any method handles, wo don't want to
  // just free them here because the host app may still have pointers to them
  // that they may try to use. better to tell them about the bug early
  ASSERT(method_handles_ == nullptr, "all methods have not been released");
}

void WrenVM::set_metaclasses(void) {
  for (auto* o : objects_) {
    if (o->type() == ObjType::STRING)
      o->set_cls(str_class_);
  }
}

int WrenVM::declare_variable(ModuleObject* module, const str_t& name) {
  if (module == nullptr)
    module = get_core_module();

  return module->declare_variable(name);
}

int WrenVM::define_variable(
    ModuleObject* module, const str_t& name, const Value& value) {
  if (module == nullptr)
    module = get_core_module();

  if (value.is_object())
    push_root(value.as_object());

  int symbol = module->define_variable(name, value);

  if (value.is_object())
    pop_root();

  return symbol;
}

void WrenVM::set_native(
    ClassObject* cls, const str_t& name, const PrimitiveFn& fn) {
  int symbol = method_names_.ensure(name);
  cls->bind_method(symbol, fn);
}

const Value& WrenVM::find_variable(const str_t& name) const {
  auto* core_module = get_core_module();
  int symbol = core_module->find_variable(name);
  return core_module->get_variable(symbol);
}

void WrenVM::push_root(BaseObject* obj) {
  // mark [obj] as a GC root so that it does not get collected

  ASSERT(obj != nullptr, "cannot root nullptr");
  ASSERT(temp_roots_.size() < kMaxTempRoots, "Too many temporary roots");
  temp_roots_.push_back(obj);
}

void WrenVM::pop_root(void) {
  // remove the most recently pushed temporary root

  ASSERT(temp_roots_.size() > 0, "No temporary roots to release");
  temp_roots_.pop_back();
}

Value WrenVM::import_module(const str_t& name) {
  // imports the module with [name]
  //
  // if the module has already been imported (or is already in the middle of
  // being imported, in the case of a circular import), returns true, oterwise
  // returns a new fiber that will execute the module's code, that fiber should
  // be called before any variables are loaded from the module
  //
  // if the module could not be found, return false

  Value name_val = StringObject::make_string(*this, name);
  PinnedGuard name_guard(*this, name_val.as_object());

  // if the module is already loaded, we do not need to do anything
  if (auto val = modules_->get(name_val); !val)
    return nullptr;

  // load the module's source code from the embedder
  str_t source_bytes = load_module_fn_(*this, name);
  if (source_bytes.empty())
    return StringObject::format(*this, "could not find module `$`", name.c_str());

  FiberObject* module_fiber = load_module(name_val, source_bytes);
  // return the fiber the executes the module
  return module_fiber;
}

void WrenVM::print_stacktrace(FiberObject* fiber) {
  std::cerr << fiber->error_cstr() << std::endl;

  fiber->riter_frames([](const CallFrame& frame, FunctionObject* fn) {
      auto& debug = fn->debug();

      // built-in libraries have no source path and are explicitly omitted,
      // from stack traces since we donot want to highlight to a user the
      // implementation detail of what part of a core library is implemented
      // in C++ and what is in Wren
      if (debug.source_path().empty())
        return;

      int line = debug.get_line(Xt::as_type<int>(frame.ip - fn->codes()) - 1);
      std::cerr
        << "[`" << debug.source_path() << "` LINE: " << line << "] in "
        << "`" << debug.name() << "`"
        << std::endl;
      });
}

FiberObject* WrenVM::runtime_error(FiberObject* fiber, const Value& error) {
  // returns the fiber that should receive the error or `nullptr` if no
  // fiber caught it

  ASSERT(fiber->error() == nullptr, "can only fail one");

  // store the error in the fiber so it can be accessed later
  fiber->set_error(error.as_string());
  // if the caller ran this fiber using `try`, give it the error
  if (fiber->caller_is_trying()) {
    FiberObject* caller = fiber->caller();

    // make the caller's try method return the error message
    caller->set_value(caller->stack_size() - 1, fiber->error());
    return caller;
  }

  // if we got here, nothing caught the error, so show the stack trace
  print_stacktrace(fiber);
  return nullptr;
}

Value WrenVM::method_not_found(ClassObject* cls, int symbol) {
  // creates a string containing an appropriate method not found error for
  // a method with [symbol] on [cls] object.

  return StringObject::format(*this, "`@` does not implement `$`",
      cls->name(), method_names_.get_name(symbol).c_str());
}

Value WrenVM::validate_superclass(
    const Value& name, const Value& supercls_val) {
  // verifies that [superclass] is a valid object to inherit from, that means
  // it must be a class and cannot be the class of any built-in type
  //
  // if successful return nullptr, otherwise returns a string for the runtime
  // error message

  if (!supercls_val.is_class())
    return StringObject::make_string(*this, "must inherit from a class");

  // make sure it does not inherit from a sealed built-in type, primitive
  // methods on these classes assume the instance is one of the other Obj*
  // types and will fail horribly if it is actually an InstanceObject
  ClassObject* superclass = supercls_val.as_class();
  if (superclass == class_class_ || superclass == fiber_class_ ||
      superclass == fn_class_ || superclass == list_class_ ||
      superclass == map_class_ || superclass == range_class_ ||
      superclass == str_class_) {
    return StringObject::format(*this,
        "`@` cannot inherit from `@`", name, superclass->name());
  }

  return nullptr;
}

bool WrenVM::isinstance_of(const Value& value, ClassObject* base_class) const {
  // returns `true` if [value] is an instance of [base_class] or any of its
  // superclasses

  // walk the superclass chain looking for the class
  for (auto* cls = get_class(value); cls != nullptr; cls = cls->superclass()) {
    if (cls == base_class)
      return true;
  }
  return false;
}

void WrenVM::call_foreign(
    FiberObject* fiber, const WrenForeignFn& foreign, int argc) {
  foreign_call_slot_ = fiber->values_at(fiber->stack_size() - argc);

  // donot include the receiver
  foreign_call_argc_ = argc;
  foreign(*this);

  // discard the stack slots for the arguments (but leave one for result)
  if (foreign_call_slot_ != nullptr) {
    *foreign_call_slot_ = nullptr;
    foreign_call_slot_ = nullptr;
  }
}

bool WrenVM::interpret(void) {
  // the main bytecode interpreter loop, this is where the magic happens, it
  // is also, as you can imagine, highly performance critical, returns `true`
  // if the fiber completed without error

#define PUSH(v)   fiber->push(v)
#define POP()     fiber->pop()
#define PEEK()    fiber->peek_value()
#define PEEK2()   fiber->peek_value(1)
#define RDBYTE()  (*frame->ip++)
#define RDWORD()  (frame->ip += 2, Xt::as_type<u16_t>((frame->ip[-2] << 8) | frame->ip[-1]))

  FiberObject* fiber = fiber_;
  CallFrame* frame{};
  FunctionObject* fn{};

// use this after a CallFrame has been pushed or popped ti refresh the
// local variables
#define LOAD_FRAME()\
  frame = &fiber->peek_frame();\
  if (frame->fn->type() == ObjType::FUNCTION) {\
    fn = Xt::down<FunctionObject>(frame->fn);\
  }\
  else {\
    fn = Xt::down<ClosureObject>(frame->fn)->fn();\
  }

// terminates the current fiber with error string [error], if another calling
// fiber is willing to catch the error, transfers control to it, otherwise
// exits the interpreter
#define RUNTIME_ERROR(error) do {\
    fiber = runtime_error(fiber, (error));\
    if (fiber == nullptr)\
      return false;\
    LOAD_FRAME();\
    DISPATCH();\
  } while (false)

#if COMPUTED_GOTOS
  static void* _dispatch_table[] = {
# undef CODEF
# define CODEF(c) &&__code_##c,
# include "codes_def.hh"
  };
# define INTERPRET_LOOP() DISPATCH();
# define CASE_CODE(name)  __code_##name
# define DISPATCH()       goto *_dispatch_table[Xt::as_type<int>(c = Xt::as_type<Code>(RDBYTE()))]
#else
# define INTERPRET_LOOP() __loop: switch (c = Xt::as_type<Code>(RDBYTE()))
# define CASE_CODE(name)  case Code::##name
# define DISPATCH()       goto __loop
#endif

  LOAD_FRAME();

  Code c;
  INTERPRET_LOOP() {
    CASE_CODE(LOAD_LOCAL_0):
    CASE_CODE(LOAD_LOCAL_1):
    CASE_CODE(LOAD_LOCAL_2):
    CASE_CODE(LOAD_LOCAL_3):
    CASE_CODE(LOAD_LOCAL_4):
    CASE_CODE(LOAD_LOCAL_5):
    CASE_CODE(LOAD_LOCAL_6):
    CASE_CODE(LOAD_LOCAL_7):
    CASE_CODE(LOAD_LOCAL_8):
    {
      PUSH(fiber->get_value(frame->stack_start + (c - Code::LOAD_LOCAL_0)));

      DISPATCH();
    }
    CASE_CODE(CONSTANT): PUSH(fn->get_constant(RDWORD())); DISPATCH();
    CASE_CODE(NIL): PUSH(nullptr); DISPATCH();
    CASE_CODE(FALSE): PUSH(false); DISPATCH();
    CASE_CODE(TRUE): PUSH(true); DISPATCH();
    CASE_CODE(LOAD_LOCAL):
    {
      PUSH(fiber->get_value(frame->stack_start + RDBYTE()));

      DISPATCH();
    }
    CASE_CODE(STORE_LOCAL):
    {
      fiber->set_value(frame->stack_start + RDBYTE(), PEEK());

      DISPATCH();
    }
    CASE_CODE(LOAD_UPVALUE):
    {
      ClosureObject* co = Xt::down<ClosureObject>(frame->fn);
      PUSH(co->get_upvalue(RDBYTE())->value_asref());

      DISPATCH();
    }
    CASE_CODE(STORE_UPVALUE):
    {
      ClosureObject* co = Xt::down<ClosureObject>(frame->fn);
      co->get_upvalue(RDBYTE())->set_value(PEEK());

      DISPATCH();
    }
    CASE_CODE(LOAD_MODULE_VAR):
    {
      PUSH(fn->module()->get_variable(RDWORD()));

      DISPATCH();
    }
    CASE_CODE(STORE_MODULE_VAR):
    {
      fn->module()->set_variable(RDWORD(), PEEK());

      DISPATCH();
    }
    CASE_CODE(LOAD_FIELD_THIS):
    {
      u8_t field = RDBYTE();
      const Value& receiver = fiber->get_value(frame->stack_start);
      ASSERT(receiver.is_instance(), "receiver should be instance");
      InstanceObject* inst = receiver.as_instance();
      ASSERT(field < inst->cls()->num_fields(), "out of bounds field");
      PUSH(inst->get_field(field));

      DISPATCH();
    }
    CASE_CODE(STORE_FIELD_THIS):
    {
      u8_t field = RDBYTE();
      const Value& receiver = fiber->get_value(frame->stack_start);
      ASSERT(receiver.is_instance(), "receiver should be instance");
      InstanceObject* inst = receiver.as_instance();
      ASSERT(field < inst->cls()->num_fields(), "out of bounds field");
      inst->set_field(field, PEEK());

      DISPATCH();
    }
    CASE_CODE(LOAD_FIELD):
    {
      u8_t field = RDBYTE();
      Value receiver = POP();
      ASSERT(receiver.is_instance(), "receiver should be instance");
      InstanceObject* inst = receiver.as_instance();
      ASSERT(field < inst->cls()->num_fields(), "out of bounds field");
      PUSH(inst->get_field(field));

      DISPATCH();
    }
    CASE_CODE(STORE_FIELD):
    {
      u8_t field = RDBYTE();
      Value receiver = POP();
      ASSERT(receiver.is_instance(), "receiver should be instance");
      InstanceObject* inst = receiver.as_instance();
      ASSERT(field < inst->cls()->num_fields(), "out of bounds field");
      inst->set_field(field, PEEK());

      DISPATCH();
    }
    CASE_CODE(POP): POP(); DISPATCH();
    CASE_CODE(DUP):
    {
      const Value& value = PEEK();
      PUSH(value);

      DISPATCH();
    }
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
      int symbol = RDWORD();

      Value* args = fiber->values_at(fiber->stack_size() - argc);
      ClassObject* cls = get_class(args[0]);

      // if the class's method table does not include the symbol, bail
      if (cls->methods_count() <= symbol) {
        RUNTIME_ERROR(method_not_found(cls, symbol));
      }

      auto& method = cls->get_method(symbol);
      switch (method.type) {
      case MethodType::PRIMITIVE:
        {
          // after calling this, the result will be in the first arg slot
          switch (method.primitive()(*this, fiber, args)) {
          case PrimitiveResult::VALUE:
            // the result is now in the first arg slot, discard the other
            // stack slots
            fiber->resize_stack(fiber->stack_size() - (argc - 1));
            break;
          case PrimitiveResult::ERROR:
            RUNTIME_ERROR(args[0]);
          case PrimitiveResult::CALL:
            fiber->call_function(args[0].as_object(), argc);
            LOAD_FRAME();
            break;
          case PrimitiveResult::RUN_FIBER:
            // if we do not have a fiber to switch to, stop interpreting
            if (args[0].is_nil())
              return true;
            fiber = args[0].as_fiber();
            fiber_ = fiber;
            LOAD_FRAME();
            break;
          }
        } break;
      case MethodType::FOREIGN:
        call_foreign(fiber, method.foreign(), argc);
        break;
      case MethodType::BLOCK:
        fiber->call_function(method.fn(), argc);
        LOAD_FRAME();
        break;
      case MethodType::NONE:
        RUNTIME_ERROR(method_not_found(cls, symbol));
        break;
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
      int symbol = RDWORD();

      Value* args = fiber->values_at(fiber->stack_size() - argc);
      ClassObject* cls = get_class(args[0]);

      // ignore methods defined on the receiver's immediate class
      cls  = cls->superclass();

      // if the class's method table does not include the symbol, bail
      if (cls->methods_count() <= symbol) {
        RUNTIME_ERROR(method_not_found(cls, symbol));
      }

      auto& method = cls->get_method(symbol);
      switch (method.type) {
      case MethodType::PRIMITIVE:
        {
          // after calling this, the result will be in the first arg slot
          switch (method.primitive()(*this, fiber, args)) {
          case PrimitiveResult::VALUE:
            // the result is now in the first arg slot, discard the other
            // stack slots
            fiber->resize_stack(fiber->stack_size() - (argc - 1));
            break;
          case PrimitiveResult::ERROR:
            RUNTIME_ERROR(args[0]);
          case PrimitiveResult::CALL:
            fiber->call_function(args[0].as_object(), argc);
            LOAD_FRAME();
            break;
          case PrimitiveResult::RUN_FIBER:
            // if we do not have a fiber to switch to, stop interpreting
            if (args[0].is_nil())
              return true;
            fiber = args[0].as_fiber();
            fiber_ = fiber;
            LOAD_FRAME();
            break;
          }
        } break;
      case MethodType::FOREIGN:
        call_foreign(fiber, method.foreign(), argc);
        break;
      case MethodType::BLOCK:
        fiber->call_function(method.fn(), argc);
        LOAD_FRAME();
        break;
      case MethodType::NONE:
        RUNTIME_ERROR(method_not_found(cls, symbol));
        break;
      }

      DISPATCH();
    }
    CASE_CODE(JUMP): frame->ip += RDWORD(); DISPATCH();
    CASE_CODE(LOOP):
    {
      // jump back to the top of the loop
      frame->ip -= RDWORD();

      DISPATCH();
    }
    CASE_CODE(JUMP_IF):
    {
      u16_t offset = RDWORD();
      Value cond = POP();

      if (cond.is_falsely())
        frame->ip += offset;

      DISPATCH();
    }
    CASE_CODE(AND):
    {
      u16_t offset = RDWORD();
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
      u16_t offset = RDWORD();
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
      Value expected = POP();
      if (!expected.is_class()) {
        RUNTIME_ERROR(StringObject::make_string(
              *this, "right operand must be a class"));
      }

      Value instance = POP();
      PUSH(isinstance_of(instance, expected.as_class()));

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

      // close any upvalues still in scope
      fiber->close_upvalues(frame->stack_start);

      // if the fiber is complete, end it
      if (fiber->empty_frame()) {
        // if this is the main fiber, we are done
        if (fiber->caller() == nullptr)
          return true;

        // we have a calling fiber to resume
        fiber = fiber->caller();
        fiber_ = fiber;

        // store the result in the resuming fiber
        fiber->set_value(fiber->stack_size() - 1, r);
      }
      else {
        // store the result of the block in the first slot, which is where the
        // caller expects it
        if (fiber->stack_size() <= frame->stack_start)
          PUSH(r);
        else
          fiber->set_value(frame->stack_start, r);

        // discard the stack slots for the call frame
        fiber->resize_stack(frame->stack_start + 1);
      }
      LOAD_FRAME();

      DISPATCH();
    }
    CASE_CODE(CLOSURE):
    {
      FunctionObject* prototype = fn->get_constant(RDWORD()).as_function();
      ASSERT(prototype->num_upvalues() > 0,
          "should not create closure for functions that donot need it");

      // create the closure and push it on the stack before creating upvalues
      // so that it does not get collected
      ClosureObject* closure = ClosureObject::make_closure(*this, prototype);
      PUSH(closure);

      // capture upvalues
      for (int i = 0; i < prototype->num_upvalues(); ++i) {
        u8_t is_local = RDBYTE();
        u8_t index = RDBYTE();
        if (is_local) {
          // make an new upvalue to close over the parent's local variable
          closure->set_upvalue(i,
              fiber->capture_upvalue(*this, frame->stack_start + index));
        }
        else {
          // use the same upvalue as the current call frame
          ClosureObject* co = Xt::down<ClosureObject>(frame->fn);
          closure->set_upvalue(i, co->get_upvalue(index));
        }
      }

      DISPATCH();
    }
    CASE_CODE(CLASS):
    {
      const Value& name = PEEK2();
      ClassObject* superclass = obj_class_;

      // use implicit object superclass if none given
      if (!PEEK().is_nil()) {
        Value error = validate_superclass(name, PEEK());
        if (!error.is_nil())
          RUNTIME_ERROR(error);
        superclass = PEEK().as_class();
      }

      int num_fields = RDBYTE();
      ClassObject* cls = ClassObject::make_class(
          *this, superclass, num_fields, name.as_string());

      // do not pop the superclass and name off the stack until the subclass
      // is done being created, to make sure it does not get collected
      POP();
      POP();

      // now that we know the total number of fields, make sure we donot
      // overflow
      if (superclass->num_fields() + num_fields > MAX_FIELDS) {
        RUNTIME_ERROR(StringObject::format(*this,
              "class `@` may not have more than 255 fields, including inherited ones",
              name));
      }
      PUSH(cls);

      DISPATCH();
    }
    CASE_CODE(METHOD_INSTANCE):
    CASE_CODE(METHOD_STATIC):
    {
      u16_t symbol = RDWORD();
      ClassObject* cls = PEEK().as_class();
      const Value& method = PEEK2();
      Value error = bind_method(symbol, c, fn->module(), cls, method);
      if (error.is_string())
        RUNTIME_ERROR(error);
      POP();
      POP();

      DISPATCH();
    }
    CASE_CODE(LOAD_MODULE):
    {
      Value result = import_module(fn->get_constant(RDWORD()));

      // if it returned a string. it was an error message
      if (result.is_string())
        RUNTIME_ERROR(result);

      // make a slot that the module's fiber can use to store its result in,
      // it ends up getting discarded, but `Code::RETURN` expects to be able
      // to place a value there
      PUSH(nullptr);

      // if it returned a fiber to execute the module body, switch to it
      if (result.is_fiber()) {
        // return to this module when that one is done
        result.as_fiber()->set_caller(fiber);

        fiber = result.as_fiber();
        fiber_ = fiber;
        LOAD_FRAME();
      }

      DISPATCH();
    }
    CASE_CODE(IMPORT_VARIABLE):
    {
      const Value& module = fn->get_constant(RDWORD());
      const Value& variable = fn->get_constant(RDWORD());

      auto [r, result] = import_variable(module, variable);
      if (r)
        PUSH(result);
      else
        RUNTIME_ERROR(result);

      DISPATCH();
    }
    CASE_CODE(END):
    {
      // a END should always preceded by a RETURN. if we get here, the compiler
      // generated wrong code
      UNREACHABLE();
      DISPATCH();
    }
  }

  // we should only exit this function from an explicit return from Code::RETURN
  // or a runtime error
  UNREACHABLE();
  return false;
}

ClassObject* WrenVM::get_class(const Value& val) const {
  if (val.is_numeric())
    return num_class_;
  if (val.is_object())
    return val.as_object()->cls();

#if NAN_TAGGING
  switch (val.tag()) {
  case Tag::NaN: return num_class_;
  case Tag::NIL: return nil_class_;
  case Tag::TRUE:
  case Tag::FALSE: return bool_class_;
  case Tag::UNDEFINED: UNREACHABLE();
  }
#else
  switch (val.type()) {
  case ValueType::NIL: return nil_class_;
  case ValueType::TRUE:
  case ValueType::FALSE: return bool_class_;
  case ValueType::NUMERIC: return num_class_;
  case ValueType::OBJECT: return val.as_object()->cls();
  case ValueType::UNDEFINED: UNREACHABLE();
  }
#endif

  UNREACHABLE();
  return nullptr;
}

InterpretRet WrenVM::interpret(
    const str_t& source_path, const str_t& source_bytes) {
  if (source_path.empty())
    return load_into_core(source_bytes);

  StringObject* name = StringObject::make_string(*this, "main");
  PinnedGuard name_guard(*this, name);

  FiberObject* fiber = load_module(name, source_bytes);
  if (fiber == nullptr)
    return InterpretRet::COMPILE_ERROR;

  fiber_ = fiber;
  return interpret() ? InterpretRet::SUCCESS : InterpretRet::RUNTIME_ERROR;
}

ModuleObject* WrenVM::get_core_module(void) const {
  // looks up the core module in the module map
  if (auto val = modules_->get(nullptr); val)
    return (*val).as_module();

  ASSERT(false, "could not find core module");
  return nullptr;
}

FiberObject* WrenVM::load_module(const Value& name, const str_t& source_bytes) {
  ModuleObject* module{};

  // see if the module has already been loaded
  if (auto m = modules_->get(name); m) {
    // execute the new code in the context of the existing module
    module = (*m).as_module();
  }
  else {
    module = ModuleObject::make_module(*this, name.as_string());

    // store it in the VM's module registry so we don't load the same module
    // multiple times
    modules_->set(name, module);

    // implicitly import the core module
    ModuleObject* core_module = get_core_module();
    core_module->iter_variables(
        [&](int i, const Value& val, const str_t& name) {
          define_variable(module, name, val);
        });
  }

  FunctionObject* fn = compile(*this,
      module, name.as_cstring(), source_bytes, true);
  if (fn == nullptr)
    return nullptr;

  PinnedGuard fn_guard(*this, fn);
  FiberObject* module_fiber = FiberObject::make_fiber(*this, fn);

  // return the fiber that executes the module
  return module_fiber;
}

Value WrenVM::import_module(const Value& name) {
  // if the module is already loaded, we do not need to do anything
  if (auto val = modules_->get(name); val)
    return nullptr;

  // load the module's source code from the embedder
  const str_t source_bytes = load_module_fn_(*this, name.as_cstring());
  if (source_bytes.empty()) {
    // could not load the module
    return StringObject::format(*this, "could not find module `@`", name);
  }

  FiberObject* module_fiber = load_module(name, source_bytes);
  // return the fiber that executes the module
  return module_fiber;
}

std::tuple<bool, Value> WrenVM::import_variable(
      const Value& module_name, const Value& variable_name) {
  ModuleObject* module{};
  if (auto m = modules_->get(module_name); m)
    module = (*m).as_module();
  ASSERT(module != nullptr, "should only look up loaded modules");

  StringObject* variable = variable_name.as_string();
  int variable_entry = module->find_variable(variable->cstr());
  // it's a runtime error if the imported variable does not exist
  if (variable_entry != -1)
    return std::make_tuple(true, module->get_variable(variable_entry));

  return std::make_tuple(false, StringObject::format(*this,
        "could not find a variable named `@` in module `@`",
        variable, module_name));
}

InterpretRet WrenVM::load_into_core(const str_t& source_bytes) {
  FunctionObject* fn = compile(*this, get_core_module(), "", source_bytes, true);
  if (fn == nullptr)
    return InterpretRet::COMPILE_ERROR;

  PinnedGuard guard(*this, fn);

  fiber_ = FiberObject::make_fiber(*this, fn);
  return interpret() ? InterpretRet::SUCCESS : InterpretRet::RUNTIME_ERROR;
}

FunctionObject* WrenVM::make_call_stub(
    ModuleObject* module, const str_t& signature) {
  // creates an [FunctionObject] that invokes a method with [signature]
  // when called

  int num_params = 0;
  for (auto c : signature) {
    if (c == '_')
      ++num_params;
  }

  int method = method_names_.ensure(signature);
  u8_t bytecode[5] = {
    Xt::as_type<u8_t>(Code::CALL_0 + num_params),
    Xt::as_type<u8_t>((method >> 8) & 0xff),
    Xt::as_type<u8_t>(method & 0xff),
    Xt::as_type<u8_t>(Code::RETURN),
    Xt::as_type<u8_t>(Code::END)
  };
  int debug_lines[] = {1, 1, 1, 1, 1};

  return FunctionObject::make_function(*this,
      module, 0, 0, bytecode, 5, nullptr, 0, "", signature, debug_lines, 5);
}

WrenForeignFn WrenVM::find_foreign_method(const str_t& module_name,
    const str_t& class_name, bool is_static, const str_t& signature) {
  // looks up a foreign method in [module_name] on [class_name] with [signature]
  //
  // this will try the host's foreign method binder first, if that falls, it
  // falls back ot handling the built-in modules

  WrenForeignFn fn{};
  if (bind_foreign_fn_) {
    if (fn = bind_foreign_fn_(*this,
          module_name, class_name, is_static, signature); fn)
      return fn;
  }

  // otherwise try the built-in libraries
  if (module_name == "core") {
    fn = io::bind_foreign(*this, class_name, signature);
    if (fn)
      return fn;
  }

  return nullptr;
}

Value WrenVM::bind_method(int i, Code method_type,
    ModuleObject* module, ClassObject* cls, const Value& method_val) {
  // defines [method_val] as a method on [cls]
  //
  // handles both foreign methods where [method_val] is a string containning
  // the method's signature and Wren methods where [method_val] is a function
  //
  // returns an error string if the method is a foreign method that could not
  // be found, otherwise returns `nil` Value

  Method method;

  bool is_static = method_type == Code::METHOD_STATIC;
  if (method_val.is_string()) {
    auto method_fn = find_foreign_method(module->name_cstr(),
        cls->name_cstr(), is_static, method_val.as_cstring());
    method.assign(method_fn);
    if (!method_fn) {
      return StringObject::format(*this,
          "could not find foreign method `@` for class `$` in module `$`",
          method_val, cls->name_cstr(), module->name_cstr());
    }
  }
  else {
    FunctionObject* method_fn = method_val.is_function() ?
      method_val.as_function() : method_val.as_closure()->fn();

    // methods are always bound against the class, and not the metaclass, even
    // for static methods, so that constructors (which are static) get bound
    // like instance methods
    cls->bind_method(method_fn);
    method.assign(method_fn);
  }

  if (is_static)
    cls = cls->cls();
  cls->bind_method(i, method);

  return nullptr;
}

void WrenVM::call_function(FiberObject* fiber, BaseObject* fn, int argc) {
  fiber->call_function(fn, argc);
}

void WrenVM::collect(void) {
  mark_object(modules_);
  // pinned objects
  for (auto* o : temp_roots_)
    mark_object(o);

  // the current fiber
  mark_object(fiber_);

  // the method handles
  for (WrenMethod* m = method_handles_; m != nullptr; m = m->next)
    mark_object(m->fiber);

  // any object the compiler is using
  if (compiler_ != nullptr)
    mark_compiler(*this, compiler_);

  // collect any unmarked objects
  for (auto it = objects_.begin(); it != objects_.end();) {
    if (!(*it)->is_marked()) {
      free_object(*it);
      objects_.erase(it++);
    }
    else {
      (*it)->set_marked(false);
      ++it;
    }
  }
  total_allocated_ = objects_.size();
}

void WrenVM::free_object(BaseObject* obj) {
#if TRACE_MEMORY
  std::cout
    << "`" << Xt::cast<void>(obj) << "` free object"
# if TRACE_OBJECT_DETAIL
    << " `" << obj->stringify() << "`"
# endif
    << std::endl;
#endif

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

  if (obj->is_marked())
    return;
  obj->set_marked(true);

  obj->gc_mark(*this);
}

void WrenVM::mark_value(const Value& val) {
  if (val.is_object())
    mark_object(val.as_object());
}

WrenMethod* WrenVM::acquire_method(
    const str_t& module, const str_t& variable, const str_t& signature) {
  // creates a handle that can be used to invoke a method with [signature] on
  // the object in [module] currently stored in top-level [variable]
  //
  // this handle can be used repeatedly to directly invoke that method from
  // C++ code using [wren_call]
  //
  // when done with this handle, it must be released by calling [release_method]

  Value module_name = StringObject::make_string(*this, module);
  PinnedGuard module_name_guard(*this, module_name.as_object());

  ModuleObject* module_obj{};
  if (auto m = modules_->get(module_name); m)
    module_obj = (*m).as_module();
  int variable_slot = module_obj->find_variable(variable);

  FunctionObject* fn = make_call_stub(module_obj, signature);
  PinnedGuard fn_guard(*this, fn);

  // create a single fiber that we can reuse each time the method is invoked
  FiberObject* fiber = FiberObject::make_fiber(*this, fn);
  PinnedGuard fiber_guard(*this, fiber);

  // create a handle that keeps track of the function that calls the method
  WrenMethod* method = new WrenMethod(fiber);
  fiber->push(module_obj->get_variable(variable_slot));

  // add it to the front of the linked list of handles
  if (method_handles_ != nullptr)
    method_handles_->prev = method;
  method->next = method_handles_;
  method_handles_ = method;

  return method;
}

void WrenVM::release_method(WrenMethod* method) {
  // releases memory associated with [method], after calling this, [method]
  // can no longer be used

  ASSERT(method != nullptr, "null method");

  // update the VM's head pointer if we are releasing the first handle
  if (method_handles_ == method)
    method_handles_ = method->next;
  if (method->prev != nullptr)
    method->prev->prev = method->next;
  if (method->next != nullptr)
    method->next->prev = method->prev;

  method->clear();
  delete method;
}

void WrenVM::wren_call(WrenMethod* method, const char* arg_types, ...) {
  va_list ap;
  va_start(ap, arg_types);

  for (const char* arg_type = arg_types; *arg_type != '\0'; ++arg_type) {
    Value value(nullptr);
    switch (*arg_type) {
      case 'b': value = Xt::as_type<bool>(va_arg(ap, int)); break;
      case 'd': value = va_arg(ap, double); break;
      case 'i': value = va_arg(ap, int); break;
      case 'n': value = nullptr; va_arg(ap, void*); break;
      case 's':
        value = StringObject::format(*this, "$", va_arg(ap, const char*));
        break;
      default: ASSERT(false, "unknown argument type"); break;
    }
    method->fiber->push(value);
  }

  va_end(ap);

  fiber_ = method->fiber;
  const Value& receiver = method->fiber->peek_value();
  BaseObject* fn = method->fiber->peek_frame().fn;

  interpret();

  // reset the fiber to get ready for the next call
  method->fiber->reset_fiber(fn);

  // push the receiver back on the stack
  method->fiber->push(receiver);
}

bool WrenVM::get_argument_bool(int index) const {
  ASSERT(foreign_call_slot_ != nullptr, "must be in foreign call");
  ASSERT(index >= 0, "index cannot be negative");
  ASSERT(index < foreign_call_argc_, "not that many arguments");

  if (!(*(foreign_call_slot_ + index)).is_boolean())
    return false;
  return (*(foreign_call_slot_ + index)).as_boolean();
}

double WrenVM::get_argument_double(int index) const {
  ASSERT(foreign_call_slot_ != nullptr, "must be in foreign call");
  ASSERT(index >= 0, "index cannot be negative");
  ASSERT(index < foreign_call_argc_, "not that many arguments");

  if (!(*(foreign_call_slot_ + index)).is_numeric())
    return 0.0;
  return (*(foreign_call_slot_ + index)).as_numeric();
}

const char* WrenVM::get_argument_string(int index) const {
  ASSERT(foreign_call_slot_ != nullptr, "must be in foreign call");
  ASSERT(index >= 0, "index cannot be negative");
  ASSERT(index < foreign_call_argc_, "not that many arguments");

  if (!(*(foreign_call_slot_ + index)).is_string())
    return nullptr;
  return (*(foreign_call_slot_ + index)).as_cstring();
}

void WrenVM::return_bool(bool value) {
  ASSERT(foreign_call_slot_ != nullptr, "must be in foreign call");

  *foreign_call_slot_ = value;
  foreign_call_slot_ = nullptr;
}

void WrenVM::return_double(double value) {
  ASSERT(foreign_call_slot_ != nullptr, "must be in foreign call");

  *foreign_call_slot_ = value;
  foreign_call_slot_ = nullptr;
}

void WrenVM::return_string(const str_t& text) {
  ASSERT(foreign_call_slot_ != nullptr, "must be in foreign call");
  ASSERT(!text.empty(), "string cannot be empty");

  *foreign_call_slot_ = StringObject::make_string(*this, text);
  foreign_call_slot_ = nullptr;
}

bool wrenGetArgumentBool(WrenVM& vm, int index) {
  return vm.get_argument_bool(index);
}

double wrenGetArgumentDouble(WrenVM& vm, int index) {
  return vm.get_argument_double(index);
}

const char* wrenGetArgumentString(WrenVM& vm, int index) {
  return vm.get_argument_string(index);
}

void wrenReturnBool(WrenVM& vm, bool value) {
  vm.return_bool(value);
}

void wrenReturnDouble(WrenVM& vm, double value) {
  vm.return_double(value);
}

void wrenReturnString(WrenVM& vm, const str_t& text) {
  vm.return_string(text);
}

}
