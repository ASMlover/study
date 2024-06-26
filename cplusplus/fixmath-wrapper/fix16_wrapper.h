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

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <libfixmath/fixmath.h>
#include <boost/python.hpp>
#include "fixmath_utils.h"

namespace py = ::boost::python;

namespace fixmath_wrapper {

class fix16 : public Copyable {
  fix16_t value_{};
public:
  fix16(void) noexcept {
  }

  fix16(fix16_t v) noexcept
    : value_(v) {
  }

  fix16(std::int16_t v) noexcept
    : value_(fix16_from_int(v)) {
  }

  fix16(float v) noexcept
    : value_(fix16_from_float(v)) {
  }

  fix16(double v) noexcept
    : value_(fix16_from_dbl(v)) {
  }

  fix16(const fix16& o) noexcept
    : value_(o.value_) {
  }

  fix16(fix16&& o) noexcept {
    std::swap(value_, o.value_);
  }

  explicit operator fix16_t(void) const noexcept {
    return value_;
  }

  explicit operator std::int16_t(void) const noexcept {
    return fix16_to_int(value_);
  }

  explicit operator float(void) const noexcept {
    return fix16_to_float(value_);
  }

  explicit operator double(void) const noexcept {
    return fix16_to_dbl(value_);
  }

  fix16 operator-(void) const noexcept {
    fix16 r{0};
    r -= *this;
    return r;
  }

  fix16& operator=(const fix16& r) noexcept {
    if (this != &r)
      value_ = r.value_;
    return *this;
  }

  fix16& operator=(fix16&& r) noexcept {
    if (this != &r)
      std::swap(value_, r.value_);
    return *this;
  }

  fix16& operator=(fix16_t r) noexcept {
    value_ = r;
    return *this;
  }

  fix16& operator=(std::int16_t r) noexcept {
    value_ = fix16_from_int(r);
    return *this;
  }

  fix16& operator=(float r) noexcept {
    value_ = fix16_from_float(r);
    return *this;
  }

  fix16& operator=(double r) noexcept {
    value_ = fix16_from_float(r);
    return *this;
  }

  fix16& operator+=(const fix16& r) noexcept {
    value_ += r.value_;
    return *this;
  }

  fix16& operator+=(fix16_t r) noexcept {
    value_ += r;
    return *this;
  }

  fix16& operator+=(std::int16_t r) noexcept {
    value_ += fix16_from_int(r);
    return *this;
  }

  fix16& operator+=(float r) noexcept {
    value_ += fix16_from_float(r);
    return *this;
  }

  fix16& operator+=(double r) noexcept {
    value_ += fix16_from_dbl(r);
    return *this;
  }

  fix16& operator-=(const fix16& r) noexcept {
    value_ -= r.value_;
    return *this;
  }

  fix16& operator-=(fix16_t r) noexcept {
    value_ -= r;
    return *this;
  }

  fix16& operator-=(std::int16_t r) noexcept {
    value_ -= fix16_from_int(r);
    return *this;
  }

  fix16& operator-=(float r) noexcept {
    value_ -= fix16_from_float(r);
    return *this;
  }

  fix16& operator-=(double r) noexcept {
    value_ -= fix16_from_dbl(r);
    return *this;
  }

  fix16& operator*=(const fix16& r) noexcept {
    value_ = fix16_mul(value_, r.value_);
    return *this;
  }

  fix16& operator*=(fix16_t r) noexcept {
    value_ = fix16_mul(value_, r);
    return *this;
  }

  fix16& operator*=(std::int16_t r) noexcept {
    value_ *= r;
    return *this;
  }

  fix16& operator*=(float r) noexcept {
    value_ = fix16_mul(value_, fix16_from_float(r));
    return *this;
  }

  fix16& operator*=(double r) noexcept {
    value_ = fix16_mul(value_, fix16_from_dbl(r));
    return *this;
  }

  fix16& operator/=(const fix16& r) noexcept {
    value_ = fix16_div(value_, r.value_);
    return *this;
  }

  fix16& operator/=(fix16_t r) noexcept {
    value_ = fix16_div(value_, r);
    return *this;
  }

  fix16& operator/=(std::int16_t r) noexcept {
    value_ /= r;
    return *this;
  }

  fix16& operator/=(float r) noexcept {
    value_ = fix16_div(value_, fix16_from_float(r));
    return *this;
  }

