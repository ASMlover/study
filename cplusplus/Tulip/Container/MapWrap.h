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
#include "../Utility.h"

namespace tulip {

template <typename _KeyTp, typename _ValTp>
struct MapWrap : public std::map<_KeyTp, _ValTp> {
  using MapType = MapWrap<_KeyTp, _ValTp>;

  inline bool contains(const _KeyTp& k) const {
    return find(k) != end();
  }

  inline void setitem(const _KeyTp& k, const _ValTp& v) {
    operator[](k) = v;
  }

  inline _ValTp& getitem(const _KeyTp& k) {
    auto pos = find(k);
    if (pos == end()) {
      PyErr_SetString(PyExc_KeyError, "Key not in C++ map.");
      py::throw_error_already_set();
    }
    return pos->second;
  }

  inline void delitem(const _KeyTp& k) {
    auto pos = find(k);
    if (pos == end()) {
      PyErr_SetString(PyExc_KeyError, "Key not in C++ map.");
      py::throw_error_already_set();
    }
    erase(pos);
  }

  inline py::object as_dict(void) const {
    return map_as_dict(*this);
  }

  inline py::tuple getinitargs(void) const {
    return py::make_tuple(py::dict(*this));
  }

  py::object get(const _KeyTp& k, const py::object& d = py::object()) {
    auto pos = find(k);
    if (pos == end())
      return d;

    typename py::return_by_value::apply<_ValTp>::type convertor;
    return _tulip_pyobject(convertor(pos->second));
  }

  py::object pop(const _KeyTp& k, const py::object& d = py::object()) {
    auto pos = find(k);
    if (pos == end())
      return d;

    typename py::return_by_value::apply<_ValTp>::type convertor;
    auto r = _tulip_pyobject(convertor(pos->second));

    erase(pos);
    return r;
  }

  py::list keys(void) const {
    py::list r;
    for (auto& x : *this)
      r.append(x.first);
    return r;
  }

  py::list values(void) const {
    py::list r;
    for (auto& x : *this)
      r.append(x.second);
    return r;
  }

  py::list items(void) const {
    py::list r;
    for (auto& x : *this)
      r.append(py::make_tuple(x.first, x.second));
    return r;
  }

  void update(const MapType& other) {
    for (auto& x : other)
      operator[](x.first) = x.second;
  }

  py::tuple popitem(void) {
    auto pos = begin();
    if (pos == end()) {
      PyErr_SetString(PyExc_KeyError, "popitem(): C++ map is empty.");
      py::throw_error_already_set();
    }

    auto r = py::make_tuple(pos->first, pos->second);
    erase(pos);

    return r;
  }

  static void wrap(const std::string& name) {
    py::class_<std::pair<const _KeyTp, _ValTp>>((name + std::string("Item")).c_str())
      .def_readonly("key", &std::pair<const _KeyTp, _ValTp>::first)
      .def_readwrite("value", &std::pair<const _KeyTp, _ValTp>::second)
      ;
    py::class_<MapType, boost::shared_ptr<MapType>>(name.c_str())
      .def(py::init<const MapType&>())
      .def("size", &MapType::size)
      .def("has_key", &MapType::contains)
      .def("clear", &MapType::clear)
      .def("get", &MapType::get, (py::arg("d") = py::object()))
      .def("pop", &MapType::pop, (py::arg("d") = py::object()))
      .def("erase", (std::size_t (MapType::*)(const _KeyTp&))&MapType::erase)
      .def("keys", &MapType::keys)
      .def("values", &MapType::values)
      .def("items", &MapType::items)
      .def("iteritems", py::iterator<MapType, py::return_internal_reference<>>())
      .def("update", &MapType::update)
      .def("popitem", &MapType::popitem)
      .def("as_dict", &MapType::as_dict)
      .def("__len__", &MapType::size)
      .def("__contains__", &MapType::contains)
      .def("__setitem__", &MapType::setitem)
      .def("__getitem__", &MapType::getitem, py::return_value_policy<py::copy_non_const_reference>())
      .def("__delitem__", &MapType::delitem)
      .def("__iter__", py::iterator<MapType, py::return_internal_reference<>>())
      .def("__getinitargs__", &MapType::getinitargs)
      ;
  }
};

}
