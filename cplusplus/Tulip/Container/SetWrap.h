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

#include <set>
#include "../Utility.h"

namespace tulip {

template <typename _Tp>
struct SetWrap : public std::set<_Tp> {
  using SetType = SetWrap<_Tp>;

  inline bool contains(const _Tp& x) const {
    return find(x) != end();
  }

  inline void insert_item(const _Tp& x) {
    insert(x);
  }

  inline void extend(const SetType& other) {
    insert(other.begin(), other.end());
  }

  inline py::object as_list(void) const {
    return set_as_list(*this);
  }

  inline py::tuple getinitargs(void) const {
    return py::make_tuple(py::tuple(*this));
  }

  static void wrap(const std::string& name) {
    py::class_<SetType, boost::shared_ptr<SetType>>(name.c_str())
      .def(py::init<const SetType&>())
      .def("size", &SetType::size)
      .def("add", &SetType::insert_item)
      .def("append", &SetType::insert_item)
      .def("discard", (std::size_t (SetType::*)(const _Tp&))&SetType::erase)
      .def("erase", (std::size_t (SetType::*)(const _Tp&))&SetType::erase)
      .def("extend", &SetType::extend)
      .def("clear", &SetType::clear)
      .def("as_list", &SetType::as_list)
      .def("__len__", &SetType::size)
      .def("__contains__", &SetType::contains)
      .def("__iter__", py::iterator<SetType>())
      .def("__getinitargs__", &SetType::getinitargs)
      ;
  }
};

}