  fix16& operator/=(double r) noexcept {
    value_ = fix16_div(value_, fix16_from_dbl(r));
    return *this;
  }

  fix16 operator+(const fix16& x) const noexcept {
    auto r = *this;
    r += x;
    return r;
  }

  fix16 operator+(fix16_t x) const noexcept {
    auto r = *this;
    r += x;
    return r;
  }

  fix16 operator+(std::int16_t x) const noexcept {
    auto r = *this;
    r += x;
    return r;
  }

  fix16 operator+(float x) const noexcept {
    auto r = *this;
    r += x;
    return r;
  }

  fix16 operator+(double x) const noexcept {
    auto r = *this;
    r += x;
    return r;
  }

  fix16 operator-(const fix16& x) const noexcept {
    auto r = *this;
    r -= x;
    return r;
  }

  fix16 operator-(fix16_t x) const noexcept {
    auto r = *this;
    r -= x;
    return x;
  }

  fix16 operator-(std::int16_t x) const noexcept {
    auto r = *this;
    r -= x;
    return r;
  }

  fix16 operator-(float x) const noexcept {
    auto r = *this;
    r -= x;
    return r;
  }

  fix16 operator-(double x) const noexcept {
    auto r = *this;
    r -= x;
    return r;
  }

  fix16 operator*(const fix16& x) const noexcept {
    auto r = *this;
    r *= x;
    return r;
  }

  fix16 operator*(fix16_t x) const noexcept {
    auto r = *this;
    r *= x;
    return r;
  }

  fix16 operator*(std::int16_t x) const noexcept {
    auto r = *this;
    r *= x;
    return r;
  }

  fix16 operator*(float x) const noexcept {
    auto r = *this;
    r *= x;
    return r;
  }

  fix16 operator*(double x) const noexcept {
    auto r = *this;
    r *= x;
    return r;
  }

  fix16 operator/(const fix16& x) const noexcept {
    auto r = *this;
    r /= x;
    return r;
  }

  fix16 operator/(fix16_t x) const noexcept {
    auto r = *this;
    r /= x;
    return r;
  }

  fix16 operator/(std::int16_t x) const noexcept {
    auto r = *this;
    r /= x;
    return r;
  }

  fix16 operator/(float x) const noexcept {
    auto r = *this;
    r /= x;
    return r;
  }

  fix16 operator/(double x) const noexcept {
    auto r = *this;
    r /= x;
    return r;
  }

  bool operator==(const fix16& r) const noexcept {
    return value_ == r.value_;
  }

  bool operator==(fix16_t r) const noexcept {
    return value_ == r;
  }

  bool operator==(std::int16_t r) const noexcept {
    return value_ == fix16_from_int(r);
  }

  bool operator==(float r) const noexcept {
    return value_ == fix16_from_float(r);
  }

  bool operator==(double r) const noexcept {
    return value_ == fix16_from_dbl(r);
  }

  bool operator!=(const fix16& r) const noexcept {
    return value_ != r.value_;
  }

  bool operator!=(fix16_t r) const noexcept {
    return value_ != r;
  }

  bool operator!=(std::int16_t r) const noexcept {
    return value_ != fix16_from_int(r);
  }

  bool operator!=(float r) const noexcept {
    return value_ != fix16_from_float(r);
  }

  bool operator!=(double r) const noexcept {
    return value_ != fix16_from_dbl(r);
  }

  bool operator>(const fix16& r) const noexcept {
    return value_ > r.value_;
  }

  bool operator>(fix16_t r) const noexcept {
    return value_ > r;
  }

  bool operator>(std::int16_t r) const noexcept {
    return value_ > fix16_from_int(r);
  }

  bool operator>(float r) const noexcept {
    return value_ > fix16_from_float(r);
  }

  bool operator>(double r) const noexcept {
    return value_ > fix16_from_dbl(r);
  }

  bool operator>=(const fix16& r) const noexcept {
    return value_ >= r.value_;
  }

  bool operator>=(fix16_t r) const noexcept {
    return value_ >= r;
  }

  bool operator>=(std::int16_t r) const noexcept {
    return value_ >= fix16_from_int(r);
  }

  bool operator>=(float r) const noexcept {
    return value_ >= fix16_from_float(r);
  }

  bool operator>=(double r) const noexcept {
    return value_ >= fix16_from_dbl(r);
  }

