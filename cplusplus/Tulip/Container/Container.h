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

#include "../Utility.h"

namespace tulip {

struct TulipDict : public py::dict {
  PyObject* obj_{};
public:
  TulipDict(PyObject* o)
    : obj_(o) {
  }

  TulipDict(PyObject* o, const py::object& v)
    : py::dict(v)
    , obj_(o) {
  }

  virtual ~TulipDict(void) {
  }

  py::ssize_t size(void) const {
    return PyDict_Size(ptr());
  }

  inline void setattr(const std::string& k, const py::object& v) {
    operator[](k) = v;
  }

  inline py::object getattr(const std::string& k) {
    auto* res = PyDict_GetItemString(ptr(), k.c_str());
    if (res == nullptr) {
      PyErr_Format(PyExc_AttributeError, "has no attribute: %s", k.c_str());
      py::throw_error_already_set();
      return py::object();
    }
    return _tulip_borrowed_object(res);
  }

  inline int delattr(const std::string& k) {
    return PyDict_DelItemString(ptr(), k.c_str());
  }

  inline void setitem(const py::object& k, const py::object& v) {
    PyDict_SetItem(ptr(), k.ptr(), v.ptr());
  }

  inline py::object getitem(const py::object& k) {
    auto* res = PyDict_GetItem(ptr(), k.ptr());
    if (res == nullptr) {
      PyErr_Format(PyExc_KeyError,
          "has no key: %s", PyString_AsString(k.ptr()));
      py::throw_error_already_set();
      return py::object();
    }
    return _tulip_borrowed_object(res);
  }

  inline int delitem(const py::object& k) {
    return PyDict_DelItem(ptr(), k.ptr());
  }

  py::object get_item(const py::object& k, const py::object& d = py::object()) {
    auto* res = PyDict_GetItem(ptr(), k.ptr());
    if (res == nullptr)
      return d;
    return _tulip_borrowed_object(res);
  }

  py::object pop_item(const py::object& k, const py::object& d = py::object()) {
    if (d.is_none())
      return this->attr("pop")(k);
    else
      return this->attr("pop")(k, d);
  }

  static bool is_tulip_dict(const py::object& o) {
    if (o.is_none())
      return false;
    py::extract<TulipDict&> td(o);
    return td.check();
  }
};

class TulipDictWrap : public TulipDict {
public:
  TulipDictWrap(PyObject* o)
    : TulipDict(o) {
  }

  TulipDictWrap(PyObject* o, const py::object& v)
    : TulipDict(o, v) {
  }

  ~TulipDictWrap(void) {
  }
};

}
