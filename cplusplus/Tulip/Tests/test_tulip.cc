// Copyright (c) 2018 ASMlover. All rights reserved.
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
#include "../Utility.h"
#include "../Container/VectorWrap.h"
#include "../Container/SetWrap.h"
#include "../Container/MapWrap.h"
#include "../Container/Container.h"

namespace tulip { namespace test {

struct _test {
  static void call_pyfuns(const tulip::TulipDict& d, const char* method) {
    PyObject* v{};
    py::ssize_t pos{};
    while (PyDict_Next(d.ptr(), &pos, nullptr, &v))
      py::call_method<void>(v, method);
  }

  static void wrap(void) {
    py::scope in_test =
      py::class_<_test>("_test")
        .def("call_pyfuns", &_test::call_pyfuns).staticmethod("call_pyfuns")
        ;

    tulip::VectorWrap<int>::wrap("ivec");
    tulip::SetWrap<int>::wrap("iset");
    tulip::MapWrap<int, std::string>::wrap("ismap");
  }
};

}}

void tulip_debug_wrap(void) {
  tulip::test::_test::wrap();
}
