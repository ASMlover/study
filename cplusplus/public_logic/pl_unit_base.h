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

#include <string>
#include "pl_python_utils.h"

namespace public_logic {

struct UnitBase : public boost::enable_shared_from_this<UnitBase> {
  int unit_id{};
  int hero_id{};
  int root_unit_id{};
  int unit_type{};
  int camp_type{};
  std::string unit_name;
  // TODO: need more class properties

  UnitBase(void) {
  }

  virtual ~UnitBase(void) {
  }

  virtual void tick(double dt) {
  }
};

class UnitBaseWrap : public UnitBase {
  PyObject* self_{};
  public:
  UnitBaseWrap(PyObject* self)
    : self_(self) {
      py::xincref(self_);
    }

  virtual ~UnitBaseWrap(void) {
    py::xdecref(self_);
  }

  virtual void tick(double dt) {
    _PL_WITHOUT_GIL_TRY {
      if (self_ != nullptr)
        py::call_method<void>(self_, "tick", dt);
    } _PL_END_TRY
  }

  void default_tick(double dt) {
    UnitBase::tick(dt);
  }
};

}
