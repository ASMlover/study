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
#include "VM.hh"
#include "Memory.hh"

namespace ms {

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
  ObjFunction* fn = closure->function();
  int max_arity = fn->arity();
  int min_arity = fn->min_arity();

  if (fn->has_rest_param()) {
    // Fixed params count = arity - 1 (rest param is always last)
    int fixed = max_arity - 1;
    if (arg_count < fixed) {
      runtime_error(std::format("Expected at least {} arguments but got {}.", fixed, arg_count));
      return false;
    }
    // Collect trailing args into ObjList for rest param
    int rest_count = arg_count - fixed;
    ObjList* rest_list = allocate<ObjList>();
    push(Value(static_cast<Object*>(rest_list))); // keep GC-reachable
    for (int i = 0; i < rest_count; ++i) {
      rest_list->elements().push_back(*(stack_top_ - rest_count - 1 + i));
    }
    // Trim stack: remove individual rest args, keep fixed args
    // stack_top_ currently = base + fixed + rest_count + 1 (rest_list on top)
    // We want: base + fixed + 1 (rest_list in last slot)
    Value* base = stack_top_ - rest_count - 1;
    *base = Value(static_cast<Object*>(rest_list)); // put list in rest slot
    stack_top_ = base + 1;
    arg_count = fixed + 1; // fixed params + rest list
  } else {
    if (arg_count < min_arity || arg_count > max_arity) {
      if (min_arity == max_arity) {
        runtime_error(std::format("Expected {} arguments but got {}.", max_arity, arg_count));
      } else {
        runtime_error(std::format("Expected {}-{} arguments but got {}.",
            min_arity, max_arity, arg_count));
      }
      return false;
    }
    // Fill missing args with defaults
    if (arg_count < max_arity && fn->default_base() >= 0) {
      const auto& constants = fn->chunk().constants();
      for (int i = arg_count; i < max_arity; ++i) {
        int def_idx = fn->default_base() + i - min_arity;
        push(constants[static_cast<sz_t>(def_idx)]);
      }
      arg_count = max_arity;
    }
  }

  if (frame_count_ == static_cast<int>(kFRAMES_MAX)) {
    runtime_error("Stack overflow.");
    return false;
  }

  CallFrame& frame = frames_[frame_count_++];
  frame.closure = closure;
  frame.ip = fn->chunk().code_data();
  frame.slots = stack_top_ - arg_count - 1;
  frame.deferred.clear();
  frame.returning = false;
  return true;
}

bool VM::call_value(Value callee, int arg_count) noexcept {
  if (callee.is_object()) {
    switch (callee.as_object()->type()) {
    case ObjectType::OBJ_CLOSURE: {
      ObjClosure* cl = as_closure(callee);
      if (cl->function()->is_generator()) {
        // Calling a generator function creates a coroutine, not an immediate call
        ObjCoroutine* coro = allocate<ObjCoroutine>(cl);
        // Save initial args so first resume can pass them to the closure
        Value* args_base = stack_top_ - arg_count;
        coro->init_args().assign(args_base, args_base + arg_count);
        stack_top_ -= arg_count + 1;
        push(Value(static_cast<Object*>(coro)));
        return true;
      }
      return call(cl, arg_count);
    }

    case ObjectType::OBJ_NATIVE: {
      NativeFn native = as_native(callee)->function();
      Value result = native(*this, arg_count, stack_top_ - arg_count);
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

bool VM::resume_coroutine(ObjCoroutine* coro, Value sent_val, u8_t result_reg) noexcept {
  if (coro->state() == CoroutineState::DEAD) {
    // Place nil in result slot — coroutine exhausted
    frames_[frame_count_ - 1].slots[result_reg] = Value();
    return true;
  }
  if (coro->state() == CoroutineState::RUNNING) {
    runtime_error("Cannot resume a running coroutine.");
    return false;
  }

  CoroutineEntry ce;
  ce.coro = coro;
  ce.parent_frame_count = frame_count_;
  ce.parent_stack_top = stack_top_;
  ce.result_reg = result_reg;
  coro_stack_.push_back(ce);
  coro->set_state(CoroutineState::RUNNING);
  coro->sent_value() = sent_val;

  if (coro->saved_frames().empty()) {
    // First resume: call the generator closure with saved initial args
    push(Value(static_cast<Object*>(coro))); // slot 0 = receiver
    int init_argc = static_cast<int>(coro->init_args().size());
    for (auto& v : coro->init_args()) push(v);
    if (!call(coro->closure(), init_argc)) {
      coro_stack_.pop_back();
      return false;
    }
  } else {
    // Restore suspended state using properly-saved SavedCallFrame entries
    int saved_count = static_cast<int>(coro->saved_frames().size());
    Value* new_base = stack_top_;
    for (sz_t i = 0; i < coro->saved_stack().size(); ++i)
      new_base[i] = coro->saved_stack()[i];
    stack_top_ = new_base + coro->saved_stack_top_offset();
    for (int fi = 0; fi < saved_count; ++fi) {
      const SavedCallFrame& sf = coro->saved_frames()[static_cast<sz_t>(fi)];
      CallFrame& f = frames_[frame_count_ + fi];
      f.closure = sf.closure;
      f.ip = sf.ip;
      f.slots = new_base + sf.slots_offset;
      f.deferred.clear();
      f.pending_return = sf.pending_return;
      f.returning = sf.returning;
    }
    frame_count_ += saved_count;
  }
  return true;
}

bool VM::invoke_from_class(ObjClass* klass, ObjString* name, int arg_count) noexcept {
  Value method;
  if (!klass->methods().get(name, &method)) {
    runtime_error(std::format("Undefined property '{}'.", name->value()));
    return false;
  }

  Value dummy;
  if (klass->has_abstract_methods() && klass->abstract_methods().get(name, &dummy)) {
    runtime_error(std::format("Cannot call abstract method '{}' on '{}'.", name->value(), klass->name()->value()));
    return false;
  }

  return call(as_closure(method), arg_count);
}

bool VM::invoke_operator(ObjString* op_name) noexcept {
  // Stack: [... lhs, rhs]  peek(1)=lhs, peek(0)=rhs
  // Check if lhs is an instance with the operator method
  if (peek(1).is_instance()) {
    ObjInstance* instance = as_instance(peek(1));
    Value method;
    if (instance->klass()->methods().get(op_name, &method)) {
      // Set up: stack_top_[-2] = lhs (receiver), stack_top_[-1] = rhs (arg)
      return call(as_closure(method), 1);
    }
  }
  return false;
}

void VM::bind_method(ObjClass* klass, ObjString* name) noexcept {
  Value method;
  if (!klass->methods().get(name, &method)) {
    runtime_error(std::format("Undefined property '{}'.", name->value()));
    return;
  }

  Value dummy;
  if (klass->has_abstract_methods() && klass->abstract_methods().get(name, &dummy)) {
    runtime_error(std::format("Cannot call abstract method '{}' on '{}'.", name->value(), klass->name()->value()));
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
    write_barrier_value(upvalue, upvalue->closed());
    open_upvalues_ = upvalue->next_upvalue();
  }
}

} // namespace ms
