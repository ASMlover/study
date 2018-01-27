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
#include "pl_component_map.h"

namespace public_logic {

ComponentPtr& ComponentMap::getitem(int key) {
  if (find(key) == end()) {
    PyErr_SetString(PyExc_KeyError, "key not in C++ ComponentMap");
    py::throw_error_already_set();
  }
  return operator[](key);
}

void ComponentMap::delitem(int key) {
  auto pos = find(key);
  if (pos == end()) {
    PyErr_SetString(PyExc_KeyError, "key not in C++ ComponentMap");
    py::throw_error_already_set();
  }
  erase(pos);
}

py::object ComponentMap::get(int key, const py::object& d) const {
  auto pos = find(key);
  if (pos == end())
    return d;

  py::return_by_value::apply<ComponentPtr>::type convertor;
  return py::object(py::handle<>(convertor(pos->second)));
}

py::object ComponentMap::pop(int key, const py::object& d) {
  auto pos = find(key);
  if (pos == end())
    return d;

  py::return_by_value::apply<ComponentPtr>::type convertor;
  auto r = py::object(py::handle<>(convertor(pos->second)));

  erase(pos);
  return r;
}

py::list ComponentMap::keys(void) const {
  py::list r;
  for (auto& v : *this)
    r.append(v.first);
  return r;
}

py::list ComponentMap::values(void) const {
  py::list r;
  for (auto& v : *this)
    r.append(v.second);
  return r;
}

py::list ComponentMap::items(void) const {
  py::list r;
  for (auto& v : *this)
    r.append(py::make_tuple(v.first, v.second));
  return r;
}

void ComponentMap::update(const ComponentMap& other) {
  for (auto& v : other)
    operator[](v.first) = v.second;
}

py::tuple ComponentMap::popitem(void) {
  auto pos = begin();
  if (pos == end()) {
    PyErr_SetString(PyExc_KeyError, "popitem(): C++ ComponentMap is empty");
    py::throw_error_already_set();
  }

  auto r = py::make_tuple(pos->first, pos->second);
  erase(pos);

  return r;
}

}
