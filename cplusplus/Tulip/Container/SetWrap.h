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

#include <set>
#include "../Utility.h"

namespace tulip {

template <typename Container>
struct SetWrap {
  using ValueType = typename Container::value_type;
  using SizeType = typename Container::size_type;

  static Container set_copy(const Container& self) {
    Container r;
    for (auto& x : self)
      r.insert(x);
    return r;
  }

  static bool set_contains(const Container& self, const ValueType& x) {
    return self.find(x) != self.end();
  }

  static void set_add(Container& self, const ValueType& x) {
    self.insert(x);
  }

  static void set_discard(Container& self, const ValueType& x) {
    auto pos = self.find(x);
    if (pos != self.end())
      self.erase(pos);
  }

  static ValueType set_pop(Container& self) {
    auto pos = self.begin();
    if (pos == self.end()) {
      PyErr_SetString(PyExc_KeyError, "pop from an empty set");
      py::throw_error_already_set();
      return ValueType();
    }

    auto r = *pos;
    self.erase(pos);
    return r;
  }

  static void set_remove(Container& self, const ValueType& x) {
    auto pos = self.find(x);
    if (pos == self.end()) {
      PyErr_SetString(PyExc_KeyError, "key not in set");
      py::throw_error_already_set();
      return;
    }
    self.erase(pos);
  }

  static void set_update(Container& self, const py::object& other) {
    // TODO:
  }

  static void set_foreach(Container& self, PyObject* callable) {
    if (callable != nullptr) {
      for (auto& x : self)
        py::call<void>(callable, x);
    }
  }

  static py::object as_list(const Container& self) {
    return tulip::set_as_list(self);
  }

  static void wrap(const char* name) {
    py::class_<Container, boost::shared_ptr<Container>>(name, py::init<>())
      .def(py::init<const Container&>())
      .def("copy", &SetWrap::set_copy)
      .def("clear", &Container::clear)
      .def("size", &Container::size)
      .def("add", &SetWrap::set_add)
      .def("discard", &SetWrap::set_discard)
      .def("pop", &SetWrap::set_pop)
      .def("remove", &SetWrap::set_remove)
      .def("update", &SetWrap::set_update)
      .def("foreach", &SetWrap::set_foreach)
      .def("as_list", &SetWrap::as_list)
      .def("__len__", &Container::size)
      .def("__iter__", py::iterator<Container>())
      .def("__contains__", &SetWrap::set_contains)
      ;
  }
};

}
