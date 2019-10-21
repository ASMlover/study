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
#include "./auxiliary/meta.hh"
#include "./auxiliary/random.hh"
#include "vm.hh"

namespace wrencc {

/// WrenVM IMPLEMENTATIONS

WrenVM::WrenVM(void) noexcept {
  modules_ = MapObject::make_map(*this);

  core::initialize(*this);
  io::load_library(*this);

  gray_objects_.reserve(gray_capacity_);

  // load aux modules
  meta::load_aux_module(*this);
  random::load_aux_module(*this);
}

WrenVM::~WrenVM(void) {
  while (!objects_.empty()) {
    BaseObject* obj = objects_.back();
    objects_.pop_back();

    free_object(obj);
  }

  // tell the user if they didn't free any handles, wo don't want to just free
  // them here because the host app may still have pointers to them that they
  // may try to use. better to tell them about the bug early
  ASSERT(value_handles_ == nullptr, "all values have not been released");
}

InterpretRet WrenVM::interpret_in_module(
    const str_t& module, const str_t& source_bytes) {
  // executes [source_bytes] in the  context of [module]

  PinnedGuard name_guard{*this};

  Value name_val{nullptr};
  if (!module.empty()) {
    name_val = StringObject::format(*this, "$", module.c_str());
    name_guard.set_guard_object(name_val.as_object());
  }
  FiberObject* fiber = load_module(name_val, source_bytes);
  if (fiber == nullptr)
    return InterpretRet::COMPILE_ERROR;

  return interpret(fiber);
}

void WrenVM::set_metaclasses(void) {
  for (auto* o : objects_) {
    if (o->type() == ObjType::STRING)
      o->set_cls(str_class_);
  }
}

int WrenVM::declare_variable(ModuleObject* module, const str_t& name) {
  return module->declare_variable(name);
}

int WrenVM::define_variable(
    ModuleObject* module, const str_t& name, const Value& value) {
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

const Value& WrenVM::find_variable(
    ModuleObject* module, const str_t& name) const {
  int symbol = module->find_variable(name);
  return module->get_variable(symbol);
}

void WrenVM::set_foreign_call_slot(const Value& value) {
  *foreign_call_slot_ = value;
  foreign_call_slot_ = nullptr;
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
  if (fiber->error().is_string())
    std::cerr << fiber->error_asstr() << std::endl;
  else
    std::cerr << "[error object]" << std::endl;

  fiber->riter_frames([](const CallFrame& frame, FunctionObject* fn) {
      ModuleObject* module = fn->module();
      const DebugObject& debug = fn->debug();

      // the built-in core module has no name, we explicitly omit it from stack
      // traces since we don't want to highlight to a user the implementation
      // detail of what part of the core module is written in C and what is Wren
      if (module->name() == nullptr)
        return;

      int line = debug.get_line(Xt::as_type<int>(frame.ip - fn->codes()) - 1);
      std::cerr
        << "[`" << module->name_cstr() << "` LINE: " << line << "] in "
        << "`" << debug.name() << "`"
        << std::endl;
      });
}

void WrenVM::runtime_error(void) {
  // handles the current fiber having aborted because of an error, switches
  // to a new fiber if there is a fiber that will handle the error, oterwise,
  // tells the VM to stop

  ASSERT(!fiber_->error().is_nil(), "should only call this after an error");

  // unhook the caller since we will never resume and return to it
  FiberObject* caller = fiber_->caller();
  fiber_->set_caller(nullptr);

  // if the caller ran this fiber using `try`, give it the error
  if (fiber_->caller_is_trying()) {
    // make the caller's try method return the error message
    caller->set_value(caller->stack_size() - 1, fiber_->error());
    fiber_ = caller;
    return;
  }

  // if we got here, nothing caught the error, so show the stack trace
  print_stacktrace(fiber_);
  fiber_ = nullptr;
}

void WrenVM::method_not_found(ClassObject* cls, int symbol) {
  // aborts the current fiber with an appropriate method not found error for
  // a method with [symbol] on [cls]

  fiber_->set_error(StringObject::format(*this, "`@` does not implement `$`",
      cls->name(), method_names_.get_name(symbol).c_str()));
}

bool WrenVM::check_arity(const Value& value, int argc) {
  // checks that [value], which must be a function or closure, does not
  // require more parameters than are provided by [argc]
  //
  // if there are not enough arguments, aborts the current fiber and returns
  // `false`

  FunctionObject* fn{};
  if (value.is_closure()) {
    fn = value.as_closure()->fn();
  }
  else {
    ASSERT(value.is_function(), "receiver must be a function or closure");
    fn = value.as_function();
  }

  // we only care about missing arguments, not extras, the `-1` is because argc
  // includes the receiver, the function itself, which we donot want to count
  if (argc - 1 >= fn->arity())
    return true;

  fiber_->set_error(
      StringObject::make_string(*this, "function expects more arguments"));
  return false;
}

Value WrenVM::validate_superclass(
    const Value& name, const Value& supercls_val, int num_fields) {
  // verifies that [supercls_val] is a valid object to inherit from, that
  // means it must be a class and cannot be the class of any built-in type
  //
  // also validates that it doesn't result in a class with too many fields
  // and the other limitations foreign classes have
  //
  // if successful return nullptr, otherwise returns a string for the runtime
  // error message

  if (!supercls_val.is_class()) {
    return StringObject::format(*this,
        "class `@` cannot inherit from a non-class object", name);
  }

  // make sure it does not inherit from a sealed built-in type, primitive
  // methods on these classes assume the instance is one of the other Obj*
  // types and will fail horribly if it is actually an InstanceObject
  ClassObject* superclass = supercls_val.as_class();
  if (superclass == class_class_ || superclass == fiber_class_ ||
      superclass == fn_class_ || superclass == list_class_ ||
      superclass == map_class_ || superclass == range_class_ ||
      superclass == str_class_) {
    return StringObject::format(*this,
        "class `@` cannot inherit from built-in class `@`",
        name, superclass->name());
  }
  if (superclass->num_fields() == -1) {
    return StringObject::format(*this,
        "class `@` cannot inherit from foreign class `@`",
        name, superclass->name());
  }
  if (num_fields == -1 && superclass->num_fields() > 0) {
    return StringObject::format(*this,
        "foreign class `@` may not inherit from a class with fields", name);
  }
  if (superclass->num_fields() + num_fields > MAX_FIELDS) {
    return StringObject::format(*this,
        "class `@` may not have more than 255 fields, including inherited",
        name);
  }

  return nullptr;
}

void WrenVM::validate_foreign_argument(int index) const {
  ASSERT(foreign_call_slot_ != nullptr, "must be in foreign call");
  ASSERT(index >= 0, "index cannot be negative");
  ASSERT(index < foreign_call_argc_, "not that many arguments");
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

void WrenVM::bind_foreign_class(ClassObject* cls, ModuleObject* module) {
  ASSERT(bind_foreign_cls_ != nullptr,
      "cannot declare foreign classes without a BindForeignClassFn");

  WrenForeignClass methods = bind_foreign_cls_(
      *this, module->name_cstr(), cls->name_cstr());

  Method method = methods.allocate;
  ASSERT(method.foreign(), "a foreign class must provide an allocate function");

  int symbol = method_names_.ensure("<allocate>");
  cls->bind_method(symbol, method);

  // add the symbol even if there is no finalizer so we can ensure that the
  // symbol itself is always in the symbol table
  symbol = method_names_.ensure("<finalize>");
  if (methods.finalize) {
    method.set_foreign(methods.finalize);
    cls->bind_method(symbol, method);
  }
}

void WrenVM::create_class(int num_fields, ModuleObject* module) {
  // creates a new class
  //
  // if [num_fields] is -1, the class is a foreign class, the name and
  // superclass should be on top of the fiber's stack, after calling this,
  // the top of the stack will contain either the new class
  //
  // aborts the current fiber if an error occurs

  // pull the name and superclass off the stack
  const Value& name = fiber_->peek_value(1);
  const Value& superclass = fiber_->peek_value();

  // we have two values on the stack and we are going to leave one, so
  // discard the other slot
  fiber_->pop();

  fiber_->set_error(validate_superclass(name, superclass, num_fields));
  if (!fiber_->error().is_nil())
    return;

  ClassObject* cls = ClassObject::make_class(
      *this, superclass.as_class(), num_fields, name.as_string());
  fiber_->set_value(fiber_->stack_size() - 1, cls);

  if (num_fields == -1)
    bind_foreign_class(cls, module);
}

void WrenVM::create_foreign(FiberObject* fiber, Value* stack) {
  ClassObject* cls = stack[0].as_class();
  ASSERT(cls->num_fields() == -1, "class must be a foreign class");

  int symbol = method_names_.get("<allocate>");
  ASSERT(symbol != -1, "should have defined <allocate> symbol");
  ASSERT(cls->methods_count() > symbol, "class should have allocator");

  const Method& method = cls->get_method(symbol);
  ASSERT(method.get_type() == MethodType::FOREIGN, "allocator should be foreign");

  // pass the constructor arguments to the allocator as well
  foreign_call_slot_ = stack;
  foreign_call_argc_ = Xt::as_type<int>(
      fiber->values_at(fiber->stack_size() - 1) - stack + 1);

  method.foreign()(*this);
}

InterpretRet WrenVM::interpret(FiberObject* fiber) {
  // the main bytecode interpreter loop, this is where the magic happens, it
  // is also, as you can imagine, highly performance critical, returns `true`
  // if the fiber completed without error

#define PUSH(v)   fiber->push(v)
#define POP()     fiber->pop()
#define PEEK()    fiber->peek_value()
#define PEEK2()   fiber->peek_value(1)
#define RDBYTE()  (*frame->ip++)
#define RDWORD()  (frame->ip += 2, Xt::as_type<u16_t>((frame->ip[-2] << 8) | frame->ip[-1]))

  // remember the current fiber so we can find it if a GC happens
  fiber_ = fiber;

  CallFrame* frame{};
  FunctionObject* fn{};

// use this after a CallFrame has been pushed or popped ti refresh the
// local variables
#define LOAD_FRAME()\
  frame = &fiber->peek_frame();\
  fn = upwrap_closure(frame->fn);

// terminates the current fiber with error string [error], if another calling
// fiber is willing to catch the error, transfers control to it, otherwise
// exits the interpreter
#define RUNTIME_ERROR() do {\
    runtime_error();\
    if (fiber_ == nullptr)\
      return InterpretRet::RUNTIME_ERROR;\
    fiber = fiber_;\
    LOAD_FRAME();\
    DISPATCH();\
  } while (false)

#if COMPUTED_GOTOS
  static void* _dispatch_table[] = {
# undef CODEF
# define CODEF(c, _) &&__code_##c,
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

    {
      // the opercodes for doing method and superclass calls share a lot of
      // code, however doing an if() test in the middle of the instruction
      // sequence to handle the bit that is special to super calls makes the
      // non-super call path noticeably slower
      //
      // instead we do this old school using an explicit goto to share code
      // for everything at the tail end of the call-handling code that is the
      // same between normal and superclass calls

      int argc;
      int symbol;

      Value* args;
      ClassObject* cls;

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
      // add one for the implicit receiver argument
      argc = c - Code::CALL_0 + 1;
      symbol = RDWORD();

      // the receiver is the first argument
      args = fiber->values_at(fiber->stack_size() - argc);
      cls = get_class(args[0]);

      goto __complete_call;

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
      // add one for the implicit receiver argument
      argc = c - Code::SUPER_0 + 1;
      symbol = RDWORD();

      // the receiver is the first argument
      args = fiber->values_at(fiber->stack_size() - argc);
      // the superclass is stored in a constant
      cls = fn->get_constant(RDWORD()).as_class();

      goto __complete_call;

__complete_call:
      // Method method = cls->get_method(symbol);
      // if the class's method table does not include the symbol, bail
      if (symbol >= cls->methods_count() ||
          (cls->get_method(symbol)).type == MethodType::NONE) {
        method_not_found(cls, symbol);
        RUNTIME_ERROR();
      }

      switch (auto& method = cls->get_method(symbol); method.type) {
      case MethodType::PRIMITIVE:
        {
          if (method.primitive()(*this, args)) {
            // the result is now in the first arg slot, discard the other
            // stack slots
            fiber->resize_stack(fiber->stack_size() - (argc - 1));
          }
          else {
            // an error or fiber switch occurred

            // if we do not have a fiber to switch to, stop interpreting
            fiber = fiber_;
            if (fiber_ == nullptr)
              return InterpretRet::SUCCESS;
            if (!fiber_->error().is_nil())
              RUNTIME_ERROR();
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
      case MethodType::FNCALL:
        if (!check_arity(args[0], argc))
          RUNTIME_ERROR();
        fiber->call_function(args[0].as_object(), argc);
        LOAD_FRAME();
        break;
      case MethodType::NONE:
        UNREACHABLE();
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
    CASE_CODE(CLOSE_UPVALUE):
    {
      fiber->close_upvalues(fiber->stack_size() - 1);
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
          return InterpretRet::SUCCESS;

        // we have a calling fiber to resume
        FiberObject* calling_fiber = fiber->caller();
        fiber->set_caller(nullptr);
        fiber = calling_fiber;
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
    CASE_CODE(CONSTRUCT):
    {
      const Value& cls = fiber->get_value(frame->stack_start);
      ASSERT(cls.is_class(), "`this` should be a class");
      fiber->set_value(frame->stack_start,
          InstanceObject::make_instance(*this, cls.as_class()));

      DISPATCH();
    }
    CASE_CODE(FOREIGN_CONSTRUCT):
    {
      const Value& cls = fiber->get_value(frame->stack_start);
      ASSERT(cls.is_class(), "`this` should be a class");
      create_foreign(fiber, fiber->values_at(frame->stack_start));

      DISPATCH();
    }
    CASE_CODE(CLASS):
    {
      create_class(RDBYTE(), nullptr);
      if (!fiber->error().is_nil())
        RUNTIME_ERROR();

      DISPATCH();
    }
    CASE_CODE(FOREIGN_CLASS):
    {
      create_class(-1, fn->module());
      if (!fiber->error().is_nil())
        RUNTIME_ERROR();

      DISPATCH();
    }
    CASE_CODE(METHOD_INSTANCE):
    CASE_CODE(METHOD_STATIC):
    {
      u16_t symbol = RDWORD();
      ClassObject* cls = PEEK().as_class();
      const Value& method = PEEK2();
      bind_method(symbol, c, fn->module(), cls, method);
      if (!fiber->error().is_nil())
        RUNTIME_ERROR();
      POP();
      POP();

      DISPATCH();
    }
    CASE_CODE(LOAD_MODULE):
    {
      Value result = import_module(fn->get_constant(RDWORD()));

      if (!fiber->error().is_nil())
        RUNTIME_ERROR();

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

      Value result = import_variable(module, variable);
      if (!fiber->error().is_nil())
        RUNTIME_ERROR();

      PUSH(result);

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
  return InterpretRet::RUNTIME_ERROR;
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

InterpretRet WrenVM::interpret(const str_t& source_bytes) {
  return interpret_in_module("main", source_bytes);
}

ModuleObject* WrenVM::get_module(const Value& name) const {
  // looks up the previously loaded module with  [name]
  //
  // returns `nullptr` if no module with that name has been loaded

  if (auto m = modules_->get(name); m)
    return (*m).as_module();
  return nullptr;
}

FiberObject* WrenVM::load_module(const Value& name, const str_t& source_bytes) {
  ModuleObject* module = get_module(name);

  // see if the module has already been loaded
  if (module == nullptr) {
    module = ModuleObject::make_module(*this, name.as_string());

    // store it in the VM's module registry so we don't load the same module
    // multiple times
    modules_->set(name, module);

    // implicitly import the core module (unless we `are` core)
    ModuleObject* core_module = get_module(nullptr);
    core_module->iter_variables(
        [&](int i, const Value& val, const str_t& name) {
          define_variable(module, name, val);
        });
  }

  FunctionObject* fn = compile(*this, module, source_bytes, true);
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
    fiber_->set_error(
        StringObject::format(*this, "could not load module `@`", name));
    return nullptr;
  }

  FiberObject* module_fiber = load_module(name, source_bytes);
  if (module_fiber == nullptr) {
    fiber_->set_error(
        StringObject::format(*this, "could not compile module `@`", name));
    return nullptr;
  }

  // return the fiber that executes the module
  return module_fiber;
}

Value WrenVM::import_variable(
    const Value& module_name, const Value& variable_name) {
  ModuleObject* module = get_module(module_name);
  ASSERT(module != nullptr, "should only look up loaded modules");

  StringObject* variable = variable_name.as_string();
  int variable_entry = module->find_variable(variable->cstr());
  // it's a runtime error if the imported variable does not exist
  if (variable_entry != -1)
    return module->get_variable(variable_entry);

  fiber_->set_error(StringObject::format(*this,
        "could not find a variable named `@` in module `@`",
        variable, module_name));
  return nullptr;
}

FunctionObject* WrenVM::make_call_stub(
    ModuleObject* module, const str_t& signature) {
  // creates an [FunctionObject] that invokes a method with [signature]
  // when called

  int num_params = 0;
  if (signature.back() == ')') {
    sz_t i = signature.size() - 2;
    for (auto c = signature[i]; i >= 0 && c != '('; c = signature[--i]) {
      if (c == '_')
        ++num_params;
    }
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

  return FunctionObject::make_function(*this, module,
      num_params + 1, 0, 0, bytecode, 5, nullptr, 0, signature, debug_lines, 5);
}

WrenForeignFn WrenVM::find_foreign_method(const str_t& module_name,
    const str_t& class_name, bool is_static, const str_t& signature) {
  // looks up a foreign method in [module_name] on [class_name] with [signature]
  //
  // this will try the host's foreign method binder first, if that falls, it
  // falls back ot handling the built-in modules

  WrenForeignFn fn{};
  // bind foreign methods in the core module
  if (module_name == "") {
    fn = io::bind_foreign(*this, class_name, signature);
    ASSERT(fn != nullptr, "failed to bind core module foreign method");
    return fn;
  }

  // for other modules, let the host bind it
  if (bind_foreign_meth_ == nullptr)
    return nullptr;

  return bind_foreign_meth_(*this, module_name, class_name, is_static, signature);
}

void WrenVM::bind_method(int i, Code method_type,
    ModuleObject* module, ClassObject* cls, const Value& method_val) {
  // defines [method_val] as a method on [cls]
  //
  // handles both foreign methods where [method_val] is a string containning
  // the method's signature and Wren methods where [method_val] is a function
  //
  // aborts the current fiber if the method is a foreign method that could not
  // be found

  const char* class_name = cls->name_cstr();
  bool is_static = method_type == Code::METHOD_STATIC;
  if (is_static)
    cls = cls->cls();

  Method method;
  if (method_val.is_string()) {
    auto method_fn = find_foreign_method(module->name_cstr(),
        class_name, is_static, method_val.as_cstring());
    method.assign(method_fn);
    if (!method_fn) {
      fiber_->set_error(StringObject::format(*this,
          "could not find foreign method `@` for class `$` in module `$`",
          method_val, cls->name_cstr(), module->name_cstr()));
      return;
    }
  }
  else {
    FunctionObject* method_fn = method_val.is_function() ?
      method_val.as_function() : method_val.as_closure()->fn();

    // patch up the bytecode now that we know the superclass
    cls->bind_method(method_fn);
    method.assign(method_fn);
  }

  cls->bind_method(i, method);
}

void WrenVM::call_function(FiberObject* fiber, BaseObject* fn, int argc) {
  fiber->call_function(fn, argc);
}

void WrenVM::collect(void) {
  gray_object(modules_);
  // pinned objects
  for (auto* o : temp_roots_)
    gray_object(o);

  // the current fiber
  gray_object(fiber_);

  // the value handles
  for (WrenValue* v = value_handles_; v != nullptr; v = v->next)
    gray_value(v->value);

  // any object the compiler is using
  if (compiler_ != nullptr)
    mark_compiler(*this, compiler_);

  // now that we have grayed the roots, do a depth-first search over all of the
  // reachable objects
  blacken_objects();

  // collect any unmarked objects
  for (auto it = objects_.begin(); it != objects_.end();) {
    if (!(*it)->is_darken()) {
      free_object(*it);
      objects_.erase(it++);
    }
    else {
      (*it)->set_darken(false);
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

  obj->finalize(*this);
  delete obj;
}

void WrenVM::append_object(BaseObject* obj) {
  if (objects_.size() > next_gc_) {
    collect();
    next_gc_ = total_allocated_ * 3 / 2;
  }

  objects_.push_back(obj);
}

void WrenVM::gray_object(BaseObject* obj) {
  // mark [obj] as reachable and still in use, this should only be called
  // during the sweep phase of a garbage collection

  if (obj == nullptr)
    return;

  // stop if the object is already marked so we don't get stuck in a cycle
  if (obj->is_darken())
    return;

  // it is been reached
  obj->set_darken(true);

  if (gray_objects_.size() >= gray_capacity_) {
    gray_capacity_ *= 2;
    gray_objects_.reserve(gray_capacity_);
  }
  gray_objects_.push_back(obj);
}

void WrenVM::gray_value(const Value& val) {
  // mark [val] as reachable and still in use, this should only be called
  // during the sweep phase of a garbage collection

  if (val.is_object())
    gray_object(val.as_object());
}

void WrenVM::blacken_objects(void) {
  // processes every object in the gray stack until all reachable objects have
  // been marked, after that, all objects are either white (freeable) or black
  // (in use and fully traversed)

  while (!gray_objects_.empty()) {
    BaseObject* obj = gray_objects_.back();
    gray_objects_.pop_back();

    obj->gc_blacken(*this);
  }
}

WrenValue* WrenVM::acquire_method(
    const str_t& module, const str_t& variable, const str_t& signature) {
  // creates a handle that can be used to invoke a method with [signature] on
  // the object in [module] currently stored in top-level [variable]
  //
  // this handle can be used repeatedly to directly invoke that method from
  // C++ code using [wren_call]
  //
  // when done with this handle, it must be released using [release_value]

  Value module_name = StringObject::make_string(*this, module);
  PinnedGuard module_name_guard(*this, module_name.as_object());

  ModuleObject* module_obj = get_module(module_name);
  int variable_slot = module_obj->find_variable(variable);

  FunctionObject* fn = make_call_stub(module_obj, signature);
  PinnedGuard fn_guard(*this, fn);

  // create a single fiber that we can reuse each time the method is invoked
  FiberObject* fiber = FiberObject::make_fiber(*this, fn);
  PinnedGuard fiber_guard(*this, fiber);

  // create a handle that keeps track of the function that calls the method
  WrenValue* method = capture_value(fiber);
  // store the receiver in the fiber's stack so we can use it later in the call
  fiber->push(module_obj->get_variable(variable_slot));

  return method;
}

InterpretRet WrenVM::wren_call(
    WrenValue* method, WrenValue*& return_value, const char* arg_types, ...) {
  va_list ap;
  va_start(ap, arg_types);
  InterpretRet result = wren_call_args(method, return_value, arg_types, ap);
  va_end(ap);

  return result;
}

InterpretRet WrenVM::wren_call_args(WrenValue* method,
    WrenValue*& return_value, const char* arg_types, va_list ap) {
  ASSERT(method->value.is_fiber(), "value must come from `acquire_method()`");
  FiberObject* fiber = method->value.as_fiber();

  // push the arguments
  for (const char* arg_type = arg_types; *arg_type != '\0'; ++arg_type) {
    Value value(nullptr);
    switch (*arg_type) {
    case 'a':
      {
        const char* bytes = va_arg(ap, const char*);
        int length = va_arg(ap, int);
        value = StringObject::make_string(*this, bytes, length);
      } break;
    case 'b': value = Xt::as_type<bool>(va_arg(ap, int)); break;
    case 'd': value = va_arg(ap, double); break;
    case 'i': value = va_arg(ap, int); break;
    case 'n': value = nullptr; va_arg(ap, void*); break;
    case 's':
      value = StringObject::format(*this, "$", va_arg(ap, const char*));
      break;
    case 'v':
      {
        // allow a nullptr value pointer for Wren nil
        WrenValue* wren_value = va_arg(ap, WrenValue*);
        if (wren_value != nullptr)
          value = wren_value->value;
      } break;
    default: ASSERT(false, "unknown argument type"); break;
    }
    fiber->push(value);
  }

  const Value& receiver = fiber->get_value(0);
  BaseObject* fn = fiber->get_frame(0).fn;
  PinnedGuard fn_guard(*this, fn);

  InterpretRet result = interpret(fiber);
  if (result == InterpretRet::SUCCESS) {
    if (return_value != nullptr) {
      // make sure the return value doesn't get collected while capturing it
      fiber->push(Value());
      return_value = capture_value(fiber->get_value(0));
    }
    // reset the fiber to get ready for the next call
    fiber->reset_fiber(fn);
    // push the receiver back on the stack
    fiber->push(receiver);
  }
  else {
    if (return_value != nullptr)
      return_value = nullptr;
  }

  return result;
}

WrenValue* WrenVM::capture_value(const Value& value) {
  // make a handle for it
  WrenValue* wrapped_value = new WrenValue(value);
  // add it to the front of the linked list of handles
  if (value_handles_ != nullptr)
    value_handles_->prev = wrapped_value;
  wrapped_value->next = value_handles_;
  value_handles_ = wrapped_value;

  return wrapped_value;
}

void WrenVM::release_value(WrenValue* value) {
  ASSERT(value != nullptr, "null value");

  // update the VM's head pointer if we're releasing the first handle
  if (value_handles_ == nullptr)
    value_handles_ = value->next;

  // unlink it from the list
  if (value->prev != nullptr)
    value->prev->next = value->next;
  if (value->next != nullptr)
    value->next->prev = value->prev;

  // clear it out, this isn't strictly necessary since we're going to free it
  // but it makes for easier debugging
  value->clear();
  delete value;
}

void* WrenVM::allocate_foreign(sz_t size) {
  ASSERT(foreign_call_slot_ != nullptr, "must be in foreign call");

  ClassObject* cls = foreign_call_slot_[0].as_class();
  ForeignObject* foreign = ForeignObject::make_foreign(*this, cls, size);
  *foreign_call_slot_ = foreign;

  return foreign->data();
}

void WrenVM::finalize_foreign(ForeignObject* foreign) {
  // invoke the finalizer for the foreign object referenced by [foreign]

  int symbol = method_names_.get("<finalize>");
  ASSERT(symbol != -1, "should have defined <finalize> symbol");

  // if there are no finalizers, donot finalize it
  if (symbol == -1)
    return;

  // if the class doesn't have a finalizer, bail out
  ClassObject* cls = foreign->cls();
  if (symbol >= cls->methods_count())
    return;

  const Method& method = cls->get_method(symbol);
  if (method.get_type() == MethodType::NONE)
    return;
  ASSERT(method.get_type() == MethodType::FOREIGN, "finalizer should be foreign");

  // pass the constructor arguments to the allocator as well
  Value slot(foreign);
  foreign_call_slot_ = &slot;
  foreign_call_argc_ = 1;

  method.foreign()(*this);
}

int WrenVM::get_argument_count(void) const {
  ASSERT(foreign_call_slot_ != nullptr, "must be in foreign call");
  return foreign_call_argc_;
}

bool WrenVM::get_argument_bool(int index) const {
  validate_foreign_argument(index);

  if (!(*(foreign_call_slot_ + index)).is_boolean())
    return false;
  return (*(foreign_call_slot_ + index)).as_boolean();
}

double WrenVM::get_argument_double(int index) const {
  validate_foreign_argument(index);

  if (!(*(foreign_call_slot_ + index)).is_numeric())
    return 0.0;
  return (*(foreign_call_slot_ + index)).as_numeric();
}

const char* WrenVM::get_argument_string(int index) const {
  validate_foreign_argument(index);

  if (!(*(foreign_call_slot_ + index)).is_string())
    return "";
  return (*(foreign_call_slot_ + index)).as_cstring();
}

WrenValue* WrenVM::get_argument_value(int index) {
  validate_foreign_argument(index);

  return capture_value(*(foreign_call_slot_ + index));
}

void* WrenVM::get_argument_foreign(int index) const {
  validate_foreign_argument(index);

  const Value& foreign_val = *(foreign_call_slot_ + index);
  if (!foreign_val.is_foreign())
    return nullptr;
  return foreign_val.as_foreign()->data();
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

void WrenVM::return_value(WrenValue* value) {
  ASSERT(foreign_call_slot_ != nullptr, "must be in foreign call");
  ASSERT(value != nullptr, "`value` cannot be `nullptr`");

  *foreign_call_slot_ = value->value;
  foreign_call_slot_ = nullptr;
}

void wrenCollectGarbage(WrenVM& vm) {
  vm.collect();
}

void wrenReleaseValue(WrenVM& vm, WrenValue* value) {
  vm.release_value(value);
}

void* wrenAllocateForeign(WrenVM& vm, sz_t size) {
  return vm.allocate_foreign(size);
}

int wrenGetArgumentCount(WrenVM& vm) {
  return vm.get_argument_count();
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

WrenValue* wrenGetArgumentValue(WrenVM& vm, int index) {
  return vm.get_argument_value(index);
}

void* wrenGetArgumentForeign(WrenVM& vm, int index) {
  return vm.get_argument_foreign(index);
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

void wrenReturnValue(WrenVM& vm, WrenValue* value) {
  vm.return_value(value);
}

}
