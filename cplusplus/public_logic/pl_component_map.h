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

#include "pl_python_utils.h"

namespace public_logic {

struct ComponentBase;
using ComponentPtr = boost::shared_ptr<ComponentBase>;

struct ComponentMap : public std::map<int, ComponentPtr> {
  inline py::object iter(void) const {
    py::handle<> it(PyObject_GetIter(keys().ptr()));
    return py::object(it);
  }

  inline py::object iterkeys(void) const {
    py::handle<> it(PyObject_GetIter(keys().ptr()));
    return py::object(it);
  }

  inline py::object itervalues(void) const {
    py::handle<> it(PyObject_GetIter(values().ptr()));
    return py::object(it);
  }

  inline py::object iteritems(void) const {
    py::handle<> it(PyObject_GetIter(items().ptr()));
    return py::object(it);
  }

  inline py::tuple getinitargs(void) const {
    return py::make_tuple(py::dict(items()));
  }

  inline py::object as_dict(void) const {
    return map_as_dict(*this);
  }

  inline bool contains(int key) const {
    return find(key) != end();
  }

  inline void setitem(int key, const ComponentPtr& val) {
    operator[](key) = val;
  }

  ComponentPtr& getitem(int key);
  void delitem(int key);
  py::object get(int key, const py::object& d = py::object()) const;
  py::object pop(int key, const py::object& d = py::object());
  py::list keys(void) const;
  py::list values(void) const;
  py::list items(void) const;
  void update(const ComponentMap& other);
  py::tuple popitem(void);
};

}
