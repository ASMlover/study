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

#include <vector>
#include "../Utility.h"

namespace tulip {

template <typename T>
struct VectorWrap : public std::vector<T> {
  inline T& getitem(py::ssize_t i) {
    return operator[](tulip::getitem_index(i, size()));
  }

  inline void setitem(py::ssize_t i, const T& x) {
    operator[](tulip::getitem_index(i, size())) = x;
  }

  inline void delitem(py::ssize_t i) {
    erase(begin() + tulip::getitem_index(i, size()));
  }

  inline void insert_item(py::ssize_t i, const T& x) {
    insert(begin() + getitem_index(i, size()), x);
  }

  inline void append_item(const T& x) {
    push_back(x);
  }

  inline void pop_item(py::ssize_t i = INT_MAX) {
    if (i == INT_MAX)
      pop_back();
    else
      erase(begin() + tulip::getitem_index(i, size()));
  }

  inline void extend(const VectorWrap<T>& other) {
    insert(end(), other.begin(), other.end());
  }

  inline py::tuple getinitargs(void) {
    return py::make_tuple(py::tuple(*this));
  }

  static void wrap(const char* name) {
    py::class_<VectorWrap<T>, boost::shared_ptr<VectorWrap<T>>>(name)
      .def(py::init<const VectorWrap<T>&>())
      .def("size", &VectorWrap<T>::size)
      .def("__len__", &VectorWrap<T>::size)
      .def("__getitem__", &VectorWrap<T>::getitem, py::return_value_policy<py::copy_non_const_reference>())
      .def("__setitem__", &VectorWrap<T>::setitem)
      .def("__delitem__", &VectorWrap<T>::delitem)
      .def("clear", &VectorWrap<T>::clear)
      .def("insert", &VectorWrap<T>::insert_item)
      .def("pop", &VectorWrap<T>::pop_item, (py::arg("i") = INT_MAX))
      .def("extend", &VectorWrap<T>::extend)
      .def("__getinitargs__", &VectorWrap<T>::getinitargs)
      ;
  }
};

}
