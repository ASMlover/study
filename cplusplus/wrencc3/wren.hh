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

#include <functional>
#include "common.hh"

namespace wrencc {

// a single virtual machine for executing `wrencc` code
//
// `wrencc` has no global state, so all state stored by a running interpreter
// lives here
class WrenVM;

// a handle to a `wrencc` object
//
// this lets code outside of the VM hold a persistent reference to an object,
// after a handle is acquired, and until it is released, this ensures the
// garbage collector will not reclaim the object it references
struct WrenHandle;

// a function callable from Wren code, but implemented in C/C++
using WrenForeignFn = std::function<void (WrenVM* vm)>;

// a finalizer function for freeing resources owned by an instance of a foreign
// class, unlike most foreign methods, finalizers do not have access to the VM
// and should not interact with it since it's in the middle of a garbage collection
using WrenFinalizeFn = std::function<void (void* data)>;

// a generic allocation function that handles all explicit memory management
// used by `wrencc`, it's used like so:
//
// - to allocate new memory, [p] is `nullptr` and [n] is the desired size, it
//   should return the allocated memory or `nullptr` on failure
//
// - to attempt to grow an existing allocation, [p] is the memory and [n] is
//   the desired size, it should return [p] if it was able to grow it in place
//   or a new pointer if it had to move it
//
// - to shrink memory, [p] and [n] are the same as above but it will always
//   return [p]
//
// - to free memory, [p] will be the memory to free and [n] will be zero, it
//   should return `nullptr`
using WrenReallocFn = std::function<void* (void* p, sz_t n)>;

// gives the host a chance to canonicalize the imported module name, potentially
// taking into account the (previously resolved) name of the module that contains
// the import, typically this is used to implement relative imports
using WrenResolveModuleFn = std::function<str_t (WrenVM& vm, const str_t& importer, const str_t& name)>;

// loads and returns the source code of the module [name]
using WrenLoadModuleFn = std::function<str_t (WrenVM& vm, const str_t& name)>;

// returns a function pointer to a foreign method on [class_name] in [module] with
// [signature]
using WrenBindForeignMethodFn = std::function<WrenForeignFn (WrenVM& vm,
    const str_t& module, const str_t& class_name, bool is_static, const str_t& signature)>;

struct WrenForeignClass {
  // the callback invoked when the foreign object is created
  //
  // this muts be provided, inside the body of this, it must call
  // [wrenSetSlotNewForeign()] exactly once
  WrenForeignFn allocate{};

  // the callback invoked when the garbage collector is about to collect a
  // foreign object's memory
  //
  // this may be `nullptr` if the foreign class foes not need to finalize
  WrenFinalizeFn finalize{};
};
// returns a pair of function pointers to the foreign methods used to allocate
// and finalize the data for instances of [class_name] in resolved [module]
using WrenBindForeignClassFn = std::function<WrenForeignClass (
    WrenVM& vm, const str_t& module, const str_t& class_name)>;

// displays a string of text to the user
using WrenWriteFn = std::function<void (WrenVM& vm, const str_t& text)>;

enum class WrenError {
  // a syntax or resolution error detected at compile time
  COMPILE,

  // the error message for a runtime error
  RUNTIME,

  // one entry of a runtime error's stack trace
  STACK_TRACE,
};
// reports an error to the user
//
// an error detected during compile time is reported by calling this once with
// [errno] `WrenError::COMPILE`, the resolved name of the [module] and [lineno]
// where the error occurs, and the compiler's error [message]
//
// a runtime is reported by calling this once with [errono] `WrenError::RUNTIME`,
// no [module] or [lineno], and the runtime error's [message], after that a series
// of [errono] `WrenError::STACK_TRACE` calls are made for each line in the stack
// trace. each of those has the resolved [module] and [lineno] where the method
// or function is defined and [message] is the name of the method of function
using WrenErrorFn = std::function<void (WrenVM& vm,
    WrenError errno, const str_t& module, int lineno, const str_t& message)>;

struct WrenConfig {
  // the callback `wrencc` will use to allocate, reallocate, and deallocate memory
  //
  // if `nullptr`, defaults to a built-in function that use `realloc` and `free`
  WrenReallocFn realloc_fn{};

  // the callback wnrecc uses to resolve a module name
  //
  // some host applications may wish to support `relative` imports, where the
  // meaning of an import string depends on the module that contains it, to
  // support that without breaking any policy into `wrencc` itself, the VM gives
  // the host a chance to resolve an import string
  //
  // before an import is loaded. it calls this, passing in the name of the
  // module that contains the import and the import string, the host app can
  // look at both of those and produce a new "canonical" string that uniquely
  // identifies the module, this string is then used as the name of the module
  // going forward, it is what is passed to [load_module_fn], how duplicate
  // imports of the same module are detected, and how the module is reported
  // in stack traces
  //
  // if you leave this function `nullptr`, then the original import string is
  // treated as the resolved string
  //
  // if an import cannot be resolved by the embedder, it should return `nullptr`
  // and `wrencc` will report that as a runtime error
  //
  // `wrencc` will take ownership of the string you return and free it for you,
  // so it should be allocated using the same allocation function you provide
  // above
  WrenResolveModuleFn res_module_fn{};

  // the callback `wrencc` uses to load a module
  //
  // since `wrencc` does not talk directly to the file system, it relies on the
  // embedder to physically locate and read the source code for a module, the
  // first time an import appears, `wrencc` will call this and pass in the name
  // of the module being imported, the VM should return the source code for that
  // module, memory for the source should be allocated using [realloc_fn] and
  // `wrencc` will take ownership over it
  //
  // this will only be called once for any given module name, `wrencc` caches the
  // result internally so subsequent imports of the same module will use the
  // previous source and not call this
  //
  // if a module with the given name could not be found by the embedder, it
  // should return `nullptr` and `wrencc` will report that as a runtime error
  WrenLoadModuleFn load_module_fn{};

  // the callback `wrencc` uses to find a foreign method and bind it to a class
  //
  // when a foreign method is declared in a class, this will be calle d with the
  // foreign method's module, class, and signature when the class body is executed,
  // it should return a function pointer to the foreign function that will be
  // bound to that method
  //
  // if the foreign function could not be found, this should return `nullptr` and
  // `wrencc` will report it as a runtime error
  WrenBindForeignMethodFn bind_foreign_meth{};

  // the callback `wrencc` uses to find a foreign class and get its foreign methods
  //
  // when a foreign class is declared, this will be called with the class's module
  // and name when the class body is executed, it should return the foreign
  // functions uses to allocate and (optionally) finalize the bytes stores in the
  // foreign object when an instance is created
  WrenBindForeignClassFn bind_foreign_cls{};

  // the callback `wrencc` uses to display text when `sys.print()` or the other
  // related functions are called
  //
  // if this is `nullptr`, `wrencc` discards any printed text
  WrenWriteFn write_fn{};

  // the callback `wrencc` uses to report errors
  //
  // when an error occurs, this will be called with the module name, line number
  // and an error message, if this is `nullptr`, `wrencc` doesn't report any error
  WrenErrorFn error_fn{};

  // user-defined data associated with the VM
  void* user_data{};
};

enum class InterpretRet {
  SUCCESS,
  COMPILE_ERROR,
  RUNTIME_ERROR,
};

// the type of an object stored in a slot
//
// this is not necessarily the obejct's *class*, but instead its low
// level representation type
enum class WrenType {
  BOOLEAN,
  NUMERIC,
  FOREIGN,
  LIST,
  NIL,
  STRING,

  // the object is of a type that isn't accessible by the C/C++ API
  UNKNOWN,
};

}
