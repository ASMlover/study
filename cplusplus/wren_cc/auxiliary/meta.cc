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
    static void meta_compile(WrenVM* vm) {
      const char* source_bytes = wrenGetSlotString(*vm, 1);
      bool is_expression = wrenGetSlotBool(*vm, 2);
      bool print_errors = wrenGetSlotBool(*vm, 3);
      // compile it
      ClosureObject* closure = vm->compile_source(
          "main", source_bytes, is_expression, print_errors);

      // return the result, we can not use the public API for this since we
      // have a bare ClosureObject
      if (closure == nullptr)
        vm->set_api_stack_asref(nullptr);
      else
        vm->set_api_stack_asref(closure);
    }

    static void meta_get_modvars(WrenVM* vm) {
      wrenEnsureSlots(*vm, 3);

      auto v = vm->modules()->get(vm->get_api_stack(1));
      if (!v) {
        vm->set_api_stack(0, nullptr);
        return;
      }
      Value module_val(*v);

      ModuleObject* module = module_val.as_module();
      ListObject* names = ListObject::make_list(*vm, module->vars_count());
      vm->set_api_stack(0, names);

      for (int i = 0; i < names->count(); ++i)
        names->set_element(i, module->get_variable_name(i));
    }

    inline WrenForeignFn bind_foreign_method(WrenVM& vm,
        const str_t& class_name, bool is_static, const str_t& signature) {
      // there is only one foreign method in the meta module
      ASSERT(class_name == "Meta", "should be in Meta class");
      ASSERT(is_static, "should be static");

      if (signature == "compile_(_,_,_)")
        return meta_compile;
      if (signature == "getModuleVars(_)")
        return meta_get_modvars;

      ASSERT(false, "unknown method");
      return nullptr;
    }
  }

  const str_t get_source(void) {
#if AUX_META
    return kLibSource;
#endif
    return "";
  }

  WrenForeignFn bind_foreign_method(WrenVM& vm,
      const str_t& class_name, bool is_static, const str_t& signature) {
#if AUX_META
    return details::bind_foreign_method(vm, class_name, is_static, signature);
#endif
    return nullptr;
  }
}

}