  bool operator<(const fix16& r) const noexcept {
    return value_ < r.value_;
  }

  bool operator<(fix16_t r) const noexcept {
    return value_ < r;
  }

  bool operator<(std::int16_t r) const noexcept {
    return value_ < fix16_from_int(r);
  }

  bool operator<(float r) const noexcept {
    return value_ < fix16_from_float(r);
  }

  bool operator<(double r) const noexcept {
    return value_ < fix16_from_dbl(r);
  }

  bool operator<=(const fix16& r) const noexcept {
    return value_ <= r.value_;
  }

  bool operator<=(fix16_t r) const noexcept {
    return value_ <= r;
  }

  bool operator<=(std::int16_t r) const noexcept {
    return value_ <= fix16_from_int(r);
  }

  bool operator<=(float r) const noexcept {
    return value_ <= fix16_from_float(r);
  }

  bool operator<=(double r) const noexcept {
    return value_ <= fix16_from_dbl(r);
  }

  fix16 sin(void) const noexcept {
    return fix16(fix16_sin(value_));
  }

  fix16 cos(void) const noexcept {
    return fix16(fix16_cos(value_));
  }

  fix16 tan(void) const noexcept {
    return fix16(fix16_tan(value_));
  }

  fix16 asin(void) const noexcept {
    return fix16(fix16_asin(value_));
  }

  fix16 acos(void) const noexcept {
    return fix16(fix16_acos(value_));
  }

  fix16 atan(void) const noexcept {
    return fix16(fix16_atan(value_));
  }

  fix16 atan2(const fix16& y) const noexcept {
    return fix16(fix16_atan2(value_, y.value_));
  }

  fix16 sin_parabola(void) const noexcept {
    return fix16(fix16_sin_parabola(value_));
  }

  fix16 rad2deg(void) const noexcept {
    return fix16(fix16_rad_to_deg(value_));
  }

  fix16 deg2rad(void) const noexcept {
    return fix16(fix16_deg_to_rad(value_));
  }

  fix16 sqrt(void) const noexcept {
    return fix16(fix16_sqrt(value_));
  }

  fix16 square(void) const noexcept {
    return fix16(fix16_sq(value_));
  }

  fix16 exp(void) const noexcept {
    return fix16(fix16_exp(value_));
  }

  fix16 log(void) const noexcept {
    return fix16(fix16_log(value_));
  }

  fix16 log2(void) const noexcept {
    return fix16(fix16_log2(value_));
  }

  fix16 abs(void) const noexcept {
    return fix16(fix16_abs(value_));
  }

  fix16 floor(void) const noexcept {
    return fix16(fix16_floor(value_));
  }

  fix16 ceil(void) const noexcept {
    return fix16(fix16_ceil(value_));
  }

  fix16 min(const fix16& r) const noexcept {
    return fix16_min(value_, r.value_);
  }

  fix16 max(const fix16& r) const noexcept {
    return fix16_max(value_, r.value_);
  }

  std::string as_repr(void) const noexcept {
    std::stringstream ss;
    ss << "fix16<"
       << std::hex << std::setw(8) << std::setfill('0') << value_ << ">";
    return ss.str();
  }

  std::string as_str(void) const noexcept {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(4) << fix16_to_float(value_);
    return ss.str();
  }

  int as_int(void) const noexcept {
    return fix16_to_int(value_);
  }

  float as_float(void) const noexcept {
    return fix16_to_float(value_);
  }

  fix16 copy(void) const noexcept {
    return fix16(value_);
  }

