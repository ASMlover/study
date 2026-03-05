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
#include <iostream>
#include <format>
#include <vector>
#include "Memory.hh"
#include "Table.hh"
#include "Logger.hh"

// Forward declaration — VM provides access to GC state
namespace ms {

// These are implemented in VM.cc and provide access to the VM's GC state
Object*& vm_objects() noexcept;
sz_t& vm_bytes_allocated() noexcept;
sz_t& vm_next_gc() noexcept;
std::vector<Object*>& vm_gray_stack() noexcept;
void vm_mark_roots() noexcept;
Table& vm_strings() noexcept;

void mark_object(Object* object) noexcept {
  if (object == nullptr) return;
  if (object->is_marked_) return;

#ifdef MAPLE_DEBUG_LOG_GC
  auto& logger = Logger::get_instance();
  logger.debug("GC", "mark {} ({})",
      static_cast<void*>(object), object->stringify());
#endif

  object->is_marked_ = true;
  vm_gray_stack().push_back(object);
}

void mark_value(Value& value) noexcept {
  if (value.is_object()) mark_object(value.as_object());
}

static void trace_references() noexcept {
  auto& gray_stack = vm_gray_stack();
  while (!gray_stack.empty()) {
    Object* object = gray_stack.back();
    gray_stack.pop_back();

#ifdef MAPLE_DEBUG_LOG_GC
    auto& logger = Logger::get_instance();
    logger.debug("GC", "blacken {} ({})",
        static_cast<void*>(object), object->stringify());
#endif

    object->trace_references();
  }
}

static void sweep() noexcept {
  Object* previous = nullptr;
  Object* object = vm_objects();

  while (object != nullptr) {
    if (object->is_marked_) {
      object->is_marked_ = false;
      previous = object;
      object = object->next_;
    } else {
      Object* unreached = object;
      object = object->next_;
      if (previous != nullptr) {
        previous->next_ = object;
      } else {
        vm_objects() = object;
      }

#ifdef MAPLE_DEBUG_LOG_GC
      auto& logger = Logger::get_instance();
      logger.debug("GC", "free {} ({})",
          static_cast<void*>(unreached), unreached->stringify());
#endif

      vm_bytes_allocated() -= unreached->size();
      delete unreached;
    }
  }
}

void collect_garbage() noexcept {
#ifdef MAPLE_DEBUG_LOG_GC
  auto& logger = Logger::get_instance();
  logger.info("GC", "-- gc begin");
  sz_t before = vm_bytes_allocated();
#endif

  vm_mark_roots();
  trace_references();

  // Remove interned strings that are unmarked before sweep
  vm_strings().remove_white();

  sweep();

  vm_next_gc() = vm_bytes_allocated() * kGC_HEAP_GROW;

#ifdef MAPLE_DEBUG_LOG_GC
  logger.info("GC", "-- gc end (collected {} bytes, from {} to {}, next at {})",
      before - vm_bytes_allocated(), before, vm_bytes_allocated(), vm_next_gc());
#endif
}

void free_objects() noexcept {
  Object* object = vm_objects();
  while (object != nullptr) {
    Object* next = object->next_;
    delete object;
    object = next;
  }
  vm_objects() = nullptr;
}

} // namespace ms
