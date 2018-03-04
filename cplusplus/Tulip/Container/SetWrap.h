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

template <typename Container>
struct SetWrap {
  using KeyType = typename Container::key_type;
  using ValueType = typename Container::value_type;
  using SizeType = typename Container::size_type;
  using ConstIterator = typename Container::const_iterator;

  static Container set_copy(const Container& self) {
    Container r;
    for (auto& x : self)
      r.insert(x);
    return r;
  }

  static bool set_contains(const Container& self, const KeyType& k) {
    return self.find(k) != self.end();
  }

  static void set_add(Container& self, const ValueType& x) {
    self.insert(x);
  }

  static py::object set_pop(Container& self) {
    auto pos = self.begin();
    if (pos == self.end()) {
      PyErr_SetString(PyExc_KeyError, "pop(): pop from an empty set.");
      py::throw_error_already_set();
      return py::object();
    }

    auto r = py::object(*pos);
    self.erase(pos);
    return r;
  }

  static void set_remove(Container& self, const KeyType& k) {
    auto pos = self.find(k);
    if (pos == self.end()) {
      PyErr_SetString(PyExc_KeyError, "remove(k): key not found in set.");
      py::throw_error_already_set();
      return;
    }
    self.erase(pos);
  }

  static void set_update(py::object& self, const py::object& other) {
    auto n = other.attr("__len__")();
    auto iter = other.attr("__iter__")();
    for (auto i = 0; i < n; ++i)
      self.attr("add")(iter.attr("next")());
  }

  static void set_foreach(Container& self, PyObject* callable) {
    if (callable != nullptr) {
      for (auto& x : self)
        py::call<void>(callable, x);
    }
  }

  static std::string set_repr(const Container& self) {
    return tulip::container_utils::convert_to_string(self,
        [](std::ostringstream& o, ConstIterator i) { o << *i; });
  }

  static py::object as_set(const Container& self) {
    return tulip::container_utils::as_pyset(self);
  }

  static void wrap(const char* name) {
    py::class_<Container, boost::shared_ptr<Container>>(name)
      .setattr("__hash__", py::object())
      .def(py::init<>())
      .def(py::init<const Container&>())
      .def("copy", &SetWrap::set_copy)
      .def("clear", &Container::clear)
      .def("size", &Container::size)
      .def("add", &SetWrap::set_add)
      .def("discard",
          (SizeType (Container::*)(const KeyType&))&Container::erase)
      .def("pop", &SetWrap::set_pop)
      .def("remove", &SetWrap::set_remove)
      .def("update", &SetWrap::set_update)
      .def("foreach", &SetWrap::set_foreach)
      .def("as_set", &SetWrap::as_set)
      .def("__len__", &Container::size)
      .def("__iter__", py::iterator<Container>())
      .def("__repr__", &SetWrap::set_repr)
      .def("__contains__", &SetWrap::set_contains)
      ;
  }
};

}