  static void wraps(void) {
    py::class_<fix16>("fix16", py::init<>())
      .def(py::init<std::int16_t>())
      .def(py::init<float>())
      .def(py::init<double>())
      .def(py::self + py::self)
      .def(py::self - py::self)
      .def(py::self * py::self)
      .def(py::self / py::self)
      .def(py::self > py::self)
      .def(py::self >= py::self)
      .def(py::self < py::self)
      .def(py::self <= py::self)
      .def(py::self == py::self)
      .def(py::self != py::self)
      .def("__repr__", &fix16::as_repr)
      .def("__str__", &fix16::as_str)
      .def("__int__", &fix16::as_int)
      .def("__float__", &fix16::as_float)
      .def("__copy__", &fix16::copy)
      .def("clone", &fix16::copy)
      .def("sin", &fix16::sin)
      .def("cos", &fix16::cos)
      .def("tan", &fix16::tan)
      .def("asin", &fix16::asin)
      .def("acos", &fix16::acos)
      .def("atan", &fix16::atan)
      .def("atan2", &fix16::atan2)
      .def("sin_parabola", &fix16::sin_parabola)
      .def("rad2deg", &fix16::rad2deg)
      .def("deg2rad", &fix16::deg2rad)
      .def("sqrt", &fix16::sqrt)
      .def("square", &fix16::square)
      .def("exp", &fix16::exp)
      .def("log", &fix16::log)
      .def("log2", &fix16::log2)
      .def("abs", &fix16::abs)
      .def("floor", &fix16::floor)
      .def("ceil", &fix16::ceil)
      .def("min", &fix16::min)
      .def("max", &fix16::max)
      ;
  }
};

template <typename _Scalar, typename _Vector, std::size_t N>
class vector_base {
public:
  vector_base(void) {
  }

  _Scalar* get_scalar(void) {
    return reinterpret_cast<_Scalar*>(this);
  }

  const _Scalar* get_scalar(void) const {
    return reinterpret_cast<const _Scalar*>(this);
  }

  void set_scaler(_Scalar s) {
    auto* ss = get_scalar();
    for (auto i = 0; i < N; ++i)
      ss[i] = s;
  }

  _Scalar get_length_squared(void) const {
    const auto* ss = get_scalar();
    _Scalar r = 0;
    for (auto i = 0; i < N; ++i)
      r += ss[i] * ss[i];
    return r;
  }

  _Scalar get_length(void) const {
    return get_length_squared().sqrt();
  }

  void normalize(void) {
    *this /= get_length();
  }

  void set_length(_Scalar o) {
    normalize();
    (*this) *= o;
  }

  _Scalar dot(const _Vector& o) {
    _Scalar* x = get_scalar();
    const auto* y = o.get_scalar();
    _Scalar r{0};
    for (auto i = 0; i < N; ++i)
      r += x[i] * y[i];
    return r;
  }

  bool operator==(const _Vector& o) const {
    const auto* left = get_scalar();
    const auto* right = o.get_scalar();
    bool r{true};
    for (auto i = 0; i < N; ++i)
      r &= left[i] == right[i];
    return r;
  }

  bool operator!=(const _Vector& o) const {
    return !(*this == o);
  }

  _Vector operator+(const _Vector& o) const {
    _Vector r;
    _Scalar* ss = r.get_scalar();
    const auto* x = get_scalar();
    const auto* y = o.get_scalar();
    for (auto i = 0; i < N; ++i)
      ss[i] = x[i] + y[i];
    return r;
  }

  _Vector operator-(const _Vector& o) const {
    _Vector r;
    _Scalar* ss = r.get_scalar();
    const auto* x = get_scalar();
    const auto* y = o.get_scalar();
    for (auto i = 0; i < N; ++i)
      ss[i] = x[i] - y[i];
    return r;
  }

  _Vector operator*(const _Vector& o) const {
    _Vector r;
    _Scalar* ss = r.get_scalar();
    const auto* x = get_scalar();
    const auto* y = o.get_scalar();
    for (auto i = 0; i < N; ++i)
      ss[i] = x[i] * y[i];
    return r;
  }

  _Vector operator/(const _Vector& o) const {
    _Vector r;
    _Scalar* ss = r.get_scalar();
    const auto* x = get_scalar();
    const auto* y = o.get_scalar();
    for (auto i = 0; i < N; ++i)
      ss[i] = x[i] / y[i];
    return r;
  }

  _Vector& operator+=(const _Vector& o) {
    _Scalar* ss = get_scalar();
    const auto* x = o.get_scalar();
    for (auto i = 0; i < N; ++i)
      ss[i] += x[i];
    return *reinterpret_cast<_Vector*>(this);
  }

  _Vector& operator+=(_Scalar o) {
    _Scalar* ss = get_scalar();
    for (auto i = 0; i < N; ++i)
      ss[i] += o;
    return *reinterpret_cast<_Vector*>(this);
  }

  _Vector& operator-=(const _Vector& o) {
    _Scalar* ss = get_scalar();
    const auto* x = o.get_scalar();
    for (auto i = 0; i < N; ++i)
      ss[i] -= x[i];
    return *reinterpret_cast<_Vector*>(this);
  }

