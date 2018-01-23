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

#include <iostream>
#include <boost/python.hpp>

namespace py = ::boost::python;

struct ComponentBase {
  py::object owner_;
  bool need_tick{};
  bool is_render_tick{};
  int comp_type{};

  ComponentBase(void)
    : owner_() {
  }

  py::object get_owner(void) {
    auto* ref = Py_None;
    if (!owner_.is_none())
      ref = PyWeakref_GetObject(owner_.ptr());

    return py::object(py::detail::borrowed_reference(ref));
  }

  void destroy(void) {
    owner_ = py::object();
    comp_type = 0;
  }

  void on_add_to_unit(const py::object& owner) {
    auto* weakref = PyWeakref_NewRef(owner.ptr(), nullptr);
    if (weakref == nullptr)
      PyErr_Print();

    owner_ = py::object(py::detail::borrowed_reference(weakref));
  }
};
