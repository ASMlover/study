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
#pragma once

#include <array>
#include <unordered_map>
#include <vector>
#include "Common.hh"
#include "Object.hh"
#include "Table.hh"

namespace ms {

enum class InterpretResult {
  INTERPRET_OK,
  INTERPRET_COMPILE_ERROR,
  INTERPRET_RUNTIME_ERROR,
};

struct CallFrame {
  ObjClosure* closure{nullptr};
  u8_t* ip{nullptr};
  Value* slots{nullptr};
};

class VM : public Singleton<VM> {
  std::array<CallFrame, kFRAMES_MAX> frames_{};
  int frame_count_{0};

  std::array<Value, kSTACK_MAX * kFRAMES_MAX> stack_{};
  Value* stack_top_{nullptr};

  Table globals_;
  Table strings_;
  ObjString* init_string_{nullptr};
  ObjUpvalue* open_upvalues_{nullptr};

  Object* objects_{nullptr};
  sz_t bytes_allocated_{0};
  sz_t next_gc_{kGC_INITIAL_SIZE};
  std::vector<Object*> gray_stack_;

  std::unordered_map<str_t, ObjModule*> modules_;

  // Pending module import tracking
  struct FromImportRequest {
    ObjString* name;
    ObjString* alias; // nullptr for non-alias imports
  };
  struct PendingImport {
    int frame_index;
    ObjModule* module;
    std::vector<ObjString*> pre_global_keys;
    std::vector<FromImportRequest> from_imports;
  };
  std::vector<PendingImport> pending_imports_;

  // VM dispatch
  InterpretResult run() noexcept;

  // Stack operations
  void push(Value value) noexcept;
  Value pop() noexcept;
  Value peek(int distance) const noexcept;
  void reset_stack() noexcept;

  // Function call support
  bool call_value(Value callee, int arg_count) noexcept;
  bool call(ObjClosure* closure, int arg_count) noexcept;
  bool invoke(ObjString* name, int arg_count) noexcept;
  bool invoke_from_class(ObjClass* klass, ObjString* name, int arg_count) noexcept;
  void bind_method(ObjClass* klass, ObjString* name) noexcept;

  // Upvalue management
  ObjUpvalue* capture_upvalue(Value* local) noexcept;
  void close_upvalues(Value* last) noexcept;

  // Import system
  void import_module(ObjString* path) noexcept;

  // Error reporting
  void runtime_error(strv_t message) noexcept;

  // String concatenation
  void concatenate() noexcept;

  // Native function registration
  void define_native(strv_t name, NativeFn function) noexcept;

public:
  VM() noexcept;
  ~VM() noexcept;

  InterpretResult interpret(strv_t source) noexcept;

  // Object allocation (accessible from Compiler/Memory)
  ObjString* copy_string(cstr_t chars, sz_t length) noexcept;
  ObjString* take_string(str_t value) noexcept;

  template <typename T, typename... Args>
  T* allocate(Args&&... args) noexcept;

  // GC interface
  Object*& gc_objects() noexcept { return objects_; }
  sz_t& gc_bytes_allocated() noexcept { return bytes_allocated_; }
  sz_t& gc_next_gc() noexcept { return next_gc_; }
  std::vector<Object*>& gc_gray_stack() noexcept { return gray_stack_; }
  void mark_roots() noexcept;
  Table& gc_strings() noexcept { return strings_; }
};

} // namespace ms
