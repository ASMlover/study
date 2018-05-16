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
#pragma once

#include <Python.h>
#include <boost/enable_shared_from_this.hpp>
#include <boost/python.hpp>

namespace py = ::boost::python;

namespace nyx {

namespace container_aux {
  template <typename Container>
  py::object as_pylist(const Container& c) {
    py::list r;
    for (auto& x : c)
      r.append(x);
    return r;
  }

  template <typename Container>
  py::object as_pydict(const Container& c) {
    py::dict r;
    for (auto& x : c)
      r[x.first] = x.second;
    return r;
  }
}

class acquire_pygil : private boost::noncopyable {
  PyGILState_STATE state_;
public:
  acquire_pygil(void) {
    state_ = PyGILState_Ensure();
  }

  ~acquire_pygil(void) {
    PyGILState_Release(state_);
  }
};

}

#define _NYXCORE_TRY {\
  nyx::acquire_pygil gil;\
  try
#define _NYXCORE_NOGIL_TRY {\
  try

#define _NYXCORE_CATCH\
  catch (const py::error_already_set&) {\
    PyErr_Print();\
  }\
  catch (...) {\
  }\
}

#define _NYXCORE_BORROWED_HANDLE(o) py::handle<>(py::borrowed(o))
#define _NYXCORE_BORROWED_OBJECT(o) py::object(_NYXCORE_BORROWED_HANDLE(o))
#define _NYXCORE_BORROWED_NEWOBJ(o) py::object(py::detail::new_reference(o))
#define _NYXCORE_PYHANDLE(o) py::handle<>(o)
#define _NYXCORE_PYOBJECT(o) py::object(_NYXCORE_PYHANDLE(o))
#define _NYXCORE_PYTYPE(o) ((PyTypeObject*)(o))
