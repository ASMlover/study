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
#include <chrono>
#include <ctime>
#include <iostream>
#include "common.hh"
#include "vm.hh"
#include "io.hh"

namespace wrencc {

#include "io.wren.hh"

namespace io {
  namespace details {
    void write_string_impl(WrenVM& vm) {
      const char* s = wrenGetSlotString(vm, 1);
      std::cout << s;
    }

    void read_impl(WrenVM& vm) {
      str_t buffer;
      if (std::getline(std::cin, buffer) && !buffer.empty())
        wrenSetSlotString(vm, 0, buffer);
    }

    void clock_impl(WrenVM& vm) {
      wrenSetSlotDouble(vm, 0,
          std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count() / 1000.0);
    }

    void time_impl(WrenVM& vm) {
      wrenSetSlotDouble(vm, 0, Xt::as_type<double>(std::time(nullptr)));
    }

    inline void load_library(WrenVM& vm) {
      vm.interpret_in_module("", kLibSource);
    }

    inline WrenForeignFn bind_foreign(WrenVM& vm,
        const str_t& class_name, const str_t& signature) {
      if (class_name != "IO")
        return nullptr;

      if (signature == "writeString(_)")
        return write_string_impl;
      if (signature == "clock")
        return clock_impl;
      if (signature == "time")
        return time_impl;
      if (signature == "read()")
        return read_impl;

      return nullptr;
    }
  }

  void load_library(WrenVM& vm) {
#if USE_LIBIO
    details::load_library(vm);
#endif
  }

  WrenForeignFn bind_foreign(WrenVM& vm,
      const str_t& class_name, const str_t& signature) {
#if USE_LIBIO
    return details::bind_foreign(vm, class_name, signature);
#endif
  }
}

}
