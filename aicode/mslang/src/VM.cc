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
#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdarg>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <format>
#include <random>
#include <sstream>
#include <unordered_set>
#include "VM.hh"
#include "Compiler.hh"
#include "Memory.hh"
#include "Debug.hh"
#include "Logger.hh"
#include "Serializer.hh"

namespace ms {

// --- VM implementation ---

VM::VM() noexcept {
  reset_stack();
  init_string_ = copy_string("init", 4);
  op_add_string_ = copy_string("__add", 5);
  op_sub_string_ = copy_string("__sub", 5);
  op_mul_string_ = copy_string("__mul", 5);
  op_div_string_ = copy_string("__div", 5);
  op_mod_string_ = copy_string("__mod", 5);
  op_eq_string_ = copy_string("__eq", 4);
  op_lt_string_ = copy_string("__lt", 4);
  op_gt_string_ = copy_string("__gt", 4);
  op_str_string_ = copy_string("__str", 5);
  op_finalize_string_ = copy_string("__finalize", 10);

  // Register native functions
  register_natives();

}

VM::~VM() noexcept {
  init_string_ = nullptr;
  op_add_string_ = nullptr;
  op_sub_string_ = nullptr;
  op_mul_string_ = nullptr;
  op_div_string_ = nullptr;
  op_mod_string_ = nullptr;
  op_eq_string_ = nullptr;
  op_lt_string_ = nullptr;
  op_gt_string_ = nullptr;
  op_str_string_ = nullptr;
  free_objects();
}

void VM::reset_stack() noexcept {
  stack_top_ = stack_.data();
  frame_count_ = 0;
  open_upvalues_ = nullptr;
}

void VM::push(Value value) noexcept {
  if (stack_top_ >= stack_.data() + stack_.size()) {
    runtime_error("Stack overflow.");
    std::exit(70);
  }
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

str_t VM::get_source_line(const str_t& script_path, int line) const noexcept {
  auto it = source_cache_.find(script_path);
  if (it == source_cache_.end()) return "";

  const str_t& source = it->second;
  int current_line = 1;
  sz_t line_start = 0;

  for (sz_t i = 0; i < source.size(); i++) {
    if (current_line == line) {
      line_start = i;
      sz_t line_end = source.find('\n', i);
      if (line_end == str_t::npos) line_end = source.size();
      // Trim trailing \r
      if (line_end > line_start && source[line_end - 1] == '\r') line_end--;
      return source.substr(line_start, line_end - line_start);
    }
    if (source[i] == '\n') current_line++;
  }
  return "";
}

void VM::runtime_error(strv_t message) noexcept {
  ObjString* msg_str = copy_string(message.data(), message.length());
  pending_exception_ = Value(static_cast<Object*>(msg_str));

  if (!exception_handlers_.empty()) {
    return;
  }

  std::cerr << std::format("RuntimeError: {}", message) << std::endl;

  for (int i = frame_count_ - 1; i >= 0; i--) {
    CallFrame& frame = frames_[i];
    ObjFunction* function = frame.closure->function();
    sz_t instruction = static_cast<sz_t>(frame.ip - function->chunk().code_data() - 1);
    int line = function->chunk().line_at(instruction);
    int column = function->chunk().column_at(instruction);
    int token_len = function->chunk().token_length_at(instruction);

    const str_t& path = function->script_path();
    strv_t func_name = function->name() != nullptr
        ? strv_t(function->name()->value()) : strv_t("script");

    std::cerr << std::format("  at {}() [{}:{}:{}]",
        func_name, path, line, column) << std::endl;

    str_t source_line = get_source_line(path, line);
    if (!source_line.empty()) {
      std::cerr << std::format("  |  {}", source_line) << std::endl;
      if (column > 0 && token_len > 0) {
        str_t indicator(static_cast<sz_t>(column - 1), ' ');
        str_t carets(static_cast<sz_t>(token_len), '^');
        std::cerr << std::format("  |  {}{}", indicator, carets) << std::endl;
      }
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
  young_bytes_ += sizeof(T);

#ifdef MAPLE_DEBUG_STRESS_GC
  collect_garbage();
#else
  // Minor GC on nursery threshold; major GC on total heap threshold
  if (young_bytes_ > next_minor_gc_) {
    minor_gc();
  }
  if (bytes_allocated_ > next_gc_) {
    major_gc();
  }
#endif

  auto* object = new T(std::forward<Args>(args)...);
  object->set_generation(GcGeneration::YOUNG);
  object->set_next(young_objects_);
  young_objects_ = object;

#ifdef MAPLE_DEBUG_LOG_GC
  auto& logger = Logger::get_instance();
  logger.debug("GC", "allocate {} for {} ({})",
      sizeof(T), static_cast<void*>(object), object->stringify());
#endif

  return object;
}

// Explicit instantiations for types allocated from outside VM.cc
template ObjFunction* VM::allocate<ObjFunction>();
template ObjList* VM::allocate<ObjList>();
template ObjWeakRef* VM::allocate<ObjWeakRef, Object*>(Object*&&);
template ObjStringBuilder* VM::allocate<ObjStringBuilder>();

void VM::mark_roots() noexcept {
  // Mark the stack
  for (Value* slot = stack_.data(); slot < stack_top_; slot++) {
    mark_value(*slot);
  }

  // Mark call frames and their deferred closures
  for (int i = 0; i < frame_count_; i++) {
    mark_object(frames_[i].closure);
    for (auto* deferred : frames_[i].deferred) {
      mark_object(deferred);
    }
    if (frames_[i].returning) {
      mark_value(frames_[i].pending_return);
    }
  }

  // Mark open upvalues
  for (ObjUpvalue* upvalue = open_upvalues_;
       upvalue != nullptr; upvalue = upvalue->next_upvalue()) {
    mark_object(upvalue);
  }

  // Mark globals
  globals_.mark_table();

  // Mark compiler roots
  mark_compiler_roots();

  // Mark init string and operator method names
  mark_object(init_string_);
  mark_object(op_add_string_);
  mark_object(op_sub_string_);
  mark_object(op_mul_string_);
  mark_object(op_div_string_);
  mark_object(op_mod_string_);
  mark_object(op_eq_string_);
  mark_object(op_lt_string_);
  mark_object(op_gt_string_);
  mark_object(op_str_string_);
  mark_object(op_finalize_string_);

  // Mark pending finalizers (kept alive until __finalize runs)
  for (auto* obj : pending_finalizers_) {
    mark_object(obj);
  }

  // Mark pending exception
  mark_value(pending_exception_);

  // Mark pending import modules
  for (auto& pending : pending_imports_) {
    mark_object(pending.module);
    for (auto& req : pending.from_imports) {
      mark_object(req.name);
      if (req.alias) mark_object(req.alias);
    }
  }
}

void VM::push_gray(Object* object) noexcept {
  gray_stack_.push_back(object);
}

void VM::remember(Object* object) noexcept {
  remembered_set_.push_back(object);
}

void VM::trace_references() noexcept {
  while (!gray_stack_.empty()) {
    Object* object = gray_stack_.back();
    gray_stack_.pop_back();

#ifdef MAPLE_DEBUG_LOG_GC
    auto& logger = Logger::get_instance();
    logger.debug("GC", "blacken {} ({})",
        static_cast<void*>(object), object->stringify());
#endif

    object->trace_references();
  }
}

void VM::sweep() noexcept {
  // Sweep old generation list
  Object* previous = nullptr;
  Object* object = old_objects_;

  while (object != nullptr) {
    if (object->is_marked()) {
      object->set_marked(false);
      previous = object;
      object = object->next();
    } else {
      Object* unreached = object;
      object = object->next();
      if (previous != nullptr) {
        previous->set_next(object);
      } else {
        old_objects_ = object;
      }

#ifdef MAPLE_DEBUG_LOG_GC
      auto& logger = Logger::get_instance();
      logger.debug("GC", "free old {} ({})",
          static_cast<void*>(unreached), unreached->stringify());
#endif

      bytes_allocated_ -= unreached->size();
      delete unreached;
    }
  }
}

void VM::collect_garbage() noexcept {
  // Full stop-the-world GC (used by stress GC mode)
  major_gc();
}

// --- Minor GC: collect young generation only ---
void VM::mark_remembered_set() noexcept {
  for (Object* obj : remembered_set_) {
    // Mark reachable young objects from old-gen references
    obj->trace_references();
  }
}

void VM::promote_object(Object* object) noexcept {
  object->set_generation(GcGeneration::OLD);
  object->set_next(old_objects_);
  old_objects_ = object;
}

void VM::sweep_young() noexcept {
  Object* previous = nullptr;
  Object* object = young_objects_;

  while (object != nullptr) {
    if (object->is_marked()) {
      object->set_marked(false);
      object->increment_age();

      if (object->age() >= kGC_PROMOTE_AGE) {
        // Promote: unlink from young list, add to old list
        Object* promoted = object;
        object = object->next();
        if (previous != nullptr) {
          previous->set_next(object);
        } else {
          young_objects_ = object;
        }
        young_bytes_ -= promoted->size();
        promote_object(promoted);

#ifdef MAPLE_DEBUG_LOG_GC
        auto& logger = Logger::get_instance();
        logger.debug("GC", "promote {} ({})",
            static_cast<void*>(promoted), promoted->stringify());
#endif
      } else {
        previous = object;
        object = object->next();
      }
    } else {
      Object* unreached = object;
      object = object->next();
      if (previous != nullptr) {
        previous->set_next(object);
      } else {
        young_objects_ = object;
      }

#ifdef MAPLE_DEBUG_LOG_GC
      auto& logger = Logger::get_instance();
      logger.debug("GC", "free young {} ({})",
          static_cast<void*>(unreached), unreached->stringify());
#endif

      sz_t obj_size = unreached->size();
      bytes_allocated_ -= obj_size;
      young_bytes_ -= obj_size;
      delete unreached;
    }
  }
}

void VM::nullify_weak_refs() noexcept {
  auto new_end = std::remove_if(weak_refs_.begin(), weak_refs_.end(),
    [](ObjWeakRef* ref) {
      // If the weak ref object itself is unmarked, it will be freed — remove from list
      if (!ref->is_marked()) return true;
      // If the target is unmarked, null it out (target is about to be freed)
      if (ref->target() != nullptr && !ref->target()->is_marked()) {
        ref->clear();
      }
      return false;
    });
  weak_refs_.erase(new_end, weak_refs_.end());
}

bool VM::needs_finalization(Object* object) noexcept {
  if (object->is_marked()) return false;
  if (object->is_finalized()) return false;
  if (object->type() != ObjectType::OBJ_INSTANCE) return false;
  auto* inst = static_cast<ObjInstance*>(object);
  // Only finalize if the class is alive (marked) and has __finalize
  if (!inst->klass()->is_marked()) return false;
  Value method;
  return inst->klass()->methods().get(op_finalize_string_, &method);
}

void VM::mark_finalizable() noexcept {
  bool found = false;
  // Scan young generation for unreachable finalizable objects
  for (Object* obj = young_objects_; obj != nullptr; obj = obj->next()) {
    if (needs_finalization(obj)) {
      mark_object(obj);
      obj->set_finalized(true);
      pending_finalizers_.push_back(obj);
      finalize_pending_ = true;
      found = true;
    }
  }
  // Scan old generation
  for (Object* obj = old_objects_; obj != nullptr; obj = obj->next()) {
    if (needs_finalization(obj)) {
      mark_object(obj);
      obj->set_finalized(true);
      pending_finalizers_.push_back(obj);
      finalize_pending_ = true;
      found = true;
    }
  }
  // Re-trace to mark everything reachable from finalizable objects
  if (found) {
    trace_references();
  }
}

void VM::run_pending_finalizers() noexcept {
  if (in_finalizer_) return;  // prevent re-entrancy
  in_finalizer_ = true;
  finalize_pending_ = false;

  while (!pending_finalizers_.empty()) {
    auto* obj = pending_finalizers_.back();
    pending_finalizers_.pop_back();

    if (obj->type() == ObjectType::OBJ_INSTANCE) {
      auto* inst = static_cast<ObjInstance*>(obj);
      Value method;
      if (inst->klass()->methods().get(op_finalize_string_, &method)) {
        // Ensure stack_top_ is past the current frame's full register window
        // so the finalizer frame doesn't overlap and corrupt locals.
        CallFrame& cur = frames_[frame_count_ - 1];
        Value* frame_end = cur.slots + cur.closure->function()->max_stack_size();
        if (stack_top_ < frame_end)
          stack_top_ = frame_end;

        Value* saved_top = stack_top_;
        int saved_base = base_frame_;
        base_frame_ = frame_count_;

        push(Value(static_cast<Object*>(inst)));
        call(as_closure(method), 0);
        run();

        base_frame_ = saved_base;
        stack_top_ = saved_top;
        continue;  // instance freed on next GC (finalized_ = true)
      }
    }
    // Object lost its __finalize (class collected?) — leave for normal sweep.
  }

  in_finalizer_ = false;
}

void VM::minor_gc() noexcept {
  gc_count_++;
#ifdef MAPLE_DEBUG_LOG_GC
  auto& logger = Logger::get_instance();
  logger.info("GC", "-- minor gc begin");
  sz_t before = bytes_allocated_;
#endif

  // Clear stale marks on old-generation objects from previous minor GC.
  // Without this, mark_object skips already-marked old objects and their
  // young-gen references go untraced, leading to premature collection.
  for (Object* obj = old_objects_; obj != nullptr; obj = obj->next()) {
    obj->set_marked(false);
  }

  // Mark roots (marks both young and old, but we only sweep young)
  mark_roots();

  // Mark references from old→young via remembered set
  mark_remembered_set();

  trace_references();

  // Mark unreachable objects with __finalize (keeps them alive for one more cycle)
  mark_finalizable();

  // Null out weak refs to unmarked young objects before sweep
  nullify_weak_refs();

  // Only sweep young generation
  sweep_young();

  // Clear remembered set (will be rebuilt by write barriers)
  remembered_set_.clear();

  next_minor_gc_ = young_bytes_ + kGC_NURSERY_SIZE;

#ifdef MAPLE_DEBUG_LOG_GC
  logger.info("GC", "-- minor gc end (collected {} bytes, from {} to {})",
      before - bytes_allocated_, before, bytes_allocated_);
#endif
}

// --- Major GC: full mark-and-sweep of both generations ---
void VM::major_gc() noexcept {
  gc_count_++;
#ifdef MAPLE_DEBUG_LOG_GC
  auto& logger = Logger::get_instance();
  logger.info("GC", "-- major gc begin");
  sz_t before = bytes_allocated_;
#endif

  // Clear all marks before full mark phase.
  // Without this, stale marks from a previous minor GC cause mark_object
  // to skip already-marked old objects, leaving their references untraced.
  for (Object* obj = old_objects_; obj != nullptr; obj = obj->next()) {
    obj->set_marked(false);
  }
  for (Object* obj = young_objects_; obj != nullptr; obj = obj->next()) {
    obj->set_marked(false);
  }

  mark_roots();
  trace_references();

  // Mark unreachable objects with __finalize (keeps them alive for one more cycle)
  mark_finalizable();

  // Remove interned strings that are unmarked before sweep
  strings_.remove_white();

  // Null out weak refs to unmarked objects before sweep
  nullify_weak_refs();

  // Sweep both generations
  sweep_young();
  sweep();

  // Clear remembered set
  remembered_set_.clear();

  next_gc_ = bytes_allocated_ * kGC_HEAP_GROW;
  next_minor_gc_ = young_bytes_ + kGC_NURSERY_SIZE;

#ifdef MAPLE_DEBUG_LOG_GC
  logger.info("GC", "-- major gc end (collected {} bytes, from {} to {}, next at {})",
      before - bytes_allocated_, before, bytes_allocated_, next_gc_);
#endif
}

// --- Incremental major GC (amortized across VM instructions) ---
void VM::begin_major_gc() noexcept {
  gc_phase_ = GcPhase::MARKING;
  major_gc_requested_ = false;
  gray_stack_.clear();
  mark_roots();
}

void VM::incremental_mark_step() noexcept {
  sz_t work = 0;
  while (!gray_stack_.empty() && work < kGC_INCREMENTAL_WORK) {
    Object* object = gray_stack_.back();
    gray_stack_.pop_back();

#ifdef MAPLE_DEBUG_LOG_GC
    auto& logger = Logger::get_instance();
    logger.debug("GC", "incremental blacken {} ({})",
        static_cast<void*>(object), object->stringify());
#endif

    object->trace_references();
    work++;
  }

  if (gray_stack_.empty()) {
    // Marking done, transition to sweeping
    mark_finalizable();
    strings_.remove_white();
    nullify_weak_refs();
    gc_phase_ = GcPhase::SWEEPING;
    sweep_cursor_ = old_objects_;
    sweep_previous_ = nullptr;
  }
}

void VM::incremental_sweep_step() noexcept {
  sz_t work = 0;
  while (sweep_cursor_ != nullptr && work < kGC_INCREMENTAL_WORK) {
    if (sweep_cursor_->is_marked()) {
      sweep_cursor_->set_marked(false);
      sweep_previous_ = sweep_cursor_;
      sweep_cursor_ = sweep_cursor_->next();
    } else {
      Object* unreached = sweep_cursor_;
      sweep_cursor_ = sweep_cursor_->next();
      if (sweep_previous_ != nullptr) {
        sweep_previous_->set_next(sweep_cursor_);
      } else {
        old_objects_ = sweep_cursor_;
      }
      bytes_allocated_ -= unreached->size();
      delete unreached;
    }
    work++;
  }

  if (sweep_cursor_ == nullptr) {
    finish_major_gc();
  }
}

void VM::finish_major_gc() noexcept {
  gc_phase_ = GcPhase::IDLE;
  remembered_set_.clear();
  next_gc_ = bytes_allocated_ * kGC_HEAP_GROW;

#ifdef MAPLE_DEBUG_LOG_GC
  auto& logger = Logger::get_instance();
  logger.info("GC", "-- incremental major gc finished (heap at {})", bytes_allocated_);
#endif
}

void VM::incremental_gc_step() noexcept {
  switch (gc_phase_) {
    case GcPhase::IDLE:
      if (major_gc_requested_) {
        begin_major_gc();
      }
      break;
    case GcPhase::MARKING:
      incremental_mark_step();
      break;
    case GcPhase::SWEEPING:
      incremental_sweep_step();
      break;
  }
}

void VM::free_objects() noexcept {
  // Free young generation
  Object* object = young_objects_;
  while (object != nullptr) {
    Object* next = object->next();
    delete object;
    object = next;
  }
  young_objects_ = nullptr;

  // Free old generation
  object = old_objects_;
  while (object != nullptr) {
    Object* next = object->next();
    delete object;
    object = next;
  }
  old_objects_ = nullptr;
}

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
      NativeFn& native = as_native(callee)->function();
      Value result = native(arg_count, stack_top_ - arg_count);
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
      f.deferred = sf.deferred;
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
  if (klass->abstract_methods().get(name, &dummy)) {
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

bool VM::invoke(ObjString* name, int arg_count) noexcept {
  Value receiver = peek(arg_count);

  // Handle module property access (e.g., math.add(1, 2))
  if (is_obj_type(receiver, ObjectType::OBJ_MODULE)) {
    ObjModule* module = as_module(receiver);
    Value export_val;
    if (!module->exports().get(name, &export_val)) {
      runtime_error(std::format("Undefined export '{}'.", name->value()));
      return false;
    }
    stack_top_[-arg_count - 1] = export_val;
    return call_value(export_val, arg_count);
  }

  // Handle static method calls (e.g., Math.max(3, 5))
  if (is_obj_type(receiver, ObjectType::OBJ_CLASS)) {
    ObjClass* klass = as_class(receiver);
    Value method;
    if (!klass->static_methods().get(name, &method)) {
      runtime_error(std::format("Undefined static method '{}'.", name->value()));
      return false;
    }
    stack_top_[-arg_count - 1] = method;
    return call_value(method, arg_count);
  }

  // Handle string method calls (e.g., s.len(), s.upper(), s.split(","))
  if (is_obj_type(receiver, ObjectType::OBJ_STRING)) {
    ObjString* str = as_string(receiver);
    const str_t& sv = str->value();
    strv_t method_name = name->value();

    if (method_name == "len") {
      if (arg_count != 0) {
        runtime_error("len() takes no arguments.");
        return false;
      }
      stack_top_[-1] = Value(static_cast<double>(sv.length()));
      return true;
    } else if (method_name == "slice") {
      if (arg_count != 2) {
        runtime_error("slice() takes exactly 2 arguments (start, end).");
        return false;
      }
      if (!peek(1).is_number() || !peek(0).is_number()) {
        runtime_error("slice() arguments must be numbers.");
        return false;
      }
      auto len = static_cast<i32_t>(sv.length());
      auto start = static_cast<i32_t>(peek(1).as_number());
      auto end = static_cast<i32_t>(peek(0).as_number());
      if (start < 0) start += len;
      if (end < 0) end += len;
      start = std::clamp(start, 0, len);
      end = std::clamp(end, start, len);
      str_t result = sv.substr(start, end - start);
      ObjString* res = copy_string(result.data(), result.length());
      stack_top_[-arg_count - 1] = Value(static_cast<Object*>(res));
      stack_top_ -= arg_count;
      return true;
    } else if (method_name == "find") {
      if (arg_count != 1) {
        runtime_error("find() takes exactly 1 argument.");
        return false;
      }
      if (!peek(0).is_string()) {
        runtime_error("find() argument must be a string.");
        return false;
      }
      const str_t& needle = as_string(peek(0))->value();
      auto pos = sv.find(needle);
      double result = (pos == str_t::npos) ? -1.0 : static_cast<double>(pos);
      stack_top_[-arg_count - 1] = Value(result);
      stack_top_ -= arg_count;
      return true;
    } else if (method_name == "replace") {
      if (arg_count != 2) {
        runtime_error("replace() takes exactly 2 arguments (old, new).");
        return false;
      }
      if (!peek(1).is_string() || !peek(0).is_string()) {
        runtime_error("replace() arguments must be strings.");
        return false;
      }
      const str_t& old_str = as_string(peek(1))->value();
      const str_t& new_str = as_string(peek(0))->value();
      str_t result = sv;
      auto pos = result.find(old_str);
      if (pos != str_t::npos) {
        result.replace(pos, old_str.length(), new_str);
      }
      ObjString* res = copy_string(result.data(), result.length());
      stack_top_[-arg_count - 1] = Value(static_cast<Object*>(res));
      stack_top_ -= arg_count;
      return true;
    } else if (method_name == "split") {
      if (arg_count != 1) {
        runtime_error("split() takes exactly 1 argument.");
        return false;
      }
      if (!peek(0).is_string()) {
        runtime_error("split() argument must be a string.");
        return false;
      }
      const str_t& delim = as_string(peek(0))->value();
      ObjList* list = allocate<ObjList>();
      // Push list to protect from GC during string allocations
      push(Value(static_cast<Object*>(list)));
      if (delim.empty()) {
        // Split into individual characters
        for (sz_t i = 0; i < sv.length(); ++i) {
          ObjString* ch = copy_string(sv.data() + i, 1);
          list->elements().push_back(Value(static_cast<Object*>(ch)));
        }
      } else {
        sz_t start = 0;
        sz_t pos;
        while ((pos = sv.find(delim, start)) != str_t::npos) {
          str_t part = sv.substr(start, pos - start);
          ObjString* s = copy_string(part.data(), part.length());
          list->elements().push_back(Value(static_cast<Object*>(s)));
          start = pos + delim.length();
        }
        str_t part = sv.substr(start);
        ObjString* s = copy_string(part.data(), part.length());
        list->elements().push_back(Value(static_cast<Object*>(s)));
      }
      pop(); // pop GC guard
      // Replace receiver + arg with result
      stack_top_[-arg_count - 1] = Value(static_cast<Object*>(list));
      stack_top_ -= arg_count;
      return true;
    } else if (method_name == "upper") {
      if (arg_count != 0) {
        runtime_error("upper() takes no arguments.");
        return false;
      }
      str_t result = sv;
      std::transform(result.begin(), result.end(), result.begin(),
        [](unsigned char c) { return std::toupper(c); });
      ObjString* res = copy_string(result.data(), result.length());
      stack_top_[-1] = Value(static_cast<Object*>(res));
      return true;
    } else if (method_name == "lower") {
      if (arg_count != 0) {
        runtime_error("lower() takes no arguments.");
        return false;
      }
      str_t result = sv;
      std::transform(result.begin(), result.end(), result.begin(),
        [](unsigned char c) { return std::tolower(c); });
      ObjString* res = copy_string(result.data(), result.length());
      stack_top_[-1] = Value(static_cast<Object*>(res));
      return true;
    } else if (method_name == "trim") {
      if (arg_count != 0) {
        runtime_error("trim() takes no arguments.");
        return false;
      }
      auto first = sv.find_first_not_of(" \t\n\r\f\v");
      if (first == str_t::npos) {
        ObjString* res = copy_string("", 0);
        stack_top_[-1] = Value(static_cast<Object*>(res));
      } else {
        auto last = sv.find_last_not_of(" \t\n\r\f\v");
        str_t result = sv.substr(first, last - first + 1);
        ObjString* res = copy_string(result.data(), result.length());
        stack_top_[-1] = Value(static_cast<Object*>(res));
      }
      return true;
    } else if (method_name == "starts_with") {
      if (arg_count != 1) {
        runtime_error("starts_with() takes exactly 1 argument.");
        return false;
      }
      if (!peek(0).is_string()) {
        runtime_error("starts_with() argument must be a string.");
        return false;
      }
      const str_t& prefix = as_string(peek(0))->value();
      bool result = sv.starts_with(prefix);
      stack_top_[-arg_count - 1] = Value(result);
      stack_top_ -= arg_count;
      return true;
    } else if (method_name == "ends_with") {
      if (arg_count != 1) {
        runtime_error("ends_with() takes exactly 1 argument.");
        return false;
      }
      if (!peek(0).is_string()) {
        runtime_error("ends_with() argument must be a string.");
        return false;
      }
      const str_t& suffix = as_string(peek(0))->value();
      bool result = sv.ends_with(suffix);
      stack_top_[-arg_count - 1] = Value(result);
      stack_top_ -= arg_count;
      return true;
    } else if (method_name == "contains") {
      if (arg_count != 1) {
        runtime_error("contains() takes exactly 1 argument.");
        return false;
      }
      if (!peek(0).is_string()) {
        runtime_error("contains() argument must be a string.");
        return false;
      }
      const str_t& needle = as_string(peek(0))->value();
      bool result = sv.find(needle) != str_t::npos;
      stack_top_[-arg_count - 1] = Value(result);
      stack_top_ -= arg_count;
      return true;
    }
    runtime_error(std::format("Undefined string method '{}'.", method_name));
    return false;
  }

  if (is_obj_type(receiver, ObjectType::OBJ_LIST)) {
    ObjList* list = as_list(receiver);
    strv_t method_name = name->value();
    if (method_name == "len") {
      if (arg_count != 0) {
        runtime_error("len() takes no arguments.");
        return false;
      }
      stack_top_[-1] = Value(static_cast<double>(list->len()));
      return true;
    } else if (method_name == "push") {
      if (arg_count != 1) {
        runtime_error("push() takes exactly 1 argument.");
        return false;
      }
      Value val = stack_top_[-1];
      list->elements().push_back(val);
      stack_top_ -= 2; // pop arg and receiver
      push(Value());   // return nil
      return true;
    } else if (method_name == "pop") {
      if (arg_count != 0) {
        runtime_error("pop() takes no arguments.");
        return false;
      }
      if (list->elements().empty()) {
        runtime_error("Cannot pop from an empty list.");
        return false;
      }
      Value val = list->elements().back();
      list->elements().pop_back();
      stack_top_[-1] = val;
      return true;
    }
    runtime_error(std::format("Undefined list method '{}'.", method_name));
    return false;
  }

  if (is_obj_type(receiver, ObjectType::OBJ_TUPLE)) {
    ObjTuple* tuple = as_tuple(receiver);
    strv_t method_name = name->value();
    if (method_name == "len") {
      if (arg_count != 0) {
        runtime_error("len() takes no arguments.");
        return false;
      }
      stack_top_[-1] = Value(static_cast<double>(tuple->len()));
      return true;
    }
    runtime_error(std::format("Undefined tuple method '{}'.", method_name));
    return false;
  }

  if (is_obj_type(receiver, ObjectType::OBJ_MAP)) {
    ObjMap* map = as_map(receiver);
    strv_t method_name = name->value();
    if (method_name == "len") {
      if (arg_count != 0) {
        runtime_error("len() takes no arguments.");
        return false;
      }
      stack_top_[-1] = Value(static_cast<double>(map->len()));
      return true;
    } else if (method_name == "keys") {
      if (arg_count != 0) {
        runtime_error("keys() takes no arguments.");
        return false;
      }
      ObjList* keys = allocate<ObjList>();
      for (auto& [k, v] : map->entries()) {
        keys->elements().push_back(k);
      }
      stack_top_[-1] = Value(static_cast<Object*>(keys));
      return true;
    } else if (method_name == "values") {
      if (arg_count != 0) {
        runtime_error("values() takes no arguments.");
        return false;
      }
      ObjList* vals = allocate<ObjList>();
      for (auto& [k, v] : map->entries()) {
        vals->elements().push_back(v);
      }
      stack_top_[-1] = Value(static_cast<Object*>(vals));
      return true;
    } else if (method_name == "has") {
      if (arg_count != 1) {
        runtime_error("has() takes exactly 1 argument.");
        return false;
      }
      Value key = stack_top_[-1];
      bool found = map->entries().find(key) != map->entries().end();
      stack_top_ -= 2; // pop arg and receiver
      push(Value(found));
      return true;
    } else if (method_name == "remove") {
      if (arg_count != 1) {
        runtime_error("remove() takes exactly 1 argument.");
        return false;
      }
      Value key = stack_top_[-1];
      map->entries().erase(key);
      stack_top_ -= 2; // pop arg and receiver
      push(Value()); // return nil
      return true;
    }
    runtime_error(std::format("Undefined map method '{}'.", method_name));
    return false;
  }

  if (is_obj_type(receiver, ObjectType::OBJ_STRING_BUILDER)) {
    ObjStringBuilder* sb = as_string_builder(receiver);
    strv_t method_name = name->value();

    if (method_name == "append") {
      if (arg_count != 1) {
        runtime_error("append() takes exactly 1 argument.");
        return false;
      }
      str_t s = peek(0).stringify();
      sb->append(s);
      // Return the builder itself for chaining
      stack_top_[-arg_count - 1] = Value(static_cast<Object*>(sb));
      stack_top_ -= arg_count;
      return true;
    } else if (method_name == "build") {
      if (arg_count != 0) {
        runtime_error("build() takes no arguments.");
        return false;
      }
      const str_t& buf = sb->buffer();
      ObjString* result = copy_string(buf.data(), buf.length());
      stack_top_[-1] = Value(static_cast<Object*>(result));
      return true;
    } else if (method_name == "len") {
      if (arg_count != 0) {
        runtime_error("len() takes no arguments.");
        return false;
      }
      stack_top_[-1] = Value(static_cast<double>(sb->len()));
      return true;
    } else if (method_name == "clear") {
      if (arg_count != 0) {
        runtime_error("clear() takes no arguments.");
        return false;
      }
      sb->clear();
      stack_top_[-1] = Value(static_cast<Object*>(sb));
      return true;
    }
    runtime_error(std::format("Undefined StringBuilder method '{}'.", method_name));
    return false;
  }

  if (is_obj_type(receiver, ObjectType::OBJ_WEAK_REF)) {
    ObjWeakRef* ref = as_weak_ref(receiver);
    strv_t method_name = name->value();

    if (method_name == "get") {
      if (arg_count != 0) {
        runtime_error("get() takes no arguments.");
        return false;
      }
      if (ref->is_alive()) {
        stack_top_[-1] = Value(ref->target());
      } else {
        stack_top_[-1] = Value(); // nil
      }
      return true;
    } else if (method_name == "alive") {
      if (arg_count != 0) {
        runtime_error("alive() takes no arguments.");
        return false;
      }
      stack_top_[-1] = Value(ref->is_alive());
      return true;
    }
    runtime_error(std::format("Undefined weak_ref method '{}'.", method_name));
    return false;
  }

  if (is_obj_type(receiver, ObjectType::OBJ_FILE)) {
    ObjFile* file = as_file(receiver);
    strv_t method_name = name->value();

    if (method_name == "read") {
      if (arg_count != 0) {
        runtime_error("read() takes no arguments.");
        return false;
      }
      if (!file->is_open()) {
        runtime_error("Cannot read from a closed file.");
        return false;
      }
      str_t content = file->read_all();
      ObjString* result = copy_string(content.data(), content.length());
      stack_top_[-1] = Value(static_cast<Object*>(result));
      return true;
    } else if (method_name == "readline") {
      if (arg_count != 0) {
        runtime_error("readline() takes no arguments.");
        return false;
      }
      if (!file->is_open()) {
        runtime_error("Cannot read from a closed file.");
        return false;
      }
      if (file->eof()) {
        stack_top_[-1] = Value(); // nil
        return true;
      }
      str_t line = file->read_line();
      if (file->eof() && line.empty()) {
        stack_top_[-1] = Value(); // nil
        return true;
      }
      ObjString* result = copy_string(line.data(), line.length());
      stack_top_[-1] = Value(static_cast<Object*>(result));
      return true;
    } else if (method_name == "write") {
      if (arg_count != 1) {
        runtime_error("write() takes exactly 1 argument.");
        return false;
      }
      if (!file->is_open()) {
        runtime_error("Cannot write to a closed file.");
        return false;
      }
      str_t data = peek(0).stringify();
      file->write(data);
      stack_top_ -= 2; // pop arg and receiver
      push(Value());   // return nil
      return true;
    } else if (method_name == "close") {
      if (arg_count != 0) {
        runtime_error("close() takes no arguments.");
        return false;
      }
      file->close();
      stack_top_[-1] = Value(); // return nil
      return true;
    } else if (method_name == "eof") {
      if (arg_count != 0) {
        runtime_error("eof() takes no arguments.");
        return false;
      }
      stack_top_[-1] = Value(file->eof());
      return true;
    } else if (method_name == "is_open") {
      if (arg_count != 0) {
        runtime_error("is_open() takes no arguments.");
        return false;
      }
      stack_top_[-1] = Value(file->is_open());
      return true;
    }
    runtime_error(std::format("Undefined file method '{}'.", method_name));
    return false;
  }

  if (!is_obj_type(receiver, ObjectType::OBJ_INSTANCE)) {
    runtime_error("Only instances have methods.");
    return false;
  }

  ObjInstance* instance = as_instance(receiver);

  // Check for field first (field shadowing method)
  Value field_val;
  if (instance->get_field(name, &field_val)) {
    stack_top_[-arg_count - 1] = field_val;
    return call_value(field_val, arg_count);
  }

  return invoke_from_class(instance->klass(), name, arg_count);
}

void VM::bind_method(ObjClass* klass, ObjString* name) noexcept {
  Value method;
  if (!klass->methods().get(name, &method)) {
    runtime_error(std::format("Undefined property '{}'.", name->value()));
    return;
  }

  Value dummy;
  if (klass->abstract_methods().get(name, &dummy)) {
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

void VM::register_io_module() noexcept {
  ObjString* name = copy_string("io", 2);
  ObjModule* module = allocate<ObjModule>(name);
  push(Value(static_cast<Object*>(module))); // GC root

  // io.open(path, mode) -> ObjFile
  auto open_fn = [this](int arg_count, Value* args) -> Value {
    if (arg_count != 2 || !args[0].is_string() || !args[1].is_string()) {
      runtime_error("io.open() takes exactly 2 string arguments (path, mode).");
      return Value();
    }
    str_t path = str_t(as_string(args[0])->value());
    str_t mode = str_t(as_string(args[1])->value());
    ObjFile* file = allocate<ObjFile>(std::move(path), std::move(mode));
    if (!file->open()) {
      runtime_error(std::format("Could not open file '{}'.", file->path()));
      return Value();
    }
    return Value(static_cast<Object*>(file));
  };

  ObjString* open_name = copy_string("open", 4);
  ObjNative* open_native = allocate<ObjNative>(NativeFn(open_fn));
  module->exports().set(open_name, Value(static_cast<Object*>(open_native)));

  modules_["io"] = module;

  // Register as global so `import "io"; io.open(...)` works
  globals_.set(name, Value(static_cast<Object*>(module)));
  pop(); // module
}

void VM::import_module(ObjString* path) noexcept {
  // Check for built-in modules first
  if (path->value() == "io") {
    auto it = modules_.find("io");
    if (it != modules_.end()) return; // already imported
    register_io_module();
    return;
  }

  // Resolve path relative to current script
  str_t resolved = ModuleLoader::resolve_path(path->value(), current_script_path_);

  // Check cache
  auto it = modules_.find(resolved);
  if (it != modules_.end()) {
    return; // already imported
  }

  // Read source file via ModuleLoader
  auto source_opt = ModuleLoader::read_source(resolved);
  if (!source_opt.has_value()) {
    runtime_error(std::format("Could not open module '{}'.", resolved));
    return;
  }

  // Cache module source for error reporting
  source_cache_[resolved] = *source_opt;

  // Try loading from .msc cache, fall back to compilation
  str_t msc = msc_path_for(resolved);
  const auto& source = *source_opt;
  ObjFunction* function = try_load_cache(msc, source);
  if (!function) {
    function = compile(strv_t(source), resolved);
    if (function == nullptr) {
      runtime_error(std::format("Could not compile module '{}'.", resolved));
      return;
    }
    serialize(function, msc, source); // best-effort cache write
  }

  // Snapshot current global keys before module execution
  std::vector<ObjString*> pre_keys;
  for (auto& entry : globals_.entries()) {
    if (entry.key != nullptr) {
      pre_keys.push_back(entry.key);
    }
  }

  // Create module object and cache it (use resolved path as key)
  ObjString* resolved_str = copy_string(resolved.c_str(), resolved.size());
  ObjModule* module = allocate<ObjModule>(resolved_str);
  modules_[resolved] = module;

  // Save current script path and switch to module path
  str_t previous_path = current_script_path_;
  current_script_path_ = resolved;

  // Execute module
  ObjClosure* closure = allocate<ObjClosure>(function);
  push(Value(static_cast<Object*>(closure)));
  call(closure, 0);

  // Track this import so OP_RETURN can collect exports
  pending_imports_.push_back({frame_count_ - 1, module, std::move(pre_keys), {}, std::move(previous_path)});
}

InterpretResult VM::interpret(strv_t source) noexcept {
  return interpret(source, "");
}

InterpretResult VM::interpret(strv_t source, strv_t script_path) noexcept {
  current_script_path_ = str_t(script_path);

  str_t path_key = current_script_path_.empty() ? "<repl>" : current_script_path_;
  source_cache_[path_key] = str_t(source);

  ObjFunction* function = compile(source, path_key);
  if (function == nullptr) return InterpretResult::INTERPRET_COMPILE_ERROR;

  push(Value(static_cast<Object*>(function)));
  ObjClosure* closure = allocate<ObjClosure>(function);
  pop();
  push(Value(static_cast<Object*>(closure)));
  call(closure, 0);

  return run();
}

InterpretResult VM::interpret_bytecode(ObjFunction* function) noexcept {
  current_script_path_ = function->script_path();

  push(Value(static_cast<Object*>(function)));
  ObjClosure* closure = allocate<ObjClosure>(function);
  pop();
  push(Value(static_cast<Object*>(closure)));
  call(closure, 0);

  return run();
}

InterpretResult VM::interpret_bytecode(ObjFunction* function, strv_t source, strv_t script_path) noexcept {
  current_script_path_ = str_t(script_path);
  str_t path_key = current_script_path_.empty() ? "<repl>" : current_script_path_;
  source_cache_[path_key] = str_t(source);

  return interpret_bytecode(function);
}

InterpretResult VM::run() noexcept {
  CallFrame* frame = &frames_[frame_count_ - 1];

  // Read one 32-bit instruction and advance IP
#define READ_INSTR() (*frame->ip++)

  // Constant pool pointer — reassigned after every frame change
  const std::vector<Value>* Kp_ = &frame->closure->function()->chunk().constants();
#define K (*Kp_)
#define RK(rk) ((rk) >= kRK_CONST_BIT \
    ? K[(rk) - kRK_CONST_BIT] \
    : frame->slots[(rk)])
#define RELOAD_K() (Kp_ = &frame->closure->function()->chunk().constants())

#ifdef MAPLE_DEBUG_TRACE
#define TRACE_INSTRUCTION() \
    do { \
      std::cout << "          "; \
      for (Value* slot = frame->slots; slot < stack_top_; slot++) { \
        std::cout << "[ " << slot->stringify() << " ]"; \
      } \
      std::cout << std::endl; \
      disassemble_instruction(frame->closure->function()->chunk(), \
          static_cast<sz_t>(frame->ip - frame->closure->function()->chunk().code_data())); \
    } while (false)
#else
#define TRACE_INSTRUCTION() ((void)0)
#endif

#ifdef MAPLE_GNUC
  // Computed goto (threaded dispatch) — GCC/Clang only.
  static void* dispatch_table[] = {
    &&op_OP_LOADK,        &&op_OP_LOADNIL,       &&op_OP_LOADTRUE,
    &&op_OP_LOADFALSE,    &&op_OP_MOVE,
    &&op_OP_GETGLOBAL,    &&op_OP_SETGLOBAL,     &&op_OP_DEFGLOBAL,
    &&op_OP_GETUPVAL,     &&op_OP_SETUPVAL,
    &&op_OP_GETPROP,      &&op_OP_SETPROP,       &&op_OP_GETSUPER,
    &&op_OP_ADD,          &&op_OP_SUB,           &&op_OP_MUL,
    &&op_OP_DIV,          &&op_OP_MOD,
    &&op_OP_EQ,           &&op_OP_LT,            &&op_OP_LE,
    &&op_OP_NEG,          &&op_OP_NOT,           &&op_OP_STR,
    &&op_OP_BAND,         &&op_OP_BOR,           &&op_OP_BXOR,
    &&op_OP_BNOT,         &&op_OP_SHL,           &&op_OP_SHR,
    &&op_OP_JMP,          &&op_OP_TEST,          &&op_OP_TESTSET,
    &&op_OP_PRINT,
    &&op_OP_CALL,         &&op_OP_INVOKE,        &&op_OP_SUPERINV,
    &&op_OP_RETURN,       &&op_OP_CLOSURE,       &&op_OP_CLOSE,
    &&op_OP_CLASS,        &&op_OP_INHERIT,       &&op_OP_METHOD,
    &&op_OP_STATICMETH,   &&op_OP_GETTER,        &&op_OP_SETTER,
    &&op_OP_ABSTMETH,
    &&op_OP_NEWLIST,      &&op_OP_NEWMAP,        &&op_OP_NEWTUPLE,
    &&op_OP_GETIDX,       &&op_OP_SETIDX,
    &&op_OP_IMPORT,       &&op_OP_IMPFROM,       &&op_OP_IMPALIAS,
    &&op_OP_FORITER,
    &&op_OP_THROW,        &&op_OP_TRY,           &&op_OP_ENDTRY,
    &&op_OP_DEFER,
    &&op_OP_NOP,
    &&op_OP_YIELD,        &&op_OP_RESUME,
    &&op_OP_EXTRAARG,
    // Quickened opcodes
    &&op_OP_ADD_II,       &&op_OP_ADD_FF,        &&op_OP_ADD_SS,
    &&op_OP_SUB_II,       &&op_OP_SUB_FF,
    &&op_OP_MUL_II,       &&op_OP_MUL_FF,
    &&op_OP_DIV_FF,
    &&op_OP_LT_II,        &&op_OP_LT_FF,
    &&op_OP_EQ_II,
  };

#define VM_DISPATCH() do { \
    if (finalize_pending_) [[unlikely]] goto run_finalizers; \
    TRACE_INSTRUCTION(); \
    instr = READ_INSTR(); \
    goto *dispatch_table[static_cast<u8_t>(instr & 0xFF)]; \
  } while (false)
#define VM_CASE(name) op_##name:
#else // !MAPLE_GNUC — MSVC fallback: standard switch dispatch
#define VM_DISPATCH() continue
#define VM_CASE(name) case OpCode::name:
#endif

  Instruction instr;

dispatch_loop:
#ifdef MAPLE_GNUC
  if (finalize_pending_) [[unlikely]] {
    run_pending_finalizers();
    frame = &frames_[frame_count_ - 1];
    RELOAD_K();
  }
  instr = READ_INSTR();
  TRACE_INSTRUCTION();
  goto *dispatch_table[static_cast<u8_t>(instr & 0xFF)];
#else
  for (;;) {
    if (finalize_pending_) [[unlikely]] {
      run_pending_finalizers();
      frame = &frames_[frame_count_ - 1];
      RELOAD_K();
    }
    instr = READ_INSTR();
    TRACE_INSTRUCTION();
    switch (decode_op(instr)) {
#endif

    // =====================================================================
    // Loading
    // =====================================================================
    VM_CASE(OP_LOADK) {
      u8_t A = decode_A(instr);
      u16_t Bx = decode_Bx(instr);
      frame->slots[A] = K[Bx];
      VM_DISPATCH();
    }

    VM_CASE(OP_LOADNIL) {
      u8_t A = decode_A(instr);
      u8_t B = decode_B(instr);
      for (u8_t i = A; i <= A + B; i++) {
        frame->slots[i] = Value();
      }
      VM_DISPATCH();
    }

    VM_CASE(OP_LOADTRUE) {
      frame->slots[decode_A(instr)] = Value(true);
      VM_DISPATCH();
    }

    VM_CASE(OP_LOADFALSE) {
      frame->slots[decode_A(instr)] = Value(false);
      VM_DISPATCH();
    }

    // =====================================================================
    // Register movement
    // =====================================================================
    VM_CASE(OP_MOVE) {
      frame->slots[decode_A(instr)] = frame->slots[decode_B(instr)];
      VM_DISPATCH();
    }

    // =====================================================================
    // Global variables
    // =====================================================================
    VM_CASE(OP_GETGLOBAL) {
      u8_t A = decode_A(instr);
      u16_t Bx = decode_Bx(instr);
      ObjString* name = as_string(K[Bx]);
      Value value;
      if (!globals_.get(name, &value)) {
        runtime_error(std::format("Undefined variable '{}'.", name->value()));
        goto handle_runtime_error;
      }
      frame->slots[A] = value;
      VM_DISPATCH();
    }

    VM_CASE(OP_SETGLOBAL) {
      u8_t A = decode_A(instr);
      u16_t Bx = decode_Bx(instr);
      ObjString* name = as_string(K[Bx]);
      if (globals_.set(name, frame->slots[A])) {
        globals_.remove(name);
        runtime_error(std::format("Undefined variable '{}'.", name->value()));
        goto handle_runtime_error;
      }
      VM_DISPATCH();
    }

    VM_CASE(OP_DEFGLOBAL) {
      u8_t A = decode_A(instr);
      u16_t Bx = decode_Bx(instr);
      ObjString* name = as_string(K[Bx]);
      globals_.set(name, frame->slots[A]);
      VM_DISPATCH();
    }

    // =====================================================================
    // Upvalues
    // =====================================================================
    VM_CASE(OP_GETUPVAL) {
      u8_t A = decode_A(instr);
      u8_t B = decode_B(instr);
      frame->slots[A] = *frame->closure->upvalue_at(B)->location();
      VM_DISPATCH();
    }

    VM_CASE(OP_SETUPVAL) {
      u8_t A = decode_A(instr);
      u8_t B = decode_B(instr);
      *frame->closure->upvalue_at(B)->location() = frame->slots[A];
      VM_DISPATCH();
    }

    // =====================================================================
    // Properties (followed by EXTRAARG for IC slot)
    // =====================================================================
    VM_CASE(OP_GETPROP) {
      u8_t A = decode_A(instr);
      u8_t B = decode_B(instr);
      u8_t C = decode_C(instr);
      Instruction extra = READ_INSTR(); // EXTRAARG with IC slot
      u8_t ic_slot = static_cast<u8_t>(decode_Bx(extra));
      Value obj_val = frame->slots[B];
      ObjString* name = as_string(K[C]);

      if (is_obj_type(obj_val, ObjectType::OBJ_MODULE)) {
        ObjModule* module = as_module(obj_val);
        Value export_val;
        if (module->exports().get(name, &export_val)) {
          frame->slots[A] = export_val;
          VM_DISPATCH();
        }
        runtime_error(std::format("Undefined export '{}'.", name->value()));
        goto handle_runtime_error;
      }

      if (is_obj_type(obj_val, ObjectType::OBJ_CLASS)) {
        ObjClass* klass = as_class(obj_val);
        Value method;
        if (klass->static_methods().get(name, &method)) {
          frame->slots[A] = method;
          VM_DISPATCH();
        }
        runtime_error(std::format("Undefined static method '{}'.", name->value()));
        goto handle_runtime_error;
      }

      if (!is_obj_type(obj_val, ObjectType::OBJ_INSTANCE)) {
        runtime_error("Only instances have properties.");
        goto handle_runtime_error;
      }

      {
        ObjInstance* instance = as_instance(obj_val);
        ObjClass* klass = instance->klass();
        InlineCache& ic = frame->closure->function()->ic_at(ic_slot);

        // IC fast path (PIC: up to 4 entries)
        if (!ic.megamorphic) {
          if (auto* e = ic.find_entry(klass)) {
            if (e->kind == ICKind::IC_FIELD && e->shape_id == instance->shape()->id()) {
              frame->slots[A] = instance->get_field(e->slot_index);
              VM_DISPATCH();
            } else if (e->kind == ICKind::IC_METHOD) {
              ObjBoundMethod* bound = allocate<ObjBoundMethod>(obj_val, as_closure(e->cached));
              frame->slots[A] = Value(static_cast<Object*>(bound));
              VM_DISPATCH();
            } else if (e->kind == ICKind::IC_GETTER) {
              frame->slots[A] = frame->slots[B];
              stack_top_ = frame->slots + A + 1;
              if (!call(as_closure(e->cached), 0)) {
                goto handle_runtime_error;
              }
              frame = &frames_[frame_count_ - 1];
              RELOAD_K();
              VM_DISPATCH();
            }
          }
        }

        // IC miss — full lookup + append to PIC
        Value field_val;
        if (instance->get_field(name, &field_val)) {
          ICEntry new_e;
          new_e.klass = klass; new_e.kind = ICKind::IC_FIELD;
          new_e.shape_id = instance->shape()->id();
          new_e.slot_index = static_cast<u32_t>(instance->shape()->find_slot(name));
          if (!ic.megamorphic && !ic.append_entry(new_e)) ic.megamorphic = true;
          frame->slots[A] = field_val;
          VM_DISPATCH();
        }

        Value getter;
        if (klass->getters().get(name, &getter)) {
          ICEntry new_e;
          new_e.klass = klass; new_e.kind = ICKind::IC_GETTER; new_e.cached = getter;
          if (!ic.megamorphic && !ic.append_entry(new_e)) ic.megamorphic = true;
          frame->slots[A] = frame->slots[B];
          stack_top_ = frame->slots + A + 1;
          if (!call(as_closure(getter), 0)) {
            goto handle_runtime_error;
          }
          frame = &frames_[frame_count_ - 1];
          RELOAD_K();
          VM_DISPATCH();
        }

        Value method;
        if (klass->methods().get(name, &method)) {
          ICEntry new_e;
          new_e.klass = klass; new_e.kind = ICKind::IC_METHOD; new_e.cached = method;
          if (!ic.megamorphic && !ic.append_entry(new_e)) ic.megamorphic = true;
          ObjBoundMethod* bound = allocate<ObjBoundMethod>(obj_val, as_closure(method));
          frame->slots[A] = Value(static_cast<Object*>(bound));
          VM_DISPATCH();
        }

        runtime_error(std::format("Undefined property '{}'.", name->value()));
        goto handle_runtime_error;
      }
    }

    VM_CASE(OP_SETPROP) {
      u8_t A = decode_A(instr);
      u8_t B = decode_B(instr);
      u8_t C = decode_C(instr);
      Instruction extra = READ_INSTR(); // EXTRAARG with IC slot
      u8_t ic_slot = static_cast<u8_t>(decode_Bx(extra));
      Value obj_val = frame->slots[A];
      ObjString* name = as_string(K[B]);
      Value rhs = frame->slots[C];

      if (!is_obj_type(obj_val, ObjectType::OBJ_INSTANCE)) {
        runtime_error("Only instances have fields.");
        goto handle_runtime_error;
      }

      {
        ObjInstance* instance = as_instance(obj_val);
        ObjClass* klass = instance->klass();
        InlineCache& ic = frame->closure->function()->ic_at(ic_slot);

        // IC fast path (PIC)
        if (!ic.megamorphic) {
          if (auto* e = ic.find_entry(klass)) {
            if (e->kind == ICKind::IC_SETTER) {
              stack_top_ = frame->slots + A + 1;
              push(rhs);
              if (!call(as_closure(e->cached), 1)) {
                goto handle_runtime_error;
              }
              frame = &frames_[frame_count_ - 1];
              RELOAD_K();
              VM_DISPATCH();
            }
            if (e->kind == ICKind::IC_FIELD && e->shape_id == instance->shape()->id()) {
              instance->set_field(e->slot_index, rhs);
              write_barrier_value(instance, rhs);
              VM_DISPATCH();
            }
          }
        }

        // IC miss — full lookup + append to PIC
        Value setter_val;
        if (klass->setters().get(name, &setter_val)) {
          ICEntry new_e;
          new_e.klass = klass; new_e.kind = ICKind::IC_SETTER; new_e.cached = setter_val;
          if (!ic.megamorphic && !ic.append_entry(new_e)) ic.megamorphic = true;
          stack_top_ = frame->slots + A + 1;
          push(rhs);
          if (!call(as_closure(setter_val), 1)) {
            goto handle_runtime_error;
          }
          frame = &frames_[frame_count_ - 1];
          RELOAD_K();
          VM_DISPATCH();
        }

        instance->set_field(name, rhs);
        {
          ICEntry new_e;
          new_e.klass = klass; new_e.kind = ICKind::IC_FIELD;
          new_e.shape_id = instance->shape()->id();
          new_e.slot_index = static_cast<u32_t>(instance->shape()->find_slot(name));
          if (!ic.megamorphic && !ic.append_entry(new_e)) ic.megamorphic = true;
        }
        write_barrier_value(instance, rhs);
        VM_DISPATCH();
      }
    }

    VM_CASE(OP_GETSUPER) {
      u8_t A = decode_A(instr);
      u8_t B = decode_B(instr);
      u8_t C = decode_C(instr);
      ObjString* name = as_string(K[C]);
      ObjClass* superclass = as_class(frame->slots[B]);
      Value receiver = frame->slots[A];

      Value method;
      if (!superclass->methods().get(name, &method)) {
        runtime_error(std::format("Undefined property '{}'.", name->value()));
        goto handle_runtime_error;
      }

      Value dummy;
      if (superclass->abstract_methods().get(name, &dummy)) {
        runtime_error(std::format("Cannot call abstract method '{}' on '{}'.",
            name->value(), superclass->name()->value()));
        goto handle_runtime_error;
      }

      ObjBoundMethod* bound = allocate<ObjBoundMethod>(receiver, as_closure(method));
      frame->slots[A] = Value(static_cast<Object*>(bound));
      VM_DISPATCH();
    }

    // =====================================================================
    // Arithmetic (B,C use RK encoding)
    // =====================================================================
    VM_CASE(OP_ADD) {
      u8_t A = decode_A(instr);
      u8_t B = decode_B(instr);
      u8_t C = decode_C(instr);
      Value lhs = RK(B);
      Value rhs = RK(C);

      if (is_obj_type(lhs, ObjectType::OBJ_STRING) &&
          is_obj_type(rhs, ObjectType::OBJ_STRING)) {
        // Quicken to ADD_SS
        const_cast<Instruction*>(frame->ip)[-1] = encode_ABC(OpCode::OP_ADD_SS, A, B, C);
        str_t result = str_t(as_string(lhs)->value()) + str_t(as_string(rhs)->value());
        frame->slots[A] = Value(static_cast<Object*>(take_string(std::move(result))));
      } else if (lhs.is_integer() && rhs.is_integer()) {
        // Quicken to ADD_II
        const_cast<Instruction*>(frame->ip)[-1] = encode_ABC(OpCode::OP_ADD_II, A, B, C);
        frame->slots[A] = Value(static_cast<i64_t>(lhs.as_integer() + rhs.as_integer()));
      } else if (lhs.is_number() && rhs.is_number()) {
        // Quicken to ADD_FF
        const_cast<Instruction*>(frame->ip)[-1] = encode_ABC(OpCode::OP_ADD_FF, A, B, C);
        frame->slots[A] = Value(lhs.as_number() + rhs.as_number());
      } else if (lhs.is_instance()) {
        // Operator overload: place [receiver, arg] at R(A), R(A+1)
        // so return value lands in R(A) via OP_RETURN's *return_base
        frame->slots[A] = lhs;
        frame->slots[A + 1] = rhs;
        stack_top_ = frame->slots + A + 2;
        if (invoke_operator(op_add_string_)) {
          frame = &frames_[frame_count_ - 1];
          RELOAD_K();
        } else {
          runtime_error("Operands must be two numbers or two strings.");
          goto handle_runtime_error;
        }
      } else {
        runtime_error("Operands must be two numbers or two strings.");
        goto handle_runtime_error;
      }
      VM_DISPATCH();
    }

    VM_CASE(OP_SUB) {
      u8_t A = decode_A(instr);
      u8_t B = decode_B(instr);
      u8_t C = decode_C(instr);
      Value lhs = RK(B);
      Value rhs = RK(C);

      if (lhs.is_instance()) {
        frame->slots[A] = lhs;
        frame->slots[A + 1] = rhs;
        stack_top_ = frame->slots + A + 2;
        if (invoke_operator(op_sub_string_)) {
          frame = &frames_[frame_count_ - 1];
          RELOAD_K();
          VM_DISPATCH();
        }
      }
      if (!lhs.is_number() || !rhs.is_number()) {
        runtime_error("Operands must be numbers.");
        goto handle_runtime_error;
      }
      if (lhs.is_integer() && rhs.is_integer()) {
        const_cast<Instruction*>(frame->ip)[-1] = encode_ABC(OpCode::OP_SUB_II, A, B, C);
        frame->slots[A] = Value(static_cast<i64_t>(lhs.as_integer() - rhs.as_integer()));
      } else {
        const_cast<Instruction*>(frame->ip)[-1] = encode_ABC(OpCode::OP_SUB_FF, A, B, C);
        frame->slots[A] = Value(lhs.as_number() - rhs.as_number());
      }
      VM_DISPATCH();
    }

    VM_CASE(OP_MUL) {
      u8_t A = decode_A(instr);
      u8_t B = decode_B(instr);
      u8_t C = decode_C(instr);
      Value lhs = RK(B);
      Value rhs = RK(C);

      if (lhs.is_instance()) {
        frame->slots[A] = lhs;
        frame->slots[A + 1] = rhs;
        stack_top_ = frame->slots + A + 2;
        if (invoke_operator(op_mul_string_)) {
          frame = &frames_[frame_count_ - 1];
          RELOAD_K();
          VM_DISPATCH();
        }
      }
      if (!lhs.is_number() || !rhs.is_number()) {
        runtime_error("Operands must be numbers.");
        goto handle_runtime_error;
      }
      if (lhs.is_integer() && rhs.is_integer()) {
        const_cast<Instruction*>(frame->ip)[-1] = encode_ABC(OpCode::OP_MUL_II, A, B, C);
        frame->slots[A] = Value(static_cast<i64_t>(lhs.as_integer() * rhs.as_integer()));
      } else {
        const_cast<Instruction*>(frame->ip)[-1] = encode_ABC(OpCode::OP_MUL_FF, A, B, C);
        frame->slots[A] = Value(lhs.as_number() * rhs.as_number());
      }
      VM_DISPATCH();
    }

    VM_CASE(OP_DIV) {
      u8_t A = decode_A(instr);
      u8_t B = decode_B(instr);
      u8_t C = decode_C(instr);
      Value lhs = RK(B);
      Value rhs = RK(C);

      if (lhs.is_instance()) {
        frame->slots[A] = lhs;
        frame->slots[A + 1] = rhs;
        stack_top_ = frame->slots + A + 2;
        if (invoke_operator(op_div_string_)) {
          frame = &frames_[frame_count_ - 1];
          RELOAD_K();
          VM_DISPATCH();
        }
      }
      if (!lhs.is_number() || !rhs.is_number()) {
        runtime_error("Operands must be numbers.");
        goto handle_runtime_error;
      }
      // int / int → float (always promote); quicken to DIV_FF
      const_cast<Instruction*>(frame->ip)[-1] = encode_ABC(OpCode::OP_DIV_FF, A, B, C);
      frame->slots[A] = Value(lhs.as_number() / rhs.as_number());
      VM_DISPATCH();
    }

    VM_CASE(OP_MOD) {
      u8_t A = decode_A(instr);
      Value lhs = RK(decode_B(instr));
      Value rhs = RK(decode_C(instr));

      if (lhs.is_instance()) {
        frame->slots[A] = lhs;
        frame->slots[A + 1] = rhs;
        stack_top_ = frame->slots + A + 2;
        if (invoke_operator(op_mod_string_)) {
          frame = &frames_[frame_count_ - 1];
          RELOAD_K();
          VM_DISPATCH();
        }
      }
      if (!lhs.is_number() || !rhs.is_number()) {
        runtime_error("Operands must be numbers.");
        goto handle_runtime_error;
      }
      if (lhs.is_integer() && rhs.is_integer()) {
        frame->slots[A] = Value(static_cast<i64_t>(lhs.as_integer() % rhs.as_integer()));
      } else {
        frame->slots[A] = Value(std::fmod(lhs.as_number(), rhs.as_number()));
      }
      VM_DISPATCH();
    }

    // =====================================================================
    // Comparison (produce bool in register)
    // =====================================================================
    VM_CASE(OP_EQ) {
      u8_t A = decode_A(instr);
      u8_t B = decode_B(instr);
      u8_t C = decode_C(instr);
      Value lhs = RK(B);
      Value rhs = RK(C);

      if (lhs.is_instance()) {
        frame->slots[A] = lhs;
        frame->slots[A + 1] = rhs;
        stack_top_ = frame->slots + A + 2;
        if (invoke_operator(op_eq_string_)) {
          frame = &frames_[frame_count_ - 1];
          RELOAD_K();
          VM_DISPATCH();
        }
        // Failed to invoke — restore stack and use default equality
        stack_top_ = frame->slots + A;
      }
      if (lhs.is_integer() && rhs.is_integer()) {
        const_cast<Instruction*>(frame->ip)[-1] = encode_ABC(OpCode::OP_EQ_II, A, B, C);
      }
      frame->slots[A] = Value(lhs.is_equal(rhs));
      VM_DISPATCH();
    }

    VM_CASE(OP_LT) {
      u8_t A = decode_A(instr);
      u8_t B = decode_B(instr);
      u8_t C = decode_C(instr);
      Value lhs = RK(B);
      Value rhs = RK(C);

      if (lhs.is_instance()) {
        frame->slots[A] = lhs;
        frame->slots[A + 1] = rhs;
        stack_top_ = frame->slots + A + 2;
        if (invoke_operator(op_lt_string_)) {
          frame = &frames_[frame_count_ - 1];
          RELOAD_K();
          VM_DISPATCH();
        }
        // Failed to invoke — restore stack
        stack_top_ = frame->slots + A;
      }
      if (!lhs.is_number() || !rhs.is_number()) {
        runtime_error("Operands must be numbers.");
        goto handle_runtime_error;
      }
      if (lhs.is_integer() && rhs.is_integer()) {
        const_cast<Instruction*>(frame->ip)[-1] = encode_ABC(OpCode::OP_LT_II, A, B, C);
        frame->slots[A] = Value(lhs.as_integer() < rhs.as_integer());
      } else {
        const_cast<Instruction*>(frame->ip)[-1] = encode_ABC(OpCode::OP_LT_FF, A, B, C);
        frame->slots[A] = Value(lhs.as_number() < rhs.as_number());
      }
      VM_DISPATCH();
    }

    VM_CASE(OP_LE) {
      u8_t A = decode_A(instr);
      Value lhs = RK(decode_B(instr));
      Value rhs = RK(decode_C(instr));

      if (!lhs.is_number() || !rhs.is_number()) {
        runtime_error("Operands must be numbers.");
        goto handle_runtime_error;
      }
      if (lhs.is_integer() && rhs.is_integer()) {
        frame->slots[A] = Value(lhs.as_integer() <= rhs.as_integer());
      } else {
        frame->slots[A] = Value(lhs.as_number() <= rhs.as_number());
      }
      VM_DISPATCH();
    }

    // =====================================================================
    // Unary
    // =====================================================================
    VM_CASE(OP_NEG) {
      u8_t A = decode_A(instr);
      Value val = frame->slots[decode_B(instr)];
      if (val.is_integer()) {
        frame->slots[A] = Value(static_cast<i64_t>(-val.as_integer()));
      } else if (val.is_double()) {
        frame->slots[A] = Value(-val.as_number());
      } else {
        runtime_error("Operand must be a number.");
        goto handle_runtime_error;
      }
      VM_DISPATCH();
    }

    VM_CASE(OP_NOT) {
      u8_t A = decode_A(instr);
      frame->slots[A] = Value(!frame->slots[decode_B(instr)].is_truthy());
      VM_DISPATCH();
    }

    VM_CASE(OP_STR) {
      u8_t A = decode_A(instr);
      u8_t B = decode_B(instr);
      Value val = frame->slots[B];
      if (val.is_instance()) {
        ObjInstance* instance = as_instance(val);
        Value method;
        if (instance->klass()->methods().get(op_str_string_, &method)) {
          // __str: place receiver at R(A) so return lands in R(A)
          frame->slots[A] = frame->slots[B];
          stack_top_ = frame->slots + A + 1;
          if (!call(as_closure(method), 0)) {
            goto handle_runtime_error;
          }
          frame = &frames_[frame_count_ - 1];
          RELOAD_K();
          VM_DISPATCH();
        }
      }
      if (val.is_string()) {
        frame->slots[A] = val;
      } else {
        str_t s = val.stringify();
        frame->slots[A] = Value(static_cast<Object*>(copy_string(s.data(), s.length())));
      }
      VM_DISPATCH();
    }

    // =====================================================================
    // Bitwise (B,C use RK encoding)
    // =====================================================================
    VM_CASE(OP_BAND) {
      u8_t A = decode_A(instr);
      Value lhs = RK(decode_B(instr));
      Value rhs = RK(decode_C(instr));
      if (!lhs.is_integer() || !rhs.is_integer()) {
        runtime_error("Operands must be integers.");
        goto handle_runtime_error;
      }
      frame->slots[A] = Value(static_cast<i64_t>(lhs.as_integer() & rhs.as_integer()));
      VM_DISPATCH();
    }

    VM_CASE(OP_BOR) {
      u8_t A = decode_A(instr);
      Value lhs = RK(decode_B(instr));
      Value rhs = RK(decode_C(instr));
      if (!lhs.is_integer() || !rhs.is_integer()) {
        runtime_error("Operands must be integers.");
        goto handle_runtime_error;
      }
      frame->slots[A] = Value(static_cast<i64_t>(lhs.as_integer() | rhs.as_integer()));
      VM_DISPATCH();
    }

    VM_CASE(OP_BXOR) {
      u8_t A = decode_A(instr);
      Value lhs = RK(decode_B(instr));
      Value rhs = RK(decode_C(instr));
      if (!lhs.is_integer() || !rhs.is_integer()) {
        runtime_error("Operands must be integers.");
        goto handle_runtime_error;
      }
      frame->slots[A] = Value(static_cast<i64_t>(lhs.as_integer() ^ rhs.as_integer()));
      VM_DISPATCH();
    }

    VM_CASE(OP_BNOT) {
      u8_t A = decode_A(instr);
      Value val = frame->slots[decode_B(instr)];
      if (!val.is_integer()) {
        runtime_error("Operand must be an integer.");
        goto handle_runtime_error;
      }
      frame->slots[A] = Value(static_cast<i64_t>(~val.as_integer()));
      VM_DISPATCH();
    }

    VM_CASE(OP_SHL) {
      u8_t A = decode_A(instr);
      Value lhs = RK(decode_B(instr));
      Value rhs = RK(decode_C(instr));
      if (!lhs.is_integer() || !rhs.is_integer()) {
        runtime_error("Operands must be integers.");
        goto handle_runtime_error;
      }
      frame->slots[A] = Value(static_cast<i64_t>(lhs.as_integer() << rhs.as_integer()));
      VM_DISPATCH();
    }

    VM_CASE(OP_SHR) {
      u8_t A = decode_A(instr);
      Value lhs = RK(decode_B(instr));
      Value rhs = RK(decode_C(instr));
      if (!lhs.is_integer() || !rhs.is_integer()) {
        runtime_error("Operands must be integers.");
        goto handle_runtime_error;
      }
      frame->slots[A] = Value(static_cast<i64_t>(lhs.as_integer() >> rhs.as_integer()));
      VM_DISPATCH();
    }

    // =====================================================================
    // Control flow
    // =====================================================================
    VM_CASE(OP_JMP) {
      int sBx = decode_sBx(instr);
      frame->ip += sBx;
      VM_DISPATCH();
    }

    VM_CASE(OP_TEST) {
      // if (bool(R(A)) != C) then skip next instruction
      u8_t A = decode_A(instr);
      u8_t C = decode_C(instr);
      if (frame->slots[A].is_truthy() != static_cast<bool>(C)) {
        frame->ip++; // skip next instruction
      }
      VM_DISPATCH();
    }

    VM_CASE(OP_TESTSET) {
      // if (bool(R(B)) == C) R(A):=R(B) else skip next instruction
      u8_t A = decode_A(instr);
      u8_t B = decode_B(instr);
      u8_t C = decode_C(instr);
      if (frame->slots[B].is_truthy() == static_cast<bool>(C)) {
        frame->slots[A] = frame->slots[B];
      } else {
        frame->ip++; // skip next instruction
      }
      VM_DISPATCH();
    }

    // =====================================================================
    // I/O
    // =====================================================================
    VM_CASE(OP_PRINT) {
      u8_t A = decode_A(instr);
      std::cout << frame->slots[A].stringify() << std::endl;
      VM_DISPATCH();
    }

    // =====================================================================
    // Function calls
    // =====================================================================
    VM_CASE(OP_CALL) {
      // R(A)..R(A+C-2) := R(A)(R(A+1)..R(A+B-1))
      u8_t A = decode_A(instr);
      u8_t B = decode_B(instr);
      int arg_count = B - 1;
      // Set stack_top_ so call() sees the arguments in the correct position
      stack_top_ = frame->slots + A + B;
      if (!call_value(frame->slots[A], arg_count)) {
        goto handle_runtime_error;
      }
      frame = &frames_[frame_count_ - 1];
      RELOAD_K();
      VM_DISPATCH();
    }

    VM_CASE(OP_INVOKE) {
      // A=base, B=argc, C=name_K  [next: EXTRAARG ic_slot]
      u8_t A = decode_A(instr);
      u8_t B = decode_B(instr);
      u8_t C = decode_C(instr);
      Instruction extra = READ_INSTR();
      u8_t ic_slot = static_cast<u8_t>(decode_Bx(extra));
      ObjString* method_name = as_string(K[C]);
      int arg_count = B;

      // Set stack_top_ past the arguments
      stack_top_ = frame->slots + A + arg_count + 1;
      Value receiver = frame->slots[A];

      // IC fast path: monomorphic instance method dispatch
      if (is_obj_type(receiver, ObjectType::OBJ_INSTANCE)) {
        ObjInstance* instance = as_instance(receiver);
        ObjClass* klass = instance->klass();
        InlineCache& ic = frame->closure->function()->ic_at(ic_slot);

        // IC fast path (PIC)
        if (!ic.megamorphic) {
          if (auto* e = ic.find_entry(klass)) {
            if (e->kind == ICKind::IC_METHOD) {
              Value field_val;
              if (!instance->get_field(method_name, &field_val)) {
                if (!call(as_closure(e->cached), arg_count)) {
                  goto handle_runtime_error;
                }
                frame = &frames_[frame_count_ - 1];
                RELOAD_K();
                VM_DISPATCH();
              }
            }
          }
        }

        if (!invoke(method_name, arg_count)) {
          goto handle_runtime_error;
        }
        if (!ic.megamorphic) {
          Value method_val;
          if (klass->methods().get(method_name, &method_val)) {
            ICEntry new_e;
            new_e.klass = klass; new_e.kind = ICKind::IC_METHOD; new_e.cached = method_val;
            if (!ic.append_entry(new_e)) ic.megamorphic = true;
          }
        }
        frame = &frames_[frame_count_ - 1];
        RELOAD_K();
        VM_DISPATCH();
      }

      // Coroutine .next() / .send() dispatch
      if (is_obj_type(receiver, ObjectType::OBJ_COROUTINE)) {
        strv_t mname = method_name->value();
        Value sent_val{};
        if (mname == "send") {
          if (arg_count != 1) {
            runtime_error("send() takes exactly 1 argument.");
            goto handle_runtime_error;
          }
          sent_val = frame->slots[A + 1];
        } else if (mname != "next") {
          runtime_error(std::format("Undefined coroutine method '{}'.", mname));
          goto handle_runtime_error;
        }
        ObjCoroutine* coro = as_coroutine(receiver);
        if (!resume_coroutine(coro, sent_val, A)) {
          goto handle_runtime_error;
        }
        frame = &frames_[frame_count_ - 1];
        RELOAD_K();
        VM_DISPATCH();
      }

      if (!invoke(method_name, arg_count)) {
        goto handle_runtime_error;
      }
      frame = &frames_[frame_count_ - 1];
      RELOAD_K();
      VM_DISPATCH();
    }

    VM_CASE(OP_SUPERINV) {
      // A=base, B=argc, C=name_K
      u8_t A = decode_A(instr);
      u8_t B = decode_B(instr);
      u8_t C = decode_C(instr);
      ObjString* method_name = as_string(K[C]);
      int arg_count = B;

      // Superclass is in R(A+argc+1), placed by compiler
      ObjClass* superclass = as_class(frame->slots[A + arg_count + 1]);
      stack_top_ = frame->slots + A + arg_count + 1;
      if (!invoke_from_class(superclass, method_name, arg_count)) {
        goto handle_runtime_error;
      }
      frame = &frames_[frame_count_ - 1];
      RELOAD_K();
      VM_DISPATCH();
    }

    VM_CASE(OP_RETURN) {
      u8_t A = decode_A(instr);
      u8_t B = decode_B(instr);
      Value result = (B >= 2) ? frame->slots[A] : Value();

      // If returning from a deferred call, discard its result
      // and restore the original return value.
      if (frame->returning) {
        result = frame->pending_return;
      }

      // Execute deferred closures in LIFO order before returning.
      if (!frame->deferred.empty()) {
        ObjClosure* deferred = frame->deferred.back();
        frame->deferred.pop_back();
        frame->pending_return = result;
        frame->returning = true;
        frame->ip--;  // rewind IP to OP_RETURN for re-entry

        // Push deferred closure on stack and call it
        stack_top_ = frame->slots + A + 1;
        push(Value(static_cast<Object*>(deferred)));
        call(deferred, 0);
        frame = &frames_[frame_count_ - 1];
        RELOAD_K();
        VM_DISPATCH();
      }

      frame->returning = false;
      close_upvalues(frame->slots);

      Value* return_base = frame->slots;
      frame_count_--;
      if (frame_count_ == base_frame_) {
        return InterpretResult::INTERPRET_OK;
      }

      // If we just returned from the last coroutine frame, mark it DEAD and restore parent
      if (!coro_stack_.empty() && frame_count_ == coro_stack_.back().parent_frame_count) {
        CoroutineEntry& ce = coro_stack_.back();
        ce.coro->set_state(CoroutineState::DEAD);
        frame_count_ = ce.parent_frame_count;
        stack_top_ = ce.parent_stack_top;
        u8_t res_reg = ce.result_reg;
        coro_stack_.pop_back();
        frame = &frames_[frame_count_ - 1];
        RELOAD_K();
        frame->slots[res_reg] = Value(); // coroutine done → nil
        VM_DISPATCH();
      }

      *return_base = result;
      stack_top_ = return_base + 1;
      frame = &frames_[frame_count_ - 1];
      RELOAD_K();

      // Check if we just returned from a module frame
      if (!pending_imports_.empty() && pending_imports_.back().frame_index == frame_count_) {
        auto pending = std::move(pending_imports_.back());
        pending_imports_.pop_back();

        current_script_path_ = std::move(pending.previous_script_path);

        std::unordered_set<ObjString*> pre_set(
            pending.pre_global_keys.begin(), pending.pre_global_keys.end());
        for (auto& entry : globals_.entries()) {
          if (entry.key != nullptr && pre_set.find(entry.key) == pre_set.end()) {
            pending.module->exports().set(entry.key, entry.value);
          }
        }

        str_t mod_path = str_t(pending.module->name()->value());
        auto slash = mod_path.find_last_of("/\\");
        str_t filename = (slash != str_t::npos) ? mod_path.substr(slash + 1) : mod_path;
        auto dot = filename.find_last_of('.');
        str_t mod_name = (dot != str_t::npos) ? filename.substr(0, dot) : filename;
        ObjString* name_str = copy_string(mod_name.c_str(), mod_name.size());
        globals_.set(name_str, Value(static_cast<Object*>(pending.module)));

        for (auto& req : pending.from_imports) {
          Value exp_val;
          if (pending.module->exports().get(req.name, &exp_val)) {
            ObjString* target = req.alias ? req.alias : req.name;
            globals_.set(target, exp_val);
          }
        }
      }
      VM_DISPATCH();
    }

    VM_CASE(OP_CLOSURE) {
      u8_t A = decode_A(instr);
      u16_t Bx = decode_Bx(instr);
      ObjFunction* function = as_function(K[Bx]);
      ObjClosure* closure = allocate<ObjClosure>(function);
      frame->slots[A] = Value(static_cast<Object*>(closure));

      for (int i = 0; i < closure->upvalue_count(); i++) {
        Instruction extra = READ_INSTR(); // EXTRAARG: A=is_local, Bx=index
        u8_t is_local = decode_A(extra);
        u8_t index = static_cast<u8_t>(decode_Bx(extra));
        if (is_local) {
          ObjUpvalue* uv = capture_upvalue(frame->slots + index);
          closure->set_upvalue_at(static_cast<sz_t>(i), uv);
          write_barrier(closure, uv);
        } else {
          ObjUpvalue* uv = frame->closure->upvalue_at(index);
          closure->set_upvalue_at(static_cast<sz_t>(i), uv);
          write_barrier(closure, uv);
        }
      }
      VM_DISPATCH();
    }

    VM_CASE(OP_CLOSE) {
      u8_t A = decode_A(instr);
      close_upvalues(frame->slots + A);
      VM_DISPATCH();
    }

    // =====================================================================
    // OOP
    // =====================================================================
    VM_CASE(OP_CLASS) {
      u8_t A = decode_A(instr);
      u16_t Bx = decode_Bx(instr);
      ObjString* name = as_string(K[Bx]);
      frame->slots[A] = Value(static_cast<Object*>(allocate<ObjClass>(name)));
      VM_DISPATCH();
    }

    VM_CASE(OP_INHERIT) {
      u8_t A = decode_A(instr);
      u8_t B = decode_B(instr);
      Value superclass = frame->slots[B];
      if (!is_obj_type(superclass, ObjectType::OBJ_CLASS)) {
        runtime_error("Superclass must be a class.");
        goto handle_runtime_error;
      }
      ObjClass* subclass = as_class(frame->slots[A]);
      ObjClass* super = as_class(superclass);
      subclass->methods().add_all(super->methods());
      subclass->static_methods().add_all(super->static_methods());
      subclass->getters().add_all(super->getters());
      subclass->setters().add_all(super->setters());
      subclass->abstract_methods().add_all(super->abstract_methods());
      VM_DISPATCH();
    }

    VM_CASE(OP_METHOD) {
      u8_t A = decode_A(instr);
      u8_t B = decode_B(instr);
      u8_t C = decode_C(instr);
      ObjString* name = as_string(K[B]);
      ObjClass* klass = as_class(frame->slots[A]);
      klass->methods().set(name, frame->slots[C]);
      write_barrier_value(klass, frame->slots[C]);
      klass->abstract_methods().remove(name);
      VM_DISPATCH();
    }

    VM_CASE(OP_STATICMETH) {
      u8_t A = decode_A(instr);
      u8_t B = decode_B(instr);
      u8_t C = decode_C(instr);
      ObjString* name = as_string(K[B]);
      ObjClass* klass = as_class(frame->slots[A]);
      klass->static_methods().set(name, frame->slots[C]);
      write_barrier_value(klass, frame->slots[C]);
      VM_DISPATCH();
    }

    VM_CASE(OP_GETTER) {
      u8_t A = decode_A(instr);
      u8_t B = decode_B(instr);
      u8_t C = decode_C(instr);
      ObjString* name = as_string(K[B]);
      ObjClass* klass = as_class(frame->slots[A]);
      klass->getters().set(name, frame->slots[C]);
      write_barrier_value(klass, frame->slots[C]);
      VM_DISPATCH();
    }

    VM_CASE(OP_SETTER) {
      u8_t A = decode_A(instr);
      u8_t B = decode_B(instr);
      u8_t C = decode_C(instr);
      ObjString* name = as_string(K[B]);
      ObjClass* klass = as_class(frame->slots[A]);
      klass->setters().set(name, frame->slots[C]);
      write_barrier_value(klass, frame->slots[C]);
      VM_DISPATCH();
    }

    VM_CASE(OP_ABSTMETH) {
      u8_t A = decode_A(instr);
      u8_t B = decode_B(instr);
      u8_t C = decode_C(instr);
      ObjString* name = as_string(K[B]);
      ObjClass* klass = as_class(frame->slots[A]);
      klass->methods().set(name, frame->slots[C]);
      write_barrier_value(klass, frame->slots[C]);
      klass->abstract_methods().set(name, Value(true));
      VM_DISPATCH();
    }

    // =====================================================================
    // Collections
    // =====================================================================
    VM_CASE(OP_NEWLIST) {
      u8_t A = decode_A(instr);
      u8_t B = decode_B(instr);
      ObjList* list = allocate<ObjList>();
      list->elements().resize(B);
      for (int i = 0; i < B; i++) {
        list->elements()[static_cast<sz_t>(i)] = frame->slots[A + 1 + i];
      }
      frame->slots[A] = Value(static_cast<Object*>(list));
      VM_DISPATCH();
    }

    VM_CASE(OP_NEWMAP) {
      u8_t A = decode_A(instr);
      u8_t B = decode_B(instr);
      ObjMap* map = allocate<ObjMap>();
      for (int i = 0; i < B; i++) {
        Value key = frame->slots[A + 1 + i * 2];
        Value val = frame->slots[A + 1 + i * 2 + 1];
        map->entries()[key] = val;
      }
      frame->slots[A] = Value(static_cast<Object*>(map));
      VM_DISPATCH();
    }

    VM_CASE(OP_NEWTUPLE) {
      u8_t A = decode_A(instr);
      u8_t B = decode_B(instr);
      std::vector<Value> elements(B);
      for (int i = 0; i < B; i++) {
        elements[static_cast<sz_t>(i)] = frame->slots[A + 1 + i];
      }
      ObjTuple* tuple = allocate<ObjTuple>(std::move(elements));
      frame->slots[A] = Value(static_cast<Object*>(tuple));
      VM_DISPATCH();
    }

    VM_CASE(OP_GETIDX) {
      u8_t A = decode_A(instr);
      Value receiver = frame->slots[decode_B(instr)];
      Value index_val = frame->slots[decode_C(instr)];

      if (is_obj_type(receiver, ObjectType::OBJ_LIST)) {
        if (!index_val.is_number()) {
          runtime_error("List index must be a number.");
          goto handle_runtime_error;
        }
        ObjList* list = as_list(receiver);
        int index = static_cast<int>(index_val.as_number());
        if (index < 0 || index >= static_cast<int>(list->len())) {
          runtime_error("List index out of bounds.");
          goto handle_runtime_error;
        }
        frame->slots[A] = list->elements()[static_cast<sz_t>(index)];
      } else if (is_obj_type(receiver, ObjectType::OBJ_MAP)) {
        ObjMap* map = as_map(receiver);
        auto it = map->entries().find(index_val);
        if (it == map->entries().end()) {
          runtime_error("Key not found in map.");
          goto handle_runtime_error;
        }
        frame->slots[A] = it->second;
      } else if (is_obj_type(receiver, ObjectType::OBJ_TUPLE)) {
        if (!index_val.is_number()) {
          runtime_error("Tuple index must be a number.");
          goto handle_runtime_error;
        }
        ObjTuple* tuple = as_tuple(receiver);
        int index = static_cast<int>(index_val.as_number());
        if (index < 0 || index >= static_cast<int>(tuple->len())) {
          runtime_error("Tuple index out of bounds.");
          goto handle_runtime_error;
        }
        frame->slots[A] = tuple->elements()[static_cast<sz_t>(index)];
      } else if (is_obj_type(receiver, ObjectType::OBJ_STRING)) {
        if (!index_val.is_number()) {
          runtime_error("String index must be a number.");
          goto handle_runtime_error;
        }
        ObjString* str = as_string(receiver);
        int index = static_cast<int>(index_val.as_number());
        if (index < 0 || index >= static_cast<int>(str->value().length())) {
          runtime_error("String index out of bounds.");
          goto handle_runtime_error;
        }
        frame->slots[A] = Value(static_cast<Object*>(
            copy_string(&str->value()[static_cast<sz_t>(index)], 1)));
      } else {
        runtime_error("Only lists, tuples, and strings can be indexed.");
        goto handle_runtime_error;
      }
      VM_DISPATCH();
    }

    VM_CASE(OP_SETIDX) {
      u8_t A = decode_A(instr);
      u8_t B = decode_B(instr);
      u8_t C = decode_C(instr);
      Value receiver = frame->slots[A];
      Value index_val = frame->slots[B];
      Value value = frame->slots[C];

      if (is_obj_type(receiver, ObjectType::OBJ_LIST)) {
        if (!index_val.is_number()) {
          runtime_error("List index must be a number.");
          goto handle_runtime_error;
        }
        ObjList* list = as_list(receiver);
        int index = static_cast<int>(index_val.as_number());
        if (index < 0 || index >= static_cast<int>(list->len())) {
          runtime_error("List index out of bounds.");
          goto handle_runtime_error;
        }
        list->elements()[static_cast<sz_t>(index)] = value;
        write_barrier_value(list, value);
      } else if (is_obj_type(receiver, ObjectType::OBJ_MAP)) {
        ObjMap* map = as_map(receiver);
        map->entries()[index_val] = value;
        write_barrier_value(map, value);
      } else if (is_obj_type(receiver, ObjectType::OBJ_TUPLE)) {
        runtime_error("Tuples are immutable.");
        goto handle_runtime_error;
      } else {
        runtime_error("Only lists and maps support index assignment.");
        goto handle_runtime_error;
      }
      VM_DISPATCH();
    }

    // =====================================================================
    // Module
    // =====================================================================
    VM_CASE(OP_IMPORT) {
      u8_t A = decode_A(instr);
      Value path_val = frame->slots[A];
      if (!is_obj_type(path_val, ObjectType::OBJ_STRING)) {
        runtime_error("Import path must be a string.");
        goto handle_runtime_error;
      }
      import_module(as_string(path_val));
      frame = &frames_[frame_count_ - 1];
      RELOAD_K();
      VM_DISPATCH();
    }

    VM_CASE(OP_IMPFROM) {
      u8_t A = decode_A(instr);
      u8_t B = decode_B(instr);
      Value path_val = frame->slots[A];
      Value name_val = frame->slots[B];
      if (!is_obj_type(path_val, ObjectType::OBJ_STRING)) {
        runtime_error("Import path must be a string.");
        goto handle_runtime_error;
      }
      ObjString* path = as_string(path_val);
      ObjString* name = as_string(name_val);
      // Built-in modules use raw name as key
      str_t raw_name = str_t(path->value());
      auto builtin_it = modules_.find(raw_name);
      bool is_builtin = (builtin_it != modules_.end());
      str_t resolved = is_builtin ? raw_name : ModuleLoader::resolve_path(path->value(), current_script_path_);
      bool was_cached = is_builtin || modules_.find(resolved) != modules_.end();
      import_module(path);
      frame = &frames_[frame_count_ - 1];
      RELOAD_K();

      if (was_cached || modules_.find(resolved) != modules_.end()) {
        auto mod_it = modules_.find(resolved);
        if (mod_it != modules_.end()) {
          Value exp_val;
          if (mod_it->second->exports().get(name, &exp_val)) {
            globals_.set(name, exp_val);
          }
        }
      } else if (!pending_imports_.empty()) {
        pending_imports_.back().from_imports.push_back({name, nullptr});
      }
      VM_DISPATCH();
    }

    VM_CASE(OP_IMPALIAS) {
      u8_t A = decode_A(instr);
      u8_t B = decode_B(instr);
      u8_t C = decode_C(instr);
      Value path_val = frame->slots[A];
      Value name_val = frame->slots[B];
      Value alias_val = frame->slots[C];
      if (!is_obj_type(path_val, ObjectType::OBJ_STRING)) {
        runtime_error("Import path must be a string.");
        goto handle_runtime_error;
      }
      ObjString* path = as_string(path_val);
      ObjString* name = as_string(name_val);
      ObjString* alias = as_string(alias_val);
      // Built-in modules use raw name as key
      str_t raw_name = str_t(path->value());
      auto builtin_it = modules_.find(raw_name);
      bool is_builtin = (builtin_it != modules_.end());
      str_t resolved = is_builtin ? raw_name : ModuleLoader::resolve_path(path->value(), current_script_path_);
      bool was_cached = is_builtin || modules_.find(resolved) != modules_.end();
      import_module(path);
      frame = &frames_[frame_count_ - 1];
      RELOAD_K();

      if (was_cached || modules_.find(resolved) != modules_.end()) {
        auto mod_it = modules_.find(resolved);
        if (mod_it != modules_.end()) {
          Value exp_val;
          if (mod_it->second->exports().get(name, &exp_val)) {
            globals_.set(alias, exp_val);
          }
        }
      } else if (!pending_imports_.empty()) {
        pending_imports_.back().from_imports.push_back({name, alias});
      }
      VM_DISPATCH();
    }

    // =====================================================================
    // Iterator
    // =====================================================================
    VM_CASE(OP_FORITER) {
      // seq=R(A), idx=R(A+1), elem→R(A+2); sBx=exit offset
      u8_t A = decode_A(instr);
      int sBx = decode_sBx(instr);
      Value& iterable = frame->slots[A];
      Value& index_val = frame->slots[A + 1];
      int idx = static_cast<int>(index_val.as_number());

      if (is_obj_type(iterable, ObjectType::OBJ_LIST)) {
        ObjList* list = as_list(iterable);
        if (idx >= static_cast<int>(list->len())) {
          frame->ip += sBx;
        } else {
          frame->slots[A + 2] = list->elements()[static_cast<sz_t>(idx)];
          index_val = Value(static_cast<double>(idx + 1));
        }
      } else if (is_obj_type(iterable, ObjectType::OBJ_TUPLE)) {
        ObjTuple* tuple = as_tuple(iterable);
        if (idx >= static_cast<int>(tuple->len())) {
          frame->ip += sBx;
        } else {
          frame->slots[A + 2] = tuple->elements()[static_cast<sz_t>(idx)];
          index_val = Value(static_cast<double>(idx + 1));
        }
      } else if (is_obj_type(iterable, ObjectType::OBJ_STRING)) {
        ObjString* str = as_string(iterable);
        if (idx >= static_cast<int>(str->value().length())) {
          frame->ip += sBx;
        } else {
          frame->slots[A + 2] = Value(static_cast<Object*>(
              copy_string(&str->value()[static_cast<sz_t>(idx)], 1)));
          index_val = Value(static_cast<double>(idx + 1));
        }
      } else if (is_obj_type(iterable, ObjectType::OBJ_MAP)) {
        ObjMap* map = as_map(iterable);
        auto& entries = map->entries();
        if (idx >= static_cast<int>(entries.size())) {
          frame->ip += sBx;
        } else {
          auto it = entries.begin();
          std::advance(it, idx);
          frame->slots[A + 2] = it->first;
          index_val = Value(static_cast<double>(idx + 1));
        }
      } else if (is_obj_type(iterable, ObjectType::OBJ_COROUTINE)) {
        ObjCoroutine* coro = as_coroutine(iterable);
        if (coro->state() == CoroutineState::DEAD) {
          frame->ip += sBx; // exit loop
        } else {
          // Resume coroutine; yielded value lands in slots[A+2] via OP_YIELD.
          // The dispatch loop enters the coroutine frame after this.
          if (!resume_coroutine(coro, Value(), static_cast<u8_t>(A + 2))) {
            goto handle_runtime_error;
          }
          frame = &frames_[frame_count_ - 1];
          RELOAD_K();
          // Execution continues inside the coroutine until OP_YIELD restores us.
        }
      } else {
        runtime_error("Can only iterate over lists, tuples, strings, and maps.");
        goto handle_runtime_error;
      }
      VM_DISPATCH();
    }

    // =====================================================================
    // Exception handling
    // =====================================================================
    VM_CASE(OP_THROW) {
      pending_exception_ = frame->slots[decode_A(instr)];
      goto handle_runtime_error;
    }

    VM_CASE(OP_TRY) {
      u8_t A = decode_A(instr);
      int sBx = decode_sBx(instr);
      exception_handlers_.push_back({
        frame_count_ - 1,
        stack_top_,
        frame->ip + sBx,
        A,
      });
      VM_DISPATCH();
    }

    VM_CASE(OP_ENDTRY) {
      exception_handlers_.pop_back();
      VM_DISPATCH();
    }

    VM_CASE(OP_DEFER) {
      u8_t A = decode_A(instr);
      frame->deferred.push_back(as_closure(frame->slots[A]));
      VM_DISPATCH();
    }

    // =====================================================================
    // Extra data (should not be dispatched directly)
    // =====================================================================
    VM_CASE(OP_NOP) {
      VM_DISPATCH();
    }

    // =========================================================================
    // Coroutine opcodes
    // =========================================================================
    VM_CASE(OP_YIELD) {
      u8_t A = decode_A(instr);
      Value yielded = frame->slots[A];

      if (coro_stack_.empty()) {
        runtime_error("yield outside of a coroutine.");
        goto handle_runtime_error;
      }

      {
        CoroutineEntry& ce = coro_stack_.back();
        ObjCoroutine* coro = ce.coro;
        coro->set_state(CoroutineState::SUSPENDED);
        coro->yielded_value() = yielded;

        // Save coroutine frames using properly-copyable SavedCallFrame structs
        int coro_frame_count = frame_count_ - ce.parent_frame_count;
        Value* coro_stack_base = ce.parent_stack_top;

        // Save stack values
        int stack_count = static_cast<int>(stack_top_ - coro_stack_base);
        coro->saved_stack().resize(static_cast<sz_t>(stack_count));
        for (int i = 0; i < stack_count; ++i)
          coro->saved_stack()[static_cast<sz_t>(i)] = coro_stack_base[i];
        coro->saved_stack_top_offset() = stack_count;

        // Save each frame as a SavedCallFrame (safe C++ copy, no memcpy UB)
        coro->saved_frames().clear();
        coro->saved_frames().resize(static_cast<sz_t>(coro_frame_count));
        for (int fi = 0; fi < coro_frame_count; ++fi) {
          const CallFrame& f = frames_[ce.parent_frame_count + fi];
          SavedCallFrame& sf = coro->saved_frames()[static_cast<sz_t>(fi)];
          sf.closure = f.closure;
          sf.ip = f.ip;
          sf.slots_offset = f.slots - coro_stack_base;
          sf.deferred = f.deferred;
          sf.pending_return = f.pending_return;
          sf.returning = f.returning;
        }

        // Restore caller state
        frame_count_ = ce.parent_frame_count;
        stack_top_ = ce.parent_stack_top;
        u8_t res_reg = ce.result_reg;
        coro_stack_.pop_back();

        frame = &frames_[frame_count_ - 1];
        RELOAD_K();
        frame->slots[res_reg] = yielded;
      }
      VM_DISPATCH();
    }

    VM_CASE(OP_RESUME) {
      u8_t A = decode_A(instr);
      u8_t B = decode_B(instr);
      u8_t C = decode_C(instr);
      Value coro_val = RK(B);
      Value sent_val = RK(C);

      if (!is_obj_type(coro_val, ObjectType::OBJ_COROUTINE)) {
        runtime_error("OP_RESUME: expected a coroutine.");
        goto handle_runtime_error;
      }

      {
        ObjCoroutine* coro = as_coroutine(coro_val);
        coro->sent_value() = sent_val;

        if (coro->state() == CoroutineState::DEAD) {
          frame->slots[A] = Value(); // nil when exhausted
          VM_DISPATCH();
        }

        if (coro->state() == CoroutineState::RUNNING) {
          runtime_error("Cannot resume a running coroutine.");
          goto handle_runtime_error;
        }

        CoroutineEntry ce;
        ce.coro = coro;
        ce.parent_frame_count = frame_count_;
        ce.parent_stack_top = stack_top_;
        ce.result_reg = A;
        coro_stack_.push_back(ce);
        coro->set_state(CoroutineState::RUNNING);

        if (coro->saved_frames().empty()) {
          // First resume: call the closure
          push(coro_val); // "self" slot (coroutine object)
          if (!call(coro->closure(), 0)) {
            coro_stack_.pop_back();
            goto handle_runtime_error;
          }
          frame = &frames_[frame_count_ - 1];
          RELOAD_K();
        } else {
          // Resume from suspended state using SavedCallFrame entries
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
            f.deferred = sf.deferred;
            f.pending_return = sf.pending_return;
            f.returning = sf.returning;
          }
          frame_count_ += saved_count;
          frame = &frames_[frame_count_ - 1];
          RELOAD_K();
          coro->sent_value() = sent_val;
        }
      }
      VM_DISPATCH();
    }

    VM_CASE(OP_EXTRAARG) {
      // EXTRAARG is consumed inline by preceding instructions; skip if orphaned
      VM_DISPATCH();
    }

    // =========================================================================
    // Quickened (runtime-specialized) arithmetic handlers
    // On deopt: revert opcode and re-execute generic logic inline.
    // =========================================================================
    VM_CASE(OP_ADD_II) {
      u8_t A = decode_A(instr); u8_t B = decode_B(instr); u8_t C = decode_C(instr);
      Value lhs = RK(B); Value rhs = RK(C);
      if (lhs.is_integer() && rhs.is_integer()) {
        frame->slots[A] = Value(static_cast<i64_t>(lhs.as_integer() + rhs.as_integer()));
        VM_DISPATCH();
      }
      // Deopt: revert to generic
      const_cast<Instruction*>(frame->ip)[-1] = encode_ABC(OpCode::OP_ADD, A, B, C);
      if (lhs.is_number() && rhs.is_number()) {
        frame->slots[A] = Value(lhs.as_number() + rhs.as_number());
      } else { runtime_error("Operands must be two numbers or two strings."); goto handle_runtime_error; }
      VM_DISPATCH();
    }
    VM_CASE(OP_ADD_FF) {
      u8_t A = decode_A(instr); u8_t B = decode_B(instr); u8_t C = decode_C(instr);
      Value lhs = RK(B); Value rhs = RK(C);
      if (lhs.is_number() && rhs.is_number()) {
        frame->slots[A] = Value(lhs.as_number() + rhs.as_number());
        VM_DISPATCH();
      }
      // Deopt
      const_cast<Instruction*>(frame->ip)[-1] = encode_ABC(OpCode::OP_ADD, A, B, C);
      if (lhs.is_integer() && rhs.is_integer()) {
        frame->slots[A] = Value(static_cast<i64_t>(lhs.as_integer() + rhs.as_integer()));
      } else { runtime_error("Operands must be two numbers or two strings."); goto handle_runtime_error; }
      VM_DISPATCH();
    }
    VM_CASE(OP_ADD_SS) {
      u8_t A = decode_A(instr); u8_t B = decode_B(instr); u8_t C = decode_C(instr);
      Value lhs = RK(B); Value rhs = RK(C);
      if (lhs.is_string() && rhs.is_string()) {
        str_t result = str_t(as_string(lhs)->value()) + str_t(as_string(rhs)->value());
        frame->slots[A] = Value(static_cast<Object*>(take_string(std::move(result))));
        VM_DISPATCH();
      }
      // Deopt: revert and execute generic add logic
      const_cast<Instruction*>(frame->ip)[-1] = encode_ABC(OpCode::OP_ADD, A, B, C);
      if (is_obj_type(lhs, ObjectType::OBJ_STRING) && is_obj_type(rhs, ObjectType::OBJ_STRING)) {
        str_t result = str_t(as_string(lhs)->value()) + str_t(as_string(rhs)->value());
        frame->slots[A] = Value(static_cast<Object*>(take_string(std::move(result))));
      } else if (lhs.is_integer() && rhs.is_integer()) {
        frame->slots[A] = Value(static_cast<i64_t>(lhs.as_integer() + rhs.as_integer()));
      } else if (lhs.is_number() && rhs.is_number()) {
        frame->slots[A] = Value(lhs.as_number() + rhs.as_number());
      } else if (lhs.is_instance()) {
        frame->slots[A] = lhs; frame->slots[A + 1] = rhs;
        stack_top_ = frame->slots + A + 2;
        if (invoke_operator(op_add_string_)) { frame = &frames_[frame_count_ - 1]; RELOAD_K(); }
        else { runtime_error("Operands must be two numbers or two strings."); goto handle_runtime_error; }
      } else { runtime_error("Operands must be two numbers or two strings."); goto handle_runtime_error; }
      VM_DISPATCH();
    }
    VM_CASE(OP_SUB_II) {
      u8_t A = decode_A(instr); u8_t B = decode_B(instr); u8_t C = decode_C(instr);
      Value lhs = RK(B); Value rhs = RK(C);
      if (lhs.is_integer() && rhs.is_integer()) {
        frame->slots[A] = Value(static_cast<i64_t>(lhs.as_integer() - rhs.as_integer()));
        VM_DISPATCH();
      }
      // Deopt: fall to generic
      const_cast<Instruction*>(frame->ip)[-1] = encode_ABC(OpCode::OP_SUB, A, B, C);
      if (!lhs.is_number() || !rhs.is_number()) { runtime_error("Operands must be numbers."); goto handle_runtime_error; }
      frame->slots[A] = Value(lhs.as_number() - rhs.as_number());
      VM_DISPATCH();
    }
    VM_CASE(OP_SUB_FF) {
      u8_t A = decode_A(instr); u8_t B = decode_B(instr); u8_t C = decode_C(instr);
      Value lhs = RK(B); Value rhs = RK(C);
      if (lhs.is_number() && rhs.is_number()) {
        frame->slots[A] = Value(lhs.as_number() - rhs.as_number());
        VM_DISPATCH();
      }
      const_cast<Instruction*>(frame->ip)[-1] = encode_ABC(OpCode::OP_SUB, A, B, C);
      if (!lhs.is_number() || !rhs.is_number()) { runtime_error("Operands must be numbers."); goto handle_runtime_error; }
      frame->slots[A] = Value(lhs.as_number() - rhs.as_number());
      VM_DISPATCH();
    }
    VM_CASE(OP_MUL_II) {
      u8_t A = decode_A(instr); u8_t B = decode_B(instr); u8_t C = decode_C(instr);
      Value lhs = RK(B); Value rhs = RK(C);
      if (lhs.is_integer() && rhs.is_integer()) {
        frame->slots[A] = Value(static_cast<i64_t>(lhs.as_integer() * rhs.as_integer()));
        VM_DISPATCH();
      }
      const_cast<Instruction*>(frame->ip)[-1] = encode_ABC(OpCode::OP_MUL, A, B, C);
      if (!lhs.is_number() || !rhs.is_number()) { runtime_error("Operands must be numbers."); goto handle_runtime_error; }
      frame->slots[A] = Value(lhs.as_number() * rhs.as_number());
      VM_DISPATCH();
    }
    VM_CASE(OP_MUL_FF) {
      u8_t A = decode_A(instr); u8_t B = decode_B(instr); u8_t C = decode_C(instr);
      Value lhs = RK(B); Value rhs = RK(C);
      if (lhs.is_number() && rhs.is_number()) {
        frame->slots[A] = Value(lhs.as_number() * rhs.as_number());
        VM_DISPATCH();
      }
      const_cast<Instruction*>(frame->ip)[-1] = encode_ABC(OpCode::OP_MUL, A, B, C);
      if (!lhs.is_number() || !rhs.is_number()) { runtime_error("Operands must be numbers."); goto handle_runtime_error; }
      frame->slots[A] = Value(lhs.as_number() * rhs.as_number());
      VM_DISPATCH();
    }
    VM_CASE(OP_DIV_FF) {
      u8_t A = decode_A(instr); u8_t B = decode_B(instr); u8_t C = decode_C(instr);
      Value lhs = RK(B); Value rhs = RK(C);
      if (lhs.is_number() && rhs.is_number()) {
        frame->slots[A] = Value(lhs.as_number() / rhs.as_number());
        VM_DISPATCH();
      }
      const_cast<Instruction*>(frame->ip)[-1] = encode_ABC(OpCode::OP_DIV, A, B, C);
      if (!lhs.is_number() || !rhs.is_number()) { runtime_error("Operands must be numbers."); goto handle_runtime_error; }
      frame->slots[A] = Value(lhs.as_number() / rhs.as_number());
      VM_DISPATCH();
    }
    VM_CASE(OP_LT_II) {
      u8_t A = decode_A(instr); u8_t B = decode_B(instr); u8_t C = decode_C(instr);
      Value lhs = RK(B); Value rhs = RK(C);
      if (lhs.is_integer() && rhs.is_integer()) {
        frame->slots[A] = Value(lhs.as_integer() < rhs.as_integer());
        VM_DISPATCH();
      }
      const_cast<Instruction*>(frame->ip)[-1] = encode_ABC(OpCode::OP_LT, A, B, C);
      if (!lhs.is_number() || !rhs.is_number()) { runtime_error("Operands must be numbers."); goto handle_runtime_error; }
      frame->slots[A] = Value(lhs.as_number() < rhs.as_number());
      VM_DISPATCH();
    }
    VM_CASE(OP_LT_FF) {
      u8_t A = decode_A(instr); u8_t B = decode_B(instr); u8_t C = decode_C(instr);
      Value lhs = RK(B); Value rhs = RK(C);
      if (lhs.is_number() && rhs.is_number()) {
        frame->slots[A] = Value(lhs.as_number() < rhs.as_number());
        VM_DISPATCH();
      }
      const_cast<Instruction*>(frame->ip)[-1] = encode_ABC(OpCode::OP_LT, A, B, C);
      if (!lhs.is_number() || !rhs.is_number()) { runtime_error("Operands must be numbers."); goto handle_runtime_error; }
      frame->slots[A] = Value(lhs.as_number() < rhs.as_number());
      VM_DISPATCH();
    }
    VM_CASE(OP_EQ_II) {
      u8_t A = decode_A(instr); u8_t B = decode_B(instr); u8_t C = decode_C(instr);
      Value lhs = RK(B); Value rhs = RK(C);
      if (lhs.is_integer() && rhs.is_integer()) {
        frame->slots[A] = Value(lhs.as_integer() == rhs.as_integer());
        VM_DISPATCH();
      }
      const_cast<Instruction*>(frame->ip)[-1] = encode_ABC(OpCode::OP_EQ, A, B, C);
      frame->slots[A] = Value(lhs.is_equal(rhs));
      VM_DISPATCH();
    }

#ifndef MAPLE_GNUC
    } // switch
  } // for
#endif

handle_runtime_error:
  if (!exception_handlers_.empty()) {
    auto handler = exception_handlers_.back();
    exception_handlers_.pop_back();

    while (frame_count_ - 1 > handler.frame_index) {
      auto& unwound = frames_[frame_count_ - 1];
      unwound.deferred.clear();
      unwound.returning = false;
      close_upvalues(unwound.slots);
      frame_count_--;
    }

    frame = &frames_[frame_count_ - 1];
    RELOAD_K();
    stack_top_ = handler.stack_depth;
    frame->ip = handler.catch_ip;
    frame->slots[handler.catch_reg] = pending_exception_;
    goto dispatch_loop;
  }
  return InterpretResult::INTERPRET_RUNTIME_ERROR;

#ifdef MAPLE_GNUC
run_finalizers:
  run_pending_finalizers();
  frame = &frames_[frame_count_ - 1];
  RELOAD_K();
  goto dispatch_loop;
#endif

#undef VM_CASE
#undef VM_DISPATCH
#undef TRACE_INSTRUCTION
#undef READ_INSTR
#undef K
#undef RK
#undef RELOAD_K
}

} // namespace ms
