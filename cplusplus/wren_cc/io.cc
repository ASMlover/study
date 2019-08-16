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
#include <iostream>
#include "common.hh"
#include "vm.hh"
#include "io.hh"

namespace wrencc {

static const str_t kLibSource =
"class IO {\n"
"  static print(obj) {\n"
"    IO.writeString(obj.toString)\n"
"    IO.writeString(\"\n\")\n"
"    return obj\n"
"  }\n"
"\n"
"  static write(obj) {\n"
"    IO.writeString(obj.toString)\n"
"    return obj\n"
"  }\n"
"}\n";

static void io_write_string(WrenVM& vm) {
  const char* s = wrenGetArgumentString(vm, 1);

  std::cout << s;
}

static void io_clock(WrenVM& vm) {
  wrenReturnDouble(vm, std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count() / 1000.0);
}

namespace io {
  namespace details {
    inline void load_library(WrenVM& vm) {
      vm.interpret("Wren IO library", kLibSource);
      wrenDefineStaticMethod(vm, "IO", "writeString", 1, io_write_string);
      wrenDefineStaticMethod(vm, "IO", "clock", 0, io_clock);
    }
  }

  void load_library(WrenVM& vm) {
#if USE_LIBIO
    details::load_library(vm);
#endif
  }
}

}
