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
#include "nyx_int_filter.h"

namespace nyx { namespace filter {

int_filter::int_filter(filter_type t, key_ref key, int v)
  : filter(t, key)
  , value_(v) {
}

int_filter::~int_filter(void) {
}

bool int_filter::done(int v) {
  switch (type_) {
  case filter_type::eq: return v == value_;
  case filter_type::ne: return v != value_;
  case filter_type::gt: return v > value_;
  case filter_type::ge: return v >= value_;
  case filter_type::lt: return v < value_;
  case filter_type::le: return v <= value_;
  }
  return false;
}

bool int_filter::done(float v) {
  switch (type_) {
  case filter_type::eq: return v == static_cast<float>(value_);
  case filter_type::ne: return v != static_cast<float>(value_);
  case filter_type::gt: return v > static_cast<float>(value_);
  case filter_type::ge: return v >= static_cast<float>(value_);
  case filter_type::lt: return v < static_cast<float>(value_);
  case filter_type::le: return v <= static_cast<float>(value_);
  }
  return false;
}

void int_filter::print(std::ostringstream& oss, bool is_root) {
  if (!is_root) {
    if (get_relation() == filter_relation::r_and)
      oss << " and ";
    else
      oss << " or ";
  }

  if (sub_)
    oss << "(";
  oss << "(" << static_cast<int>(type_) << "," << key_ << "," << value_ << ")";
  if (sub_) {
    sub_->print(oss, is_root);
    oss << ")";
  }
  if (next_)
    next_->print(oss, is_root);
}

}}
