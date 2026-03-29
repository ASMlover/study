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
#include "Serializer.hh"
#include "Module.hh"

namespace ms {

void VM::register_io_module() noexcept {
  ObjString* name = copy_string("io", 2);
  ObjModule* module = allocate<ObjModule>(name);
  push(Value(static_cast<Object*>(module))); // GC root

  // io.open(path, mode) -> ObjFile
  auto open_fn = [](VM& vm, int arg_count, Value* args) -> Value {
    if (arg_count != 2 || !args[0].is_string() || !args[1].is_string()) {
      vm.runtime_error("io.open() takes exactly 2 string arguments (path, mode).");
      return Value();
    }
    str_t path = str_t(as_string(args[0])->value());
    str_t mode = str_t(as_string(args[1])->value());
    ObjFile* file = vm.allocate<ObjFile>(std::move(path), std::move(mode));
    if (!file->open()) {
      vm.runtime_error(std::format("Could not open file '{}'.", file->path()));
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

} // namespace ms
