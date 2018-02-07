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
#include "Utility.h"
#include "Container/VectorWrap.h"
#include "Container/SetWrap.h"
#include "Container/MapWrap.h"
// #include "Property/Common.h" // Not implementation
#include "Container/Container.h"
#include <iostream>

#if defined(TULIP_DEBUG_MODE)
static void tulip_run_callscript(const tulip::TulipDict& d) {
  PyObject* k{};
  PyObject* v{};
  py::ssize_t pos{};
  while (PyDict_Next(d.ptr(), &pos, &k, &v)) {
    py::call_method<void>(v, "show");
  }
}

void tulip_debug_wrap(void) {
  tulip::VectorWrap<int>::wrap("IVec");
  tulip::SetWrap<int>::wrap("ISet");
  tulip::MapWrap<int, std::string>::wrap("ISMap");

  py::def("tulip_run_callscript", tulip_run_callscript);
}
#else
# define tulip_debug_wrap() (void)0
#endif

BOOST_PYTHON_MODULE(Tulip) {
  PyEval_InitThreads();
  tulip_debug_wrap();

  tulip::TulipList::wrap();
  tulip::TulipDict::wrap();
}
