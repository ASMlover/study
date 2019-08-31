// Copyright (c) 2019 ASMlover. All rights reserved.
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
#include <cmath>
#include <ctime>
#include "common.hh"
#include "vm.hh"
#include "math.hh"

namespace wrencc {

namespace math {
  namespace details {
    static i32_t _seed = 0x5752454e;

    void abs_impl(WrenVM& vm) {
      double value = wrenGetArgumentDouble(vm, 1);
      double r = std::fabs(value);
      wrenReturnDouble(vm, r);
    }

    void ceil_impl(WrenVM& vm) {
      double value = wrenGetArgumentDouble(vm, 1);
      double r = std::ceil(value);
      wrenReturnDouble(vm, r);
    }

    void floor_impl(WrenVM& vm) {
      double value = wrenGetArgumentDouble(vm, 1);
      double r = std::floor(value);
      wrenReturnDouble(vm, r);
    }

    void int_impl(WrenVM& vm) {
      double value = wrenGetArgumentDouble(vm, 1);
      double integer;
      double fractional = std::modf(value, &integer);
      wrenReturnDouble(vm, integer);
    }

    void frac_impl(WrenVM& vm) {
      double value = wrenGetArgumentDouble(vm, 1);
      double integer;
      double fractional = std::modf(value, &integer);
      wrenReturnDouble(vm, fractional);
    }

    void sin_impl(WrenVM& vm) {
      double angle = wrenGetArgumentDouble(vm, 1);
      double r = std::sin(angle);
      wrenReturnDouble(vm, r);
    }

    void cos_impl(WrenVM& vm) {
      double angle = wrenGetArgumentDouble(vm, 1);
      double r = std::cos(angle);
      wrenReturnDouble(vm, r);
    }

    void tan_impl(WrenVM& vm) {
      double angle = wrenGetArgumentDouble(vm, 1);
      double r = std::tan(angle);
      wrenReturnDouble(vm, r);
    }

    void deg_impl(WrenVM& vm) {
      double radians = wrenGetArgumentDouble(vm, 1);
      double degrees = std::floor(radians * 57.2957795130823208768);
      wrenReturnDouble(vm, degrees);
    }

    void rad_impl(WrenVM& vm) {
      double degrees = wrenGetArgumentDouble(vm, 1);
      double radians = std::floor(degrees / 57.2957795130823208768);
      wrenReturnDouble(vm, radians);
    }

    void srand_impl(WrenVM& vm) {
      time_t now = std::time(nullptr);
      _seed = Xt::as_type<i32_t>(now);
      // wrenRetunrNil(vm); // FIXME:
    }

    void rand_impl(WrenVM& vm) {
      _seed = (214013 * _seed + 2531011);
      i16_t value = (_seed >> 16) & 0x7fff;
      double r = Xt::as_type<double>(value) / Xt::as_type<double>(INT16_MAX - 1);
      wrenReturnDouble(vm, r);
    }

    inline void load_library(WrenVM& vm) {
      wrenDefineStaticMethod(vm, "Math", "abs", 1, abs_impl);
      wrenDefineStaticMethod(vm, "Math", "ceil", 1, ceil_impl);
      wrenDefineStaticMethod(vm, "Math", "floor", 1, floor_impl);
      wrenDefineStaticMethod(vm, "Math", "int", 1, int_impl);
      wrenDefineStaticMethod(vm, "Math", "frac", 1, frac_impl);
      wrenDefineStaticMethod(vm, "Math", "sin", 1, sin_impl);
      wrenDefineStaticMethod(vm, "Math", "cos", 1, cos_impl);
      wrenDefineStaticMethod(vm, "Math", "tan", 1, tan_impl);
      wrenDefineStaticMethod(vm, "Math", "deg", 1, deg_impl);
      wrenDefineStaticMethod(vm, "Math", "rad", 1, rad_impl);
      wrenDefineStaticMethod(vm, "Math", "srand", 0, srand_impl);
      wrenDefineStaticMethod(vm, "Math", "rand", 0, rand_impl);
    }
  }

  void load_library(WrenVM& vm) {
#if USE_LIBMATH
    details::load_library(vm);
#endif
  }
}

}
