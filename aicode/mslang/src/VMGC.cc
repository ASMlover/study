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
#include "Compiler.hh"
#include "Memory.hh"
#include "Logger.hh"

namespace ms {

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
        static_cast<void*>(object), object_stringify(object));
#endif

    object_trace(object);
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
          static_cast<void*>(unreached), object_stringify(unreached));
#endif

      bytes_allocated_ -= object_size(unreached);
      destroy_object(unreached);
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
    object_trace(obj);
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
        young_bytes_ -= object_size(promoted);
        promote_object(promoted);

#ifdef MAPLE_DEBUG_LOG_GC
        auto& logger = Logger::get_instance();
        logger.debug("GC", "promote {} ({})",
            static_cast<void*>(promoted), object_stringify(promoted));
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
          static_cast<void*>(unreached), object_stringify(unreached));
#endif

      sz_t obj_size = object_size(unreached);
      bytes_allocated_ -= obj_size;
      young_bytes_ -= obj_size;
      destroy_object(unreached);
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
        static_cast<void*>(object), object_stringify(object));
#endif

    object_trace(object);
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
      bytes_allocated_ -= object_size(unreached);
      destroy_object(unreached);
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

void VM::destroy_object(Object* obj) noexcept {
  switch (obj->type()) {
    case ObjectType::OBJ_UPVALUE: {
      auto* uv = static_cast<ObjUpvalue*>(obj);
      uv->~ObjUpvalue();
      upvalue_pool_.free(uv);
      return;
    }
    case ObjectType::OBJ_BOUND_METHOD: {
      auto* bm = static_cast<ObjBoundMethod*>(obj);
      bm->~ObjBoundMethod();
      bound_method_pool_.free(bm);
      return;
    }
    default:
      object_destroy(obj);
      return;
  }
}

void VM::free_objects() noexcept {
  // Free young generation
  Object* object = young_objects_;
  while (object != nullptr) {
    Object* next = object->next();
    destroy_object(object);
    object = next;
  }
  young_objects_ = nullptr;

  // Free old generation
  object = old_objects_;
  while (object != nullptr) {
    Object* next = object->next();
    destroy_object(object);
    object = next;
  }
  old_objects_ = nullptr;

  // Release pool slab memory
  upvalue_pool_.destroy_all();
  bound_method_pool_.destroy_all();
}

} // namespace ms
