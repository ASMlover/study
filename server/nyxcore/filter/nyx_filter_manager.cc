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
#include "nyx_filter_manager.h"

namespace nyx { namespace filter {

filter_manager::filter_manager(void) {
}

filter_manager::~filter_manager(void) {
}

void filter_manager::delete_filter_randomly(void) {
}

filter_ptr filter_manager::from_object(PyObject* v) {
  filter_ptr filter;
  return filter;
}

filter_ptr filter_manager::from_tuple(PyObject* v) {
  filter_ptr filter;
  return filter;
}

filter_ptr filter_manager::from_list(PyObject* v) {
  filter_ptr filter;
  return filter;
}

int filter_manager::add_filter(const py::object& args) {
  return 0;
}

int filter_manager::add_str_filter(
    const std::string& s, const py::object& args) {
  return 0;
}

void filter_manager::del_filter(int filter) {
}

filter_ptr filter_manager::get_filter(int index) const {
  filter_ptr filter;
  return filter;
}

int filter_manager::get_filter_index(const std::string& s) const {
  return 0;
}

void filter_manager::set_maxsize(int size) {
}

void filter_manager::print_filter(int index) {
}

}}
