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
#include "io.hh"
#include "vm.hh"

namespace wrencc {

/// WrenVM IMPLEMENTATIONS

WrenVM::WrenVM(void) noexcept {
  // implicitly create a `main` module for the REPL or entry script
  ModuleObject* main_module = ModuleObject::make_module(*this);
  {
    PinnedGuard guard(*this, main_module);

    modules_ = MapObject::make_map(*this);
    modules_->set(nullptr, main_module);
  }

  core::initialize(*this);
  io::load_library(*this);
}

WrenVM::~WrenVM(void) {
  while (!objects_.empty()) {
    BaseObject* obj = objects_.back();
    objects_.pop_back();

    free_object(obj);
  }
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
  if (int index = modules_->find(name_val); index != -1)
    return nullptr;

  // load the module's source code from the embedder
  str_t source_bytes = load_module_(*this, name);
  if (source_bytes.empty()) {
    std::stringstream ss;
    ss << "could not find module `" << name << "`";
    return StringObject::make_string(*this, ss.str());
  }

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

FiberObject* WrenVM::runtime_error(FiberObject* fiber, StringObject* error) {
  // returns the fiber that should receive the error or `nullptr` if no
  // fiber caught it

  ASSERT(fiber->error() == nullptr, "can only fail one");

  // store the error in the fiber so it can be accessed later
  fiber->set_error(error);
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

StringObject* WrenVM::method_not_found(ClassObject* cls, int symbol) {
  // creates a string containing an appropriate method not found error for
  // a method with [symbol] on [cls] object.

  // count the number of spaces to determine the number of parameters the
  // method expects
  const str_t& method_name = method_names_.get_name(symbol);
  sz_t method_len = method_name.size();
  int num_params = 0;
  while (method_name[method_len - num_params - 1] == ' ')
    ++num_params;

  std::stringstream ss;
  ss << "`" << cls->name_cstr() << "` does not implement method "
    << "`" << method_name.substr(0, method_len - num_params) << "` "
    << "with " << num_params << " argument"
    << (num_params == 1 ? "" : "s");

  return StringObject::make_string(*this, ss.str());
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
#define RDWORD()  (frame->ip += 2, (frame->ip[-2] << 8) | frame->ip[-1])

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
            RUNTIME_ERROR(args[0].as_string());
          case PrimitiveResult::CALL:
            fiber->call_function(args[0].as_object(), argc);
            LOAD_FRAME();
            break;
          case PrimitiveResult::RUN_FIBER:
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
            RUNTIME_ERROR(args[0].as_string());
          case PrimitiveResult::CALL:
            fiber->call_function(args[0].as_object(), argc);
            LOAD_FRAME();
            break;
          case PrimitiveResult::RUN_FIBER:
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

      ClassObject* actual = get_class(POP());
      bool is_instance = false;

      // walk the superclass chain looking for the class
      while (actual != nullptr) {
        if (actual == expected.as_class()) {
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
      StringObject* name = PEEK2().as_string();

      ClassObject* superclass;
      if (PEEK().is_nil())
        superclass = obj_class_; // implicit Object superclass
      else
        superclass = PEEK().as_class();

      int num_fields = RDBYTE();
      ClassObject* cls =
        ClassObject::make_class(*this, superclass, num_fields, name);

      // do not pop the superclass and name off the stack until the subclass
      // is done being created, to make sure it does not get collected
      POP();
      POP();

      // now that we know the total number of fields, make sure we donot
      // overflow
      if (superclass->num_fields() + num_fields > MAX_FIELDS) {
        std::stringstream ss;
        ss << "class `" << name->cstr() << "` may not have more thane "
          << MAX_FIELDS << " fields, including inherited ones";
        RUNTIME_ERROR(StringObject::make_string(*this, ss.str()));
      }
      PUSH(cls);

      DISPATCH();
    }
    CASE_CODE(METHOD_INSTANCE):
    CASE_CODE(METHOD_STATIC):
    {
      u16_t symbol = RDWORD();
      ClassObject* cls = PEEK().as_class();
      Value method = PEEK2();
      cls->bind_method(symbol, Xt::as_type<int>(c), method);
      POP();
      POP();

      DISPATCH();
    }
    CASE_CODE(LOAD_MODULE):
    {
      Value result = import_module(fn->get_constant(RDWORD()));

      // if it returned a string. it was an error message
      if (result.is_string())
        RUNTIME_ERROR(result.as_string());

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
        RUNTIME_ERROR(result.as_string());

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
#if NAN_TAGGING
  if (val.is_numeric())
    return num_class_;
  if (val.is_object())
    return val.as_object()->cls();

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
  return false;
}

FiberObject* WrenVM::load_module(const Value& name, const str_t& source_bytes) {
  ModuleObject* module{};

  // see if the module has already been loaded
  if (auto m = modules_->get(name); m) {
    // execute the new code in the context of the existing module
    module = (*m).as_module();
  }
  else {
    module = ModuleObject::make_module(*this);

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

  FunctionObject* fn = compile(*this, module, name.as_cstring(), source_bytes);
  if (fn == nullptr)
    return nullptr;

  PinnedGuard fn_guard(*this, fn);
  FiberObject* module_fiber = FiberObject::make_fiber(*this, fn);

  // return the fiber that executes the module
  return module_fiber;
}

Value WrenVM::import_module(const Value& name) {
  // if the module is already loaded, we do not need to do anything
  if (auto index = modules_->find(name); index != -1)
    return nullptr;

  // load the module's source code from the embedder
  const str_t source_bytes = load_module_(*this, name.as_cstring());
  if (source_bytes.empty()) {
    // could not load the module
    std::stringstream ss;
    ss << "could not find module `" << name.as_cstring() << "`";
    return StringObject::make_string(*this, ss.str());
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

  std::stringstream ss;
  ss << "could not find a variable named `" << variable->cstr() << "` "
    << "in module `" << module_name.as_cstring() << "`";
  return std::make_tuple(false, StringObject::make_string(*this, ss.str()));
}

InterpretRet WrenVM::load_into_core(const str_t& source_bytes) {
  FunctionObject* fn = compile(*this, get_core_module(), "", source_bytes);
  if (fn == nullptr)
    return InterpretRet::COMPILE_ERROR;

  PinnedGuard guard(*this, fn);

  fiber_ = FiberObject::make_fiber(*this, fn);
  return interpret() ? InterpretRet::SUCCESS : InterpretRet::RUNTIME_ERROR;
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

void WrenVM::define_method(
    const str_t& class_name, const str_t& method_name,
    int num_params, const WrenForeignFn& method, bool is_static) {
  ASSERT(!class_name.empty(), "must provide class name");
  ASSERT(!method_name.empty(), "must provide method name");
  ASSERT(method_name.size() < MAX_METHOD_NAME, "method name too long");
  ASSERT(num_params >= 0, "num_params cannot be negative");
  ASSERT(num_params <= MAX_PARAMETERS, "too many parameters");

  // find or create the class to bind the method to
  ModuleObject* core_module = get_core_module();
  int class_symbol = core_module->find_variable(class_name);
  ClassObject* cls;
  if (class_symbol != -1) {
    cls = core_module->get_variable(class_symbol).as_class();
  }
  else {
    // the class does not already exists, so create it
    StringObject* name_string = StringObject::make_string(*this, class_name);
    PinnedGuard guard(*this, name_string);

    cls = ClassObject::make_class(*this, obj_class_, 0, name_string);
    define_variable(core_module, class_name, cls);
  }

  // create a name for the method, including its arity
  str_t name(method_name);
  for (int i = 0; i < num_params; ++i)
    name.push_back(' ');

  // bind the method
  int method_symbol = method_names_.ensure(name);
  if (is_static)
    cls = cls->cls();
  cls->bind_method(method_symbol, method);
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

void wrenDefineMethod(WrenVM& vm,
    const str_t& class_name, const str_t& method_name,
    int arity, const WrenForeignFn& method) {
  vm.define_method(class_name, method_name, arity, method, false);
}

void wrenDefineStaticMethod(WrenVM& vm,
    const str_t& class_name, const str_t& method_name,
    int arity, const WrenForeignFn& method) {
  vm.define_method(class_name, method_name, arity, method, true);
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
