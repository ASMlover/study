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
  TulipDict(void) {
  }

  TulipDict(const py::object& v)
    : py::dict(v) {
  }

  ~TulipDict(void) {
  }

  inline py::ssize_t size(void) const {
    return PyDict_Size(ptr());
  }

  inline bool contains(const py::object& k) const {
    return attr("has_key")(k);
  }

  inline void setattr(const std::string& k, const py::object& v) {
    PyDict_SetItemString(ptr(), k.c_str(), v.ptr());
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

  inline void delattr(const std::string& k) {
    if (PyDict_DelItemString(ptr(), k.c_str()) == -1)
      py::throw_error_already_set();
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

  inline void delitem(const py::object& k) {
    if (PyDict_DelItem(ptr(), k.ptr()) == -1)
      py::throw_error_already_set();
  }

  inline py::object get(const py::object& k, const py::object& d) {
    auto* res = PyDict_GetItem(ptr(), k.ptr());
    if (res == nullptr)
      return d;
    return _tulip_borrowed_object(res);
  }

  inline py::object pop_1(const py::object& k) {
    return attr("pop")(k);
  }

  inline py::object pop_2(const py::object& k, const py::object& d) {
    return attr("pop")(k, d);
  }

  inline void update_dict(const py::object& other) {
    update(other);
  }

  static bool is_tulip_dict(const py::object& o) {
    if (o.is_none())
      return false;
    py::extract<TulipDict&> td(o);
    return td.check();
  }

  static void wrap(void) {
    py::class_<TulipDict, boost::shared_ptr<TulipDict>>("TulipDict", py::init<>())
      .def(py::init<const py::object&>())
      .def("size", &TulipDict::size)
      .def("has_key", &TulipDict::contains)
      .def("clear", &TulipDict::clear)
      .def("get", &TulipDict::get, (py::arg("d") = py::object()))
      .def("pop", &TulipDict::pop_1)
      .def("pop", &TulipDict::pop_2)
      .def("update", &TulipDict::update_dict)
      .def("keys", &TulipDict::keys)
      .def("values", &TulipDict::values)
      .def("items", &TulipDict::items)
      .def("iterkeys", &TulipDict::iterkeys)
      .def("itervalues", &TulipDict::itervalues)
      .def("iteritems", &TulipDict::iteritems)
      .def("__len__", &TulipDict::size)
      .def("__iter__", &TulipDict::iterkeys)
      .def("__contains__", &TulipDict::contains)
      .def("__setitem__", &TulipDict::setitem)
      .def("__getitem__", &TulipDict::getitem)
      .def("__delitem__", &TulipDict::delitem)
      .def("__setattr__", &TulipDict::setattr)
      .def("__getattr__", &TulipDict::getattr)
      .def("__delattr__", &TulipDict::delattr)
      ;
  }
};

}
