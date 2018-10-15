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
#include <sstream>
#include <string>
#include <libfixmath/fixmath.h>
#include "fixmath_utils.h"

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
};

template <typename _Scalar, typename _Vector, std::size_t N>
class vector_base {
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
    // *this /= get_length();
  }

  void set_length(_Scalar o) {
    normalize();
    // (*this) *= o;
  }

  bool operator==(const _Vector& o) const {
    const auto* left = get_scalar();
    const auto* right = get_scalar();
    bool r{true};
    for (auto i = 0; i < N; ++i)
      r &= left[i] == right[i];
    return r;
  }

  bool operator!=(const _Vector& o) const {
    return !(*this == o);
  }
};

}
