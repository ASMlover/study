// Copyright (c) 2020 ASMlover. All rights reserved.
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
#pragma once

#include <list>
#include <vector>
#include "common.hh"
#include "utility.hh"
#include "value.hh"

namespace wrencc {

enum class Code : u8_t {
#undef CODEF
#define CODEF(c, _) c,
#include "codes_def.hh"
#undef CODEF
};

template <typename _Int> inline Code operator+(Code a, _Int b) noexcept {
  return Xt::as_type<Code>(Xt::as_type<_Int>(a) + b);
}

template <typename _Int> inline Code operator+(_Int a, Code b) noexcept {
  return Xt::as_type<Code>(a + Xt::as_type<_Int>(b));
}

template <typename _Int> inline _Int operator-(Code a, Code b) noexcept {
  return Xt::as_type<_Int>(a) - Xt::as_type<_Int>(b);
}

// a handle to a value, basically just a linked list of extra GC roots
//
// NOTE: even non-heap-allocated values can be stored here
struct WrenHandle {
  Value value{};

  WrenHandle* prev{};
  WrenHandle* next{};

  WrenHandle(Value v, WrenHandle* p = nullptr, WrenHandle* n = nullptr) noexcept
    : value(v), prev(p), next(n) {
  }

  void clear() noexcept {
    value = nullptr;
    prev = next = nullptr;
  }
};

class WrenVM final : private UnCopyable {
  static constexpr sz_t kMaxTempRoots = 5;
  static constexpr sz_t kGrayCapacity = 4;

  ClassObject* fn_class_{};
  ClassObject* bool_class_{};
  ClassObject* class_class_{};
  ClassObject* fiber_class_{};
  ClassObject* nil_class_{};
  ClassObject* num_class_{};
  ClassObject* obj_class_{};
  ClassObject* str_class_{};
  ClassObject* list_class_{};
  ClassObject* range_class_{};
  ClassObject* map_class_{};

  // the fiber that is currently running
  FiberObject* fiber_{};

  // the loaded modules, each key is an StringObject (except for the main
  // module, whose key is nil) for the module's name and the value is the
  // ModuleObject for the module
  MapObject* modules_{};

  // the most recently imported module, more specifically, the module whose
  // code has most recently finished executing
  //
  // not treated like a GC root since the module is already in [modules_]
  ModuleObject* last_module_{};

  // Memory Management Data
  //
  // the number of objects that are known to be currently allocated, including
  // all memory that was proven live after the last GC, as well as any new
  // objects that were allocted since then, does *not* include objects that
  // were freed since the last GC
  sz_t totoal_allocted_{};

  // the number of total allocated objects that will trigger the next GC
  sz_t next_gc_{1 << 10};

  // list of all currently allocated objects
  std::list<BaseObject*> all_objects_;

  // the *gray* set for the garbage collector, this is the stack of unprocessed
  // objects while a garbage collection pass is in process
  sz_t gray_capacity_{};
  std::vector<BaseObject*> gray_objects_;

  // the list of temporary roots, this is for temporary or new objects that are
  // not otherwise reachable but should not be collected
  //
  // they are organized as a stack of pointers stored in this array, this implies
  // that temporary roots need to have stack semantics: only the most recently
  // pushed object can be released
  std::vector<BaseObject*> temp_roots_;

  // pointer to the first node in the linked list of active handles or nullptr
  // if there are none
  WrenHandle* handles_{};

  // pointer to the bottom of the range of stack slots available for use from
  // the C/C++ API, during a foreign method, this will be in the stack of the
  // fiber that is executing a method
  //
  // if not in foreign method, this is initially nullptr, if the user requests
  // slots by calling `wrenEnsureSlots()`, a stack is created and this is initialized
  Value* api_stack_{};

  WrenConfig config_{};

  // TODO:
  // Compiler     -> compier

  // there is a single global symbol table for all method names on all classes,
  // method calls are dispatched directly by index in this table
  SymbolTable method_names_;
public:
  WrenVM() noexcept;
  ~WrenVM();

  inline ClassObject* fn_cls() const noexcept { return fn_class_; }
  inline ClassObject* bool_cls() const noexcept { return bool_class_; }
  inline ClassObject* class_cls() const noexcept { return class_class_; }
  inline ClassObject* fiber_cls() const noexcept { return fiber_class_; }
  inline ClassObject* nil_cls() const noexcept { return nil_class_; }
  inline ClassObject* num_cls() const noexcept { return num_class_; }
  inline ClassObject* obj_cls() const noexcept { return obj_class_; }
  inline ClassObject* str_cls() const noexcept { return str_class_; }
  inline ClassObject* list_cls() const noexcept { return list_class_; }
  inline ClassObject* range_cls() const noexcept { return range_class_; }
  inline ClassObject* map_cls() const noexcept { return map_class_; }

  inline void set_fn_cls(ClassObject* cls) noexcept { fn_class_ = cls; }
  inline void set_bool_cls(ClassObject* cls) noexcept { bool_class_ = cls; }
  inline void set_class_cls(ClassObject* cls) noexcept { class_class_ = cls; }
  inline void set_fiber_cls(ClassObject* cls) noexcept { fiber_class_ = cls; }
  inline void set_nil_cls(ClassObject* cls) noexcept { nil_class_ = cls; }
  inline void set_num_cls(ClassObject* cls) noexcept { num_class_ = cls; }
  inline void set_obj_cls(ClassObject* cls) noexcept { obj_class_ = cls; }
  inline void set_str_cls(ClassObject* cls) noexcept { str_class_ = cls; }
  inline void set_list_cls(ClassObject* cls) noexcept { list_class_ = cls; }
  inline void set_range_cls(ClassObject* cls) noexcept { range_class_ = cls; }
  inline void set_map_cls(ClassObject* cls) noexcept { map_class_ = cls; }

  // markes [obj] as a GC root so that it doesn't get collected
  void push_root(BaseObject* obj);
  // removes the most recently pushed temporary root
  void pop_root();

  void collect();
  void append_object(BaseObject* obj);
  void gray_object(BaseObject* obj);
  void gray_value(const Value& val);

  InterpretRet interpret(const str_t& module, const str_t& source_bytes);
};

// temporary roots guard
class TRoosGuard final : private UnCopyable {
  WrenVM& vm_;
  bool has_guard_{};
public:
  TRoosGuard(WrenVM& vm) noexcept : vm_(vm) {}

  TRoosGuard(WrenVM& vm, BaseObject* obj) noexcept
    : vm_(vm) {
    guard_object(obj);
  }

  ~TRoosGuard() noexcept {
    if (has_guard_)
      vm_.pop_root();
  }

  inline void guard_object(BaseObject* obj) noexcept {
    vm_.push_root(obj);
    has_guard_ = true;
  }
};

}
