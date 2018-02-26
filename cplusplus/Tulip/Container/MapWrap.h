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
struct MapWrap {
  using KeyType = typename Container::key_type;
  using ValueType = typename Container::mapped_type;
  using ItemType = typename Container::value_type;
  using SizeType = typename Container::size_type;

  struct Iterkeys {
    inline py::object operator()(const ItemType& x) const {
      return py::object(x.first);
    }
  };

  struct Itervalues {
    inline py::object operator()(const ItemType& x) const {
      return py::object(x.second);
    }
  };

  struct Iteritems {
    inline py::object operator()(const ItemType& x) const {
      return py::make_tuple(x.first, x.second);
    }
  };

  static Container map_copy(const Container& self) {
    Container r;
    for (auto& x : self)
      r.insert(x);
    return r;
  }

  static bool map_contains(const Container& self, const KeyType& k) {
    return self.find(k) != self.end();
  }

  static py::object map_get(
      const Container& self, const KeyType& k, const py::object& d) {
    auto pos = self.find(k);
    if (pos == self.end())
      return d;

    return py::object(pos->second);
  }

  static py::object map_pop_1(Container& self, const KeyType& k) {
    auto pos = self.find(k);
    if (pos == self.end()) {
      PyErr_SetString(PyExc_KeyError, "pop(k): key not found.");
      py::throw_error_already_set();
      return py::object();
    }

    auto r = py::object(pos->second);
    self.erase(pos);
    return r;
  }

  static py::object map_pop_2(
      Container& self, const KeyType& k, const py::object& d) {
    auto pos = self.find(k);
    if (pos == self.end())
      return d;

    auto r = py::object(pos->second);
    self.erase(pos);
    return r;
  }

  static py::object map_popitem(Container& self) {
    auto pos = self.begin();
    if (pos == self.end()) {
      PyErr_SetString(PyExc_KeyError, "popitem(): no more items to pop.");
      py::throw_error_already_set();
      return py::object();
    }

    auto r = py::make_tuple(pos->first, pos->second);
    self.erase(pos);
    return r;
  }

  static py::object map_keys(const Container& self) {
    py::list r;
    for (auto& x : self)
      r.append(x.first);
    return r;
  }

  static py::object map_values(const Container& self) {
    py::list r;
    for (auto& x : self)
      r.append(x.second);
    return r;
  }

  static py::object map_items(const Container& self) {
    py::list r;
    for (auto& x : self)
      r.append(py::make_tuple(x.first, x.second));
    return r;
  }

  static void map_setitem(
      Container& self, const KeyType& k, const ValueType& v) {
    self[k] = v;
  }

  static py::object map_getitem(const Container& self, const KeyType& k) {
    auto pos = self.find(k);
    if (pos == self.end()) {
      PyErr_SetString(PyExc_KeyError, "__getitem__(k): key not found.");
      py::throw_error_already_set();
      return py::object();
    }

    return py::object(pos->second);
  }

  static void map_delitem(Container& self, const KeyType& k) {
    auto pos = self.find(k);
    if (pos == self.end()) {
      PyErr_SetString(PyExc_KeyError, "__delitem__(k): key not found.");
      py::throw_error_already_set();
      return;
    }
    self.erase(pos);
  }

  static void map_update(py::object& self, const py::object& other) {
    auto keys = other.attr("keys")();
    auto n = keys.attr("__len__")();
    for (auto i = 0; i < n; ++i) {
      auto k = keys.attr("__getitem__")(i);
      self.attr("__setitem__")(k, other.attr("__getitem__")(k));
    }
  }

  static void map_foreach(Container& self, PyObject* callable) {
    if (callable != nullptr) {
      for (auto& x : self)
        py::call<void>(callable, x.first, x.second);
    }
  }

  static py::object map_fromkeys(const py::object& keys, const py::object& v) {
    auto r = py::object(Container());
    auto n = keys.attr("__len__")();
    for (auto i = 0; i < n; ++i)
      r.attr("__setitem__")(keys.attr("__getitem__")(i), v);
    return r;
  }

  static py::object as_dict(const Container& self) {
    return map_as_dict(self);
  }

  static void wrap(const char* name) {
    py::class_<Container, boost::shared_ptr<Container>>(name, py::init<>())
      .def(py::init<const Container&>())
      .def("copy", &MapWrap::map_copy)
      .def("has_key", &MapWrap::map_contains)
      .def("size", &Container::size)
      .def("clear", &Container::clear)
      .def("get", &MapWrap::map_get, (py::arg("d") = py::object()))
      .def("pop", &MapWrap::map_pop_1)
      .def("pop", &MapWrap::map_pop_2)
      .def("popitem", &MapWrap::map_popitem)
      .def("keys", &MapWrap::map_keys)
      .def("values", &MapWrap::map_values)
      .def("items", &MapWrap::map_items)
      .def("iterkeys", MakeTransform<Container, Iterkeys>::make())
      .def("itervalues", MakeTransform<Container, Itervalues>::make())
      .def("iteritems", MakeTransform<Container, Iteritems>::make())
      .def("update", &MapWrap::map_update)
      .def("foreach", &MapWrap::map_foreach)
      .def("fromkeys", &MapWrap::map_fromkeys).staticmethod("fromkeys")
      .def("as_dict", &MapWrap::as_dict)
      .def("__len__", &Container::size)
      .def("__iter__", MakeTransform<Container, Iterkeys>::make())
      .def("__contains__", &MapWrap::map_contains)
      .def("__setitem__", &MapWrap::map_setitem)
      .def("__getitem__", &MapWrap::map_getitem)
      .def("__delitem__", &MapWrap::map_delitem)
      ;
  }
};

}
