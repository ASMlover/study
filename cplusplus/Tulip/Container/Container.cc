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
#include "Container.h"

namespace tulip {

py::object TulipList::as_list(void) const {
  py::list r;
  auto n = size();
  for (auto i = 0; i < n; ++i) {
    auto* v = PyList_GetItem(ptr(), i);
    if (v == nullptr)
      continue;

    if (PyInt_Check(v) || PyFloat_Check(v) || PyString_Check(v)) {
      r.append(_tulip_borrowed_handle(v));
      continue;
    }

    py::extract<TulipList&> tl_v(v);
    if (tl_v.check()) {
      auto& tl = tl_v();
      r.append(tl.as_list());
      continue;
    }

    py::extract<TulipDict&> td_v(v);
    if (td_v.check()) {
      auto& td = td_v();
      r.append(td.as_dict());
      continue;
    }
  }
  return r;
}

py::object TulipDict::as_dict(void) const {
  py::dict r;
  PyObject* k{};
  PyObject* v{};
  py::ssize_t pos{};
  while (PyDict_Next(ptr(), &pos, &k, &v)) {
    if (!PyInt_Check(k) && !PyFloat_Check(k) && !PyString_Check(k))
      continue;

    if (PyInt_Check(v) || PyFloat_Check(v) || PyString_Check(v)) {
      r[_tulip_borrowed_handle(k)] = _tulip_borrowed_handle(v);
      continue;
    }

    py::extract<TulipList&> tl_v(v);
    if (tl_v.check()) {
      auto& tl = tl_v();
      r[_tulip_borrowed_handle(k)] = tl.as_list();
      continue;
    }

    py::extract<TulipDict&> td_v(v);
    if (td_v.check()) {
      auto& td = td_v();
      r[_tulip_borrowed_handle(k)] = td.as_dict();
      continue;
    }
  }
  return r;
}

}