  _Vector& operator-=(_Scalar o) {
    _Scalar* ss = get_scalar();
    for (auto i = 0; i < N; ++i)
      ss[i] -= o;
    return *reinterpret_cast<_Vector*>(this);
  }

  _Vector& operator*=(const _Vector& o) {
    _Scalar* ss = get_scalar();
    const auto* x = o.get_scalar();
    for (auto i = 0; i < N; ++i)
      ss[i] *= x[i];
    return *reinterpret_cast<_Vector*>(this);
  }

  _Vector& operator*=(_Scalar o) {
    _Scalar* ss = get_scalar();
    for (auto i = 0; i < N; ++i)
      ss[i] *= o;
    return *reinterpret_cast<_Vector*>(this);
  }

  _Vector& operator/=(const _Vector& o) {
    _Scalar* ss = get_scalar();
    const auto* x = o.get_scalar();
    for (auto i = 0; i < N; ++i)
      ss[i] /= x[i];
    return *reinterpret_cast<_Vector*>(this);
  }

  _Vector& operator/=(_Scalar o) {
    _Scalar* ss = get_scalar();
    for (auto i = 0; i < N; ++i)
      ss[i] /= o;
    return *reinterpret_cast<_Vector*>(this);
  }

  _Vector& operator=(const _Vector& o) {
    _Scalar* ss = get_scalar();
     const auto* x = o.get_scalar();
     for (auto i = 0; i < N; ++i)
       ss[i] = x[i];
     return *reinterpret_cast<_Vector*>(this);
  }
};

class vector2 : public vector_base<fix16, vector2, 2> {
public:
  fix16 x_{};
  fix16 y_{};

  vector2(void) noexcept
    : x_(0)
    , y_(0) {
  }

  vector2(const vector2& o) noexcept
    : x_(o.x_)
    , y_(o.y_) {
  }

  vector2(const fix16& x, const fix16& y) noexcept
    : x_(x)
    , y_(y) {
  }

  vector2(const fix16& x) noexcept
    : x_(x)
    , y_(x) {
  }

  vector2 copy(void) const noexcept {
    return *this;
  }

  std::string as_repr(void) const noexcept {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(4)
       << "fix.vector2<" << (float)x_ << ", " << (float)y_ << ">";
    return ss.str();
  }

  std::string as_str(void) const noexcept {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(4)
       << "(" << (float)x_ << ", " << (float)y_ << ")";
    return ss.str();
  }
};

class vector3 : public vector_base<fix16, vector3, 3> {
public:
  fix16 x_{};
  fix16 y_{};
  fix16 z_{};

  vector3(void) noexcept
    : x_(0)
    , y_(0)
    , z_(0) {
  }

  vector3(const vector3& o) noexcept
    : x_(o.x_)
    , y_(o.y_)
    , z_(o.z_) {
  }

  vector3(const fix16& x, const fix16& y, const fix16& z) noexcept
    : x_(x)
    , y_(y)
    , z_(z) {
  }

  vector3(const fix16& v) noexcept
    : x_(v)
    , y_(v)
    , z_(v) {
  }

  vector3 copy(void) const noexcept {
    return *this;
  }

  std::string as_repr(void) const noexcept {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(4) << "fix.vector3<"
       << (float)x_ << ", " << (float)y_ << ", " << (float)z_ << ">";
    return ss.str();
  }

  std::string as_str(void) const noexcept {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(4)
       << "(" << (float)x_ << ", " << (float)y_ << ", " << (float)z_ << ")";
    return ss.str();
  }

  void set_pitch_yaw(const fix16& p, const fix16& y) noexcept {
    auto p_cos = p.cos();
    auto p_sin = -p.sin();
    auto y_cos = y.cos();
    auto y_sin = y.sin();
    x_ = p_cos * y_sin;
    y_ = p_sin;
    z_ = p_cos * y_cos;
  }

  void set_yaw(const fix16& y) {
    set_pitch_yaw(get_pitch(), y);
  }

  fix16 get_yaw(void) const noexcept {
    return x_.atan2(z_);
  }

  void set_pitch(const fix16& p) {
    set_pitch_yaw(p, get_yaw());
  }

  fix16 get_pitch(void) const noexcept {
    return -y_.atan2((x_ * x_ + z_ * z_).sqrt());
  }
};

}
