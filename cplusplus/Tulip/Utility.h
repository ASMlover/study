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

#include <map>
#include <vector>
#include <Python.h>
#include <boost/noncopyable.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/python.hpp>

namespace py = ::boost::python;

namespace tulip {

class PyGILMutex : private boost::noncopyable {
  PyGILState_STATE state_;
public:
  PyGILMutex(void) {
    state_ = PyGILState_Ensure();
  }

  ~PyGILMutex(void) {
    PyGILState_Release(state_);
  }
};

inline bool hasattr(const py::object& obj, const std::string& name) {
  return PyObject_HasAttrString(obj.ptr(), name.c_str());
}

template <typename VectorType>
py::list vector_as_list(const VectorType& v) {
  py::list r;
  for (auto& x : v)
    r.append(x);
  return r;
}

template <typename SetType>
py::list set_as_list(const SetType& s) {
  py::list r;
  for (auto& x : s)
    r.append(x);
  return r;
}

template <typename MapType>
py::dict map_as_dict(const MapType& m) {
  py::dict r;
  for (auto& x : m)
    r[x.first] = x.second;
  return r;
}

template <typename T>
inline void register_shared_ptr(void) {
  py::register_ptr_to_python<boost::shared_array<T>>();
  py::register_ptr_to_python<boost::shared_array<const T>>();
  py::implicitly_convertible<boost::shared_array<T>, boost::shared_array<const T>>();
}

inline std::size_t getitem_index(py::ssize_t i, std::size_t n, bool eq_size = false) {
  if (i >= 0) {
    auto r = static_cast<std::size_t>(i);
    if (r > n || (r == n && !eq_size))
      throw std::out_of_range("Index out of range.");
    return r;
  }
  if (static_cast<std::size_t>(-i) > n)
    throw std::out_of_range("Index out of range.");
  return n + i;
}

}

#define _tulip_try {\
  PyGILMutex gil;\
  try
#define _tulip_non_gil_try {\
  try
#define _tulip_catch\
  catch (const py::error_already_set&) {\
    PyErr_Print();\
  }\
  catch (...) {\
  }\
}

#define _tulip_borrowed_handle(o) py::handle<>(py::borrowed(o))
#define _tulip_borrowed_object(o) py::object(_tulip_borrowed_handle(o))
#define _tulip_borrowed_newobj(o) py::object(py::detail::new_reference(o))
#define _tulip_pyhandle(o) py::handle<>(o)
#define _tulip_pyobject(o) py::object(_tulip_pyhandle(o))
#define _tulip_pytype(o) ((PyTypeObject*)(o))
