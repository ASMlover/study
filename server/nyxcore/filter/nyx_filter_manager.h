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

#include <memory>
#include <map>
#include <unordered_map>
#include <boost/noncopyable.hpp>

#include "../utils/nyx_pyaux.h"
#include "nyx_filter.h"

namespace nyx { namespace filter {

using filter_map = std::map<int, filter_ptr>;
using filter_strint_map = std::unordered_map<std::string, int>;
using filter_intstr_map = std::unordered_map<int, std::string>;

class filter_manager : private boost::noncopyable {
  int index_{};
  int maxsz_{};
  filter_map filters_;
  filter_strint_map str_filters_;
  filter_intstr_map int_filters_;

  filter_manager(void) {}
  ~filter_manager(void) {}

  void delete_filter_randomly(void);
  filter_ptr from_object(PyObject* v);
  filter_ptr from_tuple(PyObject* v);
  filter_ptr from_list(PyObject* v);
public:
  static filter_manager& instance(void) {
    static filter_manager ins;
    return ins;
  }

  void set_maxsize(int size) {
    maxsz_ = size;
  }

  int add_filter(const py::object& args);
  int add_str_filter(const std::string& s, const py::object& args);
  void del_filter(int filter);

  filter_ptr get_filter(int index) const;
  int get_filter_index(const std::string& s) const;
  void print_filter(int index);
};

}}
