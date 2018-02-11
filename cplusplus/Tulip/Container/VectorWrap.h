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
struct VectorWrap {
  using ValueType = typename Container::value_type;
  using SizeType = typename Container::size_type;

  static Container vector_copy(const Container& self) {
    Container r;
    for (auto& x : self)
      r.push_back(x);
    return r;
  }

  static bool vector_contains(const Container& self, const ValueType& x) {
    return std::find(self.begin(), self.end(), x) != self.end();
  }

  static void vector_insert(
      Container& self, py::ssize_t i, const ValueType& x) {
    self.insert(self.begin() + getitem_index(i, self.size(), true), x);
  }

  static void vector_append(Container& self, const ValueType& x) {
    self.push_back(x);
  }

  static void vector_pop_1(Container& self) {
    self.pop_back();
  }

  static void vector_pop_2(Container& self, py::ssize_t i) {
    self.erase(self.begin() + getitem_index(i, self.size()));
  }

  static void vector_remove(Container& self, const ValueType& x) {
    auto pos = std::find(self.begin(), self.end(), x);
    if (pos != self.end())
      self.erase(pos);
  }

  static void vector_extend(py::object& self, const py::object& other) {
    auto n = other.attr("__len__")();
    for (auto i = 0; i < n; ++i)
      self.attr("append")(other.attr("__getitem__")(i));
  }

  static void vector_setitem(
      Container& self, py::ssize_t i, const ValueType& x) {
    self[getitem_index(i, self.size())] = x;
  }

  static py::object vector_getitem(const Container& self, py::ssize_t i) {
    py::object r(self[getitem_index(i, self.size())]);
    return r;
  }

  static void vector_delitem(Container& self, py::ssize_t i) {
    self.erase(self.begin() + getitem_index(i, self.size()));
  }

  static py::object as_list(const Container& self) {
    return vector_as_list(self);
  }

  static void wrap(const char* name) {
    py::class_<Container, boost::shared_ptr<Container>>(name, py::init<>())
      .def(py::init<const Container&>())
      .def("copy", &VectorWrap::vector_copy)
      .def("clear", &Container::clear)
      .def("size", &Container::size)
      .def("insert", &VectorWrap::vector_insert)
      .def("append", &VectorWrap::vector_append)
      .def("pop", &VectorWrap::vector_pop_1)
      .def("pop", &VectorWrap::vector_pop_2)
      .def("remove", &VectorWrap::vector_remove)
      .def("extend", &VectorWrap::vector_extend)
      .def("as_list", &VectorWrap::as_list)
      .def("__iter__", py::iterator<Container>())
      .def("__contains__", &VectorWrap::vector_contains)
      .def("__setitem__", &VectorWrap::vector_setitem)
      .def("__getitem__", &VectorWrap::vector_getitem)
      .def("__delitem__", &VectorWrap::vector_delitem)
      ;
  }
};

}
