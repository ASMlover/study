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
    static void meta_compile(WrenVM& vm) {
      // evaluate the code in the module where the calling function was defined,
      // that's one stack frame back from the top since the top-most frame is
      // the helper eval() methid in Meta itself

      Value calling_fn(vm.fiber()->peek_frame(1).fn);
      ModuleObject* module = calling_fn.is_function()
        ? calling_fn.as_function()->module()
        : calling_fn.as_closure()->fn()->module();

      // compile it
      FunctionObject* fn = compile(vm, module, wrenGetSlotString(vm, 1), false);

      // return the result, we can not use the public API for this since we
      // have a bare FunctionObject
      vm.set_foreign_stack_asref(fn);
    }

    static WrenForeignFn bind_foreign(WrenVM& vm,
        const str_t& module, const str_t& class_name,
        bool is_static, const str_t& signature) {
      ASSERT(module == "meta", "should be in meta module");
      ASSERT(class_name == "Meta", "should be in Meta class");
      ASSERT(is_static, "should be static");
      ASSERT(signature == "compile(_)", "should be compile method");

      return meta_compile;
    }

    inline void load_aux_module(WrenVM& vm) {
      auto& prev_foreign_meth = vm.get_foreign_meth();

      vm.set_foreign_meth(bind_foreign);
      vm.interpret_in_module("meta", kLibSource);
      vm.set_foreign_meth(prev_foreign_meth);
    }
  }

  void load_aux_module(WrenVM& vm) {
#if AUX_META
    details::load_aux_module(vm);
#endif
  }
}

}
