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
#include <boost/python.hpp>
#include "utils/nyx_watcher.h"
#include "nyx_container.h"

namespace py = ::boost::python;

#if defined(_NYXCORE_ENABLE_RAW)
static void _nyxcore_raw_binding(void) {
  static PyMethodDef _nyxcore_methods[] = { {nullptr} };
  auto* m = Py_InitModule3("_nyxcore",
      _nyxcore_methods, "nyx core containers.");
  nyx::nyx_watcher_wrap(m);
  nyx::nyx_list_wrap(m);
  nyx::nyx_dict_wrap(m);
}
#else
# define _nyxcore_raw_binding() (void)0
#endif

void _nyxcore_wrap(void) {
  PyEval_InitThreads();

  _nyxcore_raw_binding();
}

#if defined(_NYXCORE_SERVER)
BOOST_PYTHON_MODULE(_nyxcore) {
  _nyxcore_wrap();
}
#else
PyMODINIT_FUNC init_nyxcore(void) {
  py::detail::init_module("_nyxcore", _nyxcore_wrap);
}
#endif

