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
#include <cstdlib>
#include <ctime>
#include "nyx_int_filter.h"
#include "nyx_float_filter.h"
#include "nyx_str_filter.h"
#include "nyx_filter_manager.h"

namespace nyx { namespace filter {

static const char* kUsageError = "Usage Error";

void filter_manager::delete_filter_randomly(void) {
  if (str_filters_.empty())
    return;

  std::srand(std::time(0));
  int bucket;
  int bucket_size;
  do {
    bucket = std::rand() % str_filters_.bucket_count();
  } while ((bucket_size = str_filters_.bucket_size(bucket)) == 0);
  auto it = str_filters_.begin(bucket);
  std::advance(it, std::rand() % bucket_size);
  del_filter(it->second);
}

filter_ptr filter_manager::from_object(PyObject* v) {
  filter_ptr filter;
  if (!PyTuple_Check(v) || (Py_SIZE(v) != 3)) {
    PyErr_SetString(PyExc_TypeError, kUsageError);
    return filter;
  }
  auto* type = PyTuple_GET_ITEM(v, 0);
  auto* key = PyTuple_GET_ITEM(v, 1);
  auto* val = PyTuple_GET_ITEM(v, 2);
  if (!PyInt_Check(type) || !PyString_Check(key)) {
    PyErr_SetString(PyExc_TypeError, kUsageError);
    return filter;
  }

  auto raw_type = PyInt_AsSsize_t(type);
  if (raw_type == -1)
    return filter;
  const char* raw_key = PyString_AsString(key);
  if (raw_key == nullptr)
    return filter;

  if (PyInt_Check(val)) {
    auto raw_val = PyInt_AsSsize_t(val);
    if (raw_val == -1 && PyErr_Occurred())
      return filter;
    filter = std::make_shared<int_filter>(
        static_cast<filter::filter_type>(raw_type), raw_key, raw_val);
    return filter;
  }
  else if (PyFloat_Check(val)) {
    auto raw_val = PyFloat_AsDouble(val);
    if (raw_val == -1.f && PyErr_Occurred())
      return filter;
    filter = std::make_shared<float_filter>(
        static_cast<filter::filter_type>(raw_type), raw_key, raw_val);
    return filter;
  }
  else if (PyString_Check(val)) {
    auto raw_val = PyString_AsString(val);
    if (raw_val == nullptr)
      return filter;
    filter = std::make_shared<str_filter>(
        static_cast<filter::filter_type>(raw_type), raw_key, raw_val);
    return filter;
  }
  else {
    PyErr_SetString(PyExc_TypeError, kUsageError);
    return filter;
  }

  return filter;
}

filter_ptr filter_manager::from_tuple(PyObject* v) {
  filter_ptr filter;
  auto* arg = PyTuple_GetItem(v, 0);
  if (arg == nullptr)
    return filter;

  if (PyInt_Check(arg)) {
    return from_object(v);
  }
  else if (PyTuple_Check(arg) || PyList_Check(arg)) {
    auto n = Py_SIZE(v);
    if (n > 0) {
      filter_ptr parent;
      for (auto i = 0; i < n; ++i) {
        arg = PyTuple_GET_ITEM(v, i);
        filter_ptr tmp;
        if (PyTuple_Check(arg))
          tmp = from_tuple(arg);
        else if (PyList_Check(arg))
          tmp = from_list(arg);
        if (!tmp)
          return tmp;

        if (!filter) {
          filter = tmp;
          parent = filter;
        }
        else {
          tmp->set_relation(filter::filter_relation::r_and);
          if (parent->get_sub() || tmp->is_group())
            parent->add_next(tmp);
          else
            parent->add_sub(tmp);
          parent = tmp;
        }
      }
    }
    return filter;
  }
  return filter;
}

filter_ptr filter_manager::from_list(PyObject* v) {
  filter_ptr filter;
  auto* arg = PyList_GetItem(v, 0);
  if (arg == nullptr)
    return filter;

  if (PyInt_Check(arg)) {
    return from_object(v);
  }
  else if (PyTuple_Check(arg) || PyList_Check(arg)) {
    auto n = Py_SIZE(v);
    if (v > 0) {
      filter_ptr parent;
      for (auto i = 0; i < n; ++i) {
        arg = PyList_GET_ITEM(v, i);
        filter_ptr tmp;
        if (PyTuple_Check(arg))
          tmp = from_tuple(arg);
        else if (PyList_Check(arg))
          tmp = from_list(arg);
        if (!tmp)
          return tmp;

        if (!filter) {
          filter = tmp;
          parent = filter;
        }
        else {
          tmp->set_relation(filter::filter_relation::r_or);
          if (parent->get_sub() || tmp->is_group())
            parent->add_next(tmp);
          else
            parent->add_sub(tmp);
          parent = tmp;
        }
      }
    }
    return filter;
  }
  return filter;
}

int filter_manager::add_filter(const py::object& args) {
  if (PyTuple_Check(args.ptr())) {
    auto root = from_tuple(args.ptr());
    if (!root)
      return -1;
    auto index = ++index_;
    filters_[index] = root;
    return index;
  }
  else if (PyList_Check(args.ptr())) {
    auto root = from_list(args.ptr());
    if (!root)
      return -1;
    auto index = ++index_;
    filters_[index] = root;
    return index;
  }
  else {
    PyErr_SetString(PyExc_TypeError, kUsageError);
  }
  return -1;
}

int filter_manager::add_str_filter(
    const std::string& s, const py::object& args) {
  if (str_filters_.size() > maxsz_)
    delete_filter_randomly();

  auto index = add_filter(args);
  if (index > 0) {
    str_filters_[s] = index;
    int_filters_[index] = s;
  }
  return index;
}

void filter_manager::del_filter(int filter) {
  filters_.erase(filter);
  auto it = int_filters_.find(filter);
  if (it != int_filters_.end()) {
    str_filters_.erase(it->second);
    int_filters_.erase(it);
  }
}

filter_ptr filter_manager::get_filter(int index) const {
  filter_ptr filter;
  auto it = filters_.find(index);
  if (it != filters_.end())
    filter = it->second;
  return filter;
}

int filter_manager::get_filter_index(const std::string& s) const {
  auto it = str_filters_.find(s);
  if (it != str_filters_.end())
    return it->second;
  return -1;
}

void filter_manager::print_filter(int index) {
  auto it = filters_.find(index);
  if (it != filters_.end())
    it->second->print();
}

}}
