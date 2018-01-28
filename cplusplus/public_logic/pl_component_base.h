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

struct ComponentBase : public boost::enable_shared_from_this<ComponentBase> {
  py::object weakref_owner{};
  int comp_type{};
  bool need_tick{};
  bool is_render_tick{};

  ComponentBase(void) {
  }

  virtual ~ComponentBase(void) {
  }

  py::object get_owner(void) {
    auto* ref = Py_None;
    if (!weakref_owner.is_none())
      ref = PyWeakref_GetObject(weakref_owner.ptr());

    return py::object(py::handle<>(py::borrowed(ref)));
  }

  void destroy(void) {
    if (!weakref_owner.is_none())
      weakref_owner = py::object();
    comp_type = 0;
  }

  void register_owner(const py::object& owner) {
    auto* weakref = PyWeakref_NewRef(owner.ptr(), nullptr);
    if (weakref == nullptr)
      PyErr_Print();

    weakref_owner = py::object(py::handle<>(weakref));
  }
};

class ComponentBaseWrap : public ComponentBase {
  PyObject* self_{};
public:
  ComponentBaseWrap(PyObject* self)
    : self_(self) {
      py::xincref(self_);
    }

  virtual ~ComponentBaseWrap(void) {
    py::xdecref(self_);
  }
};

}
