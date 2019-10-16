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
#include "vm.hh"
#include "primitive.hh"
#include "compiler.hh"
#include "meta.hh"

namespace wrencc {

#include "meta.wren.hh"

namespace meta {
  namespace details {
    DEF_PRIMITIVE(meta_eval) {
      if (!validate_string(vm, args[1], "Source code"))
        return false;

      // eval the code in the module where the calling function was defined
      Value calling_fn(vm.fiber()->peek_frame().fn);
      ModuleObject* module = calling_fn.is_function()
        ? calling_fn.as_function()->module()
        : calling_fn.as_closure()->fn()->module();

      // compile it
      FunctionObject* fn = compile(vm, module, args[1].as_cstring());
      if (fn == nullptr)
        RETURN_ERR("could not compile source code");

      PinnedGuard guard(vm, fn);
      // create a fiber to run the code in
      FiberObject* eval_fiber = FiberObject::make_fiber(vm, fn);

      // remember what fiber to return to
      eval_fiber->set_caller(vm.fiber());
      // switch to the fiber
      vm.set_fiber(eval_fiber);

      return true;
    }

    inline void load_library(WrenVM& vm) {
      vm.interpret("", kLibSource);

      ModuleObject* core_module = vm.get_core_module();

      // the methods on `Meta` are static, so get the metaclass for the
      // Meta class
      ClassObject* meta_cls = vm.find_variable(core_module, "Meta").as_class();
      vm.set_primitive(meta_cls->cls(), "eval(_)", _primitive_meta_eval);
    }
  }

  void load_library(WrenVM& vm) {
#if USE_LIBMETA
    details::load_library(vm);
#endif
  }
}

}
