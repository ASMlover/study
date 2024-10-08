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
  gray_objects_.reserve(gray_capacity_);

  core::initialize(*this);
  io::load_library(*this);
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
  ASSERT(handles_ == nullptr, "all handles have not been released");
}

void WrenVM::set_metaclasses(void) {
  for (auto* o : objects_) {
    if (o->type() == ObjType::STRING)
      o->set_cls(str_class_);
  }
}

int WrenVM::declare_variable(
    ModuleObject* module, const str_t& name, int line) {
  return module->declare_variable(*this, name, line);
}

std::tuple<int, int> WrenVM::define_variable(
    ModuleObject* module, const str_t& name, const Value& value) {
  if (value.is_object())
    push_root(value.as_object());

  auto result = module->define_variable(*this, name, value);

  if (value.is_object())
    pop_root();

  return result;
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

void WrenVM::set_api_stack_asref(const Value& value) {
  *api_stack_ = value;
}

void WrenVM::set_api_stack_asptr(const Value* value) {
  api_stack_ = const_cast<Value*>(value);
}

Value WrenVM::import_module(const Value& input_name) {
  // imports the module with [name], a string object
  //
  // if the module has already been imported (or is already in the middle of
  // being imported, in the case of a circular import), return nil, otherwise
  // returns a new fiber that will execute the module's code, that fiber should
  // be called before any variables are loaded from the module
  //
  // if the module could not be found, set an error in the current fiber

  Value name = resolve_module(input_name);
  // if the module is already loaded, we do not need to do anything
  if (auto val = modules_->get(name); val && !(*val).is_undefined())
    return (*val);

  str_t source_bytes;
  // let the host try to provide the module
  if (load_module_fn_)
    source_bytes = load_module_fn_(*this, name.as_cstring());

  // if the host didn't provide it, see if it's a built-in optional module
  if (source_bytes.empty()) {
    str_t name_string(name.as_cstring());
    if (name_string == "meta")
      source_bytes = meta::get_source();
    if (name_string == "random")
      source_bytes = random::get_source();
  }

  if (source_bytes.empty()) {
    // could not load the module
    fiber_->set_error(
        StringObject::format(*this, "could not load module `@`", name));
    return nullptr;
  }

  ClosureObject* module_closure =
    compile_in_module(name, source_bytes, false, true);
  if (module_closure == nullptr) {
    fiber_->set_error(
        StringObject::format(*this, "could not compile module `@`", name));
    return nullptr;
  }

  // return the fiber that executes the module
  return module_closure;
}

Value WrenVM::get_module_variable(
    const Value& module_name, const Value& variable_name) {
  // looks up a variable from a previously loaded module
  //
  // aborts the current fiber if the module or variable could not be found

  ModuleObject* module = get_module(module_name);
  if (module == nullptr) {
    fiber_->set_error(
        StringObject::format(*this, "module `@` is not loaded", module_name));
    return nullptr;
  }

  return get_module_variable_impl(module, variable_name);
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

void WrenVM::print_stacktrace(void) {
  // bail if the host doesn't enable printing errors
  if (!error_fn_)
    return;

  FiberObject* fiber = fiber_;
  if (fiber->error().is_string())
    error_fn_(*this, WrenError::RUNTIME, "", -1, fiber->error_asstr());
  else
    error_fn_(*this, WrenError::RUNTIME, "", -1, "[error object]");

  fiber->riter_frames([this](const CallFrame& frame, FunctionObject* fn) {
      ModuleObject* module = fn->module();
      const DebugObject& debug = fn->debug();

      // skip over stub functions for calling methods from the C++ API
      if (fn->module() == nullptr)
        return;

      // the built-in core module has no name, we explicitly omit it from stack
      // traces since we don't want to highlight to a user the implementation
      // detail of what part of the core module is written in C and what is Wren
      if (module->name() == nullptr)
        return;

      int line = debug.get_line(Xt::as_type<int>(frame.ip - fn->codes()) - 1);
      error_fn_(*this,
          WrenError::SATCK_TRACE, module->name_cstr(), line, debug.name());
      });
}

void WrenVM::runtime_error(void) {
  // handles the current fiber having aborted because of an error
  //
  // walks the call chain of fibers, aborting each one until it hits a fiber
  // that handles the error, if none do, tells the VM to stop

  ASSERT(fiber_->has_error(), "should only call this after an error");

  FiberObject* current_fiber = fiber_;
  Value error = current_fiber->error();
  while (current_fiber->caller() != nullptr) {
    // every fiber along the call chain gets aborted with the same error
    current_fiber->set_error(error);

    // if the caller ran this fiber using `try`, give it the error and stop
    if (current_fiber->state() == FiberState::TRY) {
      // make the caller's try method return the error message
      int i = current_fiber->caller()->stack_size() - 1;
      current_fiber->caller()->set_value(i, fiber_->error());
      fiber_ = current_fiber->caller();
      return;
    }

    // otherwise, unhook the caller since we will never resume and return to it
    FiberObject* caller = current_fiber->caller();
    current_fiber->set_caller(nullptr);
    current_fiber = caller;
  }

  // if we got here, nothing caught the error, so show the stack trace
  print_stacktrace();
  fiber_ = nullptr;
  api_stack_ = nullptr;
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

  ASSERT(value.is_closure(), "receiver must be a closure");
  FunctionObject* fn = value.as_closure()->fn();

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

void WrenVM::validate_api_slot(int slot) const {
  // ensures that [slot] is a valid index into the API's stack of slots

  ASSERT(slot >= 0, "slot cannot be negative");
  ASSERT(slot < get_slot_count(), "not that many slots");
}

void WrenVM::call_foreign(
    FiberObject* fiber, const WrenForeignFn& foreign, int argc) {
  ASSERT(api_stack_ == nullptr, "cannot already be in foreign call");

  api_stack_ = fiber->values_at(fiber->stack_size() - argc);
  foreign(this);

  // discard the stack slots for the arguments and temporaries but leave one
  // for the result
  fiber->resize_stack(fiber->stack_size() - (argc - 1));

  api_stack_ = nullptr;
}

void WrenVM::bind_foreign_class(ClassObject* cls, ModuleObject* module) {
  WrenForeignClass methods{nullptr, nullptr};

  // check the optional built-in module first so the host can override it
  str_t module_name = module->name_cstr();
  str_t class_name = cls->name_cstr();
  if (bind_foreign_cls_)
    methods = bind_foreign_cls_(*this, module_name, class_name);
  // if the host didn't provide it, see if it's a built-in optional module
  if (!methods.allocate && !methods.finalize) {
    if (module_name == "random")
      methods = random::bind_foreign_class(*this, module_name, class_name);
  }

  Method method(MethodType::FOREIGN);
  // add the symbol even if there is no allocator so we can ensure that the
  // symbol itself is always in the symbol table
  int symbol = method_names_.ensure("<allocate>");
  if (methods.allocate) {
    method.set_foreign(methods.allocate);
    cls->bind_method(symbol, method);
  }

  // add the symbol even if there is no finalizer so we can ensure that the
  // symbol itself is always in the symbol table
  symbol = method_names_.ensure("<finalize>");
  if (methods.finalize) {
    method.set_foreign(Xt::as_type<WrenForeignFn>(methods.finalize));
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
  if (fiber_->has_error())
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
  ASSERT(api_stack_ == nullptr, "cannot already be in foreign call");

  api_stack_ = stack;
  method.foreign()(this);

  api_stack_ = nullptr;
}

InterpretRet WrenVM::interpret(FiberObject* fiber) {
  // the main bytecode interpreter loop, this is where the magic happens, it
  // is also, as you can imagine, highly performance critical

#define PUSH(v)   fiber->push(v)
#define POP()     fiber->pop()
#define PEEK()    fiber->peek_value()
#define PEEK2()   fiber->peek_value(1)
#define RDBYTE()  (*frame->ip++)
#define RDWORD()  (frame->ip += 2, Xt::as_type<u16_t>((frame->ip[-2] << 8) | frame->ip[-1]))

  // remember the current fiber so we can find it if a GC happens
  fiber_ = fiber;
  fiber->set_state(FiberState::ROOT);

  CallFrame* frame{};
  FunctionObject* fn{};

// use this after a CallFrame has been pushed or popped ti refresh the
// local variables
#define LOAD_FRAME()\
  frame = &fiber->peek_frame();\
  fn = frame->closure->fn();

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
      ClosureObject* co = frame->closure;
      PUSH(co->get_upvalue(RDBYTE())->value_asref());

      DISPATCH();
    }
    CASE_CODE(STORE_UPVALUE):
    {
      ClosureObject* co = frame->closure;
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
            if (fiber == nullptr)
              return InterpretRet::SUCCESS;
            if (fiber->has_error())
              RUNTIME_ERROR();
            LOAD_FRAME();
            break;
          }
        } break;
      case MethodType::FOREIGN:
        call_foreign(fiber, method.foreign(), argc);
        if (fiber->has_error())
          RUNTIME_ERROR();
        break;
      case MethodType::BLOCK:
        fiber->call_function(*this, method.closure(), argc);
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
        // see if there's another fiber to return to, if not, we are done
        if (fiber->caller() == nullptr) {
          // store the final result value at the beginning of the stack so
          // the C++ API can get it
          fiber->set_value_safely(0, r);
          fiber->resize_stack(1);
          return InterpretRet::SUCCESS;
        }
        FiberObject* resuming_fiber = fiber->caller();
        fiber->set_caller(nullptr);
        fiber = resuming_fiber;
        fiber_ = resuming_fiber;

        // store the result in the resuming fiber
        fiber->set_value(fiber->stack_size() - 1, r);
      }
      else {
        // store the result of the block in the first slot, which is where the
        // caller expects it
        fiber->set_value_safely(frame->stack_start, r);

        // discard the stack slots for the call frame
        fiber->resize_stack(frame->stack_start + 1);
      }
      LOAD_FRAME();

      DISPATCH();
    }
    CASE_CODE(CLOSURE):
    {
      // create the closure and push it on the stack before creating upvalues
      // so that it does not get collected
      FunctionObject* function = fn->get_constant(RDWORD()).as_function();
      ClosureObject* closure = ClosureObject::make_closure(*this, function);
      PUSH(closure);

      // capture upvalues
      for (int i = 0; i < function->num_upvalues(); ++i) {
        u8_t is_local = RDBYTE();
        u8_t index = RDBYTE();
        if (is_local) {
          // make an new upvalue to close over the parent's local variable
          closure->set_upvalue(i,
              fiber->capture_upvalue(*this, frame->stack_start + index));
        }
        else {
          // use the same upvalue as the current call frame
          closure->set_upvalue(i, frame->closure->get_upvalue(index));
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
      if (fiber->has_error())
        RUNTIME_ERROR();

      DISPATCH();
    }
    CASE_CODE(CLASS):
    {
      create_class(RDBYTE(), nullptr);
      if (fiber->has_error())
        RUNTIME_ERROR();

      DISPATCH();
    }
    CASE_CODE(FOREIGN_CLASS):
    {
      create_class(-1, fn->module());
      if (fiber->has_error())
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
      if (fiber->has_error())
        RUNTIME_ERROR();
      POP();
      POP();

      DISPATCH();
    }
    CASE_CODE(END_MODULE):
    {
      last_module_ = fn->module();
      PUSH(nullptr);

      DISPATCH();
    }
    CASE_CODE(IMPORT_MODULE):
    {
      const Value& name = fn->get_constant(RDWORD());
      Value result = import_module(name);
      if (fiber->has_error())
        RUNTIME_ERROR();

      // make a slot on the stack for the module's fiber to place the return
      // value, it will be popped after the module body code returns
      PUSH(nullptr);

      // if we get a closure, call it to execute the module body
      if (result.is_closure()) {
        ClosureObject* closure = result.as_closure();
        fiber->call_function(*this, closure, 1);

        LOAD_FRAME();
      }
      else {
        // the module has already been loaded, remember it so we can import
        // variables from it if needed
        last_module_ = result.as_module();
      }

      DISPATCH();
    }
    CASE_CODE(IMPORT_VARIABLE):
    {
      const Value& variable = fn->get_constant(RDWORD());
      ASSERT(last_module_ != nullptr, "should have already imported module");
      Value result = get_module_variable_impl(last_module_, variable);
      if (fiber->has_error())
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

InterpretRet WrenVM::interpret(const str_t& module, const str_t& source_bytes) {
  ClosureObject* closure = compile_source(module, source_bytes, false, true);
  if (closure == nullptr)
    return InterpretRet::COMPILE_ERROR;

  FiberObject* fiber;
  {
    PinnedGuard guard(*this, closure);
    fiber = FiberObject::make_fiber(*this, closure);
  }

  return interpret(fiber);
}

ClosureObject* WrenVM::compile_source(const str_t& module,
    const str_t& source_bytes, bool is_expression, bool print_errors) {
  // compile [source_bytes] in the context of [module] and wrap in a fiber
  // that can execute it
  //
  // returns nullptr if a compile error occurred

  PinnedGuard name_guard{*this};

  Value name_val{nullptr};
  if (!module.empty()) {
    name_val = StringObject::format(*this, "$", module.c_str());
    name_guard.set_guard_object(name_val.as_object());
  }
  ClosureObject* closure = compile_in_module(
      name_val, source_bytes, is_expression, print_errors);

  return closure;
}

ModuleObject* WrenVM::get_module(const Value& name) const {
  // looks up the previously loaded module with  [name]
  //
  // returns `nullptr` if no module with that name has been loaded

  if (auto m = modules_->get(name); m)
    return (*m).as_module();
  return nullptr;
}

Value WrenVM::get_module_variable_impl(
    ModuleObject* module, const Value& variable_name) {
  int variable_entry = module->find_variable(variable_name.as_cstring());
  if (variable_entry != -1)
    return module->get_variable(variable_entry);

  fiber_->set_error(StringObject::format(*this,
        "could not find a variable name `@` in moudle `$`",
        variable_name, module->name_cstr()));
  return nullptr;
}

ClosureObject* WrenVM::compile_in_module(const Value& name,
    const str_t& source_bytes, bool is_expression, bool print_errors) {
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
        [&](int i, const Value& val, StringObject* name) {
          define_variable(module, name->cstr(), val);
        });
  }

  FunctionObject* fn = compile(*this,
      module, source_bytes, is_expression, print_errors);
  if (fn == nullptr)
    return nullptr;

  PinnedGuard fn_guard(*this, fn);
  ClosureObject* closure = ClosureObject::make_closure(*this, fn);
  return closure;
}

WrenForeignFn WrenVM::find_foreign_method(const str_t& module_name,
    const str_t& class_name, bool is_static, const str_t& signature) {
  // looks up a foreign method in [module_name] on [class_name] with [signature]
  //
  // this will try the host's foreign method binder first, if that falls, it
  // falls back ot handling the built-in modules

  // bind foreign methods in the core module
  if (module_name == "") {
    auto fn = io::bind_foreign(*this, class_name, signature);
    ASSERT(fn != nullptr, "failed to bind core module foreign method");
    return fn;
  }

  WrenForeignFn method{};
  // for other modules, let the host bind it
  if (bind_foreign_meth_ != nullptr) {
    method = bind_foreign_meth_(*this,
        module_name, class_name, is_static, signature);
  }

  // if the host didn't provide it, see if it's an optional one
  if (!method) {
    if (module_name == "meta")
      method = meta::bind_foreign_method(*this, class_name, is_static, signature);
    if (module_name == "random")
      method = random::bind_foreign_method(*this, class_name, is_static, signature);
  }
  return method;
}

Value WrenVM::resolve_module(const Value& name) {
  // let the host resolve an imported module name if it wants to

  if (!res_module_fn_)
    return name;

  FiberObject* fiber = fiber_;
  FunctionObject* fn = fiber->peek_frame().closure->fn();
  StringObject* importer = fn->module()->name();

  str_t resolved = res_module_fn_(*this, importer->cstr(), name.as_cstring());
  if (resolved.empty()) {
    fiber_->set_error(StringObject::format(*this,
          "could not resolve module `@` imported from `$`",
          name, importer->cstr()));
    return nullptr;
  }

  // if they resolved to the extrat same string, we don't need to copy it
  if (importer->compare(name.as_cstring()))
    return name;

  // copy the string into a Wren StringObject
  return StringObject::make_string(*this, resolved);
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
    method.assign(method_val.as_closure());
    // patch up the bytecode now that we know the superclass
    cls->bind_method(method.closure()->fn());
  }

  cls->bind_method(i, method);
}

void WrenVM::collect(void) {
  gray_object(modules_);
  // pinned objects
  for (auto* o : temp_roots_)
    gray_object(o);

  // the rooted fibers
  gray_object(fiber_);

  // the value handles
  for (WrenHandle* h = handles_; h != nullptr; h = h->next)
    gray_value(h->value);

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
    << " type: `" << obj->type_asstr() << "`"
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

WrenHandle* WrenVM::make_call_handle(const str_t& signature) {
  // creates a handle that can be used to invoke a method with [signature] on
  // using a receiver and arguments that are set up on the stack
  //
  // this handle can be used repreatedly to directly invoke that method from
  // C++ code using [wren_call]
  //
  // when you are done with this handle, it must be released using
  // [wrenReleaseValue]

  ASSERT(!signature.empty(), "signature cannot be empty");

  int num_params = 0;
  // count the number parameters the method expects
  if (signature.back() == ')') {
    sz_t i = signature.size() - 1;
    for (auto c = signature[i]; i >= 0 && c != '('; c = signature[--i]) {
      if (c == '_')
        ++num_params;
    }
  }
  // count subscript arguments
  if (signature[0] == '[') {
    for (sz_t i = 0; i < signature.size() && signature[i] != ']'; ++i) {
      if (signature[i] == '_')
        ++num_params;
    }
  }

  // add the signature to the method table
  int method = method_names_.ensure(signature);

  // create a little stub function that assumes the arguments are on the stack
  // and calls the method
  FunctionObject* fn = FunctionObject::make_function(
      *this, nullptr, num_params + 1);
  // wrap the function in a closure and then in a handle, do this here so it
  // does not get collected as we fill it in
  WrenHandle* value = make_handle(fn);
  value->value = ClosureObject::make_closure(*this, fn);

  fn->append_code(Code::CALL_0 + num_params);
  fn->append_code((method >> 8) & 0xff);
  fn->append_code(method & 0xff);
  fn->append_code(Code::RETURN);
  fn->append_code(Code::END);
  fn->debug().set_lines({0, 0, 0, 0, 0});
  fn->bind_name(signature);

  return value;
}

InterpretRet WrenVM::wren_call(WrenHandle* method) {
  // [method] must have been created by a call to [make_call_handle], the
  // arguments to the method must be already on the stack, the receiver should
  // be in slot 0 with the remaining arguments following it, in order, it is
  // an error if the number of arguments provided does not match the method's
  // signature
  //
  // after this returns, you can access the return value from slot 0 on the
  // stack

  ASSERT(method != nullptr, "method cannot be nullptr");
  ASSERT(method->value.is_closure(), "method must be a method handle");
  ASSERT(fiber_ != nullptr, "must set up arguments for call first");
  ASSERT(api_stack_ != nullptr, "must set up arguments for call first");
  ASSERT(fiber_->empty_frame(), "can not call from a foreign method");

  ClosureObject* closure = method->value.as_closure();
  ASSERT(fiber_->stack_size() >= closure->fn()->arity(),
      "stack must have enough arguments for method");

  // clear the API stack, now that `wren_call()` has control, we no longer
  // need it, we use this being non-nil to tell if re-entrant calls to
  // foreign methods are hanppening, so it's important to clear it out now
  // so that you can call foreign methods from within calls to `wren_call()`
  api_stack_ = nullptr;

  // discard any extra temporary slots, we take for granted that the stub
  // function has exactly one slot for each arguments
  fiber_->resize_stack(closure->fn()->max_slots());
  fiber_->call_function(*this, closure, 0);
  InterpretRet result = interpret(fiber_);

  // if the call didn't abort, then set up the API stack to point to the
  // beginning of the stack so the host can access the call's return value
  if (fiber_ != nullptr)
    api_stack_ = fiber_->values_at_beg();

  return result;
}

WrenHandle* WrenVM::make_handle(const Value& value) {
  // creates a new [WrenHandle] for [value]

  PinnedGuard guard{*this};
  if (value.is_object())
    guard.set_guard_object(value.as_object());

  // make a handle for it
  WrenHandle* handle = new WrenHandle(value);

  // add it to the front of the linked list of handles
  if (handles_ != nullptr)
    handles_->prev = handle;
  handle->next = handles_;
  handles_ = handle;

  return handle;
}

void WrenVM::release_handle(WrenHandle* handle) {
  ASSERT(handle != nullptr, "handle cannot be nullptr");

  // update the VM's head pointer if we're releasing the first handle
  if (handles_ == handle)
    handles_ = handle->next;

  // unlink it from the list
  if (handle->prev != nullptr)
    handle->prev->next = handle->next;
  if (handle->next != nullptr)
    handle->next->prev = handle->prev;

  // clear it out, this isn't strictly necessary since we're going to free it
  // but it makes for easier debugging
  handle->clear();
  delete handle;
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

  WrenForeignFn foreign_fn = method.foreign();
  auto finalizer = foreign_fn.target<void (void*)>();
  finalizer(foreign->data());
}

int WrenVM::get_slot_count(void) const {
  if (api_stack_ == nullptr)
    return 0;
  return Xt::as_type<int>(fiber_->values_at_top() - api_stack_);
}

void WrenVM::ensure_slots(int num_slots) {
  // if we donot have a fiber accessible, create one for the API to use
  if (api_stack_ == nullptr) {
    fiber_ = FiberObject::make_fiber(*this, nullptr);
    api_stack_ = fiber_->values_at(0);
  }
  int current_size = Xt::as_type<int>(fiber_->values_at_top() - api_stack_);
  if (current_size >= num_slots)
    return;

  // grow the stack if needed
  int needed = num_slots + Xt::as_type<int>(api_stack_ - fiber_->values_at(0));
  fiber_->ensure_stack(*this, needed);
}

WrenType WrenVM::get_slot_type(int slot) const {
  validate_api_slot(slot);

  const Value& v = api_stack_[slot];
  if (v.is_boolean())
    return WrenType::BOOLEAN;
  if (v.is_numeric())
    return WrenType::NUMERIC;
  if (v.is_foreign())
    return WrenType::FOREIGN;
  if (v.is_list())
    return WrenType::LIST;
  if (v.is_nil())
    return WrenType::NIL;
  if (v.is_string())
    return WrenType::STRING;

  return WrenType::UNKNOWN;
}

bool WrenVM::get_slot_bool(int slot) const {
  validate_api_slot(slot);

  ASSERT(api_stack_[slot].is_boolean(), "slot must hold a boolean");
  return api_stack_[slot].as_boolean();
}

double WrenVM::get_slot_double(int slot) const {
  validate_api_slot(slot);

  ASSERT(api_stack_[slot].is_numeric(), "slot must hold a numeric");
  return api_stack_[slot].as_numeric();
}

const char* WrenVM::get_slot_string(int slot) const {
  validate_api_slot(slot);

  ASSERT(api_stack_[slot].is_string(), "slot must hold a string");
  return api_stack_[slot].as_cstring();
}

WrenHandle* WrenVM::get_slot_hanle(int slot) {
  validate_api_slot(slot);

  return make_handle(api_stack_[slot]);
}

void* WrenVM::get_slot_foreign(int slot) const {
  validate_api_slot(slot);

  const Value& foreign_val = api_stack_[slot];
  ASSERT(foreign_val.is_foreign(), "slot must hold a foreign instance");
  return foreign_val.as_foreign()->data();
}

void WrenVM::set_slot(int slot, const Value& value) {
  // stores [value] in [slot] in the foreign call stack

  validate_api_slot(slot);
  api_stack_[slot] = value;
}

void* WrenVM::set_slot_new_foreign(int slot, int class_slot, sz_t size) {
  validate_api_slot(slot);
  validate_api_slot(class_slot);
  ASSERT(api_stack_[class_slot].is_class(), "slot must hold a class");

  ClassObject* cls = api_stack_[class_slot].as_class();
  ASSERT(cls->num_fields() == -1, "class must be a foreign class");

  ForeignObject* foreign = ForeignObject::make_foreign(*this, cls, size);
  api_stack_[slot] = foreign;

  return foreign->data();
}

int WrenVM::get_list_count(int slot) {
  validate_api_slot(slot);
  ASSERT(api_stack_[slot].is_list(), "slot must hold a list");

  return api_stack_[slot].as_list()->count();
}

void WrenVM::get_list_element(int list_slot, int index, int element_slot) {
  validate_api_slot(list_slot);
  validate_api_slot(element_slot);
  ASSERT(api_stack_[list_slot].is_list(), "slot must hold a list");

  api_stack_[element_slot] = api_stack_[list_slot].as_list()->get_element(index);
}

void WrenVM::insert_into_list(int list_slot, int index, int element_slot) {
  validate_api_slot(list_slot);
  validate_api_slot(element_slot);
  ASSERT(api_stack_[list_slot].is_list(), "must insert into a list");

  ListObject* list = api_stack_[list_slot].as_list();
  // negative indices count from the end
  if (index < 0)
    index = list->count() + 1 + index;
  ASSERT(index <= list->count(), "index out of bounds");

  list->insert(index, api_stack_[element_slot]);
}

void WrenVM::get_variable(const str_t& module, const str_t& name, int slot) {
  ASSERT(!module.empty(), "module cannot be empty");
  ASSERT(!name.empty(), "variable name cannot be empty");
  validate_api_slot(slot);

  StringObject* module_name = StringObject::make_string(*this, module);
  ModuleObject* module_obj;
  {
    PinnedGuard guard(*this, module_name);

    module_obj = get_module(module_name);
    ASSERT(module_obj != nullptr, "could not find module");
  }

  int variable_slot = module_obj->find_variable(name);
  ASSERT(variable_slot != -1, "could not find variable");

  set_slot(slot, module_obj->get_variable(variable_slot));
}

void WrenVM::abort_fiber(int slot) {
  validate_api_slot(slot);
  fiber_->set_error(api_stack_[slot]);
}

void wrenCollectGarbage(WrenVM& vm) {
  vm.collect();
}

WrenHandle* wrenMakeCallHandle(WrenVM& vm, const str_t& signature) {
  return vm.make_call_handle(signature);
}

InterpretRet wrenCall(WrenVM& vm, WrenHandle* method) {
  return vm.wren_call(method);
}

void wrenReleaseHandle(WrenVM& vm, WrenHandle* handle) {
  vm.release_handle(handle);
}

int wrenGetSlotCount(WrenVM& vm) {
  return vm.get_slot_count();
}

void wrenEnsureSlots(WrenVM& vm, int num_slots) {
  vm.ensure_slots(num_slots);
}

WrenType wrenGetSlotType(WrenVM& vm, int slot) {
  return vm.get_slot_type(slot);
}

bool wrenGetSlotBool(WrenVM& vm, int slot) {
  return vm.get_slot_bool(slot);
}

double wrenGetSlotDouble(WrenVM& vm, int slot) {
  return vm.get_slot_double(slot);
}

const char* wrenGetSlotString(WrenVM& vm, int slot) {
  return vm.get_slot_string(slot);
}

WrenHandle* wrenGetSlotHandle(WrenVM& vm, int slot) {
  return vm.get_slot_hanle(slot);
}

void* wrenGetSlotForeign(WrenVM& vm, int slot) {
  return vm.get_slot_foreign(slot);
}

void wrenSetSlotBool(WrenVM& vm, int slot, bool value) {
  vm.set_slot(slot, value);
}

void wrenSetSlotDouble(WrenVM& vm, int slot, double value) {
  vm.set_slot(slot, value);
}

void wrenSetSlotNil(WrenVM& vm, int slot) {
  vm.set_slot(slot, nullptr);
}

void wrenSetSlotString(WrenVM& vm, int slot, const str_t& text) {
  ASSERT(!text.empty(), "string cannot be empty");
  vm.set_slot(slot, StringObject::make_string(vm, text));
}

void wrenSetSlotHandle(WrenVM& vm, int slot, WrenHandle* handle) {
  ASSERT(handle != nullptr, "handle cannot be nullptr");
  vm.set_slot(slot, handle->value);
}

void wrenSetSlotNewList(WrenVM& vm, int slot) {
  vm.set_slot(slot, ListObject::make_list(vm));
}

void* wrenSetSlotNewForeign(WrenVM& vm, int slot, int class_slot, sz_t size) {
  return vm.set_slot_new_foreign(slot, class_slot, size);
}

int wrenGetListCount(WrenVM& vm, int slot) {
  return vm.get_list_count(slot);
}

void wrenGetListElement(WrenVM& vm, int list_slot, int index, int element_slot) {
  vm.get_list_element(list_slot, index, element_slot);
}

void wrenInsertInList(WrenVM& vm, int list_slot, int index, int element_slot) {
  vm.insert_into_list(list_slot, list_slot, element_slot);
}

void wrenGetVariable(
    WrenVM& vm, const str_t& module, const str_t& name, int to_slot) {
  vm.get_variable(module, name, to_slot);
}

void wrenAbortFiber(WrenVM& vm, int slot) {
  vm.abort_fiber(slot);
}

void* wrenGetUserData(WrenVM& vm) {
  return vm.get_user_data();
}

void wrenSetUserData(WrenVM& vm, void* user_data) {
  vm.set_user_data(user_data);
}

}
