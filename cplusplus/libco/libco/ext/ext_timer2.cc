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
#include "ext_timer2.h"

namespace ext {

Timer2::Timer2(id_t id, std::int64_t delay, bool is_repeat)
  : id_(id)
  , delay_time_(delay)
  , is_repeat_(is_repeat) {
}

Timer2::~Timer2(void) {
}

bool Timer2::operator<(const Timer2& o) const {
  if (expire_time_ > o.expire_time_)
    return true;
  else if (expire_time_ < o.expire_time_)
    return false;
  else
    return id_ > o.id_;
}

void Timer2::clear(void) {
  boost::python::xdecref(func_);
  boost::python::xdecref(args_);
  boost::python::xdecref(kwargs_);
}

void Timer2::set_proxy(PyObject* proxy) {
  func_ = PyObject_GetAttrString(proxy, "func");
  args_ = PyObject_GetAttrString(proxy, "args");
  kwargs_ = PyObject_GetAttrString(proxy, "kwargs");
}

void Timer2::do_function(void) {
  try {
    PyObject_Call(func_, args_, kwargs_);
  }
  catch (const boost::python::error_already_set&) {
    PyErr_Print();
  }
  catch (...) {
    std::abort();
  }
}

}
