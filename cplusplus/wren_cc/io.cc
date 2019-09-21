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

static const str_t kLibSource =
"class IO {\n"
"  static print {\n"
"    IO.writeString(\"\n\")\n"
"  }\n"
"\n"
"  static print(obj) {\n"
"    IO.writeObject(obj)\n"
"    IO.writeString(\"\n\")\n"
"    return obj\n"
"  }\n"
"\n"
"  static print(a1, a2) {\n"
"    printList([a1, a2])\n"
"  }\n"
"\n"
"  static print(a1, a2, a3) {\n"
"    printList([a1, a2, a3])\n"
"  }\n"
"\n"
"  static print(a1, a2, a3, a4) {\n"
"    printList([a1, a2, a3, a4])\n"
"  }\n"
"\n"
"  static print(a1, a2, a3, a4, a5) {\n"
"    printList([a1, a2, a3, a4, a5])\n"
"  }\n"
"\n"
"  static print(a1, a2, a3, a4, a5, a6) {\n"
"    printList([a1, a2, a3, a4, a5, a6])\n"
"  }\n"
"\n"
"  static print(a1, a2, a3, a4, a5, a6, a7) {\n"
"    printList([a1, a2, a3, a4, a5, a6, a7])\n"
"  }\n"
"\n"
"  static print(a1, a2, a3, a4, a5, a6, a7, a8) {\n"
"    printList([a1, a2, a3, a4, a5, a6, a7, a8])\n"
"  }\n"
"\n"
"  static print(a1, a2, a3, a4, a5, a6, a7, a8, a9) {\n"
"    printList([a1, a2, a3, a4, a5, a6, a7, a8, a9])\n"
"  }\n"
"\n"
"  static print(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10) {\n"
"    printList([a1, a2, a3, a4, a5, a6, a7, a8, a9, a10])\n"
"  }\n"
"\n"
"  static print(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11) {\n"
"    printList([a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11])\n"
"  }\n"
"\n"
"  static print(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12) {\n"
"    printList([a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12])\n"
"  }\n"
"\n"
"  static print(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13) {\n"
"    printList([a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13])\n"
"  }\n"
"\n"
"  static print(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14) {\n"
"    printList([a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14])\n"
"  }\n"
"\n"
"  static print(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15) {\n"
"    printList([a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15])\n"
"  }\n"
"\n"
"  static print(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16) {\n"
"    printList([a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16])\n"
"  }\n"
"\n"
"  static printList(objects) {\n"
"    for (object in objects) IO.writeObject(object)\n"
"    IO.writeString(\"\n\")\n"
"  }\n"
"\n"
"  static write(obj) {\n"
"    IO.writeObject(obj)\n"
"    return obj\n"
"  }\n"
"\n"
"\n"
"  static read(prompt) {\n"
"    if (!(prompt is String)) Fiber.abort(\"Prompt must be a string\")\n"
"    IO.write(prompt)\n"
"    return IO.read\n"
"  }\n"
"\n"
"  static writeObject(obj) {\n"
"    var string = obj.toString\n"
"    if (string is String) {\n"
"      IO.writeString(string)\n"
"    } else {\n"
"      IO.writeString(\"[invalid toString]\")\n"
"    }\n"
"  }\n"
"\n"
"  foreign static writeString(string)\n"
"  foreign static clock\n"
"  foreign static time\n"
"  foreign static read\n"
"}\n";

namespace io {
  namespace details {
    void write_string_impl(WrenVM& vm) {
      const char* s = wrenGetArgumentString(vm, 1);
      std::cout << s;
    }

    void read_impl(WrenVM& vm) {
      str_t buffer;
      if (std::getline(std::cin, buffer) && !buffer.empty())
        wrenReturnString(vm, buffer);
    }

    void clock_impl(WrenVM& vm) {
      wrenReturnDouble(vm, std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count() / 1000.0);
    }

    void time_impl(WrenVM& vm) {
      wrenReturnDouble(vm, Xt::as_type<double>(std::time(nullptr)));
    }

    inline void load_library(WrenVM& vm) {
      vm.interpret("", kLibSource);
    }

    inline WrenForeignFn bind_foreign(WrenVM& vm,
        const str_t& class_name, const str_t& signature) {
      ASSERT(class_name == "IO", "should only have IO class");

      if (signature == "writeString(_)")
        return write_string_impl;
      if (signature == "clock")
        return clock_impl;
      if (signature == "time")
        return time_impl;
      if (signature == "read")
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
